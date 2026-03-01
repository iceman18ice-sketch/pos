#pragma once
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QVariant>

class DatabaseManager {
public:
  static DatabaseManager &instance();
  bool initialize(const QString &dbPath = "");
  bool isConnected() const;
  QSqlDatabase &database();

  // Auth
  bool authenticate(const QString &username, const QString &password);
  int currentUserId() const { return m_userId; }
  QString currentUserName() const { return m_userName; }
  QString currentUserRole() const { return m_userRole; }

  // Products
  QSqlQuery getProducts(const QString &search = "", int categoryId = -1);
  QSqlQuery getProductByBarcode(const QString &barcode);
  QSqlQuery getProductById(int id);
  bool addProduct(const QString &name, const QString &barcode, int categoryId,
                  int unitId, double buyPrice, double sellPrice, double taxRate,
                  double minQty, const QString &nameEn = "",
                  const QString &brandAr = "", const QString &brandEn = "",
                  const QString &sizeInfo = "", const QString &imagePath = "");
  bool updateProduct(int id, const QString &name, const QString &barcode,
                     int categoryId, int unitId, double buyPrice,
                     double sellPrice, double taxRate, double minQty,
                     const QString &nameEn = "", const QString &brandAr = "",
                     const QString &brandEn = "", const QString &sizeInfo = "",
                     const QString &imagePath = "");
  bool deleteProduct(int id);
  bool deleteAllProducts();
  bool updateProductStock(int productId, double quantity, bool add = true);

  // Categories
  QSqlQuery getCategories();
  bool addCategory(const QString &name);
  bool deleteCategory(int id);

  // Units
  QSqlQuery getUnits();

  // Customers
  QSqlQuery getCustomers(int type = -1, const QString &search = "");
  QSqlQuery getCustomerById(int id);
  bool addCustomer(const QString &name, const QString &phone,
                   const QString &address, int type, double creditLimit,
                   const QString &taxNumber);
  bool updateCustomer(int id, const QString &name, const QString &phone,
                      const QString &address, int type, double creditLimit,
                      const QString &taxNumber);
  bool deleteCustomer(int id);
  bool updateCustomerBalance(int customerId, double amount, bool add = true);

  // Sales
  int createSalesInvoice(int customerId, int stockId, double subtotal,
                         double discountRate, double discountValue,
                         double taxValue, double total, double paid,
                         double remaining, const QString &paymentType,
                         const QString &notes);
  bool addSalesInvoiceDetail(int invoiceId, int productId,
                             const QString &productName, double quantity,
                             double price, double discountRate,
                             double discountValue, double taxRate,
                             double taxValue, double total);
  QSqlQuery getSalesInvoices(const QString &dateFrom = "",
                             const QString &dateTo = "");
  QSqlQuery getSalesInvoiceDetails(int invoiceId);
  int getNextSalesInvoiceNo();

  // Purchases
  int createPurchaseInvoice(int supplierId, int stockId, double subtotal,
                            double discountRate, double discountValue,
                            double taxValue, double total, double paid,
                            double remaining, const QString &supplierInvNo,
                            const QString &paymentType, const QString &notes);
  bool addPurchaseInvoiceDetail(int invoiceId, int productId,
                                const QString &productName, double quantity,
                                double price, double discountRate,
                                double discountValue, double taxRate,
                                double taxValue, double total);
  QSqlQuery getPurchaseInvoices(const QString &dateFrom = "",
                                const QString &dateTo = "");
  QSqlQuery getPurchaseInvoiceDetails(int invoiceId);
  int getNextPurchaseInvoiceNo();

  // Stock
  QSqlQuery getStocks();
  QSqlQuery getStockMovements(int stockId = -1, const QString &dateFrom = "",
                              const QString &dateTo = "");
  bool addStockMovement(int productId, int stockId, const QString &type,
                        double quantity, const QString &refType, int refId,
                        const QString &notes);

  // Expenses
  QSqlQuery getExpenses(const QString &dateFrom = "",
                        const QString &dateTo = "");
  bool addExpense(const QString &category, const QString &description,
                  double amount, const QString &notes);

  // Sales Returns
  QSqlQuery getSalesReturns(const QString &dateFrom = "",
                            const QString &dateTo = "");
  int createSalesReturn(int originalInvoiceId, int customerId, double subtotal,
                        double taxValue, double total, const QString &notes);

  // Purchase Returns
  QSqlQuery getPurchaseReturns(const QString &dateFrom = "",
                               const QString &dateTo = "");
  int createPurchaseReturn(int originalInvoiceId, int supplierId,
                           double subtotal, double taxValue, double total,
                           const QString &notes);

  // Treasury
  QSqlQuery getTreasuryTransactions(const QString &dateFrom = "",
                                    const QString &dateTo = "");
  bool addTreasuryTransaction(const QString &type, double amount,
                              const QString &description,
                              const QString &refType = "", int refId = 0);
  double getTreasuryBalance();

  // Employees
  QSqlQuery getEmployees(const QString &search = "");
  bool addEmployee(const QString &name, const QString &phone,
                   const QString &position, double salary,
                   const QString &startDate);
  bool updateEmployee(int id, const QString &name, const QString &phone,
                      const QString &position, double salary);
  bool deleteEmployee(int id);

  // Attendance
  QSqlQuery getAttendance(const QString &dateFrom = "",
                          const QString &dateTo = "", int employeeId = -1);
  bool addAttendance(int employeeId, const QString &date,
                     const QString &checkIn, const QString &checkOut,
                     const QString &notes);

  // Price Quotes
  QSqlQuery getPriceQuotes(const QString &dateFrom = "",
                           const QString &dateTo = "");
  int createPriceQuote(int customerId, double total, const QString &notes);
  bool addPriceQuoteDetail(int quoteId, int productId, const QString &name,
                           double qty, double price, double total);

  // Stock Transfers
  QSqlQuery getStockTransfers(const QString &dateFrom = "",
                              const QString &dateTo = "");
  int createStockTransfer(int fromStockId, int toStockId, const QString &notes);
  bool addStockTransferDetail(int transferId, int productId,
                              const QString &name, double qty);

  // Bookings
  QSqlQuery getBookings(const QString &dateFrom = "",
                        const QString &dateTo = "");
  int createBooking(int customerId, double total, double deposit,
                    const QString &status, const QString &notes);

  // Salaries
  QSqlQuery getSalaries(int month, int year);
  bool addSalary(int employeeId, int month, int year, double basicSalary,
                 double additions, double deductions, double netSalary,
                 const QString &notes);

  // Vacations
  QSqlQuery getVacations(int employeeId = -1);
  bool addVacation(int employeeId, const QString &type, const QString &dateFrom,
                   const QString &dateTo, const QString &notes);

  // Maintenance
  QSqlQuery getMaintenanceRequests(const QString &status = "");
  int addMaintenanceRequest(const QString &customerName, const QString &phone,
                            const QString &deviceType, const QString &problem,
                            double cost, const QString &status);
  bool updateMaintenanceStatus(int id, const QString &status);

  // Reports
  QSqlQuery getDailySalesReport(const QString &date);
  QSqlQuery getMonthlySalesReport(int year, int month);
  QSqlQuery getTopProducts(int limit = 10);
  QSqlQuery getStockReport();
  QSqlQuery getProfitReport(const QString &dateFrom, const QString &dateTo);
  QSqlQuery getCustomerStatement(int customerId, const QString &dateFrom = "",
                                 const QString &dateTo = "");

  // Settings
  QString getSetting(const QString &key, const QString &defaultValue = "");
  bool setSetting(const QString &key, const QString &value);

  // Dashboard stats
  double getTodaySales();
  double getTodayPurchases();
  double getTodayProfit();
  int getTotalProducts();
  int getLowStockCount();
  double getTodayExpenses();

  // Database type
  bool isMySQL() const { return m_isMySQL; }
  QString dbType() const { return m_isMySQL ? "MySQL" : "SQLite"; }

  // MySQL connection
  bool initializeMySQL(const QString &host, int port,
                       const QString &dbName, const QString &user,
                       const QString &password);

  // Export functions
  bool exportDatabase(const QString &filePath);
  bool exportProducts(const QString &filePath);

private:
  DatabaseManager() = default;
  ~DatabaseManager();
  DatabaseManager(const DatabaseManager &) = delete;
  DatabaseManager &operator=(const DatabaseManager &) = delete;

  bool createTables();
  bool executeSQL(const QString &sql);
  QString adaptSQL(const QString &sql); // Adapt SQL for current DB type

  QSqlDatabase m_db;
  int m_userId = 0;
  QString m_userName;
  QString m_userRole;
  bool m_isMySQL = false;
};
