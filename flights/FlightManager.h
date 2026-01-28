#ifndef FLIGHTS_FLIGHTMANAGER_H
#define FLIGHTS_FLIGHTMANAGER_H

#include <map>
#include <vector>
#include "Flight.h"

namespace flights 
{
    class FlightManager 
    {
        map<string, Flight> flightsById;
    public:
        bool load();
        bool save() const;

        string createFlight(const string &orig, const string &dest, const string &date, int capacity);

        const Flight* get(const string &id) const;

        vector<Flight> all() const;

        bool bookSeat(const string &id);
    };
}

#endif
