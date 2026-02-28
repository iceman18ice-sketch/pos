#pragma once
#include <QDateEdit>
#include <QLabel>
#include <QTableWidget>
#include <QWidget>

class TreasuryWidget : public QWidget {
  Q_OBJECT
public:
  explicit TreasuryWidget(QWidget *parent = nullptr);
  void refresh();
private slots:
  void onCashIn();
  void onCashOut();

private:
  QTableWidget *m_table;
  QDateEdit *m_dateFrom, *m_dateTo;
  QLabel *m_balanceLabel;
  void loadData();
};
