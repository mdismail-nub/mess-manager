<div align="center">

# 🏢 Mess Manager

### A desktop Mess Management System for members, meals, expenses, bills, ledgers, and monthly reports

![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![Qt 6](https://img.shields.io/badge/Qt-6-41CD52?style=for-the-badge&logo=qt&logoColor=white)
![SQLite](https://img.shields.io/badge/SQLite-003B57?style=for-the-badge&logo=sqlite&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white)
![Status](https://img.shields.io/badge/Status-Under%20Development-orange?style=for-the-badge)
![Type](https://img.shields.io/badge/Project-University%20Group-8A2BE2?style=for-the-badge)

[Overview](#-project-overview) ·
[Features](#-key-features) ·
[Architecture](#-system-architecture) ·
[Modules](#-core-modules) ·
[Database](#-database-overview) ·
[Team](#-team) ·
[Workflow](#-git--contribution-workflow) ·
[Roadmap](#-roadmap)

</div>

---

## 📖 Project Overview

**Mess Manager** is a desktop application built with **C++17** and **Qt 6** for running a shared mess (a group dining arrangement). It tracks who the members are, how many meals each person eats, what the mess spends, and what each member owes every month.

The code follows a strict **layered architecture** (UI → Service → Repository → SQLite) so a five-person team can build modules in parallel without breaking each other's work.

> 🚧 **Heads up:** the project is **under active development**. The feature list below describes the **intended scope**, not a list of finished features.

## 🎯 Project Objectives

- 🧱 Build a production-oriented desktop application with C++17 and Qt 6
- 🧩 Apply OOP, separation of concerns, and layered architecture
- 💾 Persist data locally in SQLite using prepared statements only
- 🧮 Automate monthly meal-rate and bill calculation
- 🤝 Practise professional teamwork with Git, feature branches, and Pull Requests

## ✨ Key Features

| | Module | Intended scope |
|---|---|---|
| 🔑 | **Authentication** | Login/logout, user management, role-based access |
| 👥 | **Members** | Add/edit, search, activate/deactivate, contact and room details |
| 🍱 | **Meals** | Breakfast, lunch, dinner; daily and monthly tracking |
| 💸 | **Expenses** | Record expenses with categories and descriptions; monthly totals |
| 💳 | **Ledger** | Payments, deposits, bill charges, adjustments, balances, history |
| 🧮 | **Bill Calculation** | Meal rate, individual meal cost, fixed costs, preview, finalization |
| 📊 | **Dashboard** | Active members, today's meals, monthly expenses, meal rate, recent activity |
| 📑 | **Reports** | Monthly summaries, member statements, expense and meal summaries, CSV/PDF export |

## 🏛️ System Architecture

```mermaid
flowchart TD
    UI["🔵 Presentation Layer<br/>Qt Widgets"]
    SVC["🟢 Service Layer<br/>Business logic"]
    REPO["🟠 Repository Layer<br/>Data access"]
    DB[("🟣 SQLite Database")]

    UI --> SVC
    SVC --> REPO
    REPO --> DB

    style UI fill:#dbeafe,stroke:#2563eb,stroke-width:2px,color:#000
    style SVC fill:#dcfce7,stroke:#16a34a,stroke-width:2px,color:#000
    style REPO fill:#ffedd5,stroke:#ea580c,stroke-width:2px,color:#000
    style DB fill:#ede9fe,stroke:#7c3aed,stroke-width:2px,color:#000
```

**Dependency direction: UI → Service → Repository → Database.**

| Layer | Does | Must not |
|---|---|---|
| 🔵 **UI** | Renders screens, captures input, calls services | Run SQL or hold business rules |
| 🟢 **Service** | Validation, calculations, business rules | Reference Qt UI classes or run SQL |
| 🟠 **Repository** | Prepared SQL, maps rows to models | Contain business logic |
| 🟣 **SQLite** | Stores data, enforces constraints | Hold business logic |

📚 Full design: see the **System Architecture** page in the [Wiki](https://github.com/mdismail-nub/mess-manager/wiki).

## 🧩 Core Modules

```mermaid
flowchart LR
    AUTH["🔑 Authentication"]
    MEM["👥 Members"]
    MEAL["🍱 Meals"]
    EXP["💸 Expenses"]
    LED["💳 Ledger"]
    BILL["🧮 Bill Calculation"]
    DASH["📊 Dashboard"]
    REP["📑 Reports"]

    MEM --> MEAL
    MEAL --> BILL
    EXP --> BILL
    BILL <--> LED
    MEM --> LED
    BILL --> REP
    LED --> REP
    MEAL --> DASH
    EXP --> DASH
    BILL --> DASH

    style AUTH fill:#e0e7ff,stroke:#4f46e5,color:#000
    style MEM fill:#dcfce7,stroke:#16a34a,color:#000
    style MEAL fill:#dcfce7,stroke:#16a34a,color:#000
    style EXP fill:#fef9c3,stroke:#ca8a04,color:#000
    style LED fill:#fef9c3,stroke:#ca8a04,color:#000
    style BILL fill:#fee2e2,stroke:#dc2626,color:#000
    style DASH fill:#cffafe,stroke:#0891b2,color:#000
    style REP fill:#fce7f3,stroke:#db2777,color:#000
```

| Module | Purpose |
|---|---|
| 🔑 Authentication | Verifies users and controls access by role |
| 👥 Member Management | Manages member profiles and active status |
| 🍱 Meal Management | Records daily meals and computes monthly totals |
| 💸 Expense Management | Logs mess expenses and totals them by month |
| 💳 Individual Ledger | Tracks each member's transactions and balance |
| 🧮 Bill Calculation | Computes the meal rate and monthly bills, then finalizes them |
| 📊 Dashboard | Shows a live summary of key numbers |
| 📑 Monthly Reports | Produces monthly statements and exports |

### 🧮 How a monthly bill is calculated

```mermaid
flowchart LR
    A["Total Shared<br/>Expenses"] --> C{{"Meal Rate"}}
    B["Total Mess<br/>Meals"] --> C
    C --> D["Member Meal Cost<br/>meals × rate"]
    D --> E["+ Fixed Cost"]
    E --> F["Final Bill"]
    F --> G["Ledger<br/>Settlement"]

    style C fill:#dcfce7,stroke:#16a34a,color:#000
    style F fill:#fef9c3,stroke:#ca8a04,color:#000
    style G fill:#ffedd5,stroke:#ea580c,color:#000
```

```text
Meal Rate            = Total Shared Expenses / Total Mess Meals
Individual Meal Cost = Member Total Meals × Meal Rate
```

Zero-meal months, duplicate bills, and already-finalized bills must be handled safely (see the System Architecture page).

## 🛠️ Technology Stack

| Technology | Role |
|---|---|
| ![C++](https://img.shields.io/badge/-C%2B%2B17-00599C?logo=cplusplus&logoColor=white) | Application language |
| ![Qt](https://img.shields.io/badge/-Qt%206%20Widgets-41CD52?logo=qt&logoColor=white) | Desktop GUI |
| ![Qt SQL](https://img.shields.io/badge/-Qt%20SQL-41CD52?logo=qt&logoColor=white) | Database access API |
| ![SQLite](https://img.shields.io/badge/-SQLite-003B57?logo=sqlite&logoColor=white) | Local persistent storage |
| ![CMake](https://img.shields.io/badge/-CMake-064F8C?logo=cmake&logoColor=white) | Build system |
| ![Git](https://img.shields.io/badge/-Git%20%2F%20GitHub-181717?logo=github&logoColor=white) | Version control and collaboration |

## 📂 Project Structure

> 🗓️ This is the **planned structure**. Folders are added as modules are implemented; check the **Code** tab for what exists today.

```text
mess-manager/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── core/            # DatabaseManager, exceptions
│   ├── models/          # plain data structs
│   ├── repositories/    # SQL access (prepared statements)
│   ├── services/        # business logic
│   ├── ui/              # Qt widgets and dialogs
│   └── utils/           # security, date, formatting helpers
├── assets/              # icons, stylesheets
└── tests/               # planned
```

## 💾 Database Overview

Data is stored locally in one **SQLite** file through Qt SQL. All queries use **prepared statements**, and foreign keys are enforced with `PRAGMA foreign_keys = ON;`.

```mermaid
erDiagram
    users ||--o{ expenses : logs
    members ||--o{ meals : eats
    members ||--o{ ledger_transactions : has
    members ||--o{ bills : billed
```

| Table | Holds |
|---|---|
| `users` | Accounts, roles, hashed passwords |
| `members` | Member details, room, status |
| `meals` | Daily breakfast, lunch, dinner per member |
| `expenses` | Expenses with category, amount, date |
| `ledger_transactions` | Payments, deposits, bill charges, adjustments |
| `bills` | Monthly bills per member |

## 👥 Team

| # | Developer | Responsibility | Difficulty |
|---|---|---|---|
| 1 | **Raian Islam Eash** | Authentication + Member Management | 🟢 Easy |
| 2 | **Akash** | Meal Management + Basic Dashboard UI | 🟢 Easy |
| 3 | **Fahim Ahmed** | Expense Management + Individual Ledger | 🟡 Medium |
| 4 | **Joy Debnath** | Bill Calculation Engine | 🔴 Hard |
| 5 | **Md Ismail** | Monthly Reports + UI/UX + Integration | 🔴 Medium/Hard |

Ownership means primary responsibility, not isolation: developers coordinate whenever shared models or interfaces change. Details are on the **Team Responsibilities** page in the [Wiki](https://github.com/mdismail-nub/mess-manager/wiki).

## 🌿 Git & Contribution Workflow

```mermaid
flowchart TD
    A([Issue]) --> B[Assign developer]
    B --> C["feature/&lt;task&gt;"]
    C --> D[Implement and test locally]
    D --> E[Pull Request]
    E --> F[Code review]
    F --> G[Integration testing]
    G --> H["dev/&lt;developer&gt;"]
    H --> I([main])

    style C fill:#dbeafe,stroke:#2563eb,color:#000
    style E fill:#fef9c3,stroke:#ca8a04,color:#000
    style I fill:#dcfce7,stroke:#16a34a,color:#000
```

```text
main
  ↓
dev/<developer>
  ↓
feature/<task>
```

1. 🚫 Do not work directly on `main` or develop directly on `dev/*`.
2. 🌱 Create a `feature/<task>` branch for each task.
3. 🔀 Submit changes through a Pull Request and explain which modules are affected.
4. 👀 Review before merging.
5. 🛡️ Keep `main` stable.

## 📋 Development Requirements

- A C++17-capable compiler
- Qt 6 (Widgets and SQL modules)
- CMake
- Git

## 🚀 Build & Run

Setup and run instructions are being finalized and will be added once the application is buildable from the repository. No build commands are documented yet.

## 🧪 Testing

Automated testing is **planned but not yet implemented**. This section will be updated when tests are added.

## 📚 Documentation

Detailed documentation lives in the project **[Wiki](https://github.com/mdismail-nub/mess-manager/wiki)**:

- 🏛️ System Architecture
- 👥 Team Responsibilities & Developer Information

## 🗺️ Roadmap

```mermaid
flowchart LR
    S1["Repository<br/>setup"] --> S2["Core<br/>infrastructure"]
    S2 --> S3["Authentication"]
    S3 --> S4["Members"]
    S4 --> S5["Meals"]
    S5 --> S6["Expenses"]
    S6 --> S7["Ledger"]
    S7 --> S8["Bill<br/>calculation"]
    S8 --> S9["Dashboard"]
    S9 --> S10["Reports"]
    S10 --> S11["Testing"]
    S11 --> S12(["🏁 MVP release"])

    style S1 fill:#e5e7eb,stroke:#6b7280,color:#000
    style S2 fill:#e5e7eb,stroke:#6b7280,color:#000
    style S12 fill:#dcfce7,stroke:#16a34a,color:#000
```

- [ ] Repository setup
- [ ] Core infrastructure
- [ ] Authentication
- [ ] Members
- [ ] Meals
- [ ] Expenses
- [ ] Ledger
- [ ] Bill calculation
- [ ] Dashboard
- [ ] Reports
- [ ] Testing
- [ ] MVP release

> Tick items off as they are finished and merged into `main`. Phases may overlap in practice.

## 📌 Project Status

🚧 **Under development.** Modules are being built on feature branches; expect incomplete functionality and changes.

## 🎓 Academic Project

Mess Manager is a university group project demonstrating **object-oriented programming, GUI development, database management, software architecture, Git, and team collaboration**.

## 📄 License

Licensing will be finalized later.
