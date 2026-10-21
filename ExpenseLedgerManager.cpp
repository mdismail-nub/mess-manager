// ExpenseLedgerManager.cpp
// C++17 + Qt 6 + SQLite implementation for:
//   - Expense categorization
//   - Monthly expense totals
//   - Expense validation
//   - Individual member ledgers
//   - Payments, deposits, bill charges, and adjustments
//   - Transaction history and member balances
//
// Monetary values are stored as integer poisha.
// Example: ৳250.50 is stored as 25050.
//
// Balance convention used in this starter implementation:
//   balance = payments + deposits - bill charges + adjustments
// An adjustment may be positive or negative.
// Confirm this convention and the table names with your team before integration.

#include <QDate>
#include <QList>
#include <QMap>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QString>
#include <QStringList>

struct Expense {
    int id = 0;
    QDate date;
    QString description;
    QString category;
    qint64 amountPoisha = 0;
    int payerMemberId = 0;
    QString notes;
};

struct LedgerTransaction {
    int id = 0;
    int memberId = 0;
    QDate date;
    QString type; // PAYMENT, DEPOSIT, BILL_CHARGE, or ADJUSTMENT
    qint64 amountPoisha = 0;
    QString description;
};

class ExpenseLedgerManager {
public:
    explicit ExpenseLedgerManager(const QSqlDatabase &database)
        : db(database) {}

    bool initializeDatabase(QString *errorMessage = nullptr) {
        if (!db.isOpen()) {
            return fail(QStringLiteral("The database connection is not open."), errorMessage);
        }

        QSqlQuery query(db);
        query.exec(QStringLiteral("PRAGMA foreign_keys = ON"));

        const QString createMembers = QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS members (
                id INTEGER PRIMARY KEY,
                name TEXT NOT NULL
            )
        )SQL");

        const QString createCategories = QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS expense_categories (
                name TEXT PRIMARY KEY
            )
        )SQL");

        const QString createExpenses = QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS expenses (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                expense_date TEXT NOT NULL,
                description TEXT NOT NULL,
                category TEXT NOT NULL,
                amount_poisha INTEGER NOT NULL CHECK(amount_poisha > 0),
                payer_member_id INTEGER NOT NULL,
                notes TEXT,
                FOREIGN KEY (category) REFERENCES expense_categories(name),
                FOREIGN KEY (payer_member_id) REFERENCES members(id)
            )
        )SQL");

        const QString createLedger = QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS ledger_transactions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                member_id INTEGER NOT NULL,
                transaction_date TEXT NOT NULL,
                transaction_type TEXT NOT NULL CHECK(
                    transaction_type IN ('PAYMENT', 'DEPOSIT', 'BILL_CHARGE', 'ADJUSTMENT')
                ),
                amount_poisha INTEGER NOT NULL CHECK(amount_poisha > 0),
                description TEXT,
                FOREIGN KEY (member_id) REFERENCES members(id)
            )
        )SQL");

        for (const QString &statement : {createMembers, createCategories,
                                         createExpenses, createLedger}) {
            if (!query.exec(statement)) {
                return fail(query.lastError().text(), errorMessage);
            }
        }

        return true;
    }

    bool addExpenseCategory(const QString &category,
                            QString *errorMessage = nullptr) {
        const QString cleanedCategory = category.trimmed();
        if (cleanedCategory.isEmpty()) {
            return fail(QStringLiteral("Expense category cannot be empty."), errorMessage);
        }

        QSqlQuery query(db);
        query.prepare(QStringLiteral(
            "INSERT OR IGNORE INTO expense_categories(name) VALUES (:name)"));
        query.bindValue(QStringLiteral(":name"), cleanedCategory);

        if (!query.exec()) {
            return fail(query.lastError().text(), errorMessage);
        }
        return true;
    }

    bool validateExpense(const Expense &expense,
                         QString *errorMessage = nullptr) const {
        if (!expense.date.isValid()) {
            return fail(QStringLiteral("Expense date is invalid."), errorMessage);
        }
        if (expense.description.trimmed().isEmpty()) {
            return fail(QStringLiteral("Expense description cannot be empty."), errorMessage);
        }
        if (expense.category.trimmed().isEmpty()) {
            return fail(QStringLiteral("Expense category cannot be empty."), errorMessage);
        }
        if (expense.amountPoisha <= 0) {
            return fail(QStringLiteral("Expense amount must be greater than zero."), errorMessage);
        }
        if (expense.payerMemberId <= 0) {
            return fail(QStringLiteral("Payer member ID must be greater than zero."), errorMessage);
        }
        return true;
    }

    bool addExpense(const Expense &expense, int *newExpenseId = nullptr,
                    QString *errorMessage = nullptr) {
        if (!validateExpense(expense, errorMessage)) {
            return false;
        }
        if (!categoryExists(expense.category.trimmed())) {
            return fail(QStringLiteral("The expense category does not exist."), errorMessage);
        }
        if (!memberExists(expense.payerMemberId)) {
            return fail(QStringLiteral("The payer member does not exist."), errorMessage);
        }

        QSqlQuery query(db);
        query.prepare(QStringLiteral(R"SQL(
            INSERT INTO expenses
                (expense_date, description, category, amount_poisha,
                 payer_member_id, notes)
            VALUES (:date, :description, :category, :amount,
                    :payer_id, :notes)
        )SQL"));
        query.bindValue(QStringLiteral(":date"), expense.date.toString(Qt::ISODate));
        query.bindValue(QStringLiteral(":description"), expense.description.trimmed());
        query.bindValue(QStringLiteral(":category"), expense.category.trimmed());
        query.bindValue(QStringLiteral(":amount"), expense.amountPoisha);
        query.bindValue(QStringLiteral(":payer_id"), expense.payerMemberId);
        query.bindValue(QStringLiteral(":notes"), expense.notes.trimmed());

        if (!query.exec()) {
            return fail(query.lastError().text(), errorMessage);
        }
        if (newExpenseId != nullptr) {
            *newExpenseId = query.lastInsertId().toInt();
        }
        return true;
    }

    QMap<QString, qint64> calculateMonthlyExpenseTotals(
        int year, int month, QString *errorMessage = nullptr) const {
        QMap<QString, qint64> totals;
        if (year < 1 || month < 1 || month > 12) {
            fail(QStringLiteral("Year or month is invalid."), errorMessage);
            return totals;
        }

        const QDate firstDay(year, month, 1);
        const QDate nextMonth = firstDay.addMonths(1);

        QSqlQuery query(db);
        query.prepare(QStringLiteral(R"SQL(
            SELECT category, COALESCE(SUM(amount_poisha), 0)
            FROM expenses
            WHERE expense_date >= :start_date
              AND expense_date < :end_date
            GROUP BY category
            ORDER BY category
        )SQL"));
        query.bindValue(QStringLiteral(":start_date"), firstDay.toString(Qt::ISODate));
        query.bindValue(QStringLiteral(":end_date"), nextMonth.toString(Qt::ISODate));

        if (!query.exec()) {
            fail(query.lastError().text(), errorMessage);
            return totals;
        }

        while (query.next()) {
            totals.insert(query.value(0).toString(), query.value(1).toLongLong());
        }
        return totals;
    }

    bool validateLedgerTransaction(const LedgerTransaction &transaction,
                                   QString *errorMessage = nullptr) const {
        const QStringList validTypes = {
            QStringLiteral("PAYMENT"),
            QStringLiteral("DEPOSIT"),
            QStringLiteral("BILL_CHARGE"),
            QStringLiteral("ADJUSTMENT")
        };

        if (transaction.memberId <= 0) {
            return fail(QStringLiteral("Member ID must be greater than zero."), errorMessage);
        }
        if (!transaction.date.isValid()) {
            return fail(QStringLiteral("Transaction date is invalid."), errorMessage);
        }
        if (!validTypes.contains(transaction.type)) {
            return fail(QStringLiteral(
                "Transaction type must be PAYMENT, DEPOSIT, BILL_CHARGE, or ADJUSTMENT."),
                errorMessage);
        }
        if (transaction.amountPoisha <= 0) {
            return fail(QStringLiteral("Transaction amount must be greater than zero."), errorMessage);
        }
        return true;
    }

    bool addLedgerTransaction(const LedgerTransaction &transaction,
                              int *newTransactionId = nullptr,
                              QString *errorMessage = nullptr) {
        if (!validateLedgerTransaction(transaction, errorMessage)) {
            return false;
        }
        if (!memberExists(transaction.memberId)) {
            return fail(QStringLiteral("The member does not exist."), errorMessage);
        }

        QSqlQuery query(db);
        query.prepare(QStringLiteral(R"SQL(
            INSERT INTO ledger_transactions
                (member_id, transaction_date, transaction_type,
                 amount_poisha, description)
            VALUES (:member_id, :date, :type, :amount, :description)
        )SQL"));
        query.bindValue(QStringLiteral(":member_id"), transaction.memberId);
        query.bindValue(QStringLiteral(":date"), transaction.date.toString(Qt::ISODate));
        query.bindValue(QStringLiteral(":type"), transaction.type);
        query.bindValue(QStringLiteral(":amount"), transaction.amountPoisha);
        query.bindValue(QStringLiteral(":description"), transaction.description.trimmed());

        if (!query.exec()) {
            return fail(query.lastError().text(), errorMessage);
        }
        if (newTransactionId != nullptr) {
            *newTransactionId = query.lastInsertId().toInt();
        }
        return true;
    }

    QList<LedgerTransaction> transactionHistory(
        int memberId, QString *errorMessage = nullptr) const {
        QList<LedgerTransaction> history;
        if (memberId <= 0) {
            fail(QStringLiteral("Member ID must be greater than zero."), errorMessage);
            return history;
        }

        QSqlQuery query(db);
        query.prepare(QStringLiteral(R"SQL(
            SELECT id, member_id, transaction_date, transaction_type,
                   amount_poisha, description
            FROM ledger_transactions
            WHERE member_id = :member_id
            ORDER BY transaction_date ASC, id ASC
        )SQL"));
        query.bindValue(QStringLiteral(":member_id"), memberId);

        if (!query.exec()) {
            fail(query.lastError().text(), errorMessage);
            return history;
        }

        while (query.next()) {
            LedgerTransaction transaction;
            transaction.id = query.value(0).toInt();
            transaction.memberId = query.value(1).toInt();
            transaction.date = QDate::fromString(query.value(2).toString(), Qt::ISODate);
            transaction.type = query.value(3).toString();
            transaction.amountPoisha = query.value(4).toLongLong();
            transaction.description = query.value(5).toString();
            history.append(transaction);
        }
        return history;
    }

    qint64 calculateMemberBalance(int memberId,
                                  QString *errorMessage = nullptr) const {
        if (memberId <= 0) {
            fail(QStringLiteral("Member ID must be greater than zero."), errorMessage);
            return 0;
        }

        QSqlQuery query(db);
        query.prepare(QStringLiteral(R"SQL(
            SELECT COALESCE(SUM(
                CASE
                    WHEN transaction_type IN ('PAYMENT', 'DEPOSIT')
                        THEN amount_poisha
                    WHEN transaction_type = 'BILL_CHARGE'
                        THEN -amount_poisha
                    WHEN transaction_type = 'ADJUSTMENT'
                        THEN amount_poisha
                    ELSE 0
                END
            ), 0)
            FROM ledger_transactions
            WHERE member_id = :member_id
        )SQL"));
        query.bindValue(QStringLiteral(":member_id"), memberId);

        if (!query.exec() || !query.next()) {
            fail(query.lastError().text(), errorMessage);
            return 0;
        }
        return query.value(0).toLongLong();
    }

private:
    QSqlDatabase db;

    bool memberExists(int memberId) const {
        QSqlQuery query(db);
        query.prepare(QStringLiteral("SELECT 1 FROM members WHERE id = :id"));
        query.bindValue(QStringLiteral(":id"), memberId);
        return query.exec() && query.next();
    }

    bool categoryExists(const QString &category) const {
        QSqlQuery query(db);
        query.prepare(QStringLiteral(
            "SELECT 1 FROM expense_categories WHERE name = :name"));
        query.bindValue(QStringLiteral(":name"), category);
        return query.exec() && query.next();
    }

    static bool fail(const QString &message, QString *errorMessage) {
        if (errorMessage != nullptr) {
            *errorMessage = message;
        }
        return false;
    }
};

/*
Example usage:

QSqlDatabase database = QSqlDatabase::addDatabase("QSQLITE");
database.setDatabaseName("mess_manager.db");
database.open();

ExpenseLedgerManager manager(database);
QString error;
manager.initializeDatabase(&error);

manager.addExpenseCategory("Groceries", &error);

Expense expense;
expense.date = QDate::currentDate();
expense.description = "Monthly groceries";
expense.category = "Groceries";
expense.amountPoisha = 25050; // ৳250.50
expense.payerMemberId = 1;
manager.addExpense(expense, nullptr, &error);

LedgerTransaction payment;
payment.memberId = 1;
payment.date = QDate::currentDate();
payment.type = "PAYMENT";
payment.amountPoisha = 500000; // ৳5,000.00
payment.description = "Monthly payment";
manager.addLedgerTransaction(payment, nullptr, &error);

qint64 balance = manager.calculateMemberBalance(1, &error);
QList<LedgerTransaction> history = manager.transactionHistory(1, &error);
QMap<QString, qint64> totals =
    manager.calculateMonthlyExpenseTotals(2026, 10, &error);
*/
