#include "LoginWindow.h"
#include "database/DatabaseManager.h"
#include <QGraphicsDropShadowEffect>
#include <QKeyEvent>

LoginWindow::LoginWindow(QWidget *parent) : QWidget(parent) {
  setObjectName("loginWindow");
  setMinimumSize(800, 600);
  setWindowTitle("نظام المحاسبة - تسجيل الدخول");

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setAlignment(Qt::AlignCenter);

  // Login Card
  auto *card = new QWidget;
  card->setObjectName("loginCard");
  auto *cardLayout = new QVBoxLayout(card);
  cardLayout->setSpacing(16);
  cardLayout->setContentsMargins(40, 40, 40, 40);

  // Shadow effect
  auto *shadow = new QGraphicsDropShadowEffect;
  shadow->setBlurRadius(40);
  shadow->setColor(QColor(0, 0, 0, 100));
  shadow->setOffset(0, 8);
  card->setGraphicsEffect(shadow);

  // Icon / Logo
  auto *iconLabel = new QLabel("🏪");
  iconLabel->setAlignment(Qt::AlignCenter);
  iconLabel->setStyleSheet("font-size: 60px; background: transparent;");
  cardLayout->addWidget(iconLabel);

  // Title
  auto *titleLabel = new QLabel("نظام المحاسبة");
  titleLabel->setObjectName("loginTitle");
  titleLabel->setAlignment(Qt::AlignCenter);
  cardLayout->addWidget(titleLabel);

  // Subtitle
  auto *subtitleLabel = new QLabel("أدخل بيانات تسجيل الدخول");
  subtitleLabel->setObjectName("loginSubtitle");
  subtitleLabel->setAlignment(Qt::AlignCenter);
  cardLayout->addWidget(subtitleLabel);

  cardLayout->addSpacing(20);

  // Username
  auto *userLabel = new QLabel("اسم المستخدم");
  userLabel->setStyleSheet("color: #9CA3AF; font-size: 12px; font-weight: "
                           "bold; background: transparent;");
  cardLayout->addWidget(userLabel);

  m_usernameEdit = new QLineEdit;
  m_usernameEdit->setObjectName("loginInput");
  m_usernameEdit->setPlaceholderText("أدخل اسم المستخدم...");
  m_usernameEdit->setText("admin");
  m_usernameEdit->setAlignment(Qt::AlignRight);
  cardLayout->addWidget(m_usernameEdit);

  cardLayout->addSpacing(8);

  // Password
  auto *passLabel = new QLabel("كلمة المرور");
  passLabel->setStyleSheet("color: #9CA3AF; font-size: 12px; font-weight: "
                           "bold; background: transparent;");
  cardLayout->addWidget(passLabel);

  m_passwordEdit = new QLineEdit;
  m_passwordEdit->setObjectName("loginInput");
  m_passwordEdit->setPlaceholderText("أدخل كلمة المرور...");
  m_passwordEdit->setText("admin");
  m_passwordEdit->setEchoMode(QLineEdit::Password);
  m_passwordEdit->setAlignment(Qt::AlignRight);
  cardLayout->addWidget(m_passwordEdit);

  // Error label
  m_errorLabel = new QLabel;
  m_errorLabel->setObjectName("loginError");
  m_errorLabel->setAlignment(Qt::AlignCenter);
  m_errorLabel->hide();
  cardLayout->addWidget(m_errorLabel);

  cardLayout->addSpacing(16);

  // Login button
  m_loginBtn = new QPushButton("تسجيل الدخول");
  m_loginBtn->setObjectName("loginBtn");
  m_loginBtn->setCursor(Qt::PointingHandCursor);
  cardLayout->addWidget(m_loginBtn);

  // Version
  auto *versionLabel = new QLabel("الإصدار 1.0.0");
  versionLabel->setObjectName("labelMuted");
  versionLabel->setAlignment(Qt::AlignCenter);
  cardLayout->addWidget(versionLabel);

  mainLayout->addWidget(card);

  // Connections
  connect(m_loginBtn, &QPushButton::clicked, this,
          &LoginWindow::onLoginClicked);
  connect(m_passwordEdit, &QLineEdit::returnPressed, this,
          &LoginWindow::onLoginClicked);
  connect(m_usernameEdit, &QLineEdit::returnPressed,
          [this]() { m_passwordEdit->setFocus(); });

  m_usernameEdit->setFocus();
}

void LoginWindow::onLoginClicked() {
  QString username = m_usernameEdit->text().trimmed();
  QString password = m_passwordEdit->text();

  if (username.isEmpty() || password.isEmpty()) {
    m_errorLabel->setText("يرجى إدخال اسم المستخدم وكلمة المرور");
    m_errorLabel->show();
    return;
  }

  auto &db = DatabaseManager::instance();
  if (db.authenticate(username, password)) {
    emit loginSuccessful();
  } else {
    m_errorLabel->setText("اسم المستخدم أو كلمة المرور غير صحيحة");
    m_errorLabel->show();
    m_passwordEdit->clear();
    m_passwordEdit->setFocus();
  }
}
