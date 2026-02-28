#pragma once
#include <QDateTime>
#include <QList>
#include <QString>


struct InvoiceItem {
  int productId = 0;
  QString productName;
  double quantity = 1;
  double price = 0;
  double discountRate = 0;
  double discountValue = 0;
  double taxRate = 15;
  double taxValue = 0;
  double total = 0;
};

struct Invoice {
  int id = 0;
  int invoiceNo = 0;
  QDateTime date;
  int customerId = 1;
  int stockId = 1;
  double subtotal = 0;
  double discountRate = 0;
  double discountValue = 0;
  double taxValue = 0;
  double total = 0;
  double paid = 0;
  double remaining = 0;
  QString paymentType = "cash";
  QString status = "completed";
  int userId = 0;
  QString notes;
  QString customerName;
  QList<InvoiceItem> items;
};
