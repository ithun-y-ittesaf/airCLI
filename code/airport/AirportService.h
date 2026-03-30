#ifndef AIRPORT_AIRPORTSERVICE_H
#define AIRPORT_AIRPORTSERVICE_H

#include <string>
#include "../storage/IRepository.h"
using namespace std;

namespace airport {

    class AirportService {
    private:
        storage::IRepository &repo;
        long long             budgetCents;

        void recordEvent(const string &eventType, long long deltaCents);

    public:
        explicit AirportService(storage::IRepository &repo);

        bool      load();
        bool      save() const;
        long long getBudget() const;

        // Budget-increasing events.
        void onTicketPurchased(long long priceCents);

        // Budget-decreasing events.
        void onFlightAdded();
        void onHandlerStartedFlight(); // 5,000 BDT per flight, first time only.
    };

} // namespace airport

#endif // AIRPORT_AIRPORTSERVICE_H
