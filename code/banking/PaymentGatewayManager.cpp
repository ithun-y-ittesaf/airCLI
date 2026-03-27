#include "PaymentGatewayManager.h"
#include "../core/IdGenerator.h"

#include <sstream>
#include <ctime>
#include <iomanip>
using namespace std;

namespace banking {

static string nowDateTime() {
    time_t t = time(nullptr);
    tm tm = *localtime(&t);
    stringstream ss;
    ss << put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool PaymentGatewayManager::load() {
    methods.clear();
    receipts.clear();

    for (const auto &line : repo.readAll("payment_methods")) {
        stringstream ss(line);
        SavedPaymentMethod m;
        getline(ss, m.id, '|');
        getline(ss, m.passengerId, '|');
        getline(ss, m.nickname, '|');
        getline(ss, m.bankName, '|');
        getline(ss, m.bankUsername, '|');
        getline(ss, m.bankPassword, '|');
        if (!m.id.empty()) methods.push_back(m);
    }

    for (const auto &line : repo.readAll("payment_receipts")) {
        stringstream ss(line);
        PaymentReceipt r;
        string amount;
        getline(ss, r.id, '|');
        getline(ss, r.passengerId, '|');
        getline(ss, r.methodId, '|');
        getline(ss, r.purpose, '|');
        getline(ss, r.referenceId, '|');
        getline(ss, amount, '|');
        getline(ss, r.status, '|');
        getline(ss, r.message, '|');
        getline(ss, r.createdAt, '|');
        try { r.amountCents = stoll(amount); } catch (...) { r.amountCents = 0; }
        if (!r.id.empty()) receipts.push_back(r);
    }

    return true;
}

bool PaymentGatewayManager::saveMethods() const {
    vector<string> lines;
    for (const auto &m : methods) {
        lines.push_back(m.id + "|" + m.passengerId + "|" + m.nickname + "|" +
                        m.bankName + "|" + m.bankUsername + "|" + m.bankPassword);
    }
    return repo.writeAll("payment_methods", lines);
}

bool PaymentGatewayManager::saveReceipts() const {
    vector<string> lines;
    for (const auto &r : receipts) {
        lines.push_back(r.id + "|" + r.passengerId + "|" + r.methodId + "|" + r.purpose + "|" +
                        r.referenceId + "|" + to_string(r.amountCents) + "|" +
                        r.status + "|" + r.message + "|" + r.createdAt);
    }
    return repo.writeAll("payment_receipts", lines);
}

vector<SavedPaymentMethod> PaymentGatewayManager::getMethodsForPassenger(const string &passengerId) const {
    vector<SavedPaymentMethod> out;
    for (const auto &m : methods)
        if (m.passengerId == passengerId) out.push_back(m);
    return out;
}

const SavedPaymentMethod *PaymentGatewayManager::getMethod(const string &methodId,
                                                           const string &passengerId) const {
    for (const auto &m : methods)
        if (m.id == methodId && m.passengerId == passengerId) return &m;
    return nullptr;
}

string PaymentGatewayManager::addMethod(const string &passengerId,
                                             const string &nickname,
                                             const string &bankName,
                                             const string &bankUsername,
                                             const string &bankPassword) {
    if (passengerId.empty() || nickname.empty() || bankName.empty() ||
        bankUsername.empty() || bankPassword.empty()) {
        return "";
    }

    for (const auto &m : methods) {
        if (m.passengerId == passengerId &&
            m.bankName == bankName &&
            m.bankUsername == bankUsername) {
            return m.id;
        }
    }

    string id = core::IdGenerator::next("PM", [&](const string &candidate) {
        for (const auto &m : methods)
            if (m.id == candidate) return false;
        return true;
    });

    methods.push_back({id, passengerId, nickname, bankName, bankUsername, bankPassword});
    if (!saveMethods()) return "";
    return id;
}

bool PaymentGatewayManager::removeMethod(const string &passengerId,
                                         const string &methodId) {
    for (auto it = methods.begin(); it != methods.end(); ++it) {
        if (it->passengerId == passengerId && it->id == methodId) {
            methods.erase(it);
            return saveMethods();
        }
    }
    return false;
}

string PaymentGatewayManager::addReceipt(const string &passengerId,
                                              const string &methodId,
                                              const string &purpose,
                                              const string &referenceId,
                                              long long          amountCents,
                                              const string &status,
                                              const string &message) {
    string id = core::IdGenerator::next("RC", [&](const string &candidate) {
        for (const auto &r : receipts)
            if (r.id == candidate) return false;
        return true;
    });

    receipts.push_back({id, passengerId, methodId, purpose, referenceId,
                        amountCents, status, message, nowDateTime()});
    saveReceipts();
    return id;
}

const PaymentReceipt *PaymentGatewayManager::findLatestByReference(const string &referenceId) const {
    for (auto it = receipts.rbegin(); it != receipts.rend(); ++it)
        if (it->referenceId == referenceId) return &(*it);
    return nullptr;
}

} // namespace banking
