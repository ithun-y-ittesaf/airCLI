#!/bin/bash

# Build script for airCLI project
# This script compiles all source files and links them into an executable

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
OUTPUT_EXECUTABLE="$BUILD_DIR/aircli"
BANKING_EXECUTABLE="$BUILD_DIR/bankcli"
COMPILER="g++"
CFLAGS="-std=c++17 -Wall -Wextra -g -O2"
INCLUDE_DIRS="-I$PROJECT_DIR"

# Source files for main airCLI (all .cpp files except bankingMain.cpp)
SOURCE_FILES=(
    "$PROJECT_DIR/main.cpp"
    "$PROJECT_DIR/access/AccessControl.cpp"
    "$PROJECT_DIR/banking/BankingAccount.cpp"
    "$PROJECT_DIR/banking/BankingCLI.cpp"
    "$PROJECT_DIR/banking/BankingManager.cpp"
    "$PROJECT_DIR/banking/Transaction.cpp"
    "$PROJECT_DIR/flights/Flight.cpp"
    "$PROJECT_DIR/flights/FlightManager.cpp"
    "$PROJECT_DIR/flights/FlightsCLI.cpp"
    "$PROJECT_DIR/logging/Logger.cpp"
    "$PROJECT_DIR/storage/DataStorage.cpp"
    "$PROJECT_DIR/tickets/Ticket.cpp"
    "$PROJECT_DIR/tickets/TicketManager.cpp"
    "$PROJECT_DIR/tickets/TicketsCLI.cpp"
    "$PROJECT_DIR/users/User.cpp"
    "$PROJECT_DIR/users/UserCLI.cpp"
    "$PROJECT_DIR/users/UserManager.cpp"
)

# Source files for banking CLI
BANKING_SOURCE_FILES=(
    "$PROJECT_DIR/bankingMain.cpp"
    "$PROJECT_DIR/access/AccessControl.cpp"
    "$PROJECT_DIR/banking/BankingAccount.cpp"
    "$PROJECT_DIR/banking/BankingManager.cpp"
    "$PROJECT_DIR/banking/Transaction.cpp"
    "$PROJECT_DIR/logging/Logger.cpp"
    "$PROJECT_DIR/storage/DataStorage.cpp"
    "$PROJECT_DIR/users/User.cpp"
    "$PROJECT_DIR/users/UserCLI.cpp"
    "$PROJECT_DIR/users/UserManager.cpp"
)

# Clean function
clean() {
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
    echo -e "${GREEN}Clean complete.${NC}"
}

# Build function
build() {
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir -p "$BUILD_DIR"
    
    echo -e "${YELLOW}Starting compilation...${NC}"
    echo -e "Compiler: $COMPILER"
    echo -e "Flags: $CFLAGS"
    echo -e "Include: $INCLUDE_DIRS"
    echo ""
    
    # Compile and link main airCLI
    echo -e "${YELLOW}Building airCLI...${NC}"
    if $COMPILER $CFLAGS $INCLUDE_DIRS "${SOURCE_FILES[@]}" -o "$OUTPUT_EXECUTABLE"; then
        echo -e "${GREEN}airCLI build successful!${NC}"
        echo -e "${GREEN}Executable: $OUTPUT_EXECUTABLE${NC}"
    else
        echo -e "${RED}airCLI build failed!${NC}"
        return 1
    fi

    # Compile and link banking CLI
    echo -e "${YELLOW}Building Banking CLI...${NC}"
    if $COMPILER $CFLAGS $INCLUDE_DIRS "${BANKING_SOURCE_FILES[@]}" -o "$BANKING_EXECUTABLE"; then
        echo -e "${GREEN}Banking CLI build successful!${NC}"
        echo -e "${GREEN}Executable: $BANKING_EXECUTABLE${NC}"
    else
        echo -e "${RED}Banking CLI build failed!${NC}"
        return 1
    fi

    echo -e "${GREEN}All builds completed successfully!${NC}"
    return 0
}

# Run function
run() {
    if [ -f "$OUTPUT_EXECUTABLE" ]; then
        echo -e "${YELLOW}Running application...${NC}"
        "$OUTPUT_EXECUTABLE"
    else
        echo -e "${RED}Executable not found. Please build first.${NC}"
        return 1
    fi
}

# Help function
show_help() {
    echo "airCLI Build Script"
    echo ""
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  build       - Build the project (default)"
    echo "  clean       - Remove build directory"
    echo "  run         - Build and run the project"
    echo "  rebuild     - Clean and build"
    echo "  help        - Show this help message"
    echo ""
}

# Main script logic
case "${1:-build}" in
    build)
        build
        ;;
    clean)
        clean
        ;;
    run)
        build && run
        ;;
    rebuild)
        clean && build
        ;;
    help)
        show_help
        ;;
    *)
        echo -e "${RED}Unknown command: $1${NC}"
        show_help
        exit 1
        ;;
esac
