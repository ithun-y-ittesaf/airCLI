#include "BankingManager.h"
#include "../storage/DataStorage.h"
#include "../logging/Logger.h"
#include <chrono>

namespace banking {
    bool BankingManager::load() {
        accountsById.clear();
        userToAccountId.clear();
        
        auto lines = storage::DataStorage::readAll("accounts.txt");
        for (const auto &line : lines) {
            if (line.empty()) {
                continue;
            }
            
            BankingAccount account = BankingAccount::deserialize(line);
            accountsById[account.getId()] = account;
            userToAccountId[account.getUserId()] = account.getId();
        }
        
        return true;
    }

    bool BankingManager::save() const {
        std::vector<std::string> lines;
        lines.reserve(accountsById.size());
        
        for (const auto &pair : accountsById) {
            lines.push_back(pair.second.serialize());
        }
        
        return storage::DataStorage::writeAll("accounts.txt", lines);
    }

    string BankingManager::getOrCreateAccountForUser(const string &userId) {
        // Check if user already has an account
        auto it = userToAccountId.find(userId);
        if (it != userToAccountId.end()) {
            return it->second;
        }
        
        // Create new account for this user
        using namespace std::chrono;
        auto timestamp = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
        string accountId = "A" + std::to_string(timestamp);
        
        // Ensure ID is unique
        while (accountsById.find(accountId) != accountsById.end()) {
            ++timestamp;
            accountId = "A" + std::to_string(timestamp);
        }
        
        // Create account with zero balance
        BankingAccount newAccount(accountId, userId, 0);
        accountsById[accountId] = newAccount;
        userToAccountId[userId] = accountId;
        
        // Persist to storage
        storage::DataStorage::appendLine("accounts.txt", newAccount.serialize());
        
        return accountId;
    }

    const BankingAccount* BankingManager::getAccount(const string &accountId) const {
        auto it = accountsById.find(accountId);
        if (it == accountsById.end()) {
            return nullptr;
        }
        return &it->second;
    }

    const BankingAccount* BankingManager::getAccountByUser(const string &userId) const {
        auto it = userToAccountId.find(userId);
        if (it == userToAccountId.end()) {
            return nullptr;
        }
        return getAccount(it->second);
    }

    std::vector<Transaction> BankingManager::getTransactionsForAccount(const string &accountId) const {
        std::vector<Transaction> transactions;
        auto lines = storage::DataStorage::readAll("transactions.txt");
        
        for (const auto &line : lines) {
            if (line.empty()) {
                continue;
            }
            
            Transaction transaction = Transaction::deserialize(line);
            if (transaction.getAccountId() == accountId) {
                transactions.push_back(transaction);
            }
        }
        
        return transactions;
    }

    Result BankingManager::deposit(const string &accountId, long long amountCents, 
                                         const std::string &description) {
        // Validate amount
        if (amountCents <= 0) {
            return Result::Err("Deposit amount must be positive");
        }
        
        // Find account
        auto it = accountsById.find(accountId);
        if (it == accountsById.end()) {
            return Result::Err("Account not found");
        }
        
        // Update balance
        long long newBalance = it->second.getBalanceCents() + amountCents;
        it->second.setBalanceCents(newBalance);
        save();
        
        // Create transaction record
        using namespace std::chrono;
        auto timestamp = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
        string transactionId = "T" + std::to_string(timestamp);
        
        Transaction transaction(transactionId, accountId, amountCents, 
                              TransactionType::Credit, description, timestamp);
        storage::DataStorage::appendLine("transactions.txt", transaction.serialize());
        
        // Log activity
        logging::Logger::logCritical("Payment", 
            "ACCOUNT=" + accountId + "|CREDIT=" + std::to_string(amountCents));
        
        return Result::Ok("Deposit successful");
    }

    Result BankingManager::withdraw(const string &accountId, long long amountCents, 
                                          const std::string &description) {
        // Validate amount
        if (amountCents <= 0) {
            return Result::Err("Withdrawal amount must be positive");
        }
        
        // Find account
        auto it = accountsById.find(accountId);
        if (it == accountsById.end()) {
            return Result::Err("Account not found");
        }
        
        // Check sufficient funds
        if (it->second.getBalanceCents() < amountCents) {
            return Result::Err("Insufficient funds");
        }
        
        // Update balance
        long long newBalance = it->second.getBalanceCents() - amountCents;
        it->second.setBalanceCents(newBalance);
        save();
        
        // Create transaction record
        using namespace std::chrono;
        auto timestamp = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
        string transactionId = "T" + std::to_string(timestamp);
        
        Transaction transaction(transactionId, accountId, amountCents, 
                              TransactionType::Debit, description, timestamp);
        storage::DataStorage::appendLine("transactions.txt", transaction.serialize());
        
        logging::Logger::logCritical("Payment", 
            "ACCOUNT=" + accountId + "|DEBIT=" + std::to_string(amountCents));
        
        return Result::Ok("Withdrawal successful");
    }
}
