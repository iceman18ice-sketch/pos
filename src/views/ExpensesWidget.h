#pragma once
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QTableWidget>
#include <QWidget>

class ExpensesWidget : public QWidget {
  Q_OBJECT
public:
  explicit ExpensesWidget(QWidget *parent = nullptr);
  void refresh();

private slots:
  void onAdd();
  void onDelete();
  void onFilter();

private:
  QTableWidget *m_table;
  QDateEdit *m_dateFrom, *m_dateTo;
  QComboBox *m_categoryCombo;
  QLabel *m_totalLabel;
  void loadExpenses();
};
