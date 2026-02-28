#pragma once
#include "widgets/InvoiceTable.h"
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QTimer>
#include <QWidget>

class SalesWidget : public QWidget {
  Q_OBJECT
public:
  explicit SalesWidget(QWidget *parent = nullptr);
private slots:
  void onSearchProduct();
  void onSaveInvoice();
  void onRemoveItem();
  void onNewInvoice();
  void updateTotals();
  void loadCustomers();
  void onHoldInvoice();
  void onResumeInvoice();
  void onToggleCancelMode();
  void onSaveAndPrint();
  void onSaveAndPreview();
  void onExportPDF();
  void onBrowsePreviousInvoice();

private:
  QLineEdit *m_searchEdit;
  QComboBox *m_customerCombo;
  QComboBox *m_paymentMethodCombo;
  QComboBox *m_warehouseCombo;
  QComboBox *m_treasuryCombo;
  InvoiceTable *m_table;
  QLabel *m_subtotalLabel, *m_taxLabel, *m_totalLabel, *m_invoiceNoLabel;
  QLabel *m_remainingLabel;
  QLineEdit *m_paidEdit, *m_notesEdit, *m_discountEdit;
  int m_invoiceNo = 0;

  // Hold invoice data
  struct HeldInvoice {
    int invoiceNo;
    int customerId;
    QString notes;
    QList<InvoiceTable::ItemData> items;
  };
  QList<HeldInvoice> m_heldInvoices;
  QPushButton *m_holdBtn;
  QPushButton *m_resumeBtn;

  // Cancel mode
  bool m_cancelMode = false;
  QPushButton *m_cancelModeBtn;

  // PDF generation helper
  void generateInvoicePDF(const QString &filePath);
};
