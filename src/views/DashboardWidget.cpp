#include "DashboardWidget.h"
#include "database/DatabaseManager.h"
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>
#include <QTableWidget>


DashboardWidget::DashboardWidget(QWidget *parent) : QWidget(parent) {
  auto *scroll = new QScrollArea(this);
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  auto *outerLayout = new QVBoxLayout(this);
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->addWidget(scroll);

  auto *content = new QWidget;
  auto *layout = new QVBoxLayout(content);
  layout->setSpacing(16);
  layout->setContentsMargins(8, 8, 8, 8);
  scroll->setWidget(content);

  // Stat cards row
  auto *cardsLayout = new QHBoxLayout;
  cardsLayout->setSpacing(12);

  m_salesCard = new StatCard("مبيعات اليوم", "0 ريال", "💰", "#10B981");
  m_purchasesCard = new StatCard("مشتريات اليوم", "0 ريال", "🛒", "#F59E0B");
  m_profitCard = new StatCard("أرباح اليوم", "0 ريال", "📈", "#6C63FF");
  m_productsCard = new StatCard("عدد المنتجات", "0", "📦", "#EF4444");

  cardsLayout->addWidget(m_salesCard);
  cardsLayout->addWidget(m_purchasesCard);
  cardsLayout->addWidget(m_profitCard);
  cardsLayout->addWidget(m_productsCard);
  layout->addLayout(cardsLayout);

  // Two columns: Recent invoices + Low stock
  auto *tablesLayout = new QHBoxLayout;
  tablesLayout->setSpacing(12);

  // Recent invoices
  auto *recentGroup = new QWidget;
  recentGroup->setObjectName("statCard");
  auto *recentLayout = new QVBoxLayout(recentGroup);
  auto *recentTitle = new QLabel("📋  آخر الفواتير");
  recentTitle->setStyleSheet("font-size: 16px; font-weight: bold; padding: "
                             "8px; background: transparent;");
  recentLayout->addWidget(recentTitle);

  m_recentInvoices = new QTableWidget;
  m_recentInvoices->setColumnCount(5);
  m_recentInvoices->setHorizontalHeaderLabels(
      {"رقم", "التاريخ", "العميل", "الإجمالي", "الحالة"});
  m_recentInvoices->horizontalHeader()->setStretchLastSection(true);
  m_recentInvoices->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  m_recentInvoices->setAlternatingRowColors(true);
  m_recentInvoices->verticalHeader()->hide();
  m_recentInvoices->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_recentInvoices->setLayoutDirection(Qt::RightToLeft);
  recentLayout->addWidget(m_recentInvoices);
  tablesLayout->addWidget(recentGroup, 2);

  // Low stock alerts
  auto *lowGroup = new QWidget;
  lowGroup->setObjectName("statCard");
  auto *lowLayout = new QVBoxLayout(lowGroup);
  auto *lowTitle = new QLabel("⚠️  تنبيهات المخزون");
  lowTitle->setStyleSheet("font-size: 16px; font-weight: bold; padding: 8px; "
                          "color: #F59E0B; background: transparent;");
  lowLayout->addWidget(lowTitle);

  m_lowStockTable = new QTableWidget;
  m_lowStockTable->setColumnCount(3);
  m_lowStockTable->setHorizontalHeaderLabels(
      {"المنتج", "المتوفر", "الحد الأدنى"});
  m_lowStockTable->horizontalHeader()->setStretchLastSection(true);
  m_lowStockTable->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  m_lowStockTable->setAlternatingRowColors(true);
  m_lowStockTable->verticalHeader()->hide();
  m_lowStockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_lowStockTable->setLayoutDirection(Qt::RightToLeft);
  lowLayout->addWidget(m_lowStockTable);
  tablesLayout->addWidget(lowGroup, 1);

  layout->addLayout(tablesLayout);
  layout->addStretch();

  refresh();
}

void DashboardWidget::refresh() {
  auto &db = DatabaseManager::instance();

  m_salesCard->setValue(QString::number(db.getTodaySales(), 'f', 2) + " ريال");
  m_purchasesCard->setValue(QString::number(db.getTodayPurchases(), 'f', 2) +
                            " ريال");
  m_profitCard->setValue(QString::number(db.getTodayProfit(), 'f', 2) +
                         " ريال");
  m_productsCard->setValue(QString::number(db.getTotalProducts()));

  // Recent invoices
  m_recentInvoices->setRowCount(0);
  auto q = db.getSalesInvoices();
  int count = 0;
  while (q.next() && count < 10) {
    int row = m_recentInvoices->rowCount();
    m_recentInvoices->insertRow(row);
    auto makeItem = [](const QString &text) {
      auto *item = new QTableWidgetItem(text);
      item->setTextAlignment(Qt::AlignCenter);
      return item;
    };
    m_recentInvoices->setItem(row, 0,
                              makeItem(q.value("invoice_no").toString()));
    m_recentInvoices->setItem(
        row, 1, makeItem(q.value("date").toDateTime().toString("yyyy-MM-dd")));
    m_recentInvoices->setItem(row, 2,
                              makeItem(q.value("customer_name").toString()));
    m_recentInvoices->setItem(
        row, 3, makeItem(QString::number(q.value("total").toDouble(), 'f', 2)));
    m_recentInvoices->setItem(
        row, 4,
        makeItem(q.value("status").toString() == "completed" ? "مكتملة"
                                                             : "معلقة"));
    count++;
  }

  // Low stock
  m_lowStockTable->setRowCount(0);
  auto sq = db.getStockReport();
  while (sq.next()) {
    double stock = sq.value("current_stock").toDouble();
    double minQty = sq.value("min_quantity").toDouble();
    if (minQty > 0 && stock <= minQty) {
      int row = m_lowStockTable->rowCount();
      m_lowStockTable->insertRow(row);
      auto makeItem = [](const QString &text) {
        auto *item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignCenter);
        return item;
      };
      m_lowStockTable->setItem(row, 0, makeItem(sq.value("name").toString()));
      m_lowStockTable->setItem(row, 1,
                               makeItem(QString::number(stock, 'f', 0)));
      auto *minItem = makeItem(QString::number(minQty, 'f', 0));
      minItem->setForeground(QColor("#EF4444"));
      m_lowStockTable->setItem(row, 2, minItem);
    }
  }
}
