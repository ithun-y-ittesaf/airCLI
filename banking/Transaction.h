#ifndef BANKING_TRANSACTION_H
#define BANKING_TRANSACTION_H

#include <string>

using namespace std;

namespace banking {
    enum class TransactionType { 
        Credit = 0,  // Money added to account
        Debit = 1    // Money removed from account
    };

    class Transaction {
    private:
        string id;
        string accountId;
        long long amountCents;
        TransactionType type;
        string description;
        long long timestampSec;

    public:

        Transaction() : amountCents(0), type(TransactionType::Credit), timestampSec(0) {}

        Transaction(const string &transactionId, const string &accountId, 
                   long long amountCents, TransactionType type, 
                   const string &description, long long timestamp)
            : id(transactionId), accountId(accountId), amountCents(amountCents), 
              type(type), description(description), timestampSec(timestamp) {}

        // Getters 
        const string &getId() const { return id; }
        const string &getAccountId() const { return accountId; }
        long long getAmountCents() const { return amountCents; }
        TransactionType getType() const { return type; }
        const string &getDescription() const { return description; }
        long long getTimestampSec() const { return timestampSec; }

        // Storage
        string serialize() const;
        static Transaction deserialize(const string &line);
    };
}

#endif
