#pragma once
#include "widgets/InvoiceTable.h"
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QWidget>

class PurchasesWidget : public QWidget {
  Q_OBJECT
public:
  explicit PurchasesWidget(QWidget *parent = nullptr);

protected:
  void keyPressEvent(QKeyEvent *event) override;
private slots:
  void onSearchProduct();
  void onSaveInvoice();
  void onNewInvoice();
  void updateTotals();
  void onAddNewProduct();
  void onUploadInvoice();

private:
  QLineEdit *m_searchEdit;
  QComboBox *m_supplierCombo, *m_paymentCombo, *m_stockCombo;
  InvoiceTable *m_table;
  QLabel *m_subtotalLabel, *m_taxLabel, *m_totalLabel, *m_invoiceNoLabel;
  QLabel *m_discountTotalLabel, *m_afterDiscountLabel, *m_remainingLabel;
  QLineEdit *m_paidEdit, *m_notesEdit, *m_supplierInvEdit, *m_discountEdit;
  int m_invoiceNo = 0;
  void loadSuppliers();
  void loadStocks();
};
