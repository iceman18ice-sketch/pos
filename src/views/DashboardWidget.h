#pragma once
#include <QWidget>

class QChartView;
class StatCard;
class QTableWidget;

class DashboardWidget : public QWidget {
  Q_OBJECT
public:
  explicit DashboardWidget(QWidget *parent = nullptr);
  void refresh();

private:
  StatCard *m_salesCard;
  StatCard *m_purchasesCard;
  StatCard *m_profitCard;
  StatCard *m_productsCard;
  StatCard *m_expensesCard;
  StatCard *m_alertsCard;
  QTableWidget *m_recentInvoices;
  QTableWidget *m_lowStockTable;
  QChartView *m_salesChartView;
  QChartView *m_pieChartView;

  void setupSalesChart();
  void setupPieChart();
};
