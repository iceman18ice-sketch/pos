#pragma once
#include <QComboBox>
#include <QLineEdit>
#include <QWidget>

class SettingsWidget : public QWidget {
  Q_OBJECT
public:
  explicit SettingsWidget(QWidget *parent = nullptr);
private slots:
  void onSave();

private:
  QLineEdit *m_companyName, *m_companyPhone, *m_companyAddress, *m_taxNumber,
      *m_taxRate, *m_currency;
  QComboBox *m_defaultPrinter;
  void loadSettings();
};
