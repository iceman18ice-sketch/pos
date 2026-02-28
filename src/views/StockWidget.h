#pragma once
#include <QTableWidget>
#include <QWidget>

class StockWidget : public QWidget {
  Q_OBJECT
public:
  explicit StockWidget(QWidget *parent = nullptr);
  void refresh();

private:
  QTableWidget *m_stockTable;
  QTableWidget *m_movementsTable;
  void loadStockReport();
  void loadMovements();
};
