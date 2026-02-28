#pragma once
#include <QComboBox>
#include <QDateEdit>
#include <QTableWidget>
#include <QWidget>
class StockTransferWidget : public QWidget {
  Q_OBJECT
public:
  explicit StockTransferWidget(QWidget *parent = nullptr);
  void refresh();
private slots:
  void onAdd();

private:
  QTableWidget *m_table;
  QDateEdit *m_dateFrom, *m_dateTo;
  void loadData();
};
