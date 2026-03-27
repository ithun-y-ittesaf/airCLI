#ifndef BANKING_BANKINGTERMINAL_H
#define BANKING_BANKINGTERMINAL_H

#include <string>
#include "../storage/IRepository.h"
using namespace std;

namespace banking {

int runBankRegistrationTerminal(const string    &airUserId,
                                storage::IRepository &repo);

int runBankGatewayTerminal(const string    &airUserId,
                           const string    &methodId,
                           long long             amountCents,
                           const string    &purpose,
                           const string    &referenceId,
                           storage::IRepository &repo);

} // namespace banking

#endif // BANKING_BANKINGTERMINAL_H
