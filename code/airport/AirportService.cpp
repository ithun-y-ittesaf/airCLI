#include "AirportService.h"
#include <ctime>
using namespace std;

namespace airport {

// ---------------------------------------------------------------------------
// Cost constants (all in cents)
// ---------------------------------------------------------------------------
static const long long STARTING_BUDGET_CENTS      = 200000000LL; // 2,000,000 BDT
static const long long FLIGHT_ADDITION_COST_CENTS =      50000LL; //       500 BDT
static const long long HANDLER_FLIGHT_FEE_CENTS   =     500000LL; //     5,000 BDT

static string timestamp() {
    time_t now = time(nullptr);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

AirportService::AirportService(storage::IRepository &repo)
    : repo(repo), budgetCents(0) {}

bool AirportService::load() {
    auto lines = repo.readAll("budget");
    if (lines.empty()) {
        budgetCents = STARTING_BUDGET_CENTS;
        return save();
    }
    try { budgetCents = stoll(lines[0]); } catch (...) { budgetCents = STARTING_BUDGET_CENTS; }
    return true;
}

bool AirportService::save() const {
    return repo.writeAll("budget", { to_string(budgetCents) });
}

long long AirportService::getBudget() const { return budgetCents; }

void AirportService::recordEvent(const string &eventType, long long deltaCents) {
    budgetCents += deltaCents;
    save();
    string sign    = (deltaCents >= 0) ? "+" : "";
    string logLine = timestamp()                           + "|"
                        + eventType                             + "|"
                        + sign + to_string(deltaCents)     + "|"
                        + to_string(budgetCents);
    repo.appendLine("budget_log", logLine);
}

// Budget-increasing
void AirportService::onTicketPurchased(long long p)  { recordEvent("TICKET_PURCHASED",   +p); }

// Budget-decreasing
void AirportService::onFlightAdded()                 { recordEvent("FLIGHT_ADDED",       -FLIGHT_ADDITION_COST_CENTS);  }
void AirportService::onHandlerStartedFlight()        { recordEvent("HANDLER_FEE",        -HANDLER_FLIGHT_FEE_CENTS);    }

} // namespace airport
