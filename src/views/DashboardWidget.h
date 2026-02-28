#pragma once
#include "widgets/StatCard.h"
#include <QTableWidget>
#include <QWidget>

class DashboardWidget : public QWidget {
  Q_OBJECT
public:
  explicit DashboardWidget(QWidget *parent = nullptr);
  void refresh();

private:
  StatCard *m_salesCard, *m_purchasesCard, *m_profitCard, *m_productsCard;
  QTableWidget *m_recentInvoices;
  QTableWidget *m_lowStockTable;
};
