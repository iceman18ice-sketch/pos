#include "StatCard.h"
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>


StatCard::StatCard(const QString &title, const QString &value,
                   const QString &icon, const QString &colorStyle,
                   QWidget *parent)
    : QWidget(parent) {
  setObjectName("statCard");
  if (!colorStyle.isEmpty()) {
    setProperty("cardType", colorStyle);
    setStyleSheet(
        QString("#statCard { border-left: 4px solid %1; }").arg(colorStyle));
  }

  auto *shadow = new QGraphicsDropShadowEffect;
  shadow->setBlurRadius(20);
  shadow->setColor(QColor(0, 0, 0, 60));
  shadow->setOffset(0, 4);
  setGraphicsEffect(shadow);

  auto *mainLayout = new QHBoxLayout(this);
  mainLayout->setContentsMargins(20, 16, 20, 16);

  auto *textLayout = new QVBoxLayout;

  m_titleLabel = new QLabel(title);
  m_titleLabel->setObjectName("statCardTitle");
  textLayout->addWidget(m_titleLabel);

  m_valueLabel = new QLabel(value);
  m_valueLabel->setObjectName("statCardValue");
  textLayout->addWidget(m_valueLabel);

  mainLayout->addLayout(textLayout);
  mainLayout->addStretch();

  m_iconLabel = new QLabel(icon);
  m_iconLabel->setStyleSheet("font-size: 36px; background: transparent;");
  mainLayout->addWidget(m_iconLabel);
}

void StatCard::setValue(const QString &value) { m_valueLabel->setText(value); }

void StatCard::setTitle(const QString &title) { m_titleLabel->setText(title); }
