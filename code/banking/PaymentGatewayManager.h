#ifndef BANKING_PAYMENTGATEWAYMANAGER_H
#define BANKING_PAYMENTGATEWAYMANAGER_H

#include <string>
#include <vector>
#include "../storage/IRepository.h"
using namespace std;

namespace banking {

    struct SavedPaymentMethod {
        string id;
        string passengerId;
        string nickname;
        string bankName;
        string bankUsername;
        string bankPassword;
    };

    struct PaymentReceipt {
        string id;
        string passengerId;
        string methodId;
        string purpose;
        string referenceId;
        long long   amountCents;
        string status;
        string message;
        string createdAt;
    };

    class PaymentGatewayManager {
    private:
        storage::IRepository &repo;
        vector<SavedPaymentMethod> methods;
        vector<PaymentReceipt> receipts;

    public:
        explicit PaymentGatewayManager(storage::IRepository &repo) : repo(repo) {}

        bool load();
        bool saveMethods() const;
        bool saveReceipts() const;

        vector<SavedPaymentMethod> getMethodsForPassenger(const string &passengerId) const;
        const SavedPaymentMethod *getMethod(const string &methodId,
                                            const string &passengerId) const;

        string addMethod(const string &passengerId,
                              const string &nickname,
                              const string &bankName,
                              const string &bankUsername,
                              const string &bankPassword);
        bool removeMethod(const string &passengerId,
                  const string &methodId);

        string addReceipt(const string &passengerId,
                               const string &methodId,
                               const string &purpose,
                               const string &referenceId,
                               long long          amountCents,
                               const string &status,
                               const string &message);

        const PaymentReceipt *findLatestByReference(const string &referenceId) const;
    };

} // namespace banking

#endif // BANKING_PAYMENTGATEWAYMANAGER_H
