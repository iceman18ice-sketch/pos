#include "ReportsWidget.h"
#include "database/DatabaseManager.h"
#include <QDate>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

ReportsWidget::ReportsWidget(QWidget *parent) : QWidget(parent) {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(12);
  setLayoutDirection(Qt::RightToLeft);

  auto *title = new QLabel("📊  التقارير");
  title->setStyleSheet(
      "font-size: 20px; font-weight: bold; background: transparent;");
  layout->addWidget(title);

  // Date filter
  auto *filterBar = new QHBoxLayout;
  auto *fromLabel = new QLabel("من:");
  fromLabel->setStyleSheet("background: transparent;");
  filterBar->addWidget(fromLabel);
  m_dateFrom = new QDateEdit(QDate::currentDate().addMonths(-1));
  m_dateFrom->setCalendarPopup(true);
  m_dateFrom->setDisplayFormat("yyyy-MM-dd");
  filterBar->addWidget(m_dateFrom);

  auto *toLabel = new QLabel("إلى:");
  toLabel->setStyleSheet("background: transparent;");
  filterBar->addWidget(toLabel);
  m_dateTo = new QDateEdit(QDate::currentDate());
  m_dateTo->setCalendarPopup(true);
  m_dateTo->setDisplayFormat("yyyy-MM-dd");
  filterBar->addWidget(m_dateTo);
  filterBar->addStretch();

  auto *salesBtn = new QPushButton("📋 تقرير المبيعات");
  salesBtn->setCursor(Qt::PointingHandCursor);
  filterBar->addWidget(salesBtn);
  auto *purchBtn = new QPushButton("🛒 تقرير المشتريات");
  purchBtn->setObjectName("btnWarning");
  purchBtn->setCursor(Qt::PointingHandCursor);
  filterBar->addWidget(purchBtn);
  auto *profitBtn = new QPushButton("💰 تقرير الأرباح");
  profitBtn->setObjectName("btnSuccess");
  profitBtn->setCursor(Qt::PointingHandCursor);
  filterBar->addWidget(profitBtn);
  auto *stockBtn = new QPushButton("📦 تقرير المخزون");
  stockBtn->setObjectName("btnOutline");
  stockBtn->setCursor(Qt::PointingHandCursor);
  filterBar->addWidget(stockBtn);
  auto *expBtn = new QPushButton("💸 تقرير المصروفات");
  expBtn->setCursor(Qt::PointingHandCursor);
  expBtn->setStyleSheet(
      "QPushButton { padding: 6px 14px; border: 2px solid #EF4444; "
      "color: #EF4444; border-radius: 6px; background: transparent; }"
      "QPushButton:hover { background: rgba(239,68,68,0.15); }");
  filterBar->addWidget(expBtn);
  auto *custBtn = new QPushButton("👥 تقرير العملاء");
  custBtn->setCursor(Qt::PointingHandCursor);
  custBtn->setStyleSheet(
      "QPushButton { padding: 6px 14px; border: 2px solid #8B5CF6; "
      "color: #8B5CF6; border-radius: 6px; background: transparent; }"
      "QPushButton:hover { background: rgba(139,92,246,0.15); }");
  filterBar->addWidget(custBtn);

  layout->addLayout(filterBar);

  m_table = new QTableWidget;
  m_table->setAlternatingRowColors(true);
  m_table->verticalHeader()->hide();
  m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_table->setLayoutDirection(Qt::RightToLeft);
  m_table->horizontalHeader()->setStretchLastSection(true);
  layout->addWidget(m_table, 1);

  connect(salesBtn, &QPushButton::clicked, this,
          &ReportsWidget::loadSalesReport);
  connect(purchBtn, &QPushButton::clicked, this,
          &ReportsWidget::loadPurchasesReport);
  connect(profitBtn, &QPushButton::clicked, this,
          &ReportsWidget::loadProfitReport);
  connect(stockBtn, &QPushButton::clicked, this,
          &ReportsWidget::loadStockReport);
  connect(expBtn, &QPushButton::clicked, this,
          &ReportsWidget::loadExpensesReport);
  connect(custBtn, &QPushButton::clicked, this,
          &ReportsWidget::loadCustomersReport);

  loadSalesReport();
}

void ReportsWidget::loadSalesReport() {
  m_table->clear();
  m_table->setColumnCount(7);
  m_table->setHorizontalHeaderLabels({"رقم الفاتورة", "التاريخ", "العميل",
                                      "المجموع", "الضريبة", "الإجمالي",
                                      "الحالة"});
  m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  m_table->setRowCount(0);

  auto q = DatabaseManager::instance().getSalesInvoices(
      m_dateFrom->date().toString("yyyy-MM-dd"),
      m_dateTo->date().toString("yyyy-MM-dd"));
  double totalSum = 0;
  while (q.next()) {
    int r = m_table->rowCount();
    m_table->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    m_table->setItem(r, 0, mi(q.value("invoice_no").toString()));
    m_table->setItem(r, 1,
                     mi(q.value("date").toDateTime().toString("yyyy-MM-dd")));
    m_table->setItem(r, 2, mi(q.value("customer_name").toString()));
    m_table->setItem(
        r, 3, mi(QString::number(q.value("subtotal").toDouble(), 'f', 2)));
    m_table->setItem(
        r, 4, mi(QString::number(q.value("tax_value").toDouble(), 'f', 2)));
    m_table->setItem(r, 5,
                     mi(QString::number(q.value("total").toDouble(), 'f', 2)));
    m_table->setItem(
        r, 6,
        mi(q.value("status").toString() == "completed" ? "مكتملة" : "معلقة"));
    totalSum += q.value("total").toDouble();
  }
  int r = m_table->rowCount();
  m_table->insertRow(r);
  auto *totalItem = new QTableWidgetItem("الإجمالي");
  totalItem->setTextAlignment(Qt::AlignCenter);
  totalItem->setForeground(QColor("#10B981"));
  m_table->setItem(r, 2, totalItem);
  auto *totalVal = new QTableWidgetItem(QString::number(totalSum, 'f', 2));
  totalVal->setTextAlignment(Qt::AlignCenter);
  totalVal->setForeground(QColor("#10B981"));
  m_table->setItem(r, 5, totalVal);
}

void ReportsWidget::loadPurchasesReport() {
  m_table->clear();
  m_table->setColumnCount(6);
  m_table->setHorizontalHeaderLabels(
      {"رقم الفاتورة", "التاريخ", "المورد", "المجموع", "الضريبة", "الإجمالي"});
  m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  m_table->setRowCount(0);

  auto q = DatabaseManager::instance().getPurchaseInvoices(
      m_dateFrom->date().toString("yyyy-MM-dd"),
      m_dateTo->date().toString("yyyy-MM-dd"));
  double totalSum = 0;
  while (q.next()) {
    int r = m_table->rowCount();
    m_table->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    m_table->setItem(r, 0, mi(q.value("invoice_no").toString()));
    m_table->setItem(r, 1,
                     mi(q.value("date").toDateTime().toString("yyyy-MM-dd")));
    m_table->setItem(r, 2, mi(q.value("supplier_name").toString()));
    m_table->setItem(
        r, 3, mi(QString::number(q.value("subtotal").toDouble(), 'f', 2)));
    m_table->setItem(
        r, 4, mi(QString::number(q.value("tax_value").toDouble(), 'f', 2)));
    m_table->setItem(r, 5,
                     mi(QString::number(q.value("total").toDouble(), 'f', 2)));
    totalSum += q.value("total").toDouble();
  }
  int r = m_table->rowCount();
  m_table->insertRow(r);
  auto *ti = new QTableWidgetItem("الإجمالي");
  ti->setTextAlignment(Qt::AlignCenter);
  ti->setForeground(QColor("#F59E0B"));
  m_table->setItem(r, 2, ti);
  auto *tv = new QTableWidgetItem(QString::number(totalSum, 'f', 2));
  tv->setTextAlignment(Qt::AlignCenter);
  tv->setForeground(QColor("#F59E0B"));
  m_table->setItem(r, 5, tv);
}

void ReportsWidget::loadProfitReport() {
  m_table->clear();
  m_table->setColumnCount(4);
  m_table->setHorizontalHeaderLabels(
      {"التاريخ", "الإيرادات", "التكلفة", "الربح"});
  m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  m_table->setRowCount(0);

  auto q = DatabaseManager::instance().getProfitReport(
      m_dateFrom->date().toString("yyyy-MM-dd"),
      m_dateTo->date().toString("yyyy-MM-dd"));
  double totalProfit = 0;
  while (q.next()) {
    int r = m_table->rowCount();
    m_table->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    m_table->setItem(r, 0, mi(q.value("day").toString()));
    m_table->setItem(
        r, 1, mi(QString::number(q.value("revenue").toDouble(), 'f', 2)));
    m_table->setItem(r, 2,
                     mi(QString::number(q.value("cost").toDouble(), 'f', 2)));
    double profit = q.value("profit").toDouble();
    auto *profitItem = mi(QString::number(profit, 'f', 2));
    profitItem->setForeground(profit >= 0 ? QColor("#10B981")
                                          : QColor("#EF4444"));
    m_table->setItem(r, 3, profitItem);
    totalProfit += profit;
  }
  int r = m_table->rowCount();
  m_table->insertRow(r);
  auto *ti = new QTableWidgetItem("إجمالي الربح");
  ti->setTextAlignment(Qt::AlignCenter);
  ti->setForeground(QColor("#6C63FF"));
  m_table->setItem(r, 0, ti);
  auto *tv = new QTableWidgetItem(QString::number(totalProfit, 'f', 2));
  tv->setTextAlignment(Qt::AlignCenter);
  tv->setForeground(totalProfit >= 0 ? QColor("#10B981") : QColor("#EF4444"));
  m_table->setItem(r, 3, tv);
}

void ReportsWidget::loadStockReport() {
  m_table->clear();
  m_table->setColumnCount(5);
  m_table->setHorizontalHeaderLabels(
      {"المنتج", "التصنيف", "المتوفر", "سعر الشراء", "قيمة المخزون"});
  m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  m_table->setRowCount(0);

  auto q = DatabaseManager::instance().getStockReport();
  double totalValue = 0;
  while (q.next()) {
    int r = m_table->rowCount();
    m_table->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    m_table->setItem(r, 0, mi(q.value("name").toString()));
    m_table->setItem(r, 1, mi(q.value("category_name").toString()));
    m_table->setItem(
        r, 2, mi(QString::number(q.value("current_stock").toDouble(), 'f', 0)));
    m_table->setItem(
        r, 3, mi(QString::number(q.value("buy_price").toDouble(), 'f', 2)));
    m_table->setItem(
        r, 4, mi(QString::number(q.value("stock_value").toDouble(), 'f', 2)));
    totalValue += q.value("stock_value").toDouble();
  }
  int r = m_table->rowCount();
  m_table->insertRow(r);
  auto *ti = new QTableWidgetItem("إجمالي قيمة المخزون");
  ti->setTextAlignment(Qt::AlignCenter);
  ti->setForeground(QColor("#6C63FF"));
  m_table->setItem(r, 0, ti);
  auto *tv = new QTableWidgetItem(QString::number(totalValue, 'f', 2));
  tv->setTextAlignment(Qt::AlignCenter);
  tv->setForeground(QColor("#10B981"));
  m_table->setItem(r, 4, tv);
}

void ReportsWidget::loadExpensesReport() {
  m_table->clear();
  m_table->setColumnCount(5);
  m_table->setHorizontalHeaderLabels(
      {"رقم", "التاريخ", "الوصف", "الفئة", "المبلغ"});
  m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  m_table->setRowCount(0);

  auto q = DatabaseManager::instance().getExpenses(
      m_dateFrom->date().toString("yyyy-MM-dd"),
      m_dateTo->date().toString("yyyy-MM-dd"));
  double totalExp = 0;
  while (q.next()) {
    int r = m_table->rowCount();
    m_table->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    m_table->setItem(r, 0, mi(q.value("id").toString()));
    m_table->setItem(r, 1,
                     mi(q.value("date").toDateTime().toString("yyyy-MM-dd")));
    m_table->setItem(r, 2, mi(q.value("description").toString()));
    m_table->setItem(r, 3, mi(q.value("category").toString()));
    double amount = q.value("amount").toDouble();
    auto *amtItem = mi(QString::number(amount, 'f', 2));
    amtItem->setForeground(QColor("#EF4444"));
    m_table->setItem(r, 4, amtItem);
    totalExp += amount;
  }
  int r = m_table->rowCount();
  m_table->insertRow(r);
  auto *ti = new QTableWidgetItem("إجمالي المصروفات");
  ti->setTextAlignment(Qt::AlignCenter);
  ti->setForeground(QColor("#EF4444"));
  m_table->setItem(r, 2, ti);
  auto *tv = new QTableWidgetItem(QString::number(totalExp, 'f', 2));
  tv->setTextAlignment(Qt::AlignCenter);
  tv->setForeground(QColor("#EF4444"));
  m_table->setItem(r, 4, tv);
}

void ReportsWidget::loadCustomersReport() {
  m_table->clear();
  m_table->setColumnCount(5);
  m_table->setHorizontalHeaderLabels(
      {"رقم", "الاسم", "الهاتف", "النوع", "الرصيد"});
  m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_table->setRowCount(0);

  auto q = DatabaseManager::instance().getCustomers(0);
  double totalBalance = 0;
  while (q.next()) {
    int r = m_table->rowCount();
    m_table->insertRow(r);
    auto mi = [](const QString &t) {
      auto *i = new QTableWidgetItem(t);
      i->setTextAlignment(Qt::AlignCenter);
      return i;
    };
    m_table->setItem(r, 0, mi(q.value("id").toString()));
    m_table->setItem(r, 1, mi(q.value("name").toString()));
    m_table->setItem(r, 2, mi(q.value("phone").toString()));
    m_table->setItem(
        r, 3, mi(q.value("type").toString() == "customer" ? "عميل" : "مورد"));
    double balance = q.value("balance").toDouble();
    auto *balItem = mi(QString::number(balance, 'f', 2));
    balItem->setForeground(balance > 0 ? QColor("#EF4444") : QColor("#10B981"));
    m_table->setItem(r, 4, balItem);
    totalBalance += balance;
  }
  int r = m_table->rowCount();
  m_table->insertRow(r);
  auto *ti = new QTableWidgetItem("إجمالي الأرصدة");
  ti->setTextAlignment(Qt::AlignCenter);
  ti->setForeground(QColor("#8B5CF6"));
  m_table->setItem(r, 1, ti);
  auto *tv = new QTableWidgetItem(QString::number(totalBalance, 'f', 2));
  tv->setTextAlignment(Qt::AlignCenter);
  tv->setForeground(totalBalance > 0 ? QColor("#EF4444") : QColor("#10B981"));
  m_table->setItem(r, 4, tv);
}
