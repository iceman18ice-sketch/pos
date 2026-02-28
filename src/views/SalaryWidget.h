#pragma once
#include <QComboBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QWidget>
class SalaryWidget : public QWidget {
  Q_OBJECT
public:
  explicit SalaryWidget(QWidget *parent = nullptr);
  void refresh();
private slots:
  void onAdd();

private:
  QTableWidget *m_table;
  QSpinBox *m_monthSpin, *m_yearSpin;
  void loadData();
};
