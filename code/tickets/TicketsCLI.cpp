#include "TicketsCLI.h"
#include "../access/AccessControl.h"
#include "../flights/FlightManager.h"
#include <iostream>
#include <algorithm>

namespace tickets {
    void TicketsCLI::run(const string &currentUserId, Role currentRole) {
        (void)currentRole;
        manager.load();
        cout << "\n-- My Tickets --\n";
        auto myTickets = manager.getForPassenger(currentUserId);
        cout << "Tickets (" << myTickets.size() << "):\n";
        for (const auto &tk : myTickets) {
            cout << "- " << tk.getId() << " | Flight: " << tk.getFlightId() 
                      << " | Issued: " << tk.getIssuedDate() << "\n";
        }
    }

    void TicketsCLI::runViewTickets(const string &currentUserId, Role currentRole) {
        (void)currentRole;
        manager.load();
        
        cout << "\n========== My Tickets ==========" << "\n\n";
        auto myTickets = manager.getForPassenger(currentUserId);
        
        if (myTickets.empty()) {
            cout << "No tickets found.\n";
            return;
        }
        
        for (size_t i = 0; i < myTickets.size(); ++i) {
            const auto &tk = myTickets[i];
            cout << "[" << (i+1) << "] " << tk.getId() << " | " << tk.getOrigin() << " → "
                      << tk.getDestination() << " | " << tk.getFlightDate() << " | VALID\n";
        }
        
        cout << "\n0. Back\nSelect ticket for details > ";
        int choice = 0;
        cin >> choice;
        
        if (choice < 1 || choice > (int)myTickets.size()) {
            return;
        }
        
        const auto &selected = myTickets[choice - 1];
        cout << "\nTicket ID     : " << selected.getId() << "\n";
        cout << "Flight        : " << selected.getFlightId() << " | " << selected.getOrigin() << " → "
                  << selected.getDestination() << " | " << selected.getFlightDate() << "\n";
        cout << "Status        : VALID\n";
        cout << "Boarding Gate : Assigned at airport\n";
    }

    void TicketsCLI::runTravelHistory(const string &currentUserId, Role currentRole) {
        (void)currentRole;
        manager.load();
        flightManager.load();
        
        cout << "\n========== Travel History ==========" << "\n\n";
        auto history = manager.getForPassenger(currentUserId);
        
        if (history.empty()) {
            cout << "No travel history.\n";
            return;
        }
        
        sort(history.begin(), history.end(), [](const Ticket &a, const Ticket &b) {
            return a.getIssuedDate() > b.getIssuedDate();
        });

        for (const auto &tk : history) {
            string routeOrigin = tk.getOrigin();
            string routeDest = tk.getDestination();
            string flightDate = tk.getFlightDate();

            if (routeOrigin.empty() || routeDest.empty() || flightDate.empty()) {
                if (const auto *flight = flightManager.get(tk.getFlightId())) {
                    routeOrigin = flight->getOrigin();
                    routeDest = flight->getDestination();
                    flightDate = flight->getDate();
                }
            }

            cout << routeOrigin << " → " << routeDest << " | " << flightDate
                      << " | Ticket " << tk.getId() << "\n";
        }
    }
}
