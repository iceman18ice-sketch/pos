#pragma once
#include <QTableWidget>
class InvoiceTable : public QTableWidget {
  Q_OBJECT
public:
  explicit InvoiceTable(QWidget *parent = nullptr);
  void addItem(int productId, const QString &name, double qty, double price,
               double taxRate);
  void removeSelectedItem();
  double getSubtotal() const;
  double getTaxTotal() const;
  double getGrandTotal() const;
  struct ItemData {
    int productId;
    QString name;
    double qty, price, discount, taxRate, taxValue, total;
  };
  QList<ItemData> getAllItems() const;
  void clearAll();
  bool removeItemByProductId(int productId);
};
