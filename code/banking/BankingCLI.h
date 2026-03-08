#ifndef BANKING_BANKINGCLI_H
#define BANKING_BANKINGCLI_H

#include <string>
#include "BankingManager.h"
#include "../Role.h"

namespace banking {
    
    class BankingCLI {
    private:
        BankingManager &manager;

    public:
        
        explicit BankingCLI(BankingManager &m) : manager(m) {}

        // Admin banking interface
        // void run(const utils::ID &userId, utils::Role userRole);

        // User banking interface
        void runBanking(const string &userId, Role userRole);
    };
}

#endif
