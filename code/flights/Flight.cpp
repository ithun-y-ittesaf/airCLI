#include "Flight.h"
#include <sstream>

namespace flights {
    string Flight::serialize() const  
    {
        return id + "|" + origin + "|" + destination + "|" + date + "|" + to_string(capacity) + "|" + to_string(seatsBooked);
    }

    Flight Flight::deserialize(const string &line) 
    {
        stringstream ss(line);
        string id, orig, dest, date, capStr, bookedStr;

        getline(ss, id, '|');
        getline(ss, orig, '|');
        getline(ss, dest, '|');
        getline(ss, date, '|');
        getline(ss, capStr, '|');
        getline(ss, bookedStr, '|');

        int cap = stoi(capStr);
        int booked = stoi(bookedStr);

        return Flight(id, orig, dest, date, cap, booked);
    }
}
