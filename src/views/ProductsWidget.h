#pragma once
#include <QComboBox>
#include <QLabel>
#include <QTableWidget>
#include <QTreeWidget>
#include <QWidget>

class ProductsWidget : public QWidget {
  Q_OBJECT
public:
  explicit ProductsWidget(QWidget *parent = nullptr);
  void refresh();
private slots:
  void onAdd();
  void onEdit();
  void onDelete();
  void onSearch(const QString &text);
  void onCategoryFilter();

private:
  QTableWidget *m_table;
  QComboBox *m_categoryCombo;
  QTreeWidget *m_categoryTree;
  void loadProducts(const QString &search = "", int categoryId = -1);
  void loadCategories();
  void showProductDialog(int productId = -1);
  QString getImagesDir() const;
};
