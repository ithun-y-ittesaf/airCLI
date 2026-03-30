#include "IdGenerator.h"
#include <chrono>
using namespace std;

namespace core {

string IdGenerator::next(
    const string &prefix,
    const function<bool(const string &)> &isUnique)
{
    using namespace chrono;
    long long t = duration_cast<seconds>(
        system_clock::now().time_since_epoch()).count();

    string candidate = prefix + to_string(t);
    while (!isUnique(candidate)) {
        ++t;
        candidate = prefix + to_string(t);
    }
    return candidate;
}

} // namespace core
