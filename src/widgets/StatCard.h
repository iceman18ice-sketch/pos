#pragma once
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>


class StatCard : public QWidget {
  Q_OBJECT
public:
  explicit StatCard(const QString &title, const QString &value,
                    const QString &icon, const QString &colorStyle = "",
                    QWidget *parent = nullptr);
  void setValue(const QString &value);
  void setTitle(const QString &title);

private:
  QLabel *m_titleLabel;
  QLabel *m_valueLabel;
  QLabel *m_iconLabel;
};
