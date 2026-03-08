#ifndef BANKING_BANKINGMANAGER_H
#define BANKING_BANKINGMANAGER_H

#include <map>
#include <vector>
#include "BankingAccount.h"
#include "Transaction.h"
#include "../Result.h"

using namespace std;

namespace banking {
    
    class BankingManager {
    private:
        map<string, BankingAccount> accountsById;
        map<string, string> userToAccountId;

    public:

        bool load();

        bool save() const;

        // Get or create a bank account for a user
        string getOrCreateAccountForUser(const string &userId);

        // Get account by ID
        const BankingAccount* getAccount(const string &accountId) const;

        // Get account associated with a user
        const BankingAccount* getAccountByUser(const string &userId) const;

        // CREDIT to an account
        Result deposit(const string &accountId, long long amountCents, 
                            const string &description);

        // Remove money from an account
        Result withdraw(const string &accountId, long long amountCents, 
                             const string &description);

        // Get all transactions for an account
        vector<Transaction> getTransactionsForAccount(const string &accountId) const;
    };
}

#endif
