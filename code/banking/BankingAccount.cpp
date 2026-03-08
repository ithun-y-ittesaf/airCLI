#include "BankingAccount.h"
#include <sstream>

using namespace std;

namespace banking {
    string BankingAccount::serialize() const {
        // Format: id|userId|balanceCents
        return id + "|" + userId + "|" + to_string(balanceCents);
    }

    BankingAccount BankingAccount::deserialize(const string &line) {
        stringstream ss(line);
        string id, userId, balanceStr;
        
        // Parse pipe-delimited values
        getline(ss, id, '|');
        getline(ss, userId, '|');
        getline(ss, balanceStr, '|');
        
        // Convert balance to long long
        long long balance = 0;
        try {
            balance = stoll(balanceStr);
        } catch (...) {
            balance = 0;
        }
        
        return BankingAccount(id, userId, balance);
    }
}
