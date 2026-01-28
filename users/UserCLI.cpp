#include "UserCLI.h"
#include <iostream>

using namespace std;

namespace users {
    void UserCLI::showWelcome() {
        cout << "\n==================================================\n";
        cout << "              Welcome to airCLI ✈\n";
        cout << "              Flight Booking System\n";
        cout << "==================================================\n\n";
        cout << "1. Login\n";
        cout << "2. Create New Account\n";
        cout << "3. Exit\n\n";
        cout << "What would you like to do? > ";
    }

    bool UserCLI::runSignup(string &outUserId) {
        cout << "\n--- Create Your Account ---\n";
        
        cout << "\nFull Name     : ";
        string name;
        cin.ignore();
        getline(cin, name);
        
        cout << "Username      : ";
        string username;
        getline(cin, username);
        
        if (manager.findByUsername(username)) {
            cout << "\nSorry, that username is already taken. Please try another.\n";
            return false;
        }
        
        cout << "Password      : ";
        string password;
        getline(cin, password);
        
        cout << "Confirm       : ";
        string confirmPassword;
        getline(cin, confirmPassword);
        
        if (password != confirmPassword) {
            cout << "\nThe passwords don't match. Please try again.\n";
            return false;
        }
        
        outUserId = manager.createUser(name, username, password);
        
        cout << "\n✓ Account created successfully!\n";
        cout << "Your user ID: " << outUserId << "\n";
        cout << "You can now log in with your username and password.\n";
        
        return true;
    }

    bool UserCLI::runLogin(string &outUserId) {
        cout << "\n--- Login ---\n";
        
        cout << "\nUsername : ";
        string username;
        cin.ignore();
        getline(cin, username);
        
        cout << "Password : ";
        string password;
        getline(cin, password);
        
        if (!manager.authenticate(username, password, outUserId)) {
            cout << "\nIncorrect username or password. Please try again.\n";
            return false;
        }
        
        const User* user = manager.get(outUserId);
        cout << "\n✓ Login successful!\n";
        cout << "Welcome back, " << user->getName() << "!\n";
        
        return true;
    }

    void UserCLI::linkBankAccount(const string &userId) {
        // Future feature: link external bank account
        (void)userId;
    }

    bool UserCLI::welcomeFlow(string &outUserId, Role &outRole) {
        manager.load();
        
        while (true) {
            showWelcome();
            int choice = 0;
            cin >> choice;
            
            if (choice == 1) {
                if (runLogin(outUserId)) {
                    const User* user = manager.get(outUserId);
                    outRole = user->getRole();
                    return true;
                }
            } else if (choice == 2) {
                if (runSignup(outUserId)) {
                    const User* user = manager.get(outUserId);
                    outRole = user->getRole();
                    return true;
                }
            } else if (choice == 3) {
                cout << "\nThanks for visiting airCLI. Goodbye!\n";
                return false;
            } else {
                cout << "\nPlease enter 1, 2, or 3.\n";
            }
        }
    }
}
