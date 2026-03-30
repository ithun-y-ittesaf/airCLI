#include "BaggageCLI.h"
#include "SuspicionScorer.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <cctype>
#include <ctime>
using namespace std;


namespace baggage {

static const long long HANDLER_FIXED_FINE_CENTS = 500000; // 5,000 BDT

static int bankScoreAdjustmentForUser(const users::User *u,
                                      const banking::BankManager &bankManager) {
    if (!u || u->getLastBankName().empty()) return 20;
    for (const auto &b : bankManager.all())
        if (b.getName() == u->getLastBankName()) return b.scoreAdjustment();
    return 20;
}

static bool categoryNeedsDeclaration(const string &category) {
    string c;
    c.reserve(category.size());
    for (char ch : category) c.push_back(static_cast<char>(tolower(ch)));
    return c == "batteries" || c == "electronics" || c == "liquids" || c == "medicines";
}

static int chooseQuantity(const string &label) {
    cout << "Declared quantity for " << label << " (0-50): ";
    int qty = 0;
    cin >> qty;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (qty < 0) qty = 0;
    if (qty > 50) qty = 50;
    return qty;
}

// ---------------------------------------------------------------------------
// Authenticated entry point — user logs in and selects ticket
// ---------------------------------------------------------------------------

void BaggageCLI::runAuthenticatedFlow() {
    string userId;
    Role userRole = Role::Passenger;

    if (!userCLI.welcomeFlow(userId, userRole)) {
        cout << "\nAuthentication cancelled.\n";
        return;
    }

    if (userRole != Role::Passenger) {
        cout << "\nOnly passengers can pack baggage. Exiting.\n";
        return;
    }

    userManager.load();
    const users::User *user = userManager.get(userId);
    if (!user) {
        cout << "\nUser not found.\n";
        return;
    }

    // Show available tickets for this passenger (not cancelled, not already packed)
    ticketManager.load();
    auto myTickets = ticketManager.getForPassenger(userId);

    vector<tickets::Ticket> packableTickets;
    for (const auto &tk : myTickets) {
        if (tk.isCancelled()) continue;
        // Check if baggage already packed for this ticket
        const Bag *existingBag = baggageManager.getBagByTicket(tk.getId());
        if (existingBag) continue;
        packableTickets.push_back(tk);
    }

    if (packableTickets.empty()) {
        cout << "\nYou have no unpacked tickets available.\n";
        return;
    }

    cout << "\n========== Select Ticket to Pack ==========\n";
    cout << "Available tickets:\n\n";
    for (size_t i = 0; i < packableTickets.size(); ++i) {
        const auto &tk = packableTickets[i];
        cout << "[" << (i + 1) << "] "
             << "Ticket: " << tk.getId()
             << " | " << tk.getOrigin() << " -> " << tk.getDestination()
             << " | Date: " << tk.getFlightDate()
             << " | Issued: " << tk.getIssuedDate() << "\n";
    }

    cout << "0. Cancel\nSelect > ";
    int choice = 0;
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice < 1 || choice > static_cast<int>(packableTickets.size())) {
        cout << "\nCancelled.\n";
        return;
    }

    const tickets::Ticket &selectedTicket = packableTickets[choice - 1];
    runPackBag(userId, selectedTicket.getId());
}

// ---------------------------------------------------------------------------
// Passenger mode — declaration-form packing
// ---------------------------------------------------------------------------

void BaggageCLI::runDeclarationForm(vector<PackedItem> &selectedItems) {
    const auto items = itemDb.all();
    for (const auto &item : items) {
        if (!categoryNeedsDeclaration(item.getCategory())) continue;
        int qty = chooseQuantity(item.getCategory() + " - " + item.getName());
        for (int i = 0; i < qty; ++i) {
            PackedItem pi;
            pi.itemId = item.getId();
            pi.itemName = item.getName();
            pi.category = item.getCategory();
            pi.assignedWeightGrams = 0;
            pi.isPublic = true;
            pi.declared = true;
            selectedItems.push_back(pi);
        }
    }

    cout << "\nAny additional undeclared private items? (y/n): ";
    char addPrivate = 'n';
    cin >> addPrivate;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (addPrivate != 'y' && addPrivate != 'Y') return;

    cout << "How many private items to add (0-10): ";
    int count = 0;
    cin >> count;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (count < 0) count = 0;
    if (count > 10) count = 10;

    for (int i = 0; i < count; ++i) {
        cout << "Private item " << (i + 1) << " name: ";
        string name;
        getline(cin, name);
        cout << "Category: ";
        string category;
        getline(cin, category);

        PackedItem pi;
        pi.itemId = "CUSTOM-" + to_string(time(nullptr)) + "-" + to_string(i);
        pi.itemName = name.empty() ? "PrivateItem" : name;
        pi.category = category.empty() ? "Misc" : category;
        pi.assignedWeightGrams = 0;
        pi.isPublic = false;
        pi.declared = false;
        selectedItems.push_back(pi);
    }
}

void BaggageCLI::runPackBag(const string &passengerId,
                              const string &ticketId) {
    itemDb.load();

    cout << "\n========== Pack Your Bag ==========\n";
    cout << "Complete the declaration form. Actual weights are assigned automatically.\n";

    vector<PackedItem> selectedItems;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Bag Name Tag: ";
    string bagTag;
    getline(cin, bagTag);
    if (bagTag.empty()) {
        cout << "\nBag name tag is required.\n";
        return;
    }

    runDeclarationForm(selectedItems);

    if (selectedItems.empty()) {
        cout << "\nNo items added. Bag not created.\n"; return;
    }

    // Show estimated public weight to passenger.
    int publicWeight = 0;
    for (const auto &pi : selectedItems) {
        if (pi.isPublic) {
            const Item *def = itemDb.findById(pi.itemId);
            if (def) publicWeight += def->midWeightGrams();
        }
    }

    cout << "\nItems selected    : " << selectedItems.size() << "\n";
    cout << "Est. public weight: " << publicWeight << "g\n";
    cout << "Confirm packing? (y/n): ";
    char confirm; cin >> confirm;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (confirm != 'y' && confirm != 'Y') { cout << "Packing cancelled.\n"; return; }

    string bagId = baggageManager.packBag(passengerId, ticketId, bagTag, selectedItems);
    cout << "\nBag packed. ID: " << bagId << "\n";
}

// ---------------------------------------------------------------------------
// Handler mode — conveyor view with suspicion scoring
// ---------------------------------------------------------------------------

void BaggageCLI::runInspectTicket(const string &ticketId) {
    const tickets::Ticket *tk = ticketManager.get(ticketId);
    if (!tk) {
        cout << "Ticket not found.\n";
        return;
    }

    const Bag *selectedPtr = baggageManager.getBagByTicket(ticketId);
    if (!selectedPtr) {
        cout << "No bag was packed for this ticket.\n";
        return;
    }

    const Bag &selected = *selectedPtr;
    const users::User *user = userManager.get(selected.getPassengerId());

    cout << "\n--- Inspecting Bag: " << selected.getId() << " ---\n";
    cout << "Passenger : " << (user ? user->getName() : selected.getPassengerId()) << "\n\n";

    // Show ALL items including private.
    cout << "Contents:\n";
    for (const auto &pi : selected.getItems()) {
        cout << "  " << pi.itemName
               << " [" << pi.category << "]"
             << " | " << pi.assignedWeightGrams << "g"
             << " | " << (pi.isPublic ? "public" : "PRIVATE");
        if (pi.category == "Batteries" || pi.category == "Electronics" ||
            pi.category == "Liquids" || pi.category == "Medicines") {
            cout << " | " << (pi.declared ? "DECLARED" : "UNDECLARED");
        }
        cout << "\n";
    }

    int totalWeight = selected.totalWeightGrams();
    cout << "\nTotal weight: " << totalWeight << "g";
    if (totalWeight > 30000) {
        int over = totalWeight - 30000;
        cout << " (OVERWEIGHT by " << over << "g)";
    }
    cout << "\n";

    // Run full inspection (overweight + illegal items) and show findings,
    // then let handler choose an explicit action.
    InspectionResult result = baggageManager.inspectBag(selected.getId());

    if (result.overweight) {
        cout << "\n[OVERWEIGHT] " << result.overweightGrams << "g over limit.\n";
    }

    if (result.foundIllegal) {
        cout << "\n[ILLEGAL ITEM] " << result.illegalItemName << " found!\n";
    }

    if (result.foundUndeclared) {
        cout << "\n[UNDECLARED ITEM] Found " << result.undeclaredCount
             << " declaration-required item(s) without declaration.\n";
        cout << "Example item: " << result.undeclaredItemName << "\n";
    }

    cout << "\nSelect security action:\n";
    cout << "1. Clear passenger (allow boarding)\n";
    cout << "2. Issue fixed fine (5,000 BDT)\n";
    cout << "3. Permanent ban\n";
    cout << "0. Back\nSelect > ";

    int action = 0;
    cin >> action;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (action == 0) return;

    if (action == 1) {
        ticketManager.setSecurityCleared(ticketId, true);
        cout << "\nPassenger cleared. Ticket " << ticketId << " is now eligible for boarding.\n";
        return;
    }

    if (action == 2) {
        userManager.addPendingFine(selected.getPassengerId(), HANDLER_FIXED_FINE_CENTS);
        ticketManager.setSecurityCleared(ticketId, false);
        cout << "\nFixed fine issued: 5,000 BDT. Passenger remains uncleared until resolved.\n";
        return;
    }

    if (action == 3) {
        userManager.banUser(selected.getPassengerId());
        ticketManager.setSecurityCleared(ticketId, false);
        cout << "\nPassenger permanently banned. Login is disabled for this user.\n";
        return;
    }

    cout << "\nInvalid option.\n";
}

// ---------------------------------------------------------------------------
// Handler dashboard
// ---------------------------------------------------------------------------

void BaggageCLI::runHandlerView(const string &handlerId) {
    (void)handlerId;
    baggageManager.load();
    itemDb.load();
    ticketManager.load();
    userManager.load();

    cout << "\n========== Baggage Handler View ==========\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    bool running = true;
    while (running) {
        cout << "\n--- Security Inspection ---\n";
        cout << "1. Show passengers/tickets and inspect\n";
        cout << "2. Exit\n";
        cout << "Select > ";

        int choice = 0;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
        case 1: {
            ticketManager.load();
            userManager.load();

            struct HandlerOption {
                tickets::Ticket ticket;
                int suspicionScore;
                string suspicionLabel;
                bool hasBag;
                string bagId;
            };

            vector<HandlerOption> options;
            for (const auto &u : userManager.all()) {
                if (u.getRole() != Role::Passenger) continue;
                auto ts = ticketManager.getForPassenger(u.getId());
                for (const auto &t : ts) {
                    if (t.isCancelled()) continue;

                    const Bag *bag = baggageManager.getBagByTicket(t.getId());
                    int score = 0;
                    string label = "LOW";
                    bool hasBag = (bag != nullptr);
                    string bagId = hasBag ? bag->getId() : "NOT_PACKED";

                    if (hasBag) {
                        int totalWeight = bag->totalWeightGrams();
                        int expectedWeight = 0;
                        bool hasPrivate = false;

                        for (const auto &pi : bag->getItems()) {
                            if (pi.isPublic) {
                                const Item *def = itemDb.findById(pi.itemId);
                                if (def) expectedWeight += def->midWeightGrams();
                            } else {
                                hasPrivate = true;
                            }
                        }

                        int bankAdj = bankScoreAdjustmentForUser(&u, bankManager);
                        score = SuspicionScorer::calculate(totalWeight,
                                                           expectedWeight,
                                                           u.getCountry(),
                                                           bankAdj,
                                                           hasPrivate);
                        label = SuspicionScorer::scoreLabel(score);
                    }

                    options.push_back({t, score, label, hasBag, bagId});
                }
            }

            if (options.empty()) {
                cout << "\nNo active passenger tickets available.\n";
                break;
            }

            sort(options.begin(), options.end(),
                      [](const HandlerOption &a, const HandlerOption &b) {
                          if (a.ticket.getIssuedDate() == b.ticket.getIssuedDate())
                              return a.ticket.getId() < b.ticket.getId();
                          return a.ticket.getIssuedDate() < b.ticket.getIssuedDate();
                      });

            int mostSusIdx = -1;
            int maxScore = -1;
            for (size_t i = 0; i < options.size(); ++i) {
                if (!options[i].hasBag) continue;
                if (options[i].suspicionScore > maxScore) {
                    maxScore = options[i].suspicionScore;
                    mostSusIdx = static_cast<int>(i);
                }
            }

            cout << "\n--- Passengers Sorted by Booking Time and Ticket ---\n";
            if (mostSusIdx >= 0) {
                const auto &ms = options[mostSusIdx];
                cout << "Most suspicious right now: Ticket " << ms.ticket.getId()
                     << " | Score " << ms.suspicionScore
                     << " [" << ms.suspicionLabel << "]\n\n";
            }

            for (size_t i = 0; i < options.size(); ++i) {
                const users::User *pu = userManager.get(options[i].ticket.getPassengerId());
                cout << "[" << (i + 1) << "] "
                     << (pu ? pu->getName() : options[i].ticket.getPassengerId())
                     << " | Ticket: " << options[i].ticket.getId()
                     << " | Issued: " << options[i].ticket.getIssuedDate()
                     << " | Route: " << options[i].ticket.getOrigin() << "->" << options[i].ticket.getDestination()
                     << " | Bag: " << options[i].bagId
                     << " | Sus: " << (options[i].hasBag ? to_string(options[i].suspicionScore) + " [" + options[i].suspicionLabel + "]" : "N/A")
                     << "\n";
            }

            cout << "0. Back\nSelect ticket to inspect > ";
            int pick = 0;
            cin >> pick;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (pick < 1 || pick > static_cast<int>(options.size())) break;

            runInspectTicket(options[pick - 1].ticket.getId());
            break;
        }
        case 2:
            running = false;
            break;
        default:
            cout << "\nInvalid option.\n";
        }
    }
}

} // namespace baggage
