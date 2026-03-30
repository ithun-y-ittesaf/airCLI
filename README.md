# airCLI — Airport Simulation System

A C++17 airport simulation with role-based access, banking, baggage inspection, and budget tracking.

## Build & Run

```bash
chmod +x build.sh
./build.sh
./build/aircli
```

Requires: `g++` with C++17 support. Run from the project root so `storage/data/` is reachable.

## Roles

| Role | Access |
|---|---|
| **Passenger** | Search & book flights, pack baggage, top up cash, cancel tickets |
| **Security** | Baggage handler view — inspect passengers, view suspicion scores |
| **Admin** | Manage flights, view budget, manage items, manage user roles |
| **Banned** | Cannot log in |

See `CREDENTIALS.md` for all test accounts and bank credentials.

## Architecture

```
code/
├── core/           IdGenerator
├── storage/        IRepository, XmlRepository
├── airport/        AirportService (budget tracking)
├── access/         AccessControl
├── logging/        Logger
├── users/          User, UserManager, UserCLI
├── banking/        Bank, BankManager, BankCLI
├── flights/        Flight, FlightManager, FlightsCLI
├── tickets/        Ticket, TicketManager, TicketsCLI
└── baggage/        Item, ItemDatabase, Bag,
                    BaggageManager, SuspicionScorer, BaggageCLI
```

## Data Files (`storage/data/`)

| File | Contents |
|---|---|
| `users.xml` | User accounts |
| `banks.xml` | Bank pool balances |
| `flights.xml` | Flight records |
| `tickets.xml` | Issued tickets |
| `bags.xml` | Packed baggage |
| `items.xml` | Item database |
| `budget.xml` | Airport budget balance |
| `budget_log.xml` | Budget event log |


## Team - 03 || SPL - I
- 230042125 Ittesaf Yeasir Ithun
- 230042152 Faiaj Jarif
- 230042144 Nafian Mahid

