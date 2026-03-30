# airCLI — Test Credentials

## Application Accounts

| Username | Password | Role | Notes |
|---|---|---|---|
| `admin` | `admin123` | Admin | Full system access |
| `handler` | `handler123` | Security | Baggage handler view |
| `alice` | `alice123` | Passenger | No cash, no bank used |
| `bob` | `bob123` | Passenger | No cash, no bank used |
| `charlie` | `charlie123` | Passenger | 5,000 BDT cash, last bank: Dutch-Bangla, country: Syria (HIGH score) |

---

## Bank Accounts

All bank accounts are pre-seeded. Select the bank from the list, then enter the account number and password below.

### Dutch-Bangla Bank — Tier 1 (Premium) | Pool: 100,000,000 BDT
| Account Number | Password |
|---|---|
| `DBB-001` | `dbpass1` |
| `DBB-002` | `dbpass2` |
| `DBB-003` | `dbpass3` |

### BRAC Bank — Tier 1 (Premium) | Pool: 80,000,000 BDT
| Account Number | Password |
|---|---|
| `BRAC-001` | `bracpass1` |
| `BRAC-002` | `bracpass2` |
| `BRAC-003` | `bracpass3` |

### Islami Bank — Tier 2 (Standard) | Pool: 60,000,000 BDT
| Account Number | Password |
|---|---|
| `IB-001` | `ibpass1` |
| `IB-002` | `ibpass2` |
| `IB-003` | `ibpass3` |

### Sonali Bank — Tier 2 (Standard) | Pool: 50,000,000 BDT
| Account Number | Password |
|---|---|
| `SB-001` | `sbpass1` |
| `SB-002` | `sbpass2` |
| `SB-003` | `sbpass3` |

### Pubali Bank — Tier 3 (Low) | Pool: 30,000,000 BDT
| Account Number | Password |
|---|---|
| `PB-001` | `pbpass1` |
| `PB-002` | `pbpass2` |
| `PB-003` | `pbpass3` |

---

## Pre-seeded Flights

| ID | Route | Date | Seats | Price |
|---|---|---|---|---|
| `FL0000000001` | Dhaka → Chittagong | 2026-06-01 | 50 (2 booked) | 3,500 BDT |
| `FL0000000002` | Dhaka → Sylhet | 2026-06-05 | 40 | 4,000 BDT |
| `FL0000000003` | Chittagong → Cox's Bazar | 2026-06-10 | 30 | 2,500 BDT |
| `FL0000000004` | Dhaka → Jessore | 2026-06-15 | 45 | 3,000 BDT |

---

## Pre-seeded Items

| ID | Name | Category | Weight Range | Banned |
|---|---|---|---|---|
| `ITEM001` | Laptop | Electronics | 800–1500g | No |
| `ITEM002` | T-Shirt | Clothing | 100–300g | No |
| `ITEM003` | Book | Stationery | 200–600g | No |
| `ITEM004` | Shoes | Clothing | 400–900g | No |
| `ITEM005` | Toiletries Kit | Personal Care | 300–700g | No |
| `ITEM006` | Camera | Electronics | 300–800g | No |
| `ITEM007` | Jacket | Clothing | 500–1200g | No |
| `ITEM008` | Medicines | Medical | 100–400g | No |
| `ITEM009` | Knife | Weapons | 100–400g | **YES** |
| `ITEM010` | Explosive Device | Weapons | 500–2000g | **YES** |
| `ITEM011` | Illegal Substances | Contraband | 50–300g | **YES** |
| `ITEM012` | Firearm | Weapons | 800–3000g | **YES** |

---

## Budget Rules

| Event | Effect |
|---|---|
| Ticket purchased | + ticket price |
| Overweight fine | + 500 BDT per kg over 30kg |
| Illegal item fine | + 10,000 BDT |
| New flight added | − 500 BDT |
| Wrong inspection (clean bag) | − 2,000 BDT |
| Handler starts flight (first time) | − 5,000 BDT |
| Passenger cancels ticket | − 50% of ticket price |

Starting airport budget: **2,000,000 BDT**

---

## Suspicion Score Rules

| Condition | Points |
|---|---|
| Last bank Tier 1 (Dutch-Bangla, BRAC) | −10 |
| Last bank Tier 2 (Islami, Sonali) | +0 |
| Last bank Tier 3 (Pubali) | +20 |
| No bank used yet | +20 |
| Bag weight >20% over expected | +30 |
| Bag weight >10% over expected | +15 |
| Country on watchlist | +25 |
| Has private items | +10 |

**Score labels:** 0–24 = LOW · 25–49 = MEDIUM · 50+ = HIGH
