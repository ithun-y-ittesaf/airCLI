#ifndef TICKETS_TICKETMANAGER_H
#define TICKETS_TICKETMANAGER_H

#include <map>
#include <vector>
#include "Ticket.h"

namespace flights { class Flight; }

namespace tickets {
    class TicketManager {
    private:
        map<string, Ticket> ticketsById;
    public:
        bool load();
        bool save() const;

        string issueTicket(const string &passengerId, const flights::Flight &flight);
        const Ticket* get(const string &id) const;
        vector<Ticket> getForPassenger(const string &passengerId) const;
    };
}

#endif
