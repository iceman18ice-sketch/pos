#pragma once
#include <QTableWidget>
#include <QWidget>

class EmployeesWidget : public QWidget {
  Q_OBJECT
public:
  explicit EmployeesWidget(QWidget *parent = nullptr);
  void refresh();
private slots:
  void onAdd();
  void onEdit();
  void onDelete();

private:
  QTableWidget *m_table;
  void loadData();
  void showDialog(int id = 0);
};
