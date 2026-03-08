#include "BankingCLI.h"
#include <iostream>
#include <cstdlib>
#include <limits.h>
#include <unistd.h>

using namespace std;

namespace banking {

    void BankingCLI::runBanking(const string &, Role) {
        cout << "\n==============================\n";
        cout << "Opening Banking CLI...\n";
        cout << "A separate terminal will open.\n";
        cout << "==============================\n\n";

        char exePath[PATH_MAX];
        ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
        string bankingCmd = "./build/bankcli";
        if (len > 0) {
            exePath[len] = '\0';
            string pathStr(exePath);
            auto lastSlash = pathStr.find_last_of('/');
            if (lastSlash != string::npos) {
                string exeDir = pathStr.substr(0, lastSlash);
                bankingCmd = exeDir + "/bankcli";
            }
        }

        cout << "Launching Banking CLI...\n";

        const string cmd = "\"" + bankingCmd + "\"";
        const string candidates[] = {
            "gnome-terminal -- " + cmd + " &",
            "konsole -e " + cmd + " &",
            "xfce4-terminal --command=" + cmd + " &",
            "kitty " + cmd + " &",
            "alacritty -e " + cmd + " &"
        };

        int result = 1;
        for (const auto &c : candidates) {
            result = system(c.c_str());
            if (result == 0) break;
        }

        if (result != 0) {
            cout << "\nFailed to launch Banking CLI in a new terminal.\n";
            cout << "Please install a supported terminal (gnome-terminal, konsole, xfce4-terminal, kitty, alacritty).\n";
        }
    }

}
