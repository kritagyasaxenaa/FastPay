#include "pindialog.h"
#include "transactionengine.h"
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>

PinDialog::PinDialog(Mode mode, QWidget *parent) : QDialog(parent), m_mode(mode)
{
    setWindowTitle(mode == VerifyPin ? tr("Enter UPI PIN") : (mode == SetPin ? tr("Set UPI PIN") : tr("Change UPI PIN")));
    setMinimumWidth(280);

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_pinEdit = new QLineEdit(this);
    m_pinEdit->setEchoMode(QLineEdit::Password);
    m_pinEdit->setMaxLength(6);
    m_pinEdit->setPlaceholderText(tr("4–6 digits"));
    m_pinEdit->setObjectName("pinEdit");

    if (mode == VerifyPin) {
        layout->addWidget(new QLabel(tr("Enter your UPI PIN to approve:")));
        layout->addWidget(m_pinEdit);
    } else if (mode == SetPin) {
        layout->addWidget(new QLabel(tr("Choose a 4–6 digit UPI PIN:")));
        layout->addWidget(m_pinEdit);
        m_confirmEdit = new QLineEdit(this);
        m_confirmEdit->setEchoMode(QLineEdit::Password);
        m_confirmEdit->setMaxLength(6);
        m_confirmEdit->setPlaceholderText(tr("Re-enter PIN"));
        layout->addWidget(new QLabel(tr("Confirm PIN:")));
        layout->addWidget(m_confirmEdit);
    } else {
        layout->addWidget(new QLabel(tr("Current PIN:")));
        layout->addWidget(m_pinEdit);
        m_confirmEdit = new QLineEdit(this);
        m_confirmEdit->setEchoMode(QLineEdit::Password);
        m_confirmEdit->setMaxLength(6);
        m_confirmEdit->setPlaceholderText(tr("New PIN"));
        layout->addWidget(new QLabel(tr("New PIN:")));
        layout->addWidget(m_confirmEdit);
        m_confirmNewEdit = new QLineEdit(this);
        m_confirmNewEdit->setEchoMode(QLineEdit::Password);
        m_confirmNewEdit->setMaxLength(6);
        m_confirmNewEdit->setPlaceholderText(tr("Re-enter new PIN"));
        layout->addWidget(new QLabel(tr("Confirm new PIN:")));
        layout->addWidget(m_confirmNewEdit);
    }

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: #c62828;");
    m_errorLabel->setWordWrap(true);
    layout->addWidget(m_errorLabel);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(box, &QDialogButtonBox::accepted, this, [this]() {
        m_errorLabel->clear();
        if (m_mode == VerifyPin) {
            if (m_pinEdit->text().length() < 4) {
                m_errorLabel->setText(tr("PIN must be 4–6 digits."));
                return;
            }
            accept();
            return;
        }
        if (m_mode == SetPin) {
            if (m_pinEdit->text().length() < 4 || m_pinEdit->text().length() > 6) {
                m_errorLabel->setText(tr("PIN must be 4–6 digits."));
                return;
            }
            if (m_pinEdit->text() != m_confirmEdit->text()) {
                m_errorLabel->setText(tr("PINs do not match."));
                return;
            }
            accept();
            return;
        }
        if (m_mode == ChangePin) {
            if (m_pinEdit->text().length() < 4 || m_confirmEdit->text().length() < 4) {
                m_errorLabel->setText(tr("PIN must be 4–6 digits."));
                return;
            }
            if (m_confirmEdit->text() != m_confirmNewEdit->text()) {
                m_errorLabel->setText(tr("New PINs do not match."));
                return;
            }
            accept();
        }
    });
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(box);
}

QString PinDialog::pin() const
{
    return m_pinEdit->text();
}

QString PinDialog::newPin() const
{
    return m_confirmEdit ? m_confirmEdit->text() : m_pinEdit->text();
}

QString PinDialog::askPin(QWidget *parent, const QString &title)
{
    PinDialog dlg(VerifyPin, parent);
    if (!title.isEmpty()) dlg.setWindowTitle(title);
    return dlg.exec() == QDialog::Accepted ? dlg.pin() : QString();
}

bool PinDialog::askSetPin(QWidget *parent, QString &outPin)
{
    PinDialog dlg(SetPin, parent);
    if (dlg.exec() != QDialog::Accepted) return false;
    outPin = dlg.newPin();
    return true;
}

bool PinDialog::askChangePin(QWidget *parent, TransactionEngine *engine)
{
    if (!engine) return false;
    PinDialog dlg(ChangePin, parent);
    if (dlg.exec() != QDialog::Accepted) return false;
    return engine->changePin(dlg.pin(), dlg.newPin());
}
