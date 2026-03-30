#ifndef CORE_TERMINALLAUNCHER_H
#define CORE_TERMINALLAUNCHER_H

#include <string>
using namespace std;

namespace core {
    bool launchInTerminal(const string &command, bool block = true);

}

#endif // CORE_TERMINALLAUNCHER_H
