#pragma once
#include <QComboBox>
#include <QTableWidget>
#include <QWidget>
class MaintenanceWidget : public QWidget {
  Q_OBJECT
public:
  explicit MaintenanceWidget(QWidget *parent = nullptr);
  void refresh();
private slots:
  void onAdd();
  void onUpdateStatus();

private:
  QTableWidget *m_table;
  QComboBox *m_statusFilter;
  void loadData();
};
