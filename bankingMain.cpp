#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>

#include "storage/DataStorage.h"
#include "banking/BankingManager.h"
#include "users/UserManager.h"

using namespace std;

namespace bankauth {
    struct BankUser {
        string id;
        string name;
        string phone;
        string pin;

        string serialize() const {
            return id + "|" + name + "|" + phone + "|" + pin;
        }

        static BankUser deserialize(const string &line) {
            stringstream ss(line);
            BankUser u;
            getline(ss, u.id, '|');
            getline(ss, u.name, '|');
            getline(ss, u.phone, '|');
            getline(ss, u.pin, '|');
            return u;
        }
    };

    static vector<BankUser> loadUsers() {
        vector<BankUser> users;
        auto lines = storage::DataStorage::readAll("bank_users.txt");
        for (const auto &line : lines) {
            if (line.empty()) continue;
            users.push_back(BankUser::deserialize(line));
        }
        return users;
    }

    static bool saveUsers(const vector<BankUser> &users) {
        vector<string> lines;
        lines.reserve(users.size());
        for (const auto &u : users) {
            lines.push_back(u.serialize());
        }
        return storage::DataStorage::writeAll("bank_users.txt", lines);
    }

    static string findUserIdByPhone(const vector<BankUser> &users, const string &phone) {
        for (const auto &u : users) {
            if (u.phone == phone) return u.id;
        }
        return "";
    }

    static const BankUser* findUserById(const vector<BankUser> &users, const string &id) {
        for (const auto &u : users) {
            if (u.id == id) return &u;
        }
        return nullptr;
    }

    static string createUser(vector<BankUser> &users, const string &name, const string &phone, const string &pin) {
        using namespace std::chrono;
        auto t = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
        string id = "B" + std::to_string(t);
        while (findUserById(users, id) != nullptr) {
            ++t;
            id = "B" + std::to_string(t);
        }

        BankUser u;
        u.id = id;
        u.name = name;
        u.phone = phone;
        u.pin = pin;
        users.push_back(u);
        saveUsers(users);
        return id;
    }

    static bool authenticate(const vector<BankUser> &users, const string &phone, const string &pin, string &outId) {
        for (const auto &u : users) {
            if (u.phone == phone && u.pin == pin) {
                outId = u.id;
                return true;
            }
        }
        return false;
    }
}

static long long toCents(long long amount) {
    return amount * 100;
}

static void printFormattedBalance(long long cents) {
    long long taka = cents / 100;
    long long paisa = cents % 100;
    cout << "BDT " << taka << "." << setfill('0') << setw(2) << paisa;
}

static bool bankWelcome(string &outBankUserId) {
    while (true) {
        cout << "\n==================================================\n";
        cout << "              Welcome to BankCLI 🏦\n";
        cout << "==================================================\n\n";
        cout << "1. Login\n";
        cout << "2. Register New Bank Account\n";
        cout << "3. Exit\n\n";
        cout << "What would you like to do? > ";

        int choice = 0;
        cin >> choice;

        if (choice == 1) {
            cout << "\n--- Bank Login ---\n";
            cout << "Phone Number : ";
            string phone;
            cin.ignore();
            getline(cin, phone);

            cout << "PIN          : ";
            string pin;
            getline(cin, pin);

            auto users = bankauth::loadUsers();
            if (bankauth::authenticate(users, phone, pin, outBankUserId)) {
                const auto *u = bankauth::findUserById(users, outBankUserId);
                cout << "\n✓ Login successful!\n";
                if (u) {
                    cout << "Welcome back, " << u->name << "!\n";
                }
                return true;
            }

            cout << "\nIncorrect phone or PIN. Please try again.\n";
        } else if (choice == 2) {
            cout << "\n--- Register Bank Account ---\n";
            cout << "Full Name    : ";
            string name;
            cin.ignore();
            getline(cin, name);

            cout << "Phone Number : ";
            string phone;
            getline(cin, phone);

            auto users = bankauth::loadUsers();
            if (!bankauth::findUserIdByPhone(users, phone).empty()) {
                cout << "\nThis phone number is already registered.\n";
                continue;
            }

            cout << "PIN          : ";
            string pin;
            getline(cin, pin);
            cout << "Confirm PIN  : ";
            string confirmPin;
            getline(cin, confirmPin);

            if (pin != confirmPin) {
                cout << "\nPINs don't match. Please try again.\n";
                continue;
            }

            outBankUserId = bankauth::createUser(users, name, phone, pin);
            cout << "\n✓ Bank account created successfully!\n";
            return true;
        } else if (choice == 3) {
            cout << "\nThanks for using BankCLI. Goodbye!\n";
            return false;
        } else {
            cout << "\nPlease enter 1, 2, or 3.\n";
        }
    }
}

int main() {
    banking::BankingManager bankingManager;
    users::UserManager userManager;
    bankingManager.load();
    userManager.load();

    string bankUserId;
    if (!bankWelcome(bankUserId)) {
        return 0;
    }

    auto bankUsers = bankauth::loadUsers();
    const auto *bankUser = bankauth::findUserById(bankUsers, bankUserId);
    string bankPhone = bankUser ? bankUser->phone : "";

    bool isRunning = true;
    while (isRunning) {
        bankingManager.load();
        userManager.load();
        auto accountId = bankingManager.getOrCreateAccountForUser(bankUserId);
        auto account = bankingManager.getAccount(accountId);
        long long balance = account ? account->getBalanceCents() : 0;

        cout << "\n==============================\n";
        cout << " Banking Services\n";
        cout << "==============================\n";
        cout << "\nBank Balance: ";
        printFormattedBalance(balance);

        cout << "\n\n1. Check Bank Balance\n";
        cout << "2. Deposit Funds\n";
        cout << "3. Withdraw Funds (to Cash)\n";
        cout << "4. View Transaction History\n";
        cout << "5. Exit Banking\n";
        cout << "\nSelect an option > ";

        int choice = 0;
        cin >> choice;
        cin.ignore();

        switch (choice) {
        case 1: {
            auto acc = bankingManager.getAccount(accountId);
            long long bal = acc ? acc->getBalanceCents() : 0;
            cout << "\n--- Bank Balance ---\n";
            cout << "Your Current Balance: ";
            printFormattedBalance(bal);
            cout << "\n";
            break;
        }
        case 2: {
            cout << "\n--- Deposit Funds ---\n";
            cout << "Enter amount to deposit (BDT): ";
            long long amount;
            cin >> amount;
            cin.ignore();

            if (amount <= 0) {
                cout << "\nInvalid amount. Please enter a positive value.\n";
            } else {
                auto result = bankingManager.deposit(accountId, toCents(amount), "Manual deposit");
                if (result.ok) {
                    cout << "\n✓ Deposit successful.\n";
                } else {
                    cout << "\n✗ " << result.message << "\n";
                }
            }
            break;
        }
        case 3: {
            cout << "\n--- Withdraw Funds ---\n";
            auto acc = bankingManager.getAccount(accountId);
            long long bal = acc ? acc->getBalanceCents() : 0;

            cout << "Current Bank Balance: ";
            printFormattedBalance(bal);
            cout << "\nEnter amount to withdraw (BDT): ";
            long long amount;
            cin >> amount;
            cin.ignore();

            if (amount <= 0) {
                cout << "\nInvalid amount. Please enter a positive value.\n";
            } else if (toCents(amount) > bal) {
                cout << "\nInsufficient balance. Please try a lower amount.\n";
            } else {
                auto result = bankingManager.withdraw(accountId, toCents(amount), "Manual withdrawal");
                if (result.ok) {
                    if (!bankPhone.empty()) {
                        userManager.updateCashBalanceByPhone(bankPhone, toCents(amount));
                    }
                    cout << "\n✓ Withdrawal successful.\n";
                } else {
                    cout << "\n✗ " << result.message << "\n";
                }
            }
            break;
        }
        case 4: {
            auto transactions = bankingManager.getTransactionsForAccount(accountId);
            cout << "\n--- Transaction History ---\n";
            if (transactions.empty()) {
                cout << "No transactions yet.\n";
            } else {
                for (const auto &transaction : transactions) {
                    string typeLabel = (transaction.getType() == banking::TransactionType::Credit) ? "+" : "-";
                    cout << typeLabel << " ";
                    printFormattedBalance(transaction.getAmountCents());
                    cout << " | " << transaction.getDescription() << "\n";
                }
            }
            cout << "\n";
            break;
        }
        case 5:
            cout << "\nExiting Banking CLI. Goodbye!\n";
            isRunning = false;
            break;
        default:
            cout << "\nInvalid option. Please try again.\n";
        }
    }

    return 0;
}
