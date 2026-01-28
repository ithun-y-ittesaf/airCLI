#include "BankingCLI.h"
#include "../access/AccessControl.h"
#include <iostream>
#include <iomanip>

using namespace std;

namespace banking {

    static long long toCents(long long amount) {
        return amount * 100;
    }

    static void printFormattedBalance(long long cents) {
        long long taka = cents / 100;
        long long paisa = cents % 100;
        cout << "BDT " << taka << "." << setfill('0') << setw(2) << paisa;
    }

    void BankingCLI::runBanking(const string &userId, Role userRole) {
        manager.load();
        auto accountId = manager.getOrCreateAccountForUser(userId);
        
        cout << "\n--- Banking Services ---\n";
        cout << "1. Check Balance\n";
        cout << "2. Deposit Funds\n";
        cout << "3. Withdraw Funds\n";
        cout << "4. View Transactions\n";
        cout << "5. Back\n";
        cout << "Choice: ";
        
        int choice = 0;
        cin >> choice;
        
        if (choice == 1) {
            // View balance
            auto account = manager.getAccount(accountId);
            long long balance = account ? account->getBalanceCents() : 0;
            cout << "\nCurrent Balance: ";
            printFormattedBalance(balance);
            cout << "\n";
        } else if (choice == 2) {
            // Deposit funds
            if (!access::AccessControl::anyOf(userRole, {Role::Passenger, Role::Admin})) {
                cout << "\nAccess denied.\n";
                return;
            }
            
            cout << "\nEnter amount to deposit (BDT): ";
            long long amount;
            cin >> amount;
            
            auto result = manager.deposit(accountId, toCents(amount), "Manual deposit");
            if (result.ok) {
                cout << "\n✓ Deposit successful.\n";
            } else {
                cout << "\n" << result.message << "\n";
            }
        } else if (choice == 3) {
            // Withdraw funds
            if (!access::AccessControl::anyOf(userRole, {Role::Passenger, Role::Admin})) {
                cout << "\nAccess denied.\n";
                return;
            }
            
            cout << "\nEnter amount to withdraw (BDT): ";
            long long amount;
            cin >> amount;
            
            auto result = manager.withdraw(accountId, toCents(amount), "Manual withdrawal");
            if (result.ok) {
                cout << "\n✓ Withdrawal successful.\n";
            } else {
                cout << "\n" << result.message << "\n";
            }
        } else if (choice == 4) {
            // View transactions
            auto transactions = manager.getTransactionsForAccount(accountId);
            
            if (transactions.empty()) {
                cout << "\nNo transactions yet.\n";
            } else {
                cout << "\nTransaction History:\n";
                for (const auto &transaction : transactions) {
                    string typeLabel = (transaction.getType() == TransactionType::Credit) ? "+" : "-";
                    cout << typeLabel << " ";
                    printFormattedBalance(transaction.getAmountCents());
                    cout << " | " << transaction.getDescription() << "\n";
                }
            }
        }
    }

}
