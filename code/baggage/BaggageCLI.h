#ifndef BAGGAGE_BAGGAGECLI_H
#define BAGGAGE_BAGGAGECLI_H

#include <string>
#include <vector>

#include "BaggageManager.h"
#include "ItemDatabase.h"
#include "../tickets/TicketManager.h"
#include "../banking/BankManager.h"
#include "../airport/AirportService.h"
#include "../users/UserManager.h"
#include "../users/UserCLI.h"
#include "../flights/FlightManager.h"
#include "../Role.h"
using namespace std;

namespace baggage {

    class BaggageCLI {
    private:
        BaggageManager          &baggageManager;
        ItemDatabase            &itemDb;
        tickets::TicketManager  &ticketManager;
        banking::BankManager    &bankManager;
        airport::AirportService &airport;
        users::UserManager      &userManager;
        users::UserCLI          &userCLI;
        flights::FlightManager  &flightManager;

        void runDeclarationForm(vector<PackedItem> &selectedItems);
        void runInspectTicket(const string &ticketId);

    public:
        BaggageCLI(BaggageManager          &bm,
                   ItemDatabase            &db,
                   tickets::TicketManager  &tm,
                   banking::BankManager    &bankMgr,
                   airport::AirportService &ap,
                   users::UserManager      &um,
                   users::UserCLI          &uc,
                   flights::FlightManager  &fm)
            : baggageManager(bm), itemDb(db), ticketManager(tm),
              bankManager(bankMgr), airport(ap),
              userManager(um), userCLI(uc), flightManager(fm) {}

        // Called after booking — passenger packs their bag.
        void runPackBag(const string &passengerId,
                        const string &ticketId);
        
        // Entry point with authentication — user logs in and selects ticket.
        void runAuthenticatedFlow();

        // Handler dashboard entry point.
        void runHandlerView(const string &handlerId);
    };

} // namespace baggage

#endif // BAGGAGE_BAGGAGECLI_H
