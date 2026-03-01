#include "DashboardWidget.h"
#include "database/DatabaseManager.h"
#include "widgets/StatCard.h"
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>
#include <QTableWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QDate>
#include <QSqlQuery>

DashboardWidget::DashboardWidget(QWidget *parent) : QWidget(parent) {
  auto *scroll = new QScrollArea(this);
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  auto *outerLayout = new QVBoxLayout(this);
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->addWidget(scroll);

  auto *content = new QWidget;
  auto *layout = new QVBoxLayout(content);
  layout->setSpacing(14);
  layout->setContentsMargins(8, 8, 8, 8);
  scroll->setWidget(content);

  // === Row 1: Stat cards (6 cards) ===
  auto *cardsLayout = new QGridLayout;
  cardsLayout->setSpacing(10);

  m_salesCard = new StatCard("مبيعات اليوم", "0 ريال", "💰", "#10B981");
  m_purchasesCard = new StatCard("مشتريات اليوم", "0 ريال", "🛒", "#F59E0B");
  m_profitCard = new StatCard("أرباح اليوم", "0 ريال", "📈", "#6C63FF");
  m_productsCard = new StatCard("عدد المنتجات", "0", "📦", "#EF4444");
  m_expensesCard = new StatCard("مصروفات اليوم", "0 ريال", "💸", "#F472B6");
  m_alertsCard = new StatCard("تنبيهات المخزون", "0", "⚠️", "#FB923C");

  cardsLayout->addWidget(m_salesCard, 0, 0);
  cardsLayout->addWidget(m_purchasesCard, 0, 1);
  cardsLayout->addWidget(m_profitCard, 0, 2);
  cardsLayout->addWidget(m_productsCard, 1, 0);
  cardsLayout->addWidget(m_expensesCard, 1, 1);
  cardsLayout->addWidget(m_alertsCard, 1, 2);
  layout->addLayout(cardsLayout);

  // === Row 2: Charts ===
  auto *chartsLayout = new QHBoxLayout;
  chartsLayout->setSpacing(12);

  // Bar chart: Sales last 7 days
  auto *barGroup = new QWidget;
  barGroup->setObjectName("chartContainer");
  auto *barLayout = new QVBoxLayout(barGroup);
  auto *barTitle = new QLabel("📊  مبيعات آخر 7 أيام");
  barTitle->setStyleSheet("font-size: 14px; font-weight: bold; padding: 4px; background: transparent;");
  barLayout->addWidget(barTitle);

  m_salesChartView = new QChartView;
  m_salesChartView->setRenderHint(QPainter::Antialiasing);
  m_salesChartView->setMinimumHeight(220);
  barLayout->addWidget(m_salesChartView);
  chartsLayout->addWidget(barGroup, 2);

  // Pie chart: Category distribution
  auto *pieGroup = new QWidget;
  pieGroup->setObjectName("chartContainer");
  auto *pieLayout = new QVBoxLayout(pieGroup);
  auto *pieTitle = new QLabel("🥧  توزيع المبيعات");
  pieTitle->setStyleSheet("font-size: 14px; font-weight: bold; padding: 4px; background: transparent;");
  pieLayout->addWidget(pieTitle);

  m_pieChartView = new QChartView;
  m_pieChartView->setRenderHint(QPainter::Antialiasing);
  m_pieChartView->setMinimumHeight(220);
  pieLayout->addWidget(m_pieChartView);
  chartsLayout->addWidget(pieGroup, 1);

  layout->addLayout(chartsLayout);

  // === Row 3: Tables ===
  auto *tablesLayout = new QHBoxLayout;
  tablesLayout->setSpacing(12);

  // Recent invoices
  auto *recentGroup = new QWidget;
  recentGroup->setObjectName("statCard");
  auto *recentLayout = new QVBoxLayout(recentGroup);
  auto *recentTitle = new QLabel("📋  آخر الفواتير");
  recentTitle->setStyleSheet("font-size: 15px; font-weight: bold; padding: 8px; background: transparent;");
  recentLayout->addWidget(recentTitle);

  m_recentInvoices = new QTableWidget;
  m_recentInvoices->setColumnCount(5);
  m_recentInvoices->setHorizontalHeaderLabels(
      {"رقم", "التاريخ", "العميل", "الإجمالي", "الحالة"});
  m_recentInvoices->horizontalHeader()->setStretchLastSection(true);
  m_recentInvoices->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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
  lowTitle->setStyleSheet("font-size: 15px; font-weight: bold; padding: 8px; color: #F59E0B; background: transparent;");
  lowLayout->addWidget(lowTitle);

  m_lowStockTable = new QTableWidget;
  m_lowStockTable->setColumnCount(3);
  m_lowStockTable->setHorizontalHeaderLabels({"المنتج", "المتوفر", "الحد الأدنى"});
  m_lowStockTable->horizontalHeader()->setStretchLastSection(true);
  m_lowStockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
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

void DashboardWidget::setupSalesChart() {
  auto *chart = new QChart;
  chart->setBackgroundBrush(QColor("#1E2330"));
  chart->setBackgroundRoundness(8);
  chart->legend()->hide();
  chart->setMargins(QMargins(4, 4, 4, 4));
  chart->setAnimationOptions(QChart::SeriesAnimations);

  auto *set = new QBarSet("مبيعات");
  set->setColor(QColor("#6C63FF"));
  set->setBorderColor(QColor("#6C63FF"));

  QStringList categories;
  auto &db = DatabaseManager::instance();

  for (int i = 6; i >= 0; i--) {
    QDate d = QDate::currentDate().addDays(-i);
    categories << d.toString("MM/dd");

    QSqlQuery q(db.database());
    q.prepare("SELECT COALESCE(SUM(total), 0) FROM SalesInvoices WHERE date(date) = ?");
    q.addBindValue(d.toString("yyyy-MM-dd"));
    q.exec();
    double total = 0;
    if (q.next()) total = q.value(0).toDouble();
    *set << total;
  }

  auto *series = new QBarSeries;
  series->append(set);
  series->setBarWidth(0.6);
  chart->addSeries(series);

  auto *axisX = new QBarCategoryAxis;
  axisX->append(categories);
  axisX->setLabelsColor(QColor("#6B7280"));
  axisX->setGridLineColor(QColor("#2D3139"));
  chart->addAxis(axisX, Qt::AlignBottom);
  series->attachAxis(axisX);

  auto *axisY = new QValueAxis;
  axisY->setLabelsColor(QColor("#6B7280"));
  axisY->setGridLineColor(QColor("#2D3139"));
  axisY->setLabelFormat("%.0f");
  chart->addAxis(axisY, Qt::AlignLeft);
  series->attachAxis(axisY);

  m_salesChartView->setChart(chart);
}

void DashboardWidget::setupPieChart() {
  auto *chart = new QChart;
  chart->setBackgroundBrush(QColor("#1E2330"));
  chart->setBackgroundRoundness(8);
  chart->setMargins(QMargins(4, 4, 4, 4));
  chart->setAnimationOptions(QChart::SeriesAnimations);

  auto *series = new QPieSeries;
  auto &db = DatabaseManager::instance();

  QSqlQuery q(db.database());
  q.exec("SELECT c.name, COALESCE(SUM(d.total), 0) as cat_total "
         "FROM SalesInvoiceDetails d "
         "JOIN Products p ON d.product_id = p.id "
         "JOIN Categories c ON p.category_id = c.id "
         "GROUP BY c.id ORDER BY cat_total DESC LIMIT 5");

  QStringList colors = {"#6C63FF", "#10B981", "#F59E0B", "#EF4444", "#3B82F6"};
  int idx = 0;
  bool hasData = false;

  while (q.next()) {
    QString name = q.value("name").toString();
    double total = q.value("cat_total").toDouble();
    if (total > 0) {
      auto *slice = series->append(name, total);
      slice->setColor(QColor(colors[idx % colors.size()]));
      slice->setLabelVisible(true);
      slice->setLabelColor(QColor("#9CA3AF"));
      slice->setBorderColor(QColor("#1E2330"));
      slice->setBorderWidth(2);
      idx++;
      hasData = true;
    }
  }

  if (!hasData) {
    auto *slice = series->append("لا توجد بيانات", 1);
    slice->setColor(QColor("#374151"));
    slice->setLabelVisible(true);
    slice->setLabelColor(QColor("#6B7280"));
  }

  chart->addSeries(series);
  chart->legend()->setAlignment(Qt::AlignBottom);
  chart->legend()->setLabelColor(QColor("#9CA3AF"));
  chart->legend()->setFont(QFont("Cairo", 10));

  m_pieChartView->setChart(chart);
}

void DashboardWidget::refresh() {
  auto &db = DatabaseManager::instance();

  m_salesCard->setValue(QString::number(db.getTodaySales(), 'f', 2) + " ريال");
  m_purchasesCard->setValue(QString::number(db.getTodayPurchases(), 'f', 2) + " ريال");
  m_profitCard->setValue(QString::number(db.getTodayProfit(), 'f', 2) + " ريال");
  m_productsCard->setValue(QString::number(db.getTotalProducts()));
  m_expensesCard->setValue(QString::number(db.getTodayExpenses(), 'f', 2) + " ريال");
  m_alertsCard->setValue(QString::number(db.getLowStockCount()));

  // Charts
  setupSalesChart();
  setupPieChart();

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
    m_recentInvoices->setItem(row, 0, makeItem(q.value("invoice_no").toString()));
    m_recentInvoices->setItem(row, 1, makeItem(q.value("date").toDateTime().toString("yyyy-MM-dd")));
    m_recentInvoices->setItem(row, 2, makeItem(q.value("customer_name").toString()));
    m_recentInvoices->setItem(row, 3,
        makeItem(QString::number(q.value("total").toDouble(), 'f', 2)));
    m_recentInvoices->setItem(row, 4,
        makeItem(q.value("status").toString() == "completed" ? "✅ مكتملة" : "⏳ معلقة"));
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
      m_lowStockTable->setItem(row, 1, makeItem(QString::number(stock, 'f', 0)));
      auto *minItem = makeItem(QString::number(minQty, 'f', 0));
      minItem->setForeground(QColor("#EF4444"));
      m_lowStockTable->setItem(row, 2, minItem);
    }
  }
}
