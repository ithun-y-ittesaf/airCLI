#ifndef BANKING_BANKMANAGER_H
#define BANKING_BANKMANAGER_H

#include <vector>
#include <string>

#include "Bank.h"
#include "../Result.h"
#include "../storage/IRepository.h"
using namespace std;

namespace banking {

    struct BankUserAccount {
        string accountId;
        string bankName;
        string username;
        string password;
        long long   balanceCents;
    };

    class BankManager {
    private:
        storage::IRepository &repo;
        vector<Bank>     banks; // Always exactly 5, indexed by name.
        vector<BankUserAccount> bankUsers;

        void seedDefaults();

        // Find a bank by name (case-insensitive). Returns nullptr if not found.
        Bank *findBank(const string &name);

    public:
        explicit BankManager(storage::IRepository &repo) : repo(repo) {}

        // Load pool balances from storage. Accounts are always hardcoded.
        bool load();

        // Persist current pool balances.
        bool save() const;

        // All 5 banks (read-only).
        const vector<Bank> &all() const { return banks; }

        bool registerBankUser(const string &bankName,
                      const string &username,
                      const string &password,
                      string       &outAccountId);

        bool authenticateBankUser(const string &username,
                      const string &password,
                      BankUserAccount   &outUser) const;

        Result depositToBankUser(const string &username,
                     const string &password,
                     long long          amountCents);

        Result debitFromBankUser(const string &username,
                     const string &password,
                     long long          amountCents,
                     string       &outBankName);

    };

} // namespace banking

#endif // BANKING_BANKMANAGER_H
