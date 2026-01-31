#ifndef USERS_USERCLI_H
#define USERS_USERCLI_H

#include <string>
#include "UserManager.h"

using namespace std;

namespace users {
   
    class UserCLI {
    private:
        UserManager &manager;
        
        void showWelcome();
        bool runSignup(string &outUserId);
        bool runLogin(string &outUserId);
        
    public:
        explicit UserCLI(UserManager &m) : manager(m) {}
        
        // Run the welcome flow (login or signup)
        bool welcomeFlow(string &outUserId, Role &outRole);

        // Link bank account by phone
        bool linkBankAccount(const string &userId);
    };
}

#endif 
