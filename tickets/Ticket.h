#ifndef TICKETS_TICKET_H
#define TICKETS_TICKET_H

#include <string>
#include "../utils/Types.h"

namespace tickets {
    class Ticket {
    private:
        string id;
        string passengerId;
        string flightId;
        string issuedDate;
        string origin;
        string destination;
        string flightDate;
    public:
        Ticket(int id = 0, int passengerId = 0, int flightId = 0, string issuedDate = nullptr, string origin = nullptr, string destination = nullptr, string flightDate = nullptr)
        {
            this->id = id;
            this->passengerId = passengerId;
            this->flightDate = flightId;
            this->issuedDate = issuedDate;
            this->origin = origin;
            this->destination = destination;
            this->flightDate = flightDate;
        }

        const string &getId() const { return id; }
        const string &getPassengerId() const { return passengerId; }
        const string &getFlightId() const { return flightId; }
        const string &getIssuedDate() const { return issuedDate; }
        const string &getOrigin() const { return origin; }
        const string &getDestination() const { return destination; }
        const string &getFlightDate() const { return flightDate; }

        string serialize() const;
        static Ticket deserialize(const string &line);
    };
}

#endif
