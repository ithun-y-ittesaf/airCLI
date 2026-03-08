#include "Ticket.h"
#include <sstream>

using namespace std;

namespace tickets {
    string Ticket::serialize() const {
        return id + "|" + passengerId + "|" + flightId + "|" + issuedDate + "|" + origin + "|" + destination + "|" + flightDate;
    }

    Ticket Ticket::deserialize(const string &line) {
        stringstream ss(line);
        string id, passengerId, flightId, issuedDate, origin, destination, flightDate;

        getline(ss, id, '|');
        getline(ss, passengerId, '|');
        getline(ss, flightId, '|');
        getline(ss, issuedDate, '|');
        getline(ss, origin, '|');
        getline(ss, destination, '|');
        getline(ss, flightDate, '|');
        
        return Ticket(id, passengerId, flightId, issuedDate, origin, destination, flightDate);
    }
}
