#ifndef TICKETS_TICKET_H
#define TICKETS_TICKET_H

#include <string>

using namespace std;

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

        Ticket() {}
        Ticket(const string &ticketId, const string &passengerId, 
               const string &flightId, const string &issued,
               const string &origin, const string &destination, 
               const string &flightDate)
        {
            this->id = ticketId;
            this->passengerId = passengerId;
            this->flightId = flightId;
            this->issuedDate = issued;
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

#endif // TICKET_H
