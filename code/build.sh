#!/bin/bash
# build.sh — builds airCLI + bankcli + baggagecli
# Usage:  ./build.sh
# Output: ./build/aircli ./build/bankcli ./build/baggagecli

set -e

CXX=${CXX:-g++}
CXXFLAGS="-std=c++17 -Wall -Wextra -O2"
OUT_DIR="build"
SRC="code"

mkdir -p "$OUT_DIR"
mkdir -p "storage/data"

echo "Building airCLI binaries..."

COMMON_SOURCES=(
    "$SRC/core/IdGenerator.cpp"
    "$SRC/core/TerminalLauncher.cpp"
    "$SRC/storage/XmlRepository.cpp"
    "$SRC/airport/AirportService.cpp"
    "$SRC/access/AccessControl.cpp"
    "$SRC/logging/Logger.cpp"
    "$SRC/users/User.cpp"
    "$SRC/users/UserManager.cpp"
    "$SRC/users/UserCLI.cpp"
    "$SRC/banking/Bank.cpp"
    "$SRC/banking/BankManager.cpp"
    "$SRC/banking/PaymentGatewayManager.cpp"
    "$SRC/banking/BankingTerminal.cpp"
    "$SRC/flights/Flight.cpp"
    "$SRC/flights/FlightManager.cpp"
    "$SRC/flights/FlightsCLI.cpp"
    "$SRC/tickets/Ticket.cpp"
    "$SRC/tickets/TicketManager.cpp"
    "$SRC/tickets/TicketsCLI.cpp"
    "$SRC/baggage/Item.cpp"
    "$SRC/baggage/ItemDatabase.cpp"
    "$SRC/baggage/Bag.cpp"
    "$SRC/baggage/SuspicionScorer.cpp"
    "$SRC/baggage/BaggageManager.cpp"
    "$SRC/baggage/BaggageCLI.cpp"
    "$SRC/baggage/BaggageTerminal.cpp"
)

AIRCLI_BIN="$OUT_DIR/aircli"
BANKCLI_BIN="$OUT_DIR/bankcli"
BAGGAGECLI_BIN="$OUT_DIR/baggagecli"

$CXX $CXXFLAGS "${COMMON_SOURCES[@]}" "$SRC/main.cpp" -o "$AIRCLI_BIN"
$CXX $CXXFLAGS "${COMMON_SOURCES[@]}" "$SRC/bank_main.cpp" -o "$BANKCLI_BIN"
$CXX $CXXFLAGS "${COMMON_SOURCES[@]}" "$SRC/baggage_main.cpp" -o "$BAGGAGECLI_BIN"

echo ""
echo "Build successful:"
echo "  $AIRCLI_BIN"
echo "  $BANKCLI_BIN"
echo "  $BAGGAGECLI_BIN"
echo "Run with:"
echo "  ./$AIRCLI_BIN"
echo ""
echo "Make sure to run from the project root so storage/data/ is reachable."
