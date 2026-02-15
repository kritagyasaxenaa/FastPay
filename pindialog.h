#ifndef PINDIALOG_H
#define PINDIALOG_H

#include <QDialog>

class QLineEdit;
class QLabel;

class PinDialog : public QDialog
{
    Q_OBJECT
public:
    enum Mode { VerifyPin, SetPin, ChangePin };

    explicit PinDialog(Mode mode, QWidget *parent = nullptr);

    QString pin() const;
    QString newPin() const;  // for SetPin/ChangePin: second field

    static QString askPin(QWidget *parent, const QString &title = QString());
    static bool askSetPin(QWidget *parent, QString &outPin);
    static bool askChangePin(QWidget *parent, class TransactionEngine *engine);

private:
    Mode m_mode = VerifyPin;
    QLineEdit *m_pinEdit = nullptr;
    QLineEdit *m_confirmEdit = nullptr;
    QLineEdit *m_confirmNewEdit = nullptr;  // for ChangePin only
    QLabel *m_errorLabel = nullptr;
};

#endif // PINDIALOG_H
