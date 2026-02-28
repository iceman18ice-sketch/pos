#pragma once
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>


class LoginWindow : public QWidget {
  Q_OBJECT
public:
  explicit LoginWindow(QWidget *parent = nullptr);

signals:
  void loginSuccessful();

private slots:
  void onLoginClicked();

private:
  QLineEdit *m_usernameEdit;
  QLineEdit *m_passwordEdit;
  QPushButton *m_loginBtn;
  QLabel *m_errorLabel;
};
