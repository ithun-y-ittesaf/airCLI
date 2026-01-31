#include "UserCLI.h"
#include <iostream>
#include <sstream>
#include "../storage/DataStorage.h"

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
        
        cout << "Phone Number  : ";
        string phoneNumber;
        getline(cin, phoneNumber);

        outUserId = manager.createUser(name, username, password, phoneNumber);
        
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

    static string findBankUserIdByPhone(const string &phone) {
        auto lines = storage::DataStorage::readAll("bank_users.txt");
        for (const auto &line : lines) {
            if (line.empty()) continue;
            stringstream ss(line);
            string id, name, phoneVal, pin;
            getline(ss, id, '|');
            getline(ss, name, '|');
            getline(ss, phoneVal, '|');
            getline(ss, pin, '|');
            if (phoneVal == phone) {
                return id;
            }
        }
        return "";
    }

    bool UserCLI::linkBankAccount(const string &userId) {
        manager.load();
        const User* user = manager.get(userId);
        if (!user) {
            cout << "\nUser not found.\n";
            return false;
        }

        if (!user->getLinkedBankUserId().empty()) {
            cout << "\nBank account already added. Want to unlink? (y/n): ";
            char choice = 'n';
            cin >> choice;
            if (choice == 'y' || choice == 'Y') {
                manager.updateLinkedBankUserId(userId, "");
                cout << "\nBank account unlinked.\n";
                return false;
            }

            cout << "\nOpen Banking CLI? (y/n): ";
            cin >> choice;
            return (choice == 'y' || choice == 'Y');
        }

        cout << "\n--- Link Bank Account ---\n";
        cout << "Enter phone number: ";
        string phone;
        cin.ignore();
        getline(cin, phone);

        if (!user->getPhoneNumber().empty() && user->getPhoneNumber() != phone) {
            cout << "\nPhone number does not match your airCLI profile.\n";
            cout << "Your profile phone: " << user->getPhoneNumber() << "\n";
            return false;
        }

        if (user->getPhoneNumber().empty()) {
            manager.updatePhoneNumber(userId, phone);
        }

        string bankUserId = findBankUserIdByPhone(phone);
        if (bankUserId.empty()) {
            cout << "\nNo bank account found for this phone.\n";
            cout << "Please register in Banking CLI first.\n";
            cout << "Open Banking CLI now? (y/n): ";
            char openChoice = 'n';
            cin >> openChoice;
            return (openChoice == 'y' || openChoice == 'Y');
        }

        manager.updateLinkedBankUserId(userId, bankUserId);
        cout << "\n✓ Bank account linked successfully.\n";
        cout << "Open Banking CLI now? (y/n): ";
        char openChoice = 'n';
        cin >> openChoice;
        return (openChoice == 'y' || openChoice == 'Y');
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
