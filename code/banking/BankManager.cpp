#include "BankManager.h"
#include "../logging/Logger.h"
#include "../core/IdGenerator.h"
#include <sstream>
using namespace std;

namespace banking {

void BankManager::seedDefaults() {
    banks.clear();

    struct BankDef { string name; int tier; long long pool; };
    const BankDef defs[] = {
        {"Dutch-Bangla Bank", 1, 10000000000LL}, // 100,000,000 BDT
        {"BRAC Bank",         1,  8000000000LL}, //  80,000,000 BDT
        {"Islami Bank",       2,  6000000000LL}, //  60,000,000 BDT
        {"Sonali Bank",       2,  5000000000LL}, //  50,000,000 BDT
        {"Pubali Bank",       3,  3000000000LL}, //  30,000,000 BDT
    };

    for (const auto &d : defs) {
        Bank b(d.name, d.tier, d.pool);
        for (const auto &acc : accountsFor(d.name))
            b.addAccount(acc);
        banks.push_back(b);
    }

    save();
}

// ---------------------------------------------------------------------------
// Load / save
// ---------------------------------------------------------------------------

bool BankManager::load() {
    auto lines = repo.readAll("banks");

    if (lines.empty()) {
        seedDefaults();
        return true;
    }

    banks.clear();
    for (const auto &line : lines) {
        // Parse name first to get the right accounts.
        string name = line.substr(0, line.find('|'));
        Bank b = Bank::deserializePool(line, accountsFor(name));
        banks.push_back(b);
    }

    bankUsers.clear();
    for (const auto &line : repo.readAll("bank_users")) {
        stringstream ss(line);
        string accountId, bankName, username, password, balanceStr;
        getline(ss, accountId, '|');
        getline(ss, bankName,  '|');
        getline(ss, username,  '|');
        getline(ss, password,  '|');
        getline(ss, balanceStr,'|');

        long long balance = 0;
        try { balance = stoll(balanceStr); } catch (...) { balance = 0; }

        if (username.empty()) continue;
        bankUsers.push_back({accountId, bankName, username, password, balance});
    }

    return true;
}

bool BankManager::save() const {
    vector<string> lines;
    for (const auto &b : banks)
        lines.push_back(b.serialize());
    bool banksSaved = repo.writeAll("banks", lines);

    vector<string> userLines;
    for (const auto &u : bankUsers) {
        userLines.push_back(u.accountId + "|" + u.bankName + "|" + u.username + "|" +
                           u.password + "|" + to_string(u.balanceCents));
    }
    bool usersSaved = repo.writeAll("bank_users", userLines);

    return banksSaved && usersSaved;
}

bool BankManager::registerBankUser(const string &bankName,
                                   const string &username,
                                   const string &password,
                                   string       &outAccountId) {
    if (username.empty() || password.empty()) return false;
    Bank *bank = findBank(bankName);
    if (!bank) return false;

    for (const auto &u : bankUsers)
        if (u.username == username) return false;

    outAccountId = core::IdGenerator::next("BA", [&](const string &candidate) {
        for (const auto &u : bankUsers)
            if (u.accountId == candidate) return false;
        return true;
    });

    bankUsers.push_back({outAccountId, bankName, username, password, 0});
    save();
    return true;
}

bool BankManager::authenticateBankUser(const string &username,
                                       const string &password,
                                       BankUserAccount   &outUser) const {
    for (const auto &u : bankUsers) {
        if (u.username == username && u.password == password) {
            outUser = u;
            return true;
        }
    }
    return false;
}

Result BankManager::depositToBankUser(const string &username,
                                      const string &password,
                                      long long          amountCents) {
    if (amountCents <= 0) return Result::Err("Amount must be positive");
    for (auto &u : bankUsers) {
        if (u.username == username && u.password == password) {
            u.balanceCents += amountCents;
            save();
            logging::Logger::logCritical("BankDeposit",
                "USER=" + username + "|BANK=" + u.bankName + "|AMT=" + to_string(amountCents));
            return Result::Ok("Deposit successful");
        }
    }
    return Result::Err("Invalid bank username or password");
}

Result BankManager::debitFromBankUser(const string &username,
                                      const string &password,
                                      long long          amountCents,
                                      string       &outBankName) {
    if (amountCents <= 0) return Result::Err("Amount must be positive");
    for (auto &u : bankUsers) {
        if (u.username == username && u.password == password) {
            if (u.balanceCents < amountCents) return Result::Err("Insufficient bank balance");
            u.balanceCents -= amountCents;
            outBankName = u.bankName;
            save();
            logging::Logger::logCritical("BankDebit",
                "USER=" + username + "|BANK=" + u.bankName + "|AMT=" + to_string(amountCents));
            return Result::Ok("Gateway debit successful");
        }
    }
    return Result::Err("Invalid bank username or password");
}

Bank *BankManager::findBank(const string &name) {
    for (auto &b : banks)
        if (b.getName() == name) return &b;
    return nullptr;
}

} // namespace banking
