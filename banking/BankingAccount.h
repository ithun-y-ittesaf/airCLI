#ifndef BANKING_BANKINGACCOUNT_H
#define BANKING_BANKINGACCOUNT_H

#include <string>

using namespace std;

namespace banking {

    class BankingAccount {
    private:
        string id;
        string userId;
        long long balanceCents;

    public:

        BankingAccount() : balanceCents(0) {}

        BankingAccount(const string &accountId, const string &accountUserId, 
                      long long accountBalanceCents)
            : id(accountId), userId(accountUserId), balanceCents(accountBalanceCents) {}

        // Getters
        const string &getId() const { return id; }
        const string &getUserId() const { return userId; }
        long long getBalanceCents() const { return balanceCents; }

        // Update account balance
        void setBalanceCents(long long newBalance) { balanceCents = newBalance; }

        // Serialization for storage
        string serialize() const;
        static BankingAccount deserialize(const string &line);
    };
}

#endif
