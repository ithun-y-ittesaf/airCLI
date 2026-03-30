#ifndef BAGGAGE_BAGGAGETERMINAL_H
#define BAGGAGE_BAGGAGETERMINAL_H

#include <string>
#include "../storage/IRepository.h"
using namespace std;

namespace baggage {

    // Entry point called when executable is launched with:
    //   aircli baggage <userId> <ticketId>
    // Runs the full interactive baggage packing flow in this terminal,
    // then exits. Weight randomization happens in BaggageManager::packBag().
    int runBaggageTerminal(const string    &userId,
                           const string    &ticketId,
                           storage::IRepository &repo);

    // Entry point for authenticated baggage flow.
    // User logs in, selects a ticket, and packs baggage.
    int runBaggageTerminalAuthenticated(storage::IRepository &repo);

} // namespace baggage

#endif // BAGGAGE_BAGGAGETERMINAL_H
