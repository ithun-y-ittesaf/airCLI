#include "Bag.h"
#include <sstream>
using namespace std;

namespace baggage {

void Bag::addItem(const PackedItem &item) { items.push_back(item); }

int Bag::totalWeightGrams() const {
    int total = 0;
    for (const auto &pi : items) total += pi.assignedWeightGrams;
    return total;
}

string Bag::serialize() const {
    string line = id + "|" + ticketId + "|" + passengerId + "|" + bagTag + "|";
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) line += ",";
        const auto &pi = items[i];
        line += pi.itemId                              + ":"
              + pi.itemName                            + ":"
              + pi.category                            + ":"
              + to_string(pi.assignedWeightGrams) + ":"
              + (pi.isPublic ? "1" : "0")            + ":"
              + (pi.declared ? "1" : "0");
    }
    return line;
}

Bag Bag::deserialize(const string &line) {
    stringstream ss(line);
    string id, ticketId, passengerId, bagTag, itemsToken;
    getline(ss, id,          '|');
    getline(ss, ticketId,    '|');
    getline(ss, passengerId, '|');
    getline(ss, bagTag,      '|');
    getline(ss, itemsToken,  '|');

    // Backward compatibility for old records: id|ticketId|passengerId|items
    if (itemsToken.empty() && !bagTag.empty() && bagTag.find(':') != string::npos) {
        itemsToken = bagTag;
        bagTag.clear();
    }

    Bag bag(id, ticketId, passengerId, bagTag);
    if (itemsToken.empty()) return bag;

    stringstream itemsStream(itemsToken);
    string token;
    while (getline(itemsStream, token, ',')) {
        if (token.empty()) continue;
        stringstream ts(token);
        string itemId, itemName, category, wStr, pStr, dStr;
        getline(ts, itemId,   ':');
        getline(ts, itemName, ':');
        getline(ts, category, ':');
        getline(ts, wStr,     ':');
        getline(ts, pStr,     ':');
        getline(ts, dStr,     ':');

        // Backward compatibility: older format had no category/declared.
        if (dStr.empty() && !category.empty() &&
            category.find_first_not_of("0123456789") == string::npos) {
            dStr = pStr;
            pStr = wStr;
            wStr = category;
            category.clear();
        }

        PackedItem pi;
        pi.itemId   = itemId;
        pi.itemName = itemName;
        pi.category = category;
        try { pi.assignedWeightGrams = stoi(wStr); } catch (...) { pi.assignedWeightGrams = 0; }
        try { pi.isPublic = (stoi(pStr) == 1);     } catch (...) { pi.isPublic = true; }
        try { pi.declared = (stoi(dStr) == 1);     } catch (...) { pi.declared = true; }
        bag.addItem(pi);
    }
    return bag;
}

} // namespace baggage
