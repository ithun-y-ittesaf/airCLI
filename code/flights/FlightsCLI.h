#ifndef FLIGHTS_FLIGHTSCLI_H
#define FLIGHTS_FLIGHTSCLI_H

#include <string>
#include "FlightManager.h"
#include "../Role.h"
#include "../tickets/TicketManager.h"

namespace flights {
    class FlightsCLI {
    private:
        FlightManager &manager;
        tickets::TicketManager &ticketManager;
    public:
        explicit FlightsCLI(FlightManager &m, tickets::TicketManager &tm) : manager(m), ticketManager(tm) {}
        void run(const string &currentUserId, Role currentRole);
        void runSearchAndBook(const string &currentUserId, Role currentRole);
    };
}

#endif
