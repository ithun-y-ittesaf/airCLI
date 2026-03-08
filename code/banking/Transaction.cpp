#include "Transaction.h"
#include <sstream>

using namespace std;

namespace banking {
    string Transaction::serialize() const {
        // Format: id|accountId|amountCents|type|description|timestampSec
        return id + "|" + accountId + "|" + to_string(amountCents) + "|" +
               to_string(static_cast<int>(type)) + "|" + description + "|" +
               to_string(timestampSec);
    }

    Transaction Transaction::deserialize(const string &line) {
        stringstream ss(line);
        string id, accountId, amountStr, typeStr, description, timestampStr;
        
        // Parse pipe-delimited values
        getline(ss, id, '|');
        getline(ss, accountId, '|');
        getline(ss, amountStr, '|');
        getline(ss, typeStr, '|');
        getline(ss, description, '|');
        getline(ss, timestampStr, '|');
        
        // Convert values to appropriate types
        long long amount = 0, timestamp = 0;
        int typeValue = 0;
        
        try { amount = stoll(amountStr); } catch (...) { amount = 0; }
        try { typeValue = stoi(typeStr); } catch (...) { typeValue = 0; }
        try { timestamp = stoll(timestampStr); } catch (...) { timestamp = 0; }
        
        return Transaction(id, accountId, amount, static_cast<TransactionType>(typeValue), 
                         description, timestamp);
    }
}
