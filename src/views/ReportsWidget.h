#pragma once
#include <QDateEdit>
#include <QTableWidget>
#include <QWidget>

class ReportsWidget : public QWidget {
  Q_OBJECT
public:
  explicit ReportsWidget(QWidget *parent = nullptr);

private:
  QDateEdit *m_dateFrom, *m_dateTo;
  QTableWidget *m_table;
  void loadSalesReport();
  void loadPurchasesReport();
  void loadProfitReport();
  void loadStockReport();
  void loadExpensesReport();
  void loadCustomersReport();
};
