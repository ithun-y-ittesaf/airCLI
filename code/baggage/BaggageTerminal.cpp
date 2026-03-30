#include "BaggageTerminal.h"

#include "BaggageManager.h"
#include "ItemDatabase.h"
#include "BaggageCLI.h"
#include "../airport/AirportService.h"
#include "../users/UserManager.h"
#include "../users/UserCLI.h"
#include "../tickets/TicketManager.h"
#include "../banking/BankManager.h"
#include "../flights/FlightManager.h"

#include <iostream>
#include <limits>
#include <vector>
#include <string>
#include <cctype>
using namespace std;


namespace baggage {

static bool requiresDeclaration(const string &category) {
    string c;
    c.reserve(category.size());
    for (char ch : category) c.push_back(static_cast<char>(tolower(ch)));
    return c == "batteries" || c == "electronics" || c == "liquids" || c == "medicines";
}

// ---------------------------------------------------------------------------
// Display the current bag contents at the top of every screen.
// ---------------------------------------------------------------------------
static void showBagState(const vector<PackedItem> &items) {
    cout << "\n========================================\n";
    cout << " CURRENT BAG\n";
    cout << "========================================\n";

    vector<const PackedItem *> publicItems, privateItems;
    for (const auto &pi : items) {
        if (pi.isPublic) publicItems.push_back(&pi);
        else             privateItems.push_back(&pi);
    }

    cout << "PUBLIC ITEMS:\n";
    if (publicItems.empty()) cout << "  (none)\n";
    else for (const auto *pi : publicItems)
        cout << "  - " << pi->itemName << " [" << pi->category << "]"
             << (requiresDeclaration(pi->category)
                 ? (pi->declared ? " [DECLARED]" : " [UNDECLARED]")
                 : "")
             << "\n";

    cout << "PRIVATE ITEMS:\n";
    if (privateItems.empty()) cout << "  (none)\n";
    else for (const auto *pi : privateItems)
        cout << "  - " << pi->itemName << " [" << pi->category << "]"
             << (requiresDeclaration(pi->category)
                 ? (pi->declared ? " [DECLARED]" : " [UNDECLARED]")
                 : "")
             << "\n";

    cout << "----------------------------------------\n";
}

// ---------------------------------------------------------------------------
// Add item sub-flow.
// ---------------------------------------------------------------------------
static void addItem(vector<PackedItem> &items, ItemDatabase &itemDb) {
    static const vector<string> categories = {
        "Clothing", "Accessories", "Batteries", "Electronics",
        "Liquids", "Medicines", "Documents", "Food"
    };

    cout << "\nSelect declaration category:\n";
    for (size_t i = 0; i < categories.size(); ++i)
        cout << "[" << (i + 1) << "] " << categories[i] << "\n";
    cout << "0. Cancel\nSelect > ";
    int catChoice = 0;
    cin >> catChoice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if (catChoice < 1 || catChoice > static_cast<int>(categories.size())) return;

    const string selectedCategory = categories[catChoice - 1];

    cout << "Search item keyword in " << selectedCategory << ": ";
    string keyword;
    getline(cin, keyword);

    auto results = itemDb.findByName(keyword);
    vector<Item> filtered;
    for (const auto &it : results) {
        if (it.getCategory() == selectedCategory) filtered.push_back(it);
    }
    if (!filtered.empty()) results = filtered;

    if (results.empty()) {
        cout << "No items found.\n";
        return;
    }

    cout << "\n";
    for (size_t i = 0; i < results.size(); ++i) {
        const auto &item = results[i];
        // Show weight RANGE only — never actual weight, never banned flag.
        cout << "[" << (i + 1) << "] "
             << item.getName()
             << " | " << item.getCategory()
             << " | " << item.getMinWeightGrams()
             << "g-"  << item.getMaxWeightGrams() << "g\n";
    }

    cout << "Select item (0 to cancel): ";
    int choice = 0;
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice < 1 || choice > static_cast<int>(results.size())) return;

    const auto &chosen = results[choice - 1];

    cout << "Compartment (pub/priv): ";
    string comp;
    getline(cin, comp);
    bool isPublic = (comp != "priv");

    PackedItem pi;
    pi.itemId              = chosen.getId();
    pi.itemName            = chosen.getName();
    pi.category            = chosen.getCategory().empty() ? selectedCategory : chosen.getCategory();
    pi.assignedWeightGrams = 0; // Randomized in BaggageManager::packBag().
    pi.isPublic            = isPublic;
    if (requiresDeclaration(pi.category)) {
        cout << "Declare this item now? (y/n): ";
        char declaredChoice = 'y';
        cin >> declaredChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        pi.declared = (declaredChoice == 'y' || declaredChoice == 'Y');
    } else {
        pi.declared = true;
    }

    items.push_back(pi);
    cout << "\nAdded: " << chosen.getName()
         << " (" << (isPublic ? "public" : "private") << ")"
         << (requiresDeclaration(pi.category)
             ? (pi.declared ? " [DECLARED]" : " [UNDECLARED]")
             : "")
         << "\n";
}

// ---------------------------------------------------------------------------
// Remove item sub-flow.
// ---------------------------------------------------------------------------
static void removeItem(vector<PackedItem> &items) {
    if (items.empty()) {
        cout << "\nBag is empty.\n";
        return;
    }

    cout << "\n--- Remove Item ---\n";
    for (size_t i = 0; i < items.size(); ++i) {
        cout << "[" << (i + 1) << "] "
             << items[i].itemName
               << " | " << items[i].category
               << " | " << (items[i].isPublic ? "public" : "private") << "\n";
    }

    cout << "Select item to remove (0 to cancel): ";
    int choice = 0;
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice < 1 || choice > static_cast<int>(items.size())) return;

    string removed = items[choice - 1].itemName;
    items.erase(items.begin() + (choice - 1));
    cout << "\nRemoved: " << removed << "\n";
}

// ---------------------------------------------------------------------------
// Main terminal entry point.
// ---------------------------------------------------------------------------
int runBaggageTerminal(const string    &userId,
                       const string    &ticketId,
                       storage::IRepository &repo) {
    // Load required managers.
    // AirportService is needed by BaggageManager constructor but packBag()
    // itself does not call any airport events — those are triggered at
    // booking time in the main process. We load it here to avoid a
    // null reference; it reads/writes the same shared XML files.
    airport::AirportService airportService(repo);
    users::UserManager      userManager(repo);
    ItemDatabase            itemDb(repo);
    BaggageManager          bm(repo, airportService, itemDb, userManager);

    userManager.load();
    itemDb.load();
    bm.load();

    const users::User *user = userManager.get(userId);
    string userName = user ? user->getName() : userId;

    cout << "\n========================================\n";
    cout << " airCLI — Baggage Packing\n";
    cout << " Passenger : " << userName << "\n";
    cout << " Ticket    : " << ticketId << "\n";
    cout << "========================================\n";
    cout << "Add items to your bag.\n";
    cout << "Set a bag name tag. Only tagged bags linked to your ticket are released at airport.\n";
    cout << "Weight is assigned automatically when you finish.\n";

    string bagTag;
    while (bagTag.empty()) {
        cout << "\nBag Name Tag: ";
        getline(cin, bagTag);
        if (bagTag.empty()) cout << "Bag name tag is required.\n";
    }

    vector<PackedItem> items;
    bool running = true;

    while (running) {
        showBagState(items);

        cout << "1. Add Item\n";
        cout << "2. Remove Item\n";
        cout << "3. Finish\n";
        cout << "Select > ";

        int choice = 0;
        cin >> choice;

        switch (choice) {
        case 1:
            addItem(items, itemDb);
            break;

        case 2:
            removeItem(items);
            break;

        case 3: {
            if (items.empty()) {
                cout << "\nBag is empty. Add at least one item before finishing.\n";
                break;
            }

            cout << "\nConfirm packing " << items.size() << " item(s)? (y/n): ";
            char confirm;
            cin >> confirm;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (confirm != 'y' && confirm != 'Y') break;

            // Weight randomized inside packBag().
            string bagId = bm.packBag(userId, ticketId, bagTag, items);

            cout << "\n========================================\n";
            cout << " Bag packed successfully!\n";
            cout << " Bag ID : " << bagId << "\n";
            cout << " Tag    : " << bagTag << "\n";
            cout << "========================================\n";
            cout << "This terminal will close.\n";

            running = false;
            break;
        }

        default:
            cout << "\nInvalid option. Please enter 1, 2, or 3.\n";
        }
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Authenticated terminal entry point.
// ---------------------------------------------------------------------------
int runBaggageTerminalAuthenticated(storage::IRepository &repo) {
    // Load all required managers.
    airport::AirportService airportService(repo);
    users::UserManager      userManager(repo);
    users::UserCLI          userCLI(userManager);
    tickets::TicketManager  ticketManager(repo);
    banking::BankManager    bankManager(repo);
    ItemDatabase            itemDb(repo);
    BaggageManager          baggageManager(repo, airportService, itemDb, userManager);
    flights::FlightManager  flightManager(repo, airportService);

    BaggageCLI baggageCLI(baggageManager, itemDb, ticketManager,
                          bankManager, airportService, userManager,
                          userCLI, flightManager);

    cout << "\n========================================\n";
    cout << " airCLI — Baggage Packing Terminal\n";
    cout << "========================================\n";

    baggageCLI.runAuthenticatedFlow();

    cout << "\nThank you for using baggage terminal. Goodbye!\n";
    return 0;
}

} // namespace baggage
