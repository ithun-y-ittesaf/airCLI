#include "Ticket.h"
#include <sstream>

namespace tickets {
    string Ticket::serialize() const {
        return id + "|" + passengerId + "|" + flightId + "|" + issuedDate + "|" + origin + "|" + destination + "|" + flightDate;
    }

    Ticket Ticket::deserialize(const string &line) {
        stringstream ss(line);
        string id, passId, flightId, issued, orig, dest, fDate;
        getline(ss, id, '|');
        getline(ss, passId, '|');
        getline(ss, flightId, '|');
        getline(ss, issued, '|');
        getline(ss, orig, '|');
        getline(ss, dest, '|');
        getline(ss, fDate, '|');
        
        return Ticket(id, passId, flightId, issued, orig, dest, fDate);
    }
}
