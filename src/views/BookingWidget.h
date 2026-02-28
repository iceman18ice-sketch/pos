#pragma once
#include <QDateEdit>
#include <QTableWidget>
#include <QWidget>
class BookingWidget : public QWidget {
  Q_OBJECT
public:
  explicit BookingWidget(QWidget *parent = nullptr);
  void refresh();
private slots:
  void onAdd();

private:
  QTableWidget *m_table;
  QDateEdit *m_dateFrom, *m_dateTo;
  void loadData();
};
