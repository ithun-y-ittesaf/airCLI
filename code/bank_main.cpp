#include <iostream>
#include <string>

#include "storage/XmlRepository.h"
#include "banking/BankingTerminal.h"
using namespace std;

int main(int argc, char *argv[]) {
    storage::XmlRepository repo("storage/data");

    // Gateway mode launched from airCLI.
    // Usage: bankcli --gateway <airUserId> <methodId> <amountCents> <purpose> <referenceId>
    if (argc >= 7 && string(argv[1]) == "--gateway") {
        string airUserId = argv[2];
        string methodId = argv[3];
        long long amountCents = 0;
        try { amountCents = stoll(argv[4]); } catch (...) { amountCents = 0; }
        string purpose = argv[5];
        string referenceId = argv[6];

        return banking::runBankGatewayTerminal(airUserId, methodId, amountCents,
                                               purpose, referenceId, repo);
    }

    return banking::runBankRegistrationTerminal("standalone", repo);
}
