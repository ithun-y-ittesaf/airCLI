#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>

#ifdef __linux__
#include <unistd.h>
#include <termios.h>
#endif

#include "Role.h"
#include "storage/XmlRepository.h"
#include "airport/AirportService.h"
#include "users/UserManager.h"
#include "users/UserCLI.h"
#include "banking/BankManager.h"
#include "banking/PaymentGatewayManager.h"
#include "flights/FlightManager.h"
#include "flights/FlightsCLI.h"
#include "tickets/TicketManager.h"
#include "tickets/TicketsCLI.h"
#include "baggage/ItemDatabase.h"
#include "baggage/BaggageManager.h"
#include "baggage/BaggageCLI.h"
#include "baggage/BaggageTerminal.h"
#include "core/TerminalLauncher.h"
using namespace std;

static string executablePath() {
#ifdef _WIN32
    return "aircli.exe";
#else
    char buf[4096];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) { buf[len] = '\0'; return string(buf); }
    return "./build/aircli";
#endif
}

static string siblingExecutablePath(const string &name) {
    filesystem::path self(executablePath());
    return (self.parent_path() / name).string();
}

static void printBudget(long long cents) {
    cout << "BDT " << (cents / 100) << "."
              << setfill('0') << setw(2) << (cents % 100);
}

static string readMaskedLine() {
#ifdef __linux__
    if (isatty(STDIN_FILENO)) {
        termios oldt{};
        termios newt{};
        if (tcgetattr(STDIN_FILENO, &oldt) == 0) {
            newt = oldt;
            newt.c_lflag &= static_cast<unsigned>(~ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
            string s;
            getline(cin, s);
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            cout << "\n";
            return s;
        }
    }
#endif
    string s;
    getline(cin, s);
    return s;
}

static const banking::PaymentReceipt *waitForGatewayReceipt(banking::PaymentGatewayManager &gateway,
                                                            const string &referenceId,
                                                            int timeoutSeconds = 180) {
    const int pollMs = 250;
    int waited = 0;
    while (waited <= timeoutSeconds * 1000) {
        gateway.load();
        const banking::PaymentReceipt *r = gateway.findLatestByReference(referenceId);
        if (r) return r;
        this_thread::sleep_for(chrono::milliseconds(pollMs));
        waited += pollMs;
    }
    return nullptr;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    srand(static_cast<unsigned>(time(nullptr)));

    storage::XmlRepository repo("storage/data");

    // ------------------------------------------------------------------
    // Normal mode.
    // ------------------------------------------------------------------

    airport::AirportService  airportService(repo);
    users::UserManager       userManager(repo);
    banking::BankManager     bankManager(repo);
    banking::PaymentGatewayManager paymentGateway(repo);
    flights::FlightManager   flightManager(repo, airportService);
    tickets::TicketManager   ticketManager(repo);
    baggage::ItemDatabase    itemDatabase(repo);
    baggage::BaggageManager  baggageManager(repo, airportService,
                                             itemDatabase, userManager);

    airportService.load();
    userManager.load();
    bankManager.load();
    paymentGateway.load();
    flightManager.load();
    ticketManager.load();
    itemDatabase.load();
    baggageManager.load();

    users::UserCLI   userCLI(userManager);
    flights::FlightsCLI flightsCLI(flightManager, ticketManager,
                                    userManager, paymentGateway, airportService);
    tickets::TicketsCLI ticketsCLI(ticketManager, flightManager,
                                    userManager, airportService);
    ticketsCLI.setBaggageManager(&baggageManager);

    baggage::BaggageCLI baggageCLI(baggageManager, itemDatabase,
                                    ticketManager, bankManager,
                                    airportService, userManager,
                                    userCLI, flightManager);

    string bankCliPath = siblingExecutablePath("bankcli");
    string baggageCliPath = siblingExecutablePath("baggagecli");

    // At payment time: launch bank gateway terminal
    flightsCLI.setOnBankGatewayPayment(
        [&](const string &uid, const string &methodId, long long amountCents,
            const string &purpose, const string &referenceId) {
            cout << "\nOpening bank gateway in a separate terminal...\n";
            string cmd = "\"" + bankCliPath + "\" --gateway "
                            + "\"" + uid + "\" "
                            + "\"" + methodId + "\" "
                            + to_string(amountCents) + " "
                            + "\"" + purpose + "\" "
                            + "\"" + referenceId + "\"";
            if (!core::launchInTerminal(cmd, true)) {
                paymentGateway.load();
                paymentGateway.addReceipt(uid, methodId, purpose, referenceId,
                                          amountCents, "FAILED", "Unable to launch bank gateway terminal");
            }
        });

    flightsCLI.setOnValidateBankCredentials(
        [&](const string &bankName,
            const string &bankUsername,
            const string &bankPassword,
            string &outError) {
            bankManager.load();
            banking::BankUserAccount account;
            if (!bankManager.authenticateBankUser(bankUsername, bankPassword, account)) {
                outError = "invalid bank username or password";
                return false;
            }
            if (account.bankName != bankName) {
                outError = "bank name does not match the provided account";
                return false;
            }
            return true;
        });

    flightsCLI.setOnListBankNames([&]() {
        bankManager.load();
        vector<string> names;
        for (const auto &b : bankManager.all()) names.push_back(b.getName());
        return names;
    });

    // After booking: launch baggage packing terminal, wait for it to close.
    flightsCLI.setOnBookedCallback(
        [&](const string &uid, const string &ticketId) {
            cout << "\nPress Enter to continue to baggage packing...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();

            string cmd = "\"" + baggageCliPath + "\" "
                            + "\"" + uid + "\" "
                            + "\"" + ticketId + "\"";
            core::launchInTerminal(cmd, true);

            userManager.load();
            cout << "\nBaggage packing complete. Returning to dashboard.\n";
        });

    // ------------------------------------------------------------------
    // Authentication.
    // ------------------------------------------------------------------
    string userId;
    Role        userRole = Role::Passenger;
    if (!userCLI.welcomeFlow(userId, userRole)) return 0;

    // ------------------------------------------------------------------
    // Dashboard.
    // ------------------------------------------------------------------
    bool isRunning = true;
    while (isRunning) {
        cout << "\n==============================\n";
        cout << " airCLI Dashboard\n";
        cout << "==============================\n";

        if (userRole == Role::Admin) {
            cout << "1. Manage Flights\n";
            cout << "2. View Airport Budget\n";
            cout << "3. Manage Item Database\n";
            cout << "4. View All Passengers\n";
            cout << "5. Manage User Roles\n";
            cout << "6. Logout\n";
        } else if (userRole == Role::Security ||
                   userRole == Role::SeniorSecurity) {
            cout << "1. Baggage Handler View\n";
            cout << "2. Logout\n";
        } else {
            userManager.load();
            const users::User *me = userManager.get(userId);
            cout << "1. Search & Book Flights\n";
            cout << "2. My Tickets\n";
            cout << "3. Travel History\n";
            cout << "4. Pay Pending Fine\n";
            cout << "5. Board Plane\n";
            cout << "6. Logout\n";
            if (me && me->getPendingFineCents() > 0) {
                cout << "\nPending Fine : ";
                printBudget(me->getPendingFineCents());
                cout << "\n";
            }
        }

        cout << "\nSelect > ";
        int choice = 0;
        cin >> choice;

        if (userRole == Role::Admin) {
            switch (choice) {
            case 1:
                flightsCLI.run(userId, userRole);
                break;
            case 2:
                airportService.load();
                cout << "\nAirport Budget: ";
                printBudget(airportService.getBudget());
                cout << "\n\nRecent Events:\n";
                for (const auto &line : repo.readAll("budget_log"))
                    cout << "  " << line << "\n";
                break;
            case 3: {
                itemDatabase.load();
                bool itemRunning = true;
                while (itemRunning) {
                    cout << "\n--- Item Database ---\n";
                    cout << "1. List all items\n2. Add item\n3. Remove item\n4. Back\nSelect > ";
                    int ic = 0; cin >> ic;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (ic == 1) {
                        for (const auto &it : itemDatabase.all())
                            cout << it.getId() << " | " << it.getName()
                                      << " | " << it.getCategory()
                                      << " | " << it.getMinWeightGrams()
                                      << "-" << it.getMaxWeightGrams() << "g"
                                      << (it.getIsBanned() ? " [BANNED]" : "") << "\n";
                    } else if (ic == 2) {
                        cout << "Name     : "; string name; getline(cin, name);
                        cout << "Category : "; string cat;  getline(cin, cat);
                        cout << "Min weight (g): "; int minW = 0; cin >> minW;
                        cout << "Max weight (g): "; int maxW = 0; cin >> maxW;
                        cout << "Banned? (1/0): "; int banned = 0; cin >> banned;
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        baggage::Item ni("I" + to_string(time(nullptr)),
                                         name, cat, minW, maxW, banned == 1);
                        itemDatabase.addItem(ni);
                        cout << "Item added.\n";
                    } else if (ic == 3) {
                        cout << "Item ID: "; string rid; getline(cin, rid);
                        cout << (itemDatabase.removeItem(rid) ? "Removed.\n" : "Not found.\n");
                    } else { itemRunning = false; }
                }
                break;
            }
            case 4: {
                userManager.load(); ticketManager.load();
                for (const auto &u : userManager.all()) {
                    if (u.getRole() != Role::Passenger) continue;
                    cout << u.getId() << " | " << u.getName()
                              << " | Pending Fine: "; printBudget(u.getPendingFineCents());
                    cout << " | " << u.getCountry()
                              << " | " << (u.getLastBankName().empty() ? "No bank" : u.getLastBankName()) << "\n";
                    for (const auto &tk : ticketManager.getForPassenger(u.getId()))
                        cout << "  " << tk.getId() << " | "
                                  << tk.getOrigin() << "->" << tk.getDestination()
                                  << " | " << tk.getFlightDate()
                                  << (tk.isCancelled() ? " [CANCELLED]" : "") << "\n";
                }
                break;
            }
            case 5: {
                userManager.load();
                auto users = userManager.all();
                for (size_t i = 0; i < users.size(); ++i) {
                    string rs;
                    switch (users[i].getRole()) {
                        case Role::Passenger:      rs = "Passenger";      break;
                        case Role::Security:       rs = "Security";       break;
                        case Role::SeniorSecurity: rs = "SeniorSecurity"; break;
                        case Role::Admin:          rs = "Admin";          break;
                        case Role::Banned:         rs = "Banned";         break;
                    }
                    cout << "[" << (i+1) << "] " << users[i].getName()
                              << " | " << users[i].getUsername() << " | " << rs << "\n";
                }
                cout << "Select user (0 cancel): ";
                int uc = 0; cin >> uc;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (uc < 1 || uc > static_cast<int>(users.size())) break;
                cout << "1.Passenger 2.Security 3.SeniorSecurity 4.Admin 5.Banned\nSelect > ";
                int rc = 0; cin >> rc;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                Role nr = Role::Passenger;
                switch (rc) {
                    case 2: nr = Role::Security;       break;
                    case 3: nr = Role::SeniorSecurity; break;
                    case 4: nr = Role::Admin;          break;
                    case 5: nr = Role::Banned;         break;
                    default: nr = Role::Passenger;
                }
                userManager.updateRole(users[uc-1].getId(), nr);
                cout << "Role updated.\n";
                break;
            }
            case 6: cout << "\nLogging out...\n"; isRunning = false; break;
            default: cout << "\nInvalid option.\n";
            }

        } else if (userRole == Role::Security || userRole == Role::SeniorSecurity) {
            switch (choice) {
            case 1: baggageCLI.runHandlerView(userId); break;
            case 2: cout << "\nLogging out...\n"; isRunning = false; break;
            default: cout << "\nInvalid option.\n";
            }

        } else {
            switch (choice) {
            case 1: flightsCLI.runSearchAndBook(userId, userRole); break;
            case 2: ticketsCLI.runViewTickets(userId, userRole);   break;
            case 3: ticketsCLI.runTravelHistory(userId, userRole); break;
            case 4: {
                userManager.load();
                const users::User *u = userManager.get(userId);
                if (!u || u->getPendingFineCents() <= 0) {
                    cout << "\nNo pending fine to pay.\n";
                    break;
                }

                cout << "\nPending Fine: ";
                printBudget(u->getPendingFineCents());
                while (true) {
                    paymentGateway.load();
                    auto methods = paymentGateway.getMethodsForPassenger(userId);

                    cout << "\nChoose payment method:\n";
                    for (size_t i = 0; i < methods.size(); ++i)
                        cout << "[" << (i + 1) << "] " << methods[i].nickname
                                  << " | " << methods[i].bankName << "\n";
                    int addNewChoice = static_cast<int>(methods.size()) + 1;
                    cout << "[" << addNewChoice << "] Add new payment method\n";
                    cout << "0. Cancel\nSelect > ";

                    int mc = 0;
                    cin >> mc;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    if (mc == 0) break;

                    bool temporaryMethod = false;
                    bool keepAfterSuccess = true;
                    string selectedMethodId;

                    if (mc >= 1 && mc <= static_cast<int>(methods.size())) {
                        selectedMethodId = methods[mc - 1].id;
                    } else if (mc == addNewChoice) {
                        bankManager.load();
                        const auto banks = bankManager.all();
                        if (banks.empty()) {
                            cout << "\nNo banks are available right now.\n";
                            continue;
                        }

                        cout << "Nickname          : ";
                        string nickname; getline(cin, nickname);

                        cout << "Select Bank:\n";
                        for (size_t i = 0; i < banks.size(); ++i)
                            cout << "[" << (i + 1) << "] " << banks[i].getName() << "\n";
                        cout << "0. Cancel\nSelect > ";
                        int bankChoice = 0;
                        cin >> bankChoice;
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        if (bankChoice < 1 || bankChoice > static_cast<int>(banks.size())) {
                            cout << "\nCancelled adding new method.\n";
                            continue;
                        }
                        string bankName = banks[bankChoice - 1].getName();

                        cout << "Bank Username     : ";
                        string bankUser; getline(cin, bankUser);
                        cout << "Bank Password     : ";
                        string bankPass = readMaskedLine();

                        if (nickname.empty() || bankName.empty() || bankUser.empty() || bankPass.empty()) {
                            cout << "\nAll fields are required.\n";
                            continue;
                        }

                        banking::BankUserAccount account;
                        if (!bankManager.authenticateBankUser(bankUser, bankPass, account)) {
                            cout << "\nInvalid bank username or password.\n";
                            continue;
                        }
                        if (account.bankName != bankName) {
                            cout << "\nBank name does not match this account.\n";
                            continue;
                        }

                        cout << "Save this method for future use? (y/n): ";
                        char keep = 'y';
                        cin >> keep;
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        keepAfterSuccess = (keep == 'y' || keep == 'Y');

                        selectedMethodId = paymentGateway.addMethod(userId, nickname, bankName, bankUser, bankPass);
                        if (selectedMethodId.empty()) {
                            cout << "\nUnable to create payment method.\n";
                            continue;
                        }
                        temporaryMethod = true;
                    } else {
                        cout << "\nInvalid option.\n";
                        continue;
                    }

                    string referenceId = "FINE-" + to_string(time(nullptr)) + "-" + userId;
                    string cmd = "\"" + bankCliPath + "\" --gateway "
                                    + "\"" + userId + "\" "
                                    + "\"" + selectedMethodId + "\" "
                                    + to_string(u->getPendingFineCents()) + " "
                                    + "\"PENDING_FINE\" "
                                    + "\"" + referenceId + "\"";
                    if (!core::launchInTerminal(cmd, true)) {
                        paymentGateway.load();
                        paymentGateway.addReceipt(userId, selectedMethodId, "PENDING_FINE", referenceId,
                                                  u->getPendingFineCents(), "FAILED",
                                                  "Unable to launch bank gateway terminal");
                    }

                    cout << "\nWaiting for gateway confirmation...\n";
                    const banking::PaymentReceipt *r = waitForGatewayReceipt(paymentGateway, referenceId);
                    if (!r || r->status != "SUCCESS") {
                        if (temporaryMethod) paymentGateway.removeMethod(userId, selectedMethodId);
                        cout << "\nFine payment was not completed";
                        if (r && !r->message.empty()) cout << ": " << r->message;
                        else if (!r) cout << ": gateway response timed out";
                        cout << ".\n";
                        break;
                    }

                    if (temporaryMethod && !keepAfterSuccess)
                        paymentGateway.removeMethod(userId, selectedMethodId);

                    userManager.clearPendingFine(userId);
                    cout << "\nPending fine paid successfully. Receipt: " << r->id << "\n";
                    break;
                }
                break;
            }
            case 5: {
                ticketManager.load();
                flightManager.load();
                auto mine = ticketManager.getForPassenger(userId);
                vector<tickets::Ticket> boardable;
                for (const auto &tk : mine) {
                    // Skip if: cancelled, not cleared, or already boarded
                    if (tk.isCancelled() || !tk.isSecurityCleared() || tk.isBoarded()) continue;
                    const flights::Flight *f = flightManager.get(tk.getFlightId());
                    if (!f) continue;
                    // Skip if flight has already departed or landed
                    if (f->getStatus() == "CANCELLED" || f->getStatus() == "FLYING" || f->getStatus() == "LANDED") continue;
                    boardable.push_back(tk);
                }

                if (boardable.empty()) {
                    cout << "\nNo eligible cleared tickets available for boarding.\n";
                    break;
                }

                // Sort by flight date for logical ordering
                sort(boardable.begin(), boardable.end(), [&](const tickets::Ticket &a, const tickets::Ticket &b) {
                    string dateA = a.getFlightDate();
                    string dateB = b.getFlightDate();
                    return dateA < dateB;
                });

                cout << "\n========== Board Plane ==========\n";
                cout << "Showing only security-cleared tickets for available flights:\n\n";
                for (size_t i = 0; i < boardable.size(); ++i) {
                    const flights::Flight *f = flightManager.get(boardable[i].getFlightId());
                    cout << "[" << (i + 1) << "] " << boardable[i].getId()
                              << " | " << boardable[i].getOrigin() << " -> " << boardable[i].getDestination()
                              << " | " << boardable[i].getFlightDate()
                              << " | " << (f ? f->getStatus() : "UNKNOWN") << "\n";
                }
                cout << "0. Cancel\nSelect ticket to board > ";

                int bc = 0;
                cin >> bc;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (bc < 1 || bc > static_cast<int>(boardable.size())) break;

                if (ticketManager.setBoarded(boardable[bc - 1].getId(), true)) {
                    cout << "\nBoarding confirmed for ticket " << boardable[bc - 1].getId() << ".\n";
                } else {
                    cout << "\nFailed to update boarding status.\n";
                }
                break;
            }
            case 6: cout << "\nLogging out...\n"; isRunning = false; break;
            default: cout << "\nInvalid option.\n";
            }
        }
    }

    cout << "\nThanks for using airCLI. Goodbye!\n";
    return 0;
}
