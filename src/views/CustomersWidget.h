#pragma once
#include <QComboBox>
#include <QTableWidget>
#include <QWidget>

class CustomersWidget : public QWidget {
  Q_OBJECT
public:
  explicit CustomersWidget(QWidget *parent = nullptr);
  void refresh();
private slots:
  void onAdd();
  void onEdit();
  void onDelete();
  void onSearch(const QString &text);

private:
  QTableWidget *m_table;
  QComboBox *m_typeCombo;
  void loadCustomers(const QString &search = "", int type = -1);
  void showDialog(int id = -1);
};
