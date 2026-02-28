#pragma once
#include <QComboBox>
#include <QTableWidget>
#include <QWidget>
class VacationWidget : public QWidget {
  Q_OBJECT
public:
  explicit VacationWidget(QWidget *parent = nullptr);
  void refresh();
private slots:
  void onAdd();

private:
  QTableWidget *m_table;
  QComboBox *m_empCombo;
  void loadData();
};
