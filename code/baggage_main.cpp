#include <iostream>
#include <string>

#include "storage/XmlRepository.h"
#include "baggage/BaggageTerminal.h"
using namespace std;

int main(int argc, char *argv[]) {
    storage::XmlRepository repo("storage/data");

    string userId;
    string ticketId;

    if (argc >= 3) {
        userId = argv[1];
        ticketId = argv[2];
        // Direct packing mode (called from aircli after booking)
        return baggage::runBaggageTerminal(userId, ticketId, repo);
    }

    // If no arguments, use authenticated flow (independent baggage packing)
    return baggage::runBaggageTerminalAuthenticated(repo);
}
