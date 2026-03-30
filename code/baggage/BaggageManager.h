#ifndef BAGGAGE_BAGGAGEMANAGER_H
#define BAGGAGE_BAGGAGEMANAGER_H

#include <map>
#include <string>
#include <vector>

#include "Bag.h"
#include "ItemDatabase.h"
#include "../storage/IRepository.h"
#include "../airport/AirportService.h"
#include "../users/UserManager.h"
using namespace std;

namespace tickets { class TicketManager; }

namespace baggage {

    static const long long ILLEGAL_ITEM_FINE_CENTS = 1000000LL; // 10,000 BDT

    struct InspectionResult {
        bool        foundIllegal;
        string illegalItemName;
        long long   fineApplied;
        bool        overweight;
        int         overweightGrams;
        long long   overweightFineCents;
        bool        foundUndeclared;
        int         undeclaredCount;
        string undeclaredItemName;
    };

    class BaggageManager {
    private:
        storage::IRepository       &repo;
        airport::AirportService    &airport;
        ItemDatabase               &itemDb;
        users::UserManager         &userManager;
        map<string, Bag>  bagsById;

    public:
        BaggageManager(storage::IRepository   &repo,
                       airport::AirportService &airport,
                       ItemDatabase            &itemDb,
                       users::UserManager      &userManager)
            : repo(repo), airport(airport),
              itemDb(itemDb), userManager(userManager) {}

        bool load();
        bool save() const;

        // Pack a bag — weight randomization happens here via ItemDatabase.
        string packBag(const string             &passengerId,
                            const string             &ticketId,
                            const string             &bagTag,
                            const vector<PackedItem> &items);

        const Bag *getBagByTicket(const string &ticketId) const;

        vector<Bag> getBagsForFlight(
            const string            &flightId,
            const tickets::TicketManager &ticketManager) const;

        // Full inspection: checks overweight AND illegal items.
        // Applies fines, bans passenger if illegal item found.
        InspectionResult inspectBag(const string &bagId);

        // Delete a bag when its ticket is cancelled.
        bool deleteBagForTicket(const string &ticketId);
    };

} // namespace baggage

#endif // BAGGAGE_BAGGAGEMANAGER_H
