#include "TerminalLauncher.h"
#include <cstdlib>
#include <string>
#include <unistd.h>
using namespace std;

namespace core {

static string escapeForSingleQuotes(const string &s) {
    string out;
    out.reserve(s.size() + 16);
    for (char ch : s) {
        if (ch == '\'') out += "'\\''";
        else out.push_back(ch);
    }
    return out;
}

bool launchInTerminal(const string &command, bool block) {
#ifdef _WIN32
    // Windows: use "start" to open a new cmd window.
    // /wait makes start block until the window closes.
    string waitFlag = block ? "/wait " : "";
    string fullCmd  = "start " + waitFlag + "cmd /c \"" + command + "\"";
    int result = system(fullCmd.c_str());
    return (result == 0);

#else
    // Linux/macOS: try known terminal emulators in order.
    // For blocking, we omit "&" so system() waits for the process to finish.
    // Each terminal emulator needs a different flag to run a command.

    char cwdBuf[4096] = {0};
    if (!getcwd(cwdBuf, sizeof(cwdBuf) - 1)) {
        cwdBuf[0] = '.';
        cwdBuf[1] = '\0';
    }

    const string escapedCwd = escapeForSingleQuotes(string(cwdBuf));
    const string escapedCmd = "cd '" + escapedCwd + "' && " + command;
    const string escaped = escapeForSingleQuotes(escapedCmd);

    const char *candidates[] = {
        "gnome-terminal",
        "konsole",
        "xfce4-terminal",
        "kitty",
        "alacritty",
        "x-terminal-emulator"
    };

    for (const auto *termName : candidates) {
        // Check if the terminal exists by testing with 'which'.
        string checkCmd = string("which ") + termName + " > /dev/null 2>&1";
        if (system(checkCmd.c_str()) == 0) {
            string launchCmd;
            if (string(termName) == "gnome-terminal") {
                launchCmd = "gnome-terminal " + string(block ? "--wait " : "") + "-- bash -lc '" + escaped + "'";
            } else if (string(termName) == "konsole") {
                launchCmd = "konsole -e bash -lc '" + escaped + "'";
            } else if (string(termName) == "xfce4-terminal") {
                launchCmd = "xfce4-terminal -e \"bash -lc '" + escaped + "'\"";
            } else if (string(termName) == "kitty") {
                launchCmd = "kitty bash -lc '" + escaped + "'";
            } else if (string(termName) == "alacritty") {
                launchCmd = "alacritty -e bash -lc '" + escaped + "'";
            } else {
                launchCmd = "x-terminal-emulator -e bash -lc '" + escaped + "'";
            }

            if (!block) launchCmd += " &";
            int rc = system(launchCmd.c_str());
            if (rc == 0) return true;
            continue;
        }
    }

    // Fallback: if no graphical terminal found, run in current terminal.
    // This gracefully handles headless/SSH environments.
    int rc = system(command.c_str());
    return (rc == 0);
#endif
}

} // namespace core
