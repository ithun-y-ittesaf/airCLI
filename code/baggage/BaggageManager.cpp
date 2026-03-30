#include "BaggageManager.h"
#include "../core/IdGenerator.h"
#include "../tickets/TicketManager.h"
#include "../logging/Logger.h"
#include <cstdlib>
#include <cctype>
using namespace std;

namespace baggage {

// 30 kg limit.
static const int WEIGHT_LIMIT_GRAMS = 30000;

static bool requiresDeclaration(const string &category) {
    string c;
    c.reserve(category.size());
    for (char ch : category) c.push_back(static_cast<char>(tolower(ch)));
    return c == "batteries" || c == "electronics" || c == "liquids" || c == "medicines";
}

bool BaggageManager::load() {
    bagsById.clear();
    for (const auto &line : repo.readAll("bags")) {
        Bag bag = Bag::deserialize(line);
        bagsById[bag.getId()] = bag;
    }
    return true;
}

bool BaggageManager::save() const {
    vector<string> lines;
    for (const auto &p : bagsById)
        lines.push_back(p.second.serialize());
    return repo.writeAll("bags", lines);
}

string BaggageManager::packBag(const string             &passengerId,
                                     const string             &ticketId,
                                     const string             &bagTag,
                                     const vector<PackedItem> &items) {
    string bagId = core::IdGenerator::next("BG", [&](const string &c) {
        return bagsById.count(c) == 0;
    });

    Bag bag(bagId, ticketId, passengerId, bagTag);

    for (auto item : items) {
        const Item *def = itemDb.findById(item.itemId);
        if (def) {
            int minW  = def->getMinWeightGrams();
            int maxW  = def->getMaxWeightGrams();
            int range = (maxW > minW) ? (maxW - minW) : 0;
            item.assignedWeightGrams = minW + (range > 0 ? rand() % range : 0);
            item.itemName = def->getName();
            if (item.category.empty()) item.category = def->getCategory();
        }
        bag.addItem(item);
    }

    bagsById[bagId] = bag;
    repo.appendLine("bags", bag.serialize());

    logging::Logger::logCritical("BagPacked",
        "PASS=" + passengerId + "|TICKET=" + ticketId + "|BAG=" + bagId);

    return bagId;
}

const Bag *BaggageManager::getBagByTicket(const string &ticketId) const {
    for (const auto &p : bagsById)
        if (p.second.getTicketId() == ticketId) return &p.second;
    return nullptr;
}

vector<Bag> BaggageManager::getBagsForFlight(
    const string            &flightId,
    const tickets::TicketManager &ticketManager) const
{
    vector<Bag> result;
    for (const auto &ticket : ticketManager.getForFlight(flightId)) {
        const Bag *bag = getBagByTicket(ticket.getId());
        if (bag && !bag->getBagTag().empty()) result.push_back(*bag);
    }
    return result;
}

InspectionResult BaggageManager::inspectBag(const string &bagId) {
    InspectionResult result{false, "", 0, false, 0, 0, false, 0, ""};

    auto it = bagsById.find(bagId);
    if (it == bagsById.end()) return result;

    const Bag &bag = it->second;

    // --- Overweight check ---
    int totalWeight = bag.totalWeightGrams();
    if (totalWeight > WEIGHT_LIMIT_GRAMS) {
        int overGrams        = totalWeight - WEIGHT_LIMIT_GRAMS;

        result.overweight          = true;
        result.overweightGrams     = overGrams;
        result.overweightFineCents = 0;

        logging::Logger::logCritical("OverweightFine",
            "BAG="  + bagId +
            "|PASS=" + bag.getPassengerId() +
            "|OVER=" + to_string(overGrams) + "g");
    }

    // --- Illegal item check (private compartment only) ---
    for (const auto &pi : bag.getItems()) {
        if (requiresDeclaration(pi.category) && !pi.declared) {
            result.foundUndeclared = true;
            result.undeclaredCount++;
            if (result.undeclaredItemName.empty()) result.undeclaredItemName = pi.itemName;
        }

        if (pi.isPublic) continue;
        const Item *def = itemDb.findById(pi.itemId);
        if (def && def->getIsBanned()) {
            result.foundIllegal     = true;
            result.illegalItemName  = pi.itemName;
            result.fineApplied      = 0;

            logging::Logger::logCritical("IllegalItem",
                "BAG="  + bagId +
                "|PASS=" + bag.getPassengerId() +
                "|ITEM=" + pi.itemName +
                "|BAN=1");

            return result; // Stop at first illegal item.
        }
    }

    return result;
}

bool BaggageManager::deleteBagForTicket(const string &ticketId) {
    for (auto it = bagsById.begin(); it != bagsById.end(); ++it) {
        if (it->second.getTicketId() == ticketId) {
            bagsById.erase(it);
            return save();
        }
    }
    return false; // No bag found — that's fine, passenger may not have packed.
}

} // namespace baggage
