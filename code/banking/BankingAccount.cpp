#include "Bank.h"
#include <sstream>
using namespace std;

namespace banking {

const BankAccount *Bank::findAccount(const string &accountNumber) const {
    for (const auto &acc : accounts)
        if (acc.accountNumber == accountNumber) return &acc;
    return nullptr;
}

bool Bank::authenticate(const string &accountNumber,
                         const string &password) const {
    const BankAccount *acc = findAccount(accountNumber);
    return acc && acc->password == password;
}

bool Bank::deductFromPool(long long amountCents) {
    if (poolCents < amountCents) return false;
    poolCents -= amountCents;
    return true;
}

int Bank::scoreAdjustment() const {
    switch (tier) {
        case 1:  return -10;
        case 2:  return   0;
        case 3:  return  20;
        default: return  20; // Unknown tier treated as worst case.
    }
}

string Bank::serialize() const {
    // Format: name|tier|poolCents
    return name + "|" + to_string(tier) + "|" + to_string(poolCents);
}

Bank Bank::deserializePool(const string &line,
                            const vector<BankAccount> &accs) {
    stringstream ss(line);
    string name, tierStr, poolStr;
    getline(ss, name,    '|');
    getline(ss, tierStr, '|');
    getline(ss, poolStr, '|');

    int       tier = 2;
    long long pool = 0;
    try { tier = stoi(tierStr); } catch (...) {}
    try { pool = stoll(poolStr); } catch (...) {}

    Bank b(name, tier, pool);
    for (const auto &a : accs) b.addAccount(a);
    return b;
}

} // namespace banking
