#pragma once
#include <QComboBox>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>

class SettingsWidget : public QWidget {
  Q_OBJECT
public:
  explicit SettingsWidget(QWidget *parent = nullptr);
private slots:
  void onSave();

private:
  QLineEdit *m_companyPhone, *m_taxRate, *m_currency;
  QComboBox *m_defaultPrinter;

  // ZATCA E-Invoicing
  QLineEdit *m_zatcaVat, *m_zatcaCRN, *m_zatcaCompanyAr, *m_zatcaCompanyEn;
  QLineEdit *m_zatcaStreet, *m_zatcaBuildingNo, *m_zatcaDistrict, *m_zatcaCity;
  QLineEdit *m_zatcaPostalCode, *m_zatcaCountry;
  QComboBox *m_zatcaInvoiceType, *m_zatcaEnvironment;
  QLineEdit *m_zatcaSdkPath, *m_zatcaCertPath, *m_zatcaPrivKeyPath;
  QLabel *m_zatcaStatus;

  void loadSettings();
};
