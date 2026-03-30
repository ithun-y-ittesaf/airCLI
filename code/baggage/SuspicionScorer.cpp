#include "SuspicionScorer.h"
#include <algorithm>
#include <cctype>
#include <vector>
using namespace std;

namespace baggage {

static const vector<string> WATCHLIST = {
    "Afghanistan", "Iran", "Iraq", "Libya", "Mali",
    "Myanmar", "North Korea", "Somalia", "Sudan", "Syria", "Yemen"
};

static string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

static bool onWatchlist(const string &country) {
    if (country.empty()) return false;
    string lc = toLower(country);
    for (const auto &e : WATCHLIST)
        if (toLower(e) == lc) return true;
    return false;
}

int SuspicionScorer::calculate(int               totalWeightGrams,
                                int               expectedWeightGrams,
                                const string &country,
                                int               bankScoreAdjustment,
                                bool              hasPrivateItems) {
    int score = 0;

    // Bank tier adjustment (Tier1=-10, Tier2=0, Tier3=+20, no bank=+20).
    score += bankScoreAdjustment;

    // Weight deviation above expected.
    if (expectedWeightGrams > 0) {
        double dev = static_cast<double>(totalWeightGrams - expectedWeightGrams)
                   / static_cast<double>(expectedWeightGrams);
        if      (dev > 0.20) score += 30;
        else if (dev > 0.10) score += 15;
    }

    // Country on watchlist.
    if (onWatchlist(country)) score += 25;

    // Has private items.
    if (hasPrivateItems) score += 10;

    return score;
}

string SuspicionScorer::scoreLabel(int score) {
    if (score >= 50) return "HIGH";
    if (score >= 25) return "MEDIUM";
    return "LOW";
}

} // namespace baggage
