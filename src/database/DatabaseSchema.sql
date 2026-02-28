-- ============================================
-- نظام محاسبي للمتاجر والسوبر ماركت
-- POS Accounting System - Database Schema
-- ============================================

-- جدول المستخدمين
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

-- جدول التصنيفات
CREATE TABLE IF NOT EXISTS Categories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    parent_id INTEGER DEFAULT 0,
    description TEXT
);

-- جدول الوحدات
CREATE TABLE IF NOT EXISTS Units (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL
);

-- جدول المنتجات
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
    FOREIGN KEY (category_id) REFERENCES Categories(id),
    FOREIGN KEY (unit_id) REFERENCES Units(id)
);

-- جدول العملاء والموردين
CREATE TABLE IF NOT EXISTS Customers (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    phone TEXT,
    address TEXT,
    type INTEGER DEFAULT 0, -- 0=عميل, 1=مورد, 2=كلاهما
    balance REAL DEFAULT 0,
    credit_limit REAL DEFAULT 0,
    tax_number TEXT,
    notes TEXT,
    active INTEGER DEFAULT 1,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- جدول المستودعات
CREATE TABLE IF NOT EXISTS Stocks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    address TEXT,
    active INTEGER DEFAULT 1
);

-- جدول فواتير المبيعات
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

-- جدول تفاصيل فواتير المبيعات
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

-- جدول مرتجعات المبيعات
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
    notes TEXT,
    FOREIGN KEY (original_invoice_id) REFERENCES SalesInvoices(id),
    FOREIGN KEY (customer_id) REFERENCES Customers(id)
);

-- جدول فواتير المشتريات
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

-- جدول تفاصيل فواتير المشتريات
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

-- جدول مرتجعات المشتريات
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

-- جدول حركات المخزون
CREATE TABLE IF NOT EXISTS StockMovements (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    product_id INTEGER NOT NULL,
    stock_id INTEGER DEFAULT 1,
    type TEXT NOT NULL, -- 'in', 'out', 'transfer', 'adjust'
    quantity REAL DEFAULT 0,
    reference_type TEXT, -- 'sale', 'purchase', 'return', 'manual'
    reference_id INTEGER,
    date DATETIME DEFAULT CURRENT_TIMESTAMP,
    user_id INTEGER,
    notes TEXT,
    FOREIGN KEY (product_id) REFERENCES Products(id),
    FOREIGN KEY (stock_id) REFERENCES Stocks(id)
);

-- جدول المصروفات
CREATE TABLE IF NOT EXISTS Expenses (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    date DATETIME DEFAULT CURRENT_TIMESTAMP,
    category TEXT,
    description TEXT NOT NULL,
    amount REAL DEFAULT 0,
    user_id INTEGER,
    notes TEXT
);

-- جدول الصلاحيات
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

-- جدول الإعدادات
CREATE TABLE IF NOT EXISTS Settings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    key TEXT NOT NULL UNIQUE,
    value TEXT,
    description TEXT
);

-- جدول سجل العمليات
CREATE TABLE IF NOT EXISTS AuditLog (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER,
    action TEXT NOT NULL,
    table_name TEXT,
    record_id INTEGER,
    details TEXT,
    date DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- ============================================
-- البيانات الافتراضية
-- ============================================

-- المستخدم الافتراضي (admin / admin)
INSERT OR IGNORE INTO Users (username, password_hash, full_name, role) 
VALUES ('admin', 'admin', 'مدير النظام', 'admin');

-- الوحدات الافتراضية
INSERT OR IGNORE INTO Units (name) VALUES ('قطعة');
INSERT OR IGNORE INTO Units (name) VALUES ('كيلو');
INSERT OR IGNORE INTO Units (name) VALUES ('متر');
INSERT OR IGNORE INTO Units (name) VALUES ('لتر');
INSERT OR IGNORE INTO Units (name) VALUES ('علبة');
INSERT OR IGNORE INTO Units (name) VALUES ('كرتون');
INSERT OR IGNORE INTO Units (name) VALUES ('حبة');
INSERT OR IGNORE INTO Units (name) VALUES ('طن');

-- التصنيفات الافتراضية
INSERT OR IGNORE INTO Categories (name) VALUES ('عام');
INSERT OR IGNORE INTO Categories (name) VALUES ('مواد غذائية');
INSERT OR IGNORE INTO Categories (name) VALUES ('مشروبات');
INSERT OR IGNORE INTO Categories (name) VALUES ('منظفات');
INSERT OR IGNORE INTO Categories (name) VALUES ('إلكترونيات');

-- المستودع الافتراضي
INSERT OR IGNORE INTO Stocks (name, address) VALUES ('المستودع الرئيسي', 'المقر الرئيسي');

-- العميل النقدي الافتراضي
INSERT OR IGNORE INTO Customers (name, phone, type) VALUES ('عميل نقدي', '', 0);

-- الإعدادات الافتراضية
INSERT OR IGNORE INTO Settings (key, value, description) VALUES ('company_name', 'متجري', 'اسم الشركة');
INSERT OR IGNORE INTO Settings (key, value, description) VALUES ('company_phone', '', 'هاتف الشركة');
INSERT OR IGNORE INTO Settings (key, value, description) VALUES ('company_address', '', 'عنوان الشركة');
INSERT OR IGNORE INTO Settings (key, value, description) VALUES ('tax_number', '', 'الرقم الضريبي');
INSERT OR IGNORE INTO Settings (key, value, description) VALUES ('tax_rate', '15', 'نسبة الضريبة');
INSERT OR IGNORE INTO Settings (key, value, description) VALUES ('currency', 'ريال', 'العملة');
INSERT OR IGNORE INTO Settings (key, value, description) VALUES ('language', 'ar', 'اللغة');
INSERT OR IGNORE INTO Settings (key, value, description) VALUES ('printer_name', '', 'اسم الطابعة');
INSERT OR IGNORE INTO Settings (key, value, description) VALUES ('paper_size', 'A4', 'حجم الورق');
INSERT OR IGNORE INTO Settings (key, value, description) VALUES ('invoice_copies', '1', 'عدد نسخ الفاتورة');
