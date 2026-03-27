#ifndef BANKING_BANK_H
#define BANKING_BANK_H

#include <string>
#include <vector>
using namespace std;

namespace banking {

    // One pre-seeded account inside a bank.
    struct BankAccount {
        string accountNumber;
        string password;
        long long   balanceCents; // Individual account balance (not the pool).
    };

    // A predefined bank institution with its own pool balance and tier.
    class Bank {
    private:
        string              name;
        int                      tier;        // 1 = premium, 2 = standard, 3 = low
        long long                poolCents;   // Shared pool all accounts draw from.
        vector<BankAccount> accounts;

    public:
        Bank() : tier(0), poolCents(0) {}

        Bank(const string &name, int tier, long long poolCents)
            : name(name), tier(tier), poolCents(poolCents) {}

        const string              &getName()     const { return name;     }
        int                             getTier()     const { return tier;     }
        long long                       getPool()     const { return poolCents; }
        const vector<BankAccount> &getAccounts() const { return accounts; }

        void setPool(long long v) { poolCents = v; }
        void addAccount(const BankAccount &acc) { accounts.push_back(acc); }

        // Find account by number. Returns nullptr if not found.
        const BankAccount *findAccount(const string &accountNumber) const;

        // Authenticate: account number + password must match.
        bool authenticate(const string &accountNumber,
                          const string &password) const;

        // Deduct from pool. Returns false if insufficient.
        bool deductFromPool(long long amountCents);

        // Suspicion score adjustment for this bank's tier.
        // Tier 1 = -10, Tier 2 = 0, Tier 3 = +20
        int scoreAdjustment() const;

        // Storage — bank pool line format: name|tier|poolCents
        string serialize() const;
        static Bank deserializePool(const string &line,
                                    const vector<BankAccount> &accounts);
    };

} // namespace banking

#endif // BANKING_BANK_H
