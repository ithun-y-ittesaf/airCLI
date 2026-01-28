#ifndef FLIGHTS_FLIGHTSCLI_H
#define FLIGHTS_FLIGHTSCLI_H

#include <string>
#include "FlightManager.h"
#include "../tickets/TicketManager.h"
#include "../utils/Types.h"

namespace flights {
    class FlightsCLI {
    private:
        FlightManager &manager;
        tickets::TicketManager &ticketManager;
    public:
        explicit FlightsCLI(FlightManager &m, tickets::TicketManager &tm) : manager(m), ticketManager(tm) {}
        void run(const utils::ID &currentUserId, utils::Role currentRole);
        void runSearchAndBook(const utils::ID &currentUserId, utils::Role currentRole);
    };
}

#endif
