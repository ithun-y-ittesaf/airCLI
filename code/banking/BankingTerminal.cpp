#include "BankingTerminal.h"

#include "BankManager.h"
#include "PaymentGatewayManager.h"
#include "../users/UserManager.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <termios.h>
#include <unistd.h>
using namespace std;


namespace banking {

static string readMasked() {
    if (!isatty(STDIN_FILENO)) {
        string s; getline(cin, s); return s;
    }
    termios old, nw;
    tcgetattr(STDIN_FILENO, &old);
    nw = old; nw.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &nw);
    string pw; char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != '\n') {
        pw += c; cout << '*'; cout.flush();
    }
    cout << '\n';
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
    return pw;
}

static void printAmount(long long cents) {
    cout << "BDT " << (cents / 100) << "."
         << setfill('0') << setw(2) << (cents % 100);
}

static int selectBank(const vector<Bank> &banks) {
    cout << "\n--- Select Bank ---\n";
    for (size_t i = 0; i < banks.size(); ++i)
        cout << "[" << (i + 1) << "] " << banks[i].getName() << "\n";
    cout << "0. Cancel\nSelect > ";
    int c = 0;
    cin >> c;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return c;
}

int runBankRegistrationTerminal(const string    &airUserId,
                                storage::IRepository &repo) {
    BankManager bankManager(repo);
    users::UserManager userManager(repo);

    bankManager.load();
    userManager.load();
    (void)airUserId;

    cout << "\n========================================\n";
    cout << " bankCLI — Banking App\n";
    cout << "========================================\n";

    bool running = true;
    string bankUsername;
    string bankPassword;
    bool loggedIn = false;

    while (running) {
        cout << "\n1. Register Bank Account\n";
        cout << "2. Login To Bank Account\n";
        cout << "3. Deposit Money\n";
        cout << "4. Exit\n";
        cout << "Select > ";

        int choice = 0;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) {
            auto banks = bankManager.all();
            int bankChoice = selectBank(banks);
            if (bankChoice < 1 || bankChoice > static_cast<int>(banks.size())) continue;

            const string bankName = banks[bankChoice - 1].getName();
            cout << "Set bank username: ";
            string newUser; getline(cin, newUser);
            cout << "Set bank password: ";
            string newPass = readMasked();

            string accountId;
            if (!bankManager.registerBankUser(bankName, newUser, newPass, accountId)) {
                cout << "\nRegistration failed. Username may already exist.\n";
                continue;
            }

            cout << "\nBank account created successfully.\n";
            cout << "Account ID: " << accountId << "\n";
            cout << "Bank     : " << bankName << "\n";
            continue;
        }

        if (choice == 2) {
            cout << "Bank username: ";
            getline(cin, bankUsername);
            cout << "Bank password: ";
            bankPassword = readMasked();

            BankUserAccount account;
            if (!bankManager.authenticateBankUser(bankUsername, bankPassword, account)) {
                cout << "\nInvalid bank credentials.\n";
                loggedIn = false;
                continue;
            }

            loggedIn = true;
            cout << "\nLogin successful.\n";
            cout << "Bank    : " << account.bankName << "\n";
            cout << "Balance : "; printAmount(account.balanceCents); cout << "\n";
            continue;
        }

        if (choice == 3) {
            if (!loggedIn) {
                cout << "\nPlease login first.\n";
                continue;
            }

            cout << "Deposit amount (BDT): ";
            long long amountBdt = 0;
            cin >> amountBdt;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            Result res = bankManager.depositToBankUser(bankUsername, bankPassword, amountBdt * 100);
            if (!res.ok) {
                cout << "\n" << res.message << "\n";
                continue;
            }

            BankUserAccount account;
            bankManager.load();
            bankManager.authenticateBankUser(bankUsername, bankPassword, account);
            cout << "\nDeposit successful.\n";
            cout << "New Balance: "; printAmount(account.balanceCents); cout << "\n";
            continue;
        }

        if (choice == 4) {
            cout << "\nClosing bankCLI.\n";
            break;
        }

        cout << "\nInvalid option.\n";
    }

    return 0;
}

int runBankGatewayTerminal(const string    &airUserId,
                           const string    &methodId,
                           long long             amountCents,
                           const string    &purpose,
                           const string    &referenceId,
                           storage::IRepository &repo) {
    BankManager bankManager(repo);
    PaymentGatewayManager gateway(repo);

    bankManager.load();
    gateway.load();

    const SavedPaymentMethod *method = gateway.getMethod(methodId, airUserId);
    if (!method) {
        gateway.addReceipt(airUserId, methodId, purpose, referenceId,
                           amountCents, "FAILED", "Saved payment method not found");
        cout << "\nInvalid payment method.\n";
        return 0;
    }

    cout << "\n========================================\n";
    cout << " airCLI Gateway — Payment Channel\n";
    cout << " Method  : " << method->nickname << " (" << method->bankName << ")\n";
    cout << " Amount  : "; printAmount(amountCents); cout << "\n";
    cout << " Purpose : " << purpose << "\n";
    cout << "========================================\n\n";

    cout << "Confirm payment? (y/n): ";
    char ch = 'n';
    cin >> ch;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (ch != 'y' && ch != 'Y') {
        gateway.addReceipt(airUserId, methodId, purpose, referenceId,
                           amountCents, "CANCELLED", "Gateway payment cancelled");
        cout << "\nPayment cancelled.\n";
        return 0;
    }

    cout << "Re-enter bank password for verification: ";
    string verifyPassword = readMasked();

    if (verifyPassword != method->bankPassword) {
        gateway.addReceipt(airUserId, methodId, purpose, referenceId,
                           amountCents, "FAILED", "Password verification failed");
        cout << "\nPassword verification failed.\n";
        return 0;
    }

    string bankName;
    Result debit = bankManager.debitFromBankUser(method->bankUsername,
                                                 method->bankPassword,
                                                 amountCents,
                                                 bankName);

    if (!debit.ok) {
        gateway.addReceipt(airUserId, methodId, purpose, referenceId,
                           amountCents, "FAILED", debit.message);
        cout << "\nPayment failed: " << debit.message << "\n";
        return 0;
    }

    string receiptId = gateway.addReceipt(airUserId, methodId, purpose, referenceId,
                                               amountCents, "SUCCESS", "Payment approved");

    cout << "\nPayment successful.\n";
    cout << "Receipt ID: " << receiptId << "\n";
    cout << "This gateway will now close.\n";
    return 0;
}

} // namespace banking
