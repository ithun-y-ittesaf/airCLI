#ifndef FLIGHTS_FLIGHT_H
#define FLIGHTS_FLIGHT_H

#include <string>
#include "../utils/Types.h"

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
        Flight(int id = 0, string origin = nullptr, string destination = nullptr, string date = nullptr, int capacity = 0, int seatsBooked = 0)
        {
        this->id = id;
        this->origin = origin;
        this->destination = destination;
        this->date = date;
        this->capacity = capacity;
        this->seatsBooked = seatsBooked;
    }
    
        string getId()
        {
            return id; 
        }

        string getOrigin()
        {
            return origin;
        }

        string getDestination()
        { 
            return  destination;
        }

        string getDate()
        { 
            return date; 
        }

        int getCapacity()  
        { 
            return capacity; 
        }

        int getSeatsBooked()  
        { 
            return seatsBooked; 
        }

        void setSeatsBooked(int v) 
        { 
            seatsBooked = v; 
        }

        string serialize() ;
        static Flight deserialize( string &line);
    };
}

#endif
