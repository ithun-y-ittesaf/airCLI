#include <iostream>
#include "users/UserManager.h"
#include "users/UserCLI.h"
#include "banking/BankingManager.h"
#include "banking/BankingCLI.h"
#include "flights/FlightManager.h"
#include "flights/FlightsCLI.h"
#include "tickets/TicketManager.h"
#include "tickets/TicketsCLI.h"

int main() {
    // Initialize all our core services
    users::UserManager userManager;
    banking::BankingManager bankingManager;
    flights::FlightManager flightManager;
    tickets::TicketManager ticketManager;

    // Load existing data from storage
    userManager.load();
    bankingManager.load();
    flightManager.load();
    ticketManager.load();

    // Set up the CLI interfaces
    users::UserCLI userCLI(userManager);
    banking::BankingCLI bankingCLI(bankingManager);
    flights::FlightsCLI flightsCLI(flightManager, ticketManager);
    tickets::TicketsCLI ticketsCLI(ticketManager, flightManager);

    // Handle user authentication
    string userId;
    Role userRole = Role::Passenger;

    if (!userCLI.welcomeFlow(userId, userRole)) {
        return 0;
    }

    // Main dashboard loop
    bool isRunning = true;
    while (isRunning) {
        std::cout << "\n==============================\n";
        std::cout << " airCLI Dashboard\n";
        std::cout << "==============================\n";
        std::cout << "1. Search & Book Flights\n";
        std::cout << "2. My Tickets\n";
        std::cout << "3. Travel History\n";
        std::cout << "4. Link Bank\n";
        
        if (userRole == Role::Admin) {
            std::cout << "5. Admin: Manage Flights\n";
            std::cout << "6. Logout\n";
        } else {
            std::cout << "5. Logout\n";
        }
        
        std::cout << "\nSelect an option > ";

        int choice = 0;
        std::cin >> choice;

        switch (choice) {
        case 1:
            flightsCLI.runSearchAndBook(userId, userRole);
            break;
        case 2:
            ticketsCLI.runViewTickets(userId, userRole);
            break;
        case 3:
            ticketsCLI.runTravelHistory(userId, userRole);
            break;
        case 4: {
            bool openBanking = userCLI.linkBankAccount(userId);
            if (openBanking) {
                bankingCLI.runBanking(userId, userRole);
            }
            break;
        }
        case 5:
            if (userRole == Role::Admin) {
            flightsCLI.run(userId, userRole);
            } else {
            std::cout << "\nLogging out...\n";
            isRunning = false;
            }
            break;
        case 6:
            if (userRole == Role::Admin) {
            std::cout << "\nLogging out...\n";
            isRunning = false;
            } else {
            std::cout << "\nInvalid option. Please try again.\n";
            }
            break;
        default:
            std::cout << "\nInvalid option. Please try again.\n";
        }
    }

    std::cout << "\nThanks for using airCLI. Goodbye!\n";
    return 0;
}
