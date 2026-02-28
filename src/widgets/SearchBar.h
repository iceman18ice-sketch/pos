#pragma once
#include <QLineEdit>
class SearchBar : public QLineEdit {
  Q_OBJECT
public:
  explicit SearchBar(const QString &placeholder = "بحث...",
                     QWidget *parent = nullptr);
};
