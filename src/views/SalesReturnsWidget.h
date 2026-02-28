#pragma once
#include <QDateEdit>
#include <QTableWidget>
#include <QWidget>

class SalesReturnsWidget : public QWidget {
  Q_OBJECT
public:
  explicit SalesReturnsWidget(QWidget *parent = nullptr);
  void refresh();
private slots:
  void onAdd();

private:
  QTableWidget *m_table;
  QDateEdit *m_dateFrom, *m_dateTo;
  void loadData();
};
