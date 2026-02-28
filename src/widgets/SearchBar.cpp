#include "SearchBar.h"
SearchBar::SearchBar(const QString &placeholder, QWidget *parent)
    : QLineEdit(parent) {
  setObjectName("searchBar");
  setPlaceholderText(placeholder);
  setAlignment(Qt::AlignRight);
  setClearButtonEnabled(true);
}
