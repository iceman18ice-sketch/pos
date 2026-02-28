#include "DatabaseManager.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QStandardPaths>

DatabaseManager &DatabaseManager::instance() {
  static DatabaseManager inst;
  return inst;
}

DatabaseManager::~DatabaseManager() {
  if (m_db.isOpen())
    m_db.close();
}

bool DatabaseManager::initialize(const QString &dbPath) {
  QString path = dbPath;
  if (path.isEmpty()) {
    QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir);
    dir.mkpath("data");
    path = appDir + "/data/pos_database.db";
  }

  m_db = QSqlDatabase::addDatabase("QSQLITE");
  m_db.setDatabaseName(path);

  if (!m_db.open()) {
    qCritical() << "Cannot open database:" << m_db.lastError().text();
    return false;
  }

  // Enable foreign keys
  QSqlQuery q(m_db);
  q.exec("PRAGMA foreign_keys = ON");
  q.exec("PRAGMA journal_mode = WAL");

  return createTables();
}

bool DatabaseManager::isConnected() const { return m_db.isOpen(); }

QSqlDatabase &DatabaseManager::database() { return m_db; }

bool DatabaseManager::createTables() {
  // Read schema from file or use embedded
  QString schema = R"SQL(
CREATE TABLE IF NOT EXISTS Users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    full_name TEXT NOT NULL,
    role TEXT DEFAULT 'cashier',
    phone TEXT,
    active INTEGER DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS Categories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    parent_id INTEGER DEFAULT 0,
    description TEXT
);
CREATE TABLE IF NOT EXISTS Units (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS Products (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    barcode TEXT UNIQUE,
    category_id INTEGER,
    unit_id INTEGER DEFAULT 1,
    buy_price REAL DEFAULT 0,
    sell_price REAL DEFAULT 0,
    tax_rate REAL DEFAULT 15,
    min_quantity REAL DEFAULT 0,
    current_stock REAL DEFAULT 0,
    description TEXT,
    active INTEGER DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    name_en TEXT DEFAULT '',
    brand_ar TEXT DEFAULT '',
    brand_en TEXT DEFAULT '',
    size_info TEXT DEFAULT '',
    image_path TEXT DEFAULT '',
    FOREIGN KEY (category_id) REFERENCES Categories(id),
    FOREIGN KEY (unit_id) REFERENCES Units(id)
);
CREATE TABLE IF NOT EXISTS Customers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    phone TEXT,
    address TEXT,
    type INTEGER DEFAULT 0,
    balance REAL DEFAULT 0,
    credit_limit REAL DEFAULT 0,
    tax_number TEXT,
    notes TEXT,
    active INTEGER DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS Stocks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    address TEXT,
    active INTEGER DEFAULT 1
);
CREATE TABLE IF NOT EXISTS SalesInvoices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    invoice_no INTEGER NOT NULL,
    date DATETIME DEFAULT CURRENT_TIMESTAMP,
    customer_id INTEGER,
    stock_id INTEGER DEFAULT 1,
    subtotal REAL DEFAULT 0,
    discount_rate REAL DEFAULT 0,
    discount_value REAL DEFAULT 0,
    tax_value REAL DEFAULT 0,
    total REAL DEFAULT 0,
    paid REAL DEFAULT 0,
    remaining REAL DEFAULT 0,
    payment_type TEXT DEFAULT 'cash',
    status TEXT DEFAULT 'completed',
    user_id INTEGER,
    notes TEXT,
    FOREIGN KEY (customer_id) REFERENCES Customers(id),
    FOREIGN KEY (stock_id) REFERENCES Stocks(id),
    FOREIGN KEY (user_id) REFERENCES Users(id)
);
CREATE TABLE IF NOT EXISTS SalesInvoiceDetails (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    invoice_id INTEGER NOT NULL,
    product_id INTEGER NOT NULL,
    product_name TEXT,
    quantity REAL DEFAULT 1,
    price REAL DEFAULT 0,
    discount_rate REAL DEFAULT 0,
    discount_value REAL DEFAULT 0,
    tax_rate REAL DEFAULT 15,
    tax_value REAL DEFAULT 0,
    total REAL DEFAULT 0,
    FOREIGN KEY (invoice_id) REFERENCES SalesInvoices(id),
    FOREIGN KEY (product_id) REFERENCES Products(id)
);
CREATE TABLE IF NOT EXISTS SalesReturns (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    return_no INTEGER NOT NULL,
    original_invoice_id INTEGER,
    date DATETIME DEFAULT CURRENT_TIMESTAMP,
    customer_id INTEGER,
    subtotal REAL DEFAULT 0,
    tax_value REAL DEFAULT 0,
    total REAL DEFAULT 0,
    user_id INTEGER,
    notes TEXT
);
CREATE TABLE IF NOT EXISTS PurchaseInvoices (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    invoice_no INTEGER NOT NULL,
    date DATETIME DEFAULT CURRENT_TIMESTAMP,
    supplier_id INTEGER,
    stock_id INTEGER DEFAULT 1,
    subtotal REAL DEFAULT 0,
    discount_rate REAL DEFAULT 0,
    discount_value REAL DEFAULT 0,
    tax_value REAL DEFAULT 0,
    total REAL DEFAULT 0,
    paid REAL DEFAULT 0,
    remaining REAL DEFAULT 0,
    supplier_invoice_no TEXT,
    payment_type TEXT DEFAULT 'cash',
    status TEXT DEFAULT 'completed',
    user_id INTEGER,
    notes TEXT,
    FOREIGN KEY (supplier_id) REFERENCES Customers(id),
    FOREIGN KEY (stock_id) REFERENCES Stocks(id)
);
CREATE TABLE IF NOT EXISTS PurchaseInvoiceDetails (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    invoice_id INTEGER NOT NULL,
    product_id INTEGER NOT NULL,
    product_name TEXT,
    quantity REAL DEFAULT 1,
    price REAL DEFAULT 0,
    discount_rate REAL DEFAULT 0,
    discount_value REAL DEFAULT 0,
    tax_rate REAL DEFAULT 15,
    tax_value REAL DEFAULT 0,
    total REAL DEFAULT 0,
    FOREIGN KEY (invoice_id) REFERENCES PurchaseInvoices(id),
    FOREIGN KEY (product_id) REFERENCES Products(id)
);
CREATE TABLE IF NOT EXISTS PurchaseReturns (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    return_no INTEGER NOT NULL,
    original_invoice_id INTEGER,
    date DATETIME DEFAULT CURRENT_TIMESTAMP,
    supplier_id INTEGER,
    subtotal REAL DEFAULT 0,
    tax_value REAL DEFAULT 0,
    total REAL DEFAULT 0,
    user_id INTEGER,
    notes TEXT
);
CREATE TABLE IF NOT EXISTS StockMovements (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    product_id INTEGER NOT NULL,
    stock_id INTEGER DEFAULT 1,
    type TEXT NOT NULL,
    quantity REAL DEFAULT 0,
    reference_type TEXT,
    reference_id INTEGER,
    date DATETIME DEFAULT CURRENT_TIMESTAMP,
    user_id INTEGER,
    notes TEXT,
    FOREIGN KEY (product_id) REFERENCES Products(id),
    FOREIGN KEY (stock_id) REFERENCES Stocks(id)
);
CREATE TABLE IF NOT EXISTS Expenses (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    date DATETIME DEFAULT CURRENT_TIMESTAMP,
    category TEXT,
    description TEXT NOT NULL,
    amount REAL DEFAULT 0,
    user_id INTEGER,
    notes TEXT
);
CREATE TABLE IF NOT EXISTS UserPermissions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    module TEXT NOT NULL,
    can_view INTEGER DEFAULT 1,
    can_add INTEGER DEFAULT 0,
    can_edit INTEGER DEFAULT 0,
    can_delete INTEGER DEFAULT 0,
    can_print INTEGER DEFAULT 0,
    FOREIGN KEY (user_id) REFERENCES Users(id)
);
CREATE TABLE IF NOT EXISTS Settings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    key TEXT NOT NULL UNIQUE,
    value TEXT,
    description TEXT
);
CREATE TABLE IF NOT EXISTS AuditLog (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER,
    action TEXT NOT NULL,
    table_name TEXT,
    record_id INTEGER,
    details TEXT,
    date DATETIME DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS Treasury (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    date DATETIME DEFAULT CURRENT_TIMESTAMP,
    type TEXT NOT NULL,
    amount REAL DEFAULT 0,
    description TEXT,
    reference_type TEXT,
    reference_id INTEGER,
    user_id INTEGER
);
CREATE TABLE IF NOT EXISTS Employees (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    phone TEXT,
    position TEXT,
    salary REAL DEFAULT 0,
    start_date TEXT,
    active INTEGER DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
CREATE TABLE IF NOT EXISTS Attendance (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    employee_id INTEGER NOT NULL,
    date TEXT NOT NULL,
    check_in TEXT,
    check_out TEXT,
    notes TEXT,
    FOREIGN KEY (employee_id) REFERENCES Employees(id)
);
CREATE TABLE IF NOT EXISTS PriceQuotes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    quote_no INTEGER NOT NULL,
    date DATETIME DEFAULT CURRENT_TIMESTAMP,
    customer_id INTEGER,
    total REAL DEFAULT 0,
    status TEXT DEFAULT 'pending',
    user_id INTEGER,
    notes TEXT,
    FOREIGN KEY (customer_id) REFERENCES Customers(id)
);
CREATE TABLE IF NOT EXISTS PriceQuoteDetails (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    quote_id INTEGER NOT NULL,
    product_id INTEGER,
    product_name TEXT,
    quantity REAL DEFAULT 1,
    price REAL DEFAULT 0,
    total REAL DEFAULT 0,
    FOREIGN KEY (quote_id) REFERENCES PriceQuotes(id)
);
CREATE TABLE IF NOT EXISTS StockTransfers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    transfer_no INTEGER NOT NULL,
    date DATETIME DEFAULT CURRENT_TIMESTAMP,
    from_stock_id INTEGER,
    to_stock_id INTEGER,
    user_id INTEGER,
    notes TEXT
);
CREATE TABLE IF NOT EXISTS StockTransferDetails (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    transfer_id INTEGER NOT NULL,
    product_id INTEGER,
    product_name TEXT,
    quantity REAL DEFAULT 0,
    FOREIGN KEY (transfer_id) REFERENCES StockTransfers(id)
);
CREATE TABLE IF NOT EXISTS Bookings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    booking_no INTEGER NOT NULL,
    date DATETIME DEFAULT CURRENT_TIMESTAMP,
    customer_id INTEGER,
    total REAL DEFAULT 0,
    deposit REAL DEFAULT 0,
    status TEXT DEFAULT 'pending',
    user_id INTEGER,
    notes TEXT,
    FOREIGN KEY (customer_id) REFERENCES Customers(id)
);
CREATE TABLE IF NOT EXISTS Salaries (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    employee_id INTEGER NOT NULL,
    month INTEGER NOT NULL,
    year INTEGER NOT NULL,
    basic_salary REAL DEFAULT 0,
    additions REAL DEFAULT 0,
    deductions REAL DEFAULT 0,
    net_salary REAL DEFAULT 0,
    paid_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    notes TEXT,
    FOREIGN KEY (employee_id) REFERENCES Employees(id)
);
CREATE TABLE IF NOT EXISTS Vacations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    employee_id INTEGER NOT NULL,
    type TEXT DEFAULT 'annual',
    date_from TEXT NOT NULL,
    date_to TEXT NOT NULL,
    status TEXT DEFAULT 'approved',
    notes TEXT,
    FOREIGN KEY (employee_id) REFERENCES Employees(id)
);
CREATE TABLE IF NOT EXISTS Maintenance (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    date DATETIME DEFAULT CURRENT_TIMESTAMP,
    customer_name TEXT,
    phone TEXT,
    device_type TEXT,
    problem TEXT,
    cost REAL DEFAULT 0,
    status TEXT DEFAULT 'pending',
    user_id INTEGER,
    notes TEXT
);
)SQL";

  QStringList statements = schema.split(";", Qt::SkipEmptyParts);
  for (const QString &stmt : statements) {
    QString trimmed = stmt.trimmed();
    if (trimmed.isEmpty())
      continue;
    if (!executeSQL(trimmed))
      return false;
  }

  // Insert defaults
  executeSQL("INSERT OR IGNORE INTO Users (username, password_hash, full_name, "
             "role) VALUES ('admin', 'admin', 'مدير النظام', 'admin')");
  executeSQL("INSERT OR IGNORE INTO Units (name) VALUES ('قطعة')");
  executeSQL("INSERT OR IGNORE INTO Units (name) VALUES ('كيلو')");
  executeSQL("INSERT OR IGNORE INTO Units (name) VALUES ('متر')");
  executeSQL("INSERT OR IGNORE INTO Units (name) VALUES ('لتر')");
  executeSQL("INSERT OR IGNORE INTO Units (name) VALUES ('علبة')");
  executeSQL("INSERT OR IGNORE INTO Units (name) VALUES ('كرتون')");
  executeSQL("INSERT OR IGNORE INTO Units (name) VALUES ('حبة')");
  executeSQL("INSERT OR IGNORE INTO Categories (name) VALUES ('عام')");
  executeSQL("INSERT OR IGNORE INTO Categories (name) VALUES ('مواد غذائية')");
  executeSQL("INSERT OR IGNORE INTO Categories (name) VALUES ('مشروبات')");
  executeSQL("INSERT OR IGNORE INTO Categories (name) VALUES ('منظفات')");
  executeSQL("INSERT OR IGNORE INTO Categories (name) VALUES ('إلكترونيات')");
  executeSQL("INSERT OR IGNORE INTO Stocks (name, address) VALUES ('المستودع "
             "الرئيسي', 'المقر الرئيسي')");
  executeSQL("INSERT OR IGNORE INTO Customers (name, phone, type) VALUES "
             "('عميل نقدي', '', 0)");
  executeSQL("INSERT OR IGNORE INTO Settings (key, value, description) VALUES "
             "('company_name', 'متجري', 'اسم الشركة')");
  executeSQL("INSERT OR IGNORE INTO Settings (key, value, description) VALUES "
             "('tax_rate', '15', 'نسبة الضريبة')");
  executeSQL("INSERT OR IGNORE INTO Settings (key, value, description) VALUES "
             "('currency', 'ريال', 'العملة')");
  executeSQL("INSERT OR IGNORE INTO Settings (key, value, description) VALUES "
             "('tax_number', '', 'الرقم الضريبي')");
  executeSQL("INSERT OR IGNORE INTO Settings (key, value, description) VALUES "
             "('company_phone', '', 'هاتف الشركة')");
  executeSQL("INSERT OR IGNORE INTO Settings (key, value, description) VALUES "
             "('company_address', '', 'عنوان الشركة')");
  // Migration: add new product columns for existing databases
  executeSQL("ALTER TABLE Products ADD COLUMN name_en TEXT DEFAULT ''");
  executeSQL("ALTER TABLE Products ADD COLUMN brand_ar TEXT DEFAULT ''");
  executeSQL("ALTER TABLE Products ADD COLUMN brand_en TEXT DEFAULT ''");
  executeSQL("ALTER TABLE Products ADD COLUMN size_info TEXT DEFAULT ''");
  executeSQL("ALTER TABLE Products ADD COLUMN image_path TEXT DEFAULT ''");

  return true;
}

bool DatabaseManager::executeSQL(const QString &sql) {
  QSqlQuery q(m_db);
  if (!q.exec(sql)) {
    qWarning() << "SQL Error:" << q.lastError().text() << "\nQuery:" << sql;
    return false;
  }
  return true;
}

// ============ Authentication ============
bool DatabaseManager::authenticate(const QString &username,
                                   const QString &password) {
  QSqlQuery q(m_db);
  q.prepare("SELECT id, full_name, role FROM Users WHERE username = ? AND "
            "password_hash = ? AND active = 1");
  q.addBindValue(username);
  q.addBindValue(password);
  if (q.exec() && q.next()) {
    m_userId = q.value(0).toInt();
    m_userName = q.value(1).toString();
    m_userRole = q.value(2).toString();
    return true;
  }
  return false;
}

// ============ Products ============
QSqlQuery DatabaseManager::getProducts(const QString &search, int categoryId) {
  QSqlQuery q(m_db);
  QString sql = "SELECT p.*, c.name as category_name, u.name as unit_name "
                "FROM Products p "
                "LEFT JOIN Categories c ON p.category_id = c.id "
                "LEFT JOIN Units u ON p.unit_id = u.id "
                "WHERE p.active = 1";
  if (!search.isEmpty())
    sql += " AND (p.name LIKE '%" + search + "%' OR p.barcode LIKE '%" +
           search + "%')";
  if (categoryId > 0)
    sql += " AND p.category_id = " + QString::number(categoryId);
  sql += " ORDER BY p.name";
  q.exec(sql);
  return q;
}

QSqlQuery DatabaseManager::getProductByBarcode(const QString &barcode) {
  QSqlQuery q(m_db);
  q.prepare("SELECT p.*, c.name as category_name, u.name as unit_name "
            "FROM Products p LEFT JOIN Categories c ON p.category_id = c.id "
            "LEFT JOIN Units u ON p.unit_id = u.id WHERE p.barcode = ? AND "
            "p.active = 1");
  q.addBindValue(barcode);
  q.exec();
  return q;
}

QSqlQuery DatabaseManager::getProductById(int id) {
  QSqlQuery q(m_db);
  q.prepare("SELECT p.*, c.name as category_name, u.name as unit_name "
            "FROM Products p LEFT JOIN Categories c ON p.category_id = c.id "
            "LEFT JOIN Units u ON p.unit_id = u.id WHERE p.id = ?");
  q.addBindValue(id);
  q.exec();
  return q;
}

bool DatabaseManager::addProduct(const QString &name, const QString &barcode,
                                 int categoryId, int unitId, double buyPrice,
                                 double sellPrice, double taxRate,
                                 double minQty, const QString &nameEn,
                                 const QString &brandAr, const QString &brandEn,
                                 const QString &sizeInfo,
                                 const QString &imagePath) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO Products (name, barcode, category_id, unit_id, "
            "buy_price, sell_price, tax_rate, min_quantity, "
            "name_en, brand_ar, brand_en, size_info, image_path) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  q.addBindValue(name);
  q.addBindValue(barcode.isEmpty() ? QVariant() : barcode);
  q.addBindValue(categoryId);
  q.addBindValue(unitId);
  q.addBindValue(buyPrice);
  q.addBindValue(sellPrice);
  q.addBindValue(taxRate);
  q.addBindValue(minQty);
  q.addBindValue(nameEn);
  q.addBindValue(brandAr);
  q.addBindValue(brandEn);
  q.addBindValue(sizeInfo);
  q.addBindValue(imagePath);
  return q.exec();
}

bool DatabaseManager::updateProduct(
    int id, const QString &name, const QString &barcode, int categoryId,
    int unitId, double buyPrice, double sellPrice, double taxRate,
    double minQty, const QString &nameEn, const QString &brandAr,
    const QString &brandEn, const QString &sizeInfo, const QString &imagePath) {
  QSqlQuery q(m_db);
  q.prepare("UPDATE Products SET name=?, barcode=?, category_id=?, unit_id=?, "
            "buy_price=?, sell_price=?, tax_rate=?, min_quantity=?, "
            "name_en=?, brand_ar=?, brand_en=?, size_info=?, image_path=? "
            "WHERE id=?");
  q.addBindValue(name);
  q.addBindValue(barcode.isEmpty() ? QVariant() : barcode);
  q.addBindValue(categoryId);
  q.addBindValue(unitId);
  q.addBindValue(buyPrice);
  q.addBindValue(sellPrice);
  q.addBindValue(taxRate);
  q.addBindValue(minQty);
  q.addBindValue(nameEn);
  q.addBindValue(brandAr);
  q.addBindValue(brandEn);
  q.addBindValue(sizeInfo);
  q.addBindValue(imagePath);
  q.addBindValue(id);
  return q.exec();
}

bool DatabaseManager::deleteProduct(int id) {
  QSqlQuery q(m_db);
  q.prepare("UPDATE Products SET active = 0 WHERE id = ?");
  q.addBindValue(id);
  return q.exec();
}

bool DatabaseManager::updateProductStock(int productId, double quantity,
                                         bool add) {
  QSqlQuery q(m_db);
  QString op = add ? "+" : "-";
  q.prepare("UPDATE Products SET current_stock = current_stock " + op +
            " ? WHERE id = ?");
  q.addBindValue(quantity);
  q.addBindValue(productId);
  return q.exec();
}

// ============ Categories ============
QSqlQuery DatabaseManager::getCategories() {
  QSqlQuery q(m_db);
  q.exec("SELECT * FROM Categories ORDER BY name");
  return q;
}

bool DatabaseManager::addCategory(const QString &name) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO Categories (name) VALUES (?)");
  q.addBindValue(name);
  return q.exec();
}

bool DatabaseManager::deleteCategory(int id) {
  QSqlQuery q(m_db);
  q.prepare("DELETE FROM Categories WHERE id = ?");
  q.addBindValue(id);
  return q.exec();
}

// ============ Units ============
QSqlQuery DatabaseManager::getUnits() {
  QSqlQuery q(m_db);
  q.exec("SELECT * FROM Units ORDER BY name");
  return q;
}

// ============ Customers ============
QSqlQuery DatabaseManager::getCustomers(int type, const QString &search) {
  QSqlQuery q(m_db);
  QString sql = "SELECT * FROM Customers WHERE active = 1";
  if (type >= 0)
    sql += " AND (type = " + QString::number(type) + " OR type = 2)";
  if (!search.isEmpty())
    sql +=
        " AND (name LIKE '%" + search + "%' OR phone LIKE '%" + search + "%')";
  sql += " ORDER BY name";
  q.exec(sql);
  return q;
}

QSqlQuery DatabaseManager::getCustomerById(int id) {
  QSqlQuery q(m_db);
  q.prepare("SELECT * FROM Customers WHERE id = ?");
  q.addBindValue(id);
  q.exec();
  return q;
}

bool DatabaseManager::addCustomer(const QString &name, const QString &phone,
                                  const QString &address, int type,
                                  double creditLimit,
                                  const QString &taxNumber) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO Customers (name, phone, address, type, credit_limit, "
            "tax_number) VALUES (?, ?, ?, ?, ?, ?)");
  q.addBindValue(name);
  q.addBindValue(phone);
  q.addBindValue(address);
  q.addBindValue(type);
  q.addBindValue(creditLimit);
  q.addBindValue(taxNumber);
  return q.exec();
}

bool DatabaseManager::updateCustomer(int id, const QString &name,
                                     const QString &phone,
                                     const QString &address, int type,
                                     double creditLimit,
                                     const QString &taxNumber) {
  QSqlQuery q(m_db);
  q.prepare("UPDATE Customers SET name=?, phone=?, address=?, type=?, "
            "credit_limit=?, tax_number=? WHERE id=?");
  q.addBindValue(name);
  q.addBindValue(phone);
  q.addBindValue(address);
  q.addBindValue(type);
  q.addBindValue(creditLimit);
  q.addBindValue(taxNumber);
  q.addBindValue(id);
  return q.exec();
}

bool DatabaseManager::deleteCustomer(int id) {
  QSqlQuery q(m_db);
  q.prepare("UPDATE Customers SET active = 0 WHERE id = ?");
  q.addBindValue(id);
  return q.exec();
}

bool DatabaseManager::updateCustomerBalance(int customerId, double amount,
                                            bool add) {
  QSqlQuery q(m_db);
  QString op = add ? "+" : "-";
  q.prepare("UPDATE Customers SET balance = balance " + op + " ? WHERE id = ?");
  q.addBindValue(amount);
  q.addBindValue(customerId);
  return q.exec();
}

// ============ Sales ============
int DatabaseManager::getNextSalesInvoiceNo() {
  QSqlQuery q(m_db);
  q.exec("SELECT COALESCE(MAX(invoice_no), 0) + 1 FROM SalesInvoices");
  if (q.next())
    return q.value(0).toInt();
  return 1;
}

int DatabaseManager::createSalesInvoice(
    int customerId, int stockId, double subtotal, double discountRate,
    double discountValue, double taxValue, double total, double paid,
    double remaining, const QString &paymentType, const QString &notes) {
  QSqlQuery q(m_db);
  int invNo = getNextSalesInvoiceNo();
  q.prepare("INSERT INTO SalesInvoices (invoice_no, customer_id, stock_id, "
            "subtotal, discount_rate, discount_value, "
            "tax_value, total, paid, remaining, payment_type, user_id, notes) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  q.addBindValue(invNo);
  q.addBindValue(customerId);
  q.addBindValue(stockId);
  q.addBindValue(subtotal);
  q.addBindValue(discountRate);
  q.addBindValue(discountValue);
  q.addBindValue(taxValue);
  q.addBindValue(total);
  q.addBindValue(paid);
  q.addBindValue(remaining);
  q.addBindValue(paymentType);
  q.addBindValue(m_userId);
  q.addBindValue(notes);
  if (q.exec())
    return q.lastInsertId().toInt();
  return -1;
}

bool DatabaseManager::addSalesInvoiceDetail(
    int invoiceId, int productId, const QString &productName, double quantity,
    double price, double discountRate, double discountValue, double taxRate,
    double taxValue, double total) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO SalesInvoiceDetails (invoice_id, product_id, "
            "product_name, quantity, price, "
            "discount_rate, discount_value, tax_rate, tax_value, total) VALUES "
            "(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  q.addBindValue(invoiceId);
  q.addBindValue(productId);
  q.addBindValue(productName);
  q.addBindValue(quantity);
  q.addBindValue(price);
  q.addBindValue(discountRate);
  q.addBindValue(discountValue);
  q.addBindValue(taxRate);
  q.addBindValue(taxValue);
  q.addBindValue(total);
  return q.exec();
}

QSqlQuery DatabaseManager::getSalesInvoices(const QString &dateFrom,
                                            const QString &dateTo) {
  QSqlQuery q(m_db);
  QString sql = "SELECT si.*, c.name as customer_name FROM SalesInvoices si "
                "LEFT JOIN Customers c ON si.customer_id = c.id WHERE 1=1";
  if (!dateFrom.isEmpty())
    sql += " AND DATE(si.date) >= '" + dateFrom + "'";
  if (!dateTo.isEmpty())
    sql += " AND DATE(si.date) <= '" + dateTo + "'";
  sql += " ORDER BY si.id DESC";
  q.exec(sql);
  return q;
}

QSqlQuery DatabaseManager::getSalesInvoiceDetails(int invoiceId) {
  QSqlQuery q(m_db);
  q.prepare("SELECT * FROM SalesInvoiceDetails WHERE invoice_id = ?");
  q.addBindValue(invoiceId);
  q.exec();
  return q;
}

// ============ Purchases ============
int DatabaseManager::getNextPurchaseInvoiceNo() {
  QSqlQuery q(m_db);
  q.exec("SELECT COALESCE(MAX(invoice_no), 0) + 1 FROM PurchaseInvoices");
  if (q.next())
    return q.value(0).toInt();
  return 1;
}

int DatabaseManager::createPurchaseInvoice(
    int supplierId, int stockId, double subtotal, double discountRate,
    double discountValue, double taxValue, double total, double paid,
    double remaining, const QString &supplierInvNo, const QString &paymentType,
    const QString &notes) {
  QSqlQuery q(m_db);
  int invNo = getNextPurchaseInvoiceNo();
  q.prepare("INSERT INTO PurchaseInvoices (invoice_no, supplier_id, stock_id, "
            "subtotal, discount_rate, discount_value, "
            "tax_value, total, paid, remaining, supplier_invoice_no, "
            "payment_type, user_id, notes) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  q.addBindValue(invNo);
  q.addBindValue(supplierId);
  q.addBindValue(stockId);
  q.addBindValue(subtotal);
  q.addBindValue(discountRate);
  q.addBindValue(discountValue);
  q.addBindValue(taxValue);
  q.addBindValue(total);
  q.addBindValue(paid);
  q.addBindValue(remaining);
  q.addBindValue(supplierInvNo);
  q.addBindValue(paymentType);
  q.addBindValue(m_userId);
  q.addBindValue(notes);
  if (q.exec())
    return q.lastInsertId().toInt();
  return -1;
}

bool DatabaseManager::addPurchaseInvoiceDetail(
    int invoiceId, int productId, const QString &productName, double quantity,
    double price, double discountRate, double discountValue, double taxRate,
    double taxValue, double total) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO PurchaseInvoiceDetails (invoice_id, product_id, "
            "product_name, quantity, price, "
            "discount_rate, discount_value, tax_rate, tax_value, total) VALUES "
            "(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  q.addBindValue(invoiceId);
  q.addBindValue(productId);
  q.addBindValue(productName);
  q.addBindValue(quantity);
  q.addBindValue(price);
  q.addBindValue(discountRate);
  q.addBindValue(discountValue);
  q.addBindValue(taxRate);
  q.addBindValue(taxValue);
  q.addBindValue(total);
  return q.exec();
}

QSqlQuery DatabaseManager::getPurchaseInvoices(const QString &dateFrom,
                                               const QString &dateTo) {
  QSqlQuery q(m_db);
  QString sql = "SELECT pi.*, c.name as supplier_name FROM PurchaseInvoices pi "
                "LEFT JOIN Customers c ON pi.supplier_id = c.id WHERE 1=1";
  if (!dateFrom.isEmpty())
    sql += " AND DATE(pi.date) >= '" + dateFrom + "'";
  if (!dateTo.isEmpty())
    sql += " AND DATE(pi.date) <= '" + dateTo + "'";
  sql += " ORDER BY pi.id DESC";
  q.exec(sql);
  return q;
}

QSqlQuery DatabaseManager::getPurchaseInvoiceDetails(int invoiceId) {
  QSqlQuery q(m_db);
  q.prepare("SELECT * FROM PurchaseInvoiceDetails WHERE invoice_id = ?");
  q.addBindValue(invoiceId);
  q.exec();
  return q;
}

// ============ Stock ============
QSqlQuery DatabaseManager::getStocks() {
  QSqlQuery q(m_db);
  q.exec("SELECT * FROM Stocks WHERE active = 1 ORDER BY name");
  return q;
}

QSqlQuery DatabaseManager::getStockMovements(int stockId,
                                             const QString &dateFrom,
                                             const QString &dateTo) {
  QSqlQuery q(m_db);
  QString sql = "SELECT sm.*, p.name as product_name FROM StockMovements sm "
                "LEFT JOIN Products p ON sm.product_id = p.id WHERE 1=1";
  if (stockId > 0)
    sql += " AND sm.stock_id = " + QString::number(stockId);
  if (!dateFrom.isEmpty())
    sql += " AND DATE(sm.date) >= '" + dateFrom + "'";
  if (!dateTo.isEmpty())
    sql += " AND DATE(sm.date) <= '" + dateTo + "'";
  sql += " ORDER BY sm.id DESC";
  q.exec(sql);
  return q;
}

bool DatabaseManager::addStockMovement(int productId, int stockId,
                                       const QString &type, double quantity,
                                       const QString &refType, int refId,
                                       const QString &notes) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO StockMovements (product_id, stock_id, type, quantity, "
            "reference_type, reference_id, user_id, notes) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
  q.addBindValue(productId);
  q.addBindValue(stockId);
  q.addBindValue(type);
  q.addBindValue(quantity);
  q.addBindValue(refType);
  q.addBindValue(refId);
  q.addBindValue(m_userId);
  q.addBindValue(notes);
  return q.exec();
}

// ============ Expenses ============
QSqlQuery DatabaseManager::getExpenses(const QString &dateFrom,
                                       const QString &dateTo) {
  QSqlQuery q(m_db);
  QString sql = "SELECT * FROM Expenses WHERE 1=1";
  if (!dateFrom.isEmpty())
    sql += " AND DATE(date) >= '" + dateFrom + "'";
  if (!dateTo.isEmpty())
    sql += " AND DATE(date) <= '" + dateTo + "'";
  sql += " ORDER BY id DESC";
  q.exec(sql);
  return q;
}

bool DatabaseManager::addExpense(const QString &category,
                                 const QString &description, double amount,
                                 const QString &notes) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO Expenses (category, description, amount, user_id, "
            "notes) VALUES (?, ?, ?, ?, ?)");
  q.addBindValue(category);
  q.addBindValue(description);
  q.addBindValue(amount);
  q.addBindValue(m_userId);
  q.addBindValue(notes);
  return q.exec();
}

// ============ Reports ============
QSqlQuery DatabaseManager::getDailySalesReport(const QString &date) {
  QSqlQuery q(m_db);
  q.prepare("SELECT si.*, c.name as customer_name FROM SalesInvoices si "
            "LEFT JOIN Customers c ON si.customer_id = c.id WHERE "
            "DATE(si.date) = ? ORDER BY si.id");
  q.addBindValue(date);
  q.exec();
  return q;
}

QSqlQuery DatabaseManager::getMonthlySalesReport(int year, int month) {
  QSqlQuery q(m_db);
  q.prepare("SELECT DATE(date) as day, COUNT(*) as count, SUM(total) as total "
            "FROM SalesInvoices WHERE strftime('%Y', date) = ? AND "
            "strftime('%m', date) = ? "
            "GROUP BY DATE(date) ORDER BY day");
  q.addBindValue(QString::number(year));
  q.addBindValue(QString::number(month).rightJustified(2, '0'));
  q.exec();
  return q;
}

QSqlQuery DatabaseManager::getTopProducts(int limit) {
  QSqlQuery q(m_db);
  q.prepare("SELECT product_id, product_name, SUM(quantity) as total_qty, "
            "SUM(total) as total_sales "
            "FROM SalesInvoiceDetails GROUP BY product_id ORDER BY total_sales "
            "DESC LIMIT ?");
  q.addBindValue(limit);
  q.exec();
  return q;
}

QSqlQuery DatabaseManager::getStockReport() {
  QSqlQuery q(m_db);
  q.exec(
      "SELECT p.id, p.name, p.barcode, p.current_stock, p.min_quantity, "
      "p.buy_price, p.sell_price, "
      "c.name as category_name, (p.current_stock * p.buy_price) as stock_value "
      "FROM Products p LEFT JOIN Categories c ON p.category_id = c.id "
      "WHERE p.active = 1 ORDER BY p.name");
  return q;
}

QSqlQuery DatabaseManager::getProfitReport(const QString &dateFrom,
                                           const QString &dateTo) {
  QSqlQuery q(m_db);
  QString sql = "SELECT DATE(si.date) as day, "
                "SUM(sid.total) as revenue, "
                "SUM(sid.quantity * p.buy_price) as cost, "
                "SUM(sid.total - (sid.quantity * p.buy_price)) as profit "
                "FROM SalesInvoiceDetails sid "
                "JOIN SalesInvoices si ON sid.invoice_id = si.id "
                "JOIN Products p ON sid.product_id = p.id WHERE 1=1";
  if (!dateFrom.isEmpty())
    sql += " AND DATE(si.date) >= '" + dateFrom + "'";
  if (!dateTo.isEmpty())
    sql += " AND DATE(si.date) <= '" + dateTo + "'";
  sql += " GROUP BY DATE(si.date) ORDER BY day";
  q.exec(sql);
  return q;
}

QSqlQuery DatabaseManager::getCustomerStatement(int customerId,
                                                const QString &dateFrom,
                                                const QString &dateTo) {
  QSqlQuery q(m_db);
  QString sql = "SELECT 'sale' as type, invoice_no as ref_no, date, total as "
                "debit, paid as credit, notes "
                "FROM SalesInvoices WHERE customer_id = " +
                QString::number(customerId);
  if (!dateFrom.isEmpty())
    sql += " AND DATE(date) >= '" + dateFrom + "'";
  if (!dateTo.isEmpty())
    sql += " AND DATE(date) <= '" + dateTo + "'";
  sql += " UNION ALL SELECT 'purchase' as type, invoice_no as ref_no, date, 0 "
         "as debit, total as credit, notes "
         "FROM PurchaseInvoices WHERE supplier_id = " +
         QString::number(customerId);
  if (!dateFrom.isEmpty())
    sql += " AND DATE(date) >= '" + dateFrom + "'";
  if (!dateTo.isEmpty())
    sql += " AND DATE(date) <= '" + dateTo + "'";
  sql += " ORDER BY date";
  q.exec(sql);
  return q;
}

// ============ Settings ============
QString DatabaseManager::getSetting(const QString &key,
                                    const QString &defaultValue) {
  QSqlQuery q(m_db);
  q.prepare("SELECT value FROM Settings WHERE key = ?");
  q.addBindValue(key);
  if (q.exec() && q.next())
    return q.value(0).toString();
  return defaultValue;
}

bool DatabaseManager::setSetting(const QString &key, const QString &value) {
  QSqlQuery q(m_db);
  q.prepare("INSERT OR REPLACE INTO Settings (key, value) VALUES (?, ?)");
  q.addBindValue(key);
  q.addBindValue(value);
  return q.exec();
}

// ============ Dashboard ============
double DatabaseManager::getTodaySales() {
  QSqlQuery q(m_db);
  q.exec("SELECT COALESCE(SUM(total), 0) FROM SalesInvoices WHERE DATE(date) = "
         "DATE('now')");
  if (q.next())
    return q.value(0).toDouble();
  return 0;
}

double DatabaseManager::getTodayPurchases() {
  QSqlQuery q(m_db);
  q.exec("SELECT COALESCE(SUM(total), 0) FROM PurchaseInvoices WHERE "
         "DATE(date) = DATE('now')");
  if (q.next())
    return q.value(0).toDouble();
  return 0;
}

double DatabaseManager::getTodayProfit() {
  QSqlQuery q(m_db);
  q.exec("SELECT COALESCE(SUM(sid.total - (sid.quantity * p.buy_price)), 0) "
         "FROM SalesInvoiceDetails sid "
         "JOIN SalesInvoices si ON sid.invoice_id = si.id "
         "JOIN Products p ON sid.product_id = p.id "
         "WHERE DATE(si.date) = DATE('now')");
  if (q.next())
    return q.value(0).toDouble();
  return 0;
}

int DatabaseManager::getTotalProducts() {
  QSqlQuery q(m_db);
  q.exec("SELECT COUNT(*) FROM Products WHERE active = 1");
  if (q.next())
    return q.value(0).toInt();
  return 0;
}

int DatabaseManager::getLowStockCount() {
  QSqlQuery q(m_db);
  q.exec("SELECT COUNT(*) FROM Products WHERE active = 1 AND current_stock <= "
         "min_quantity AND min_quantity > 0");
  if (q.next())
    return q.value(0).toInt();
  return 0;
}

double DatabaseManager::getTodayExpenses() {
  QSqlQuery q(m_db);
  q.exec("SELECT COALESCE(SUM(amount), 0) FROM Expenses WHERE DATE(date) = "
         "DATE('now')");
  if (q.next())
    return q.value(0).toDouble();
  return 0;
}

// ============ Sales Returns ============
QSqlQuery DatabaseManager::getSalesReturns(const QString &dateFrom,
                                           const QString &dateTo) {
  QSqlQuery q(m_db);
  QString sql = "SELECT sr.*, c.name as customer_name FROM SalesReturns sr "
                "LEFT JOIN Customers c ON sr.customer_id = c.id WHERE 1=1";
  if (!dateFrom.isEmpty())
    sql += " AND DATE(sr.date) >= '" + dateFrom + "'";
  if (!dateTo.isEmpty())
    sql += " AND DATE(sr.date) <= '" + dateTo + "'";
  sql += " ORDER BY sr.id DESC";
  q.exec(sql);
  return q;
}

int DatabaseManager::createSalesReturn(int originalInvoiceId, int customerId,
                                       double subtotal, double taxValue,
                                       double total, const QString &notes) {
  QSqlQuery q(m_db);
  q.exec("SELECT COALESCE(MAX(return_no), 0) + 1 FROM SalesReturns");
  int retNo = q.next() ? q.value(0).toInt() : 1;
  q.prepare(
      "INSERT INTO SalesReturns (return_no, original_invoice_id, customer_id, "
      "subtotal, tax_value, total, user_id, notes) VALUES (?, ?, ?, ?, ?, ?, "
      "?, ?)");
  q.addBindValue(retNo);
  q.addBindValue(originalInvoiceId);
  q.addBindValue(customerId);
  q.addBindValue(subtotal);
  q.addBindValue(taxValue);
  q.addBindValue(total);
  q.addBindValue(m_userId);
  q.addBindValue(notes);
  if (q.exec())
    return q.lastInsertId().toInt();
  return -1;
}

// ============ Purchase Returns ============
QSqlQuery DatabaseManager::getPurchaseReturns(const QString &dateFrom,
                                              const QString &dateTo) {
  QSqlQuery q(m_db);
  QString sql = "SELECT pr.*, c.name as supplier_name FROM PurchaseReturns pr "
                "LEFT JOIN Customers c ON pr.supplier_id = c.id WHERE 1=1";
  if (!dateFrom.isEmpty())
    sql += " AND DATE(pr.date) >= '" + dateFrom + "'";
  if (!dateTo.isEmpty())
    sql += " AND DATE(pr.date) <= '" + dateTo + "'";
  sql += " ORDER BY pr.id DESC";
  q.exec(sql);
  return q;
}

int DatabaseManager::createPurchaseReturn(int originalInvoiceId, int supplierId,
                                          double subtotal, double taxValue,
                                          double total, const QString &notes) {
  QSqlQuery q(m_db);
  q.exec("SELECT COALESCE(MAX(return_no), 0) + 1 FROM PurchaseReturns");
  int retNo = q.next() ? q.value(0).toInt() : 1;
  q.prepare("INSERT INTO PurchaseReturns (return_no, original_invoice_id, "
            "supplier_id, "
            "subtotal, tax_value, total, user_id, notes) VALUES (?, ?, ?, ?, "
            "?, ?, ?, ?)");
  q.addBindValue(retNo);
  q.addBindValue(originalInvoiceId);
  q.addBindValue(supplierId);
  q.addBindValue(subtotal);
  q.addBindValue(taxValue);
  q.addBindValue(total);
  q.addBindValue(m_userId);
  q.addBindValue(notes);
  if (q.exec())
    return q.lastInsertId().toInt();
  return -1;
}

// ============ Treasury ============
QSqlQuery DatabaseManager::getTreasuryTransactions(const QString &dateFrom,
                                                   const QString &dateTo) {
  QSqlQuery q(m_db);
  QString sql = "SELECT * FROM Treasury WHERE 1=1";
  if (!dateFrom.isEmpty())
    sql += " AND DATE(date) >= '" + dateFrom + "'";
  if (!dateTo.isEmpty())
    sql += " AND DATE(date) <= '" + dateTo + "'";
  sql += " ORDER BY id DESC";
  q.exec(sql);
  return q;
}

bool DatabaseManager::addTreasuryTransaction(const QString &type, double amount,
                                             const QString &description,
                                             const QString &refType,
                                             int refId) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO Treasury (type, amount, description, reference_type, "
            "reference_id, user_id) VALUES (?, ?, ?, ?, ?, ?)");
  q.addBindValue(type);
  q.addBindValue(amount);
  q.addBindValue(description);
  q.addBindValue(refType);
  q.addBindValue(refId);
  q.addBindValue(m_userId);
  return q.exec();
}

double DatabaseManager::getTreasuryBalance() {
  QSqlQuery q(m_db);
  q.exec("SELECT COALESCE(SUM(CASE WHEN type='in' THEN amount ELSE -amount "
         "END), 0) FROM Treasury");
  if (q.next())
    return q.value(0).toDouble();
  return 0;
}

// ============ Employees ============
QSqlQuery DatabaseManager::getEmployees(const QString &search) {
  QSqlQuery q(m_db);
  QString sql = "SELECT * FROM Employees WHERE active = 1";
  if (!search.isEmpty())
    sql +=
        " AND (name LIKE '%" + search + "%' OR phone LIKE '%" + search + "%')";
  sql += " ORDER BY name";
  q.exec(sql);
  return q;
}

bool DatabaseManager::addEmployee(const QString &name, const QString &phone,
                                  const QString &position, double salary,
                                  const QString &startDate) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO Employees (name, phone, position, salary, start_date) "
            "VALUES (?, ?, ?, ?, ?)");
  q.addBindValue(name);
  q.addBindValue(phone);
  q.addBindValue(position);
  q.addBindValue(salary);
  q.addBindValue(startDate);
  return q.exec();
}

bool DatabaseManager::updateEmployee(int id, const QString &name,
                                     const QString &phone,
                                     const QString &position, double salary) {
  QSqlQuery q(m_db);
  q.prepare(
      "UPDATE Employees SET name=?, phone=?, position=?, salary=? WHERE id=?");
  q.addBindValue(name);
  q.addBindValue(phone);
  q.addBindValue(position);
  q.addBindValue(salary);
  q.addBindValue(id);
  return q.exec();
}

bool DatabaseManager::deleteEmployee(int id) {
  QSqlQuery q(m_db);
  q.prepare("UPDATE Employees SET active = 0 WHERE id = ?");
  q.addBindValue(id);
  return q.exec();
}

// ============ Attendance ============
QSqlQuery DatabaseManager::getAttendance(const QString &dateFrom,
                                         const QString &dateTo,
                                         int employeeId) {
  QSqlQuery q(m_db);
  QString sql = "SELECT a.*, e.name as employee_name FROM Attendance a "
                "LEFT JOIN Employees e ON a.employee_id = e.id WHERE 1=1";
  if (employeeId > 0)
    sql += " AND a.employee_id = " + QString::number(employeeId);
  if (!dateFrom.isEmpty())
    sql += " AND a.date >= '" + dateFrom + "'";
  if (!dateTo.isEmpty())
    sql += " AND a.date <= '" + dateTo + "'";
  sql += " ORDER BY a.date DESC, e.name";
  q.exec(sql);
  return q;
}

bool DatabaseManager::addAttendance(int employeeId, const QString &date,
                                    const QString &checkIn,
                                    const QString &checkOut,
                                    const QString &notes) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO Attendance (employee_id, date, check_in, check_out, "
            "notes) VALUES (?, ?, ?, ?, ?)");
  q.addBindValue(employeeId);
  q.addBindValue(date);
  q.addBindValue(checkIn);
  q.addBindValue(checkOut);
  q.addBindValue(notes);
  return q.exec();
}

// ============ Price Quotes ============
QSqlQuery DatabaseManager::getPriceQuotes(const QString &dateFrom,
                                          const QString &dateTo) {
  QSqlQuery q(m_db);
  QString sql = "SELECT pq.*, c.name as customer_name FROM PriceQuotes pq "
                "LEFT JOIN Customers c ON pq.customer_id = c.id WHERE 1=1";
  if (!dateFrom.isEmpty())
    sql += " AND DATE(pq.date) >= '" + dateFrom + "'";
  if (!dateTo.isEmpty())
    sql += " AND DATE(pq.date) <= '" + dateTo + "'";
  sql += " ORDER BY pq.id DESC";
  q.exec(sql);
  return q;
}

int DatabaseManager::createPriceQuote(int customerId, double total,
                                      const QString &notes) {
  QSqlQuery q(m_db);
  q.exec("SELECT COALESCE(MAX(quote_no), 0) + 1 FROM PriceQuotes");
  int qNo = q.next() ? q.value(0).toInt() : 1;
  q.prepare("INSERT INTO PriceQuotes (quote_no, customer_id, total, user_id, "
            "notes) VALUES (?, ?, ?, ?, ?)");
  q.addBindValue(qNo);
  q.addBindValue(customerId);
  q.addBindValue(total);
  q.addBindValue(m_userId);
  q.addBindValue(notes);
  if (q.exec())
    return q.lastInsertId().toInt();
  return -1;
}

bool DatabaseManager::addPriceQuoteDetail(int quoteId, int productId,
                                          const QString &name, double qty,
                                          double price, double total) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO PriceQuoteDetails (quote_id, product_id, "
            "product_name, quantity, price, total) "
            "VALUES (?, ?, ?, ?, ?, ?)");
  q.addBindValue(quoteId);
  q.addBindValue(productId);
  q.addBindValue(name);
  q.addBindValue(qty);
  q.addBindValue(price);
  q.addBindValue(total);
  return q.exec();
}

// ==================== Stock Transfers ====================
QSqlQuery DatabaseManager::getStockTransfers(const QString &dateFrom,
                                             const QString &dateTo) {
  QSqlQuery q(m_db);
  QString sql = "SELECT * FROM StockTransfers";
  if (!dateFrom.isEmpty() && !dateTo.isEmpty())
    sql +=
        " WHERE date BETWEEN '" + dateFrom + "' AND '" + dateTo + " 23:59:59'";
  sql += " ORDER BY id DESC";
  q.exec(sql);
  return q;
}

int DatabaseManager::createStockTransfer(int fromStockId, int toStockId,
                                         const QString &notes) {
  QSqlQuery q(m_db);
  q.exec("SELECT COALESCE(MAX(transfer_no), 0) + 1 FROM StockTransfers");
  int no = q.next() ? q.value(0).toInt() : 1;
  q.prepare("INSERT INTO StockTransfers (transfer_no, from_stock_id, "
            "to_stock_id, user_id, notes) VALUES (?, ?, ?, ?, ?)");
  q.addBindValue(no);
  q.addBindValue(fromStockId);
  q.addBindValue(toStockId);
  q.addBindValue(m_userId);
  q.addBindValue(notes);
  if (q.exec())
    return q.lastInsertId().toInt();
  return -1;
}

bool DatabaseManager::addStockTransferDetail(int transferId, int productId,
                                             const QString &name, double qty) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO StockTransferDetails (transfer_id, product_id, "
            "product_name, quantity) VALUES (?, ?, ?, ?)");
  q.addBindValue(transferId);
  q.addBindValue(productId);
  q.addBindValue(name);
  q.addBindValue(qty);
  return q.exec();
}

// ==================== Bookings ====================
QSqlQuery DatabaseManager::getBookings(const QString &dateFrom,
                                       const QString &dateTo) {
  QSqlQuery q(m_db);
  QString sql = "SELECT b.*, c.name as customer_name FROM Bookings b LEFT JOIN "
                "Customers c ON b.customer_id = c.id";
  if (!dateFrom.isEmpty() && !dateTo.isEmpty())
    sql += " WHERE b.date BETWEEN '" + dateFrom + "' AND '" + dateTo +
           " 23:59:59'";
  sql += " ORDER BY b.id DESC";
  q.exec(sql);
  return q;
}

int DatabaseManager::createBooking(int customerId, double total, double deposit,
                                   const QString &status,
                                   const QString &notes) {
  QSqlQuery q(m_db);
  q.exec("SELECT COALESCE(MAX(booking_no), 0) + 1 FROM Bookings");
  int no = q.next() ? q.value(0).toInt() : 1;
  q.prepare("INSERT INTO Bookings (booking_no, customer_id, total, deposit, "
            "status, user_id, notes) VALUES (?, ?, ?, ?, ?, ?, ?)");
  q.addBindValue(no);
  q.addBindValue(customerId);
  q.addBindValue(total);
  q.addBindValue(deposit);
  q.addBindValue(status);
  q.addBindValue(m_userId);
  q.addBindValue(notes);
  if (q.exec())
    return q.lastInsertId().toInt();
  return -1;
}

// ==================== Salaries ====================
QSqlQuery DatabaseManager::getSalaries(int month, int year) {
  QSqlQuery q(m_db);
  q.prepare("SELECT s.*, e.name as employee_name, e.position FROM Salaries s "
            "LEFT JOIN Employees e ON s.employee_id = e.id "
            "WHERE s.month = ? AND s.year = ? ORDER BY e.name");
  q.addBindValue(month);
  q.addBindValue(year);
  q.exec();
  return q;
}

bool DatabaseManager::addSalary(int employeeId, int month, int year,
                                double basicSalary, double additions,
                                double deductions, double netSalary,
                                const QString &notes) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO Salaries (employee_id, month, year, basic_salary, "
            "additions, deductions, net_salary, notes) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
  q.addBindValue(employeeId);
  q.addBindValue(month);
  q.addBindValue(year);
  q.addBindValue(basicSalary);
  q.addBindValue(additions);
  q.addBindValue(deductions);
  q.addBindValue(netSalary);
  q.addBindValue(notes);
  return q.exec();
}

// ==================== Vacations ====================
QSqlQuery DatabaseManager::getVacations(int employeeId) {
  QSqlQuery q(m_db);
  QString sql = "SELECT v.*, e.name as employee_name FROM Vacations v "
                "LEFT JOIN Employees e ON v.employee_id = e.id";
  if (employeeId > 0)
    sql += " WHERE v.employee_id = " + QString::number(employeeId);
  sql += " ORDER BY v.id DESC";
  q.exec(sql);
  return q;
}

bool DatabaseManager::addVacation(int employeeId, const QString &type,
                                  const QString &dateFrom,
                                  const QString &dateTo, const QString &notes) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO Vacations (employee_id, type, date_from, date_to, "
            "notes) VALUES (?, ?, ?, ?, ?)");
  q.addBindValue(employeeId);
  q.addBindValue(type);
  q.addBindValue(dateFrom);
  q.addBindValue(dateTo);
  q.addBindValue(notes);
  return q.exec();
}

// ==================== Maintenance ====================
QSqlQuery DatabaseManager::getMaintenanceRequests(const QString &status) {
  QSqlQuery q(m_db);
  QString sql = "SELECT * FROM Maintenance";
  if (!status.isEmpty())
    sql += " WHERE status = '" + status + "'";
  sql += " ORDER BY id DESC";
  q.exec(sql);
  return q;
}

int DatabaseManager::addMaintenanceRequest(const QString &customerName,
                                           const QString &phone,
                                           const QString &deviceType,
                                           const QString &problem, double cost,
                                           const QString &status) {
  QSqlQuery q(m_db);
  q.prepare("INSERT INTO Maintenance (customer_name, phone, device_type, "
            "problem, cost, status, user_id) "
            "VALUES (?, ?, ?, ?, ?, ?, ?)");
  q.addBindValue(customerName);
  q.addBindValue(phone);
  q.addBindValue(deviceType);
  q.addBindValue(problem);
  q.addBindValue(cost);
  q.addBindValue(status);
  q.addBindValue(m_userId);
  if (q.exec())
    return q.lastInsertId().toInt();
  return -1;
}

bool DatabaseManager::updateMaintenanceStatus(int id, const QString &status) {
  QSqlQuery q(m_db);
  q.prepare("UPDATE Maintenance SET status = ? WHERE id = ?");
  q.addBindValue(status);
  q.addBindValue(id);
  return q.exec();
}
