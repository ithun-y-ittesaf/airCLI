#ifndef FLIGHTS_FLIGHT_H
#define FLIGHTS_FLIGHT_H

#include <string>

using namespace std;

namespace flights {
    class Flight {
        string id;
        string origin;
        string destination;
        string date;
        int capacity;
        int seatsBooked;

    public:
        Flight(string id = "", string origin = "", string destination = "", string date = "", int capacity = 0, int seatsBooked = 0)
        {
        this->id = id;
        this->origin = origin;
        this->destination = destination;
        this->date = date;
        this->capacity = capacity;
        this->seatsBooked = seatsBooked;
    }
    
        const string& getId() const
        {
            return id; 
        }

        const string& getOrigin() const
        {
            return origin;
        }

        const string& getDestination() const
        { 
            return  destination;
        }

        const string& getDate() const
        { 
            return date; 
        }

        int getCapacity() const  
        { 
            return capacity; 
        }

        int getSeatsBooked() const  
        { 
            return seatsBooked; 
        }

        void setSeatsBooked(int v) 
        { 
            seatsBooked = v; 
        }

        string serialize() const;
        static Flight deserialize(const string &line);
    };
}

#endif
