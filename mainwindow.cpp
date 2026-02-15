#include "mainwindow.h"
#include "transactionengine.h"
#include "ultrasoundhelper.h"
#include "transactionhistory.h"
#include "pindialog.h"
#include "server_config.h"
#include <QMessageBox>
#include <QDateTime>
#include <QTableWidget>
#include <QDialog>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QScrollArea>
#include <QFrame>
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_engine = new TransactionEngine(this);
    m_ultrasound = new UltrasoundHelper(this);
    
    // Set server URL from config
    m_engine->setServerBaseUrl(ServerConfig::CURRENT_SERVER);

    connect(m_engine, &TransactionEngine::accountFrozen, this, &MainWindow::onAccountFrozen);
    connect(m_engine, &TransactionEngine::onlineTransactionCompleted, this, &MainWindow::onOnlineTransactionCompleted);
    connect(m_engine, &TransactionEngine::onlineTransactionFailed, this, &MainWindow::onOnlineTransactionFailed);
    connect(m_ultrasound, &UltrasoundHelper::keyReceived, this, &MainWindow::onKeyReceivedFromMic);

    setupUi();
    applyStyleSheet();
    ensurePinSet();
    setWindowTitle("FastPay – Transaction Platform");
    setMinimumSize(420, 620);
}

MainWindow::~MainWindow()
{
}

void MainWindow::applyStyleSheet()
{
    setStyleSheet(R"(
        QMainWindow { background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #0d1117, stop:1 #161b22); }
        QWidget { background: transparent; color: #e6edf3; }
        QLabel { color: #e6edf3; }
        QGroupBox { color: #58a6ff; font-weight: bold; border: 1px solid #30363d; border-radius: 8px; margin-top: 10px; padding-top: 8px; }
        QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }
        QLineEdit, QComboBox {
            background: #21262d; color: #e6edf3; border: 1px solid #30363d; border-radius: 6px;
            padding: 8px; min-height: 20px; selection-background-color: #388bfd;
        }
        QLineEdit:focus { border-color: #58a6ff; }
        QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #238636, stop:1 #2ea043);
            color: white; border: none; border-radius: 8px; padding: 10px 16px; font-weight: bold;
        }
        QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #2ea043, stop:1 #3fb950); }
        QPushButton:pressed { background: #238636; }
        QPushButton:checked {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #1f6feb, stop:1 #388bfd);
        }
        QPushButton#btnOnline:checked, QPushButton#btnOffline:checked { color: white; }
        QPushButton#btnSecondary {
            background: #21262d; color: #58a6ff; border: 1px solid #30363d;
        }
        QPushButton#btnSecondary:hover { background: #30363d; }
        QTabWidget::pane { border: 1px solid #30363d; border-radius: 8px; background: #161b22; margin-top: -1px; }
        QTabBar::tab { background: #21262d; color: #8b949e; padding: 8px 16px; margin-right: 2px; border-top-left-radius: 6px; border-top-right-radius: 6px; }
        QTabBar::tab:selected { background: #161b22; color: #58a6ff; }
        QTabBar::tab:hover:!selected { background: #30363d; }
        QScrollArea { border: none; background: transparent; }
        QTableWidget { background: #21262d; color: #e6edf3; gridline-color: #30363d; border-radius: 8px; }
        QTableWidget::item { padding: 4px; }
        QHeaderView::section { background: #161b22; color: #58a6ff; padding: 8px; }
        QDialog { background: #161b22; }
    )");
}

void MainWindow::ensurePinSet()
{
    if (!m_engine->hasPinSet()) {
        QString pin;
        if (PinDialog::askSetPin(this, pin))
            m_engine->setPin(pin);
    }
}

void MainWindow::setupUi()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(12);

    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *logoLabel = new QLabel(this);
    QPixmap logoPix(":/assets/logo.png");
    if (!logoPix.isNull())
        logoLabel->setPixmap(logoPix.scaled(56, 56, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setFixedSize(60, 60);
    headerRow->addWidget(logoLabel);
    QVBoxLayout *titleCol = new QVBoxLayout();
    QLabel *title = new QLabel("FastPay");
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: #58a6ff;");
    titleCol->addWidget(title);
    QLabel *sub = new QLabel("Online ↔ Online  |  Offline ↔ Offline");
    sub->setStyleSheet("color: #8b949e; font-size: 12px;");
    titleCol->addWidget(sub);
    headerRow->addLayout(titleCol);
    headerRow->addStretch();
    mainLayout->addLayout(headerRow);

    QGroupBox *identityBox = new QGroupBox(tr("Your identity (phone → public key)"));
    QFormLayout *identityForm = new QFormLayout(identityBox);
    QLineEdit *lePhone = new QLineEdit(this);
    lePhone->setPlaceholderText(tr("e.g. 9876543210"));
    lePhone->setObjectName("phoneNumber");
    connect(lePhone, &QLineEdit::textChanged, this, &MainWindow::onPhoneNumberChanged);
    identityForm->addRow(tr("Phone number:"), lePhone);
    QLabel *pubKeyLabel = new QLabel(tr("(Enter phone to generate)"));
    pubKeyLabel->setObjectName("publicKeyLabel");
    pubKeyLabel->setWordWrap(true);
    pubKeyLabel->setStyleSheet("color: #8b949e; font-size: 11px;");
    identityForm->addRow(tr("Public key (hash):"), pubKeyLabel);
    QLineEdit *leServerUrl = new QLineEdit(this);
    leServerUrl->setPlaceholderText(tr("e.g. http://localhost:8000 (optional)"));
    leServerUrl->setObjectName("serverBaseUrl");
    identityForm->addRow(tr("Server URL:"), leServerUrl);
    mainLayout->addWidget(identityBox);

    QHBoxLayout *modeRow = new QHBoxLayout();
    QPushButton *btnOnline = new QPushButton(tr("Online"));
    QPushButton *btnOffline = new QPushButton(tr("Offline"));
    btnOnline->setObjectName("btnOnline");
    btnOffline->setObjectName("btnOffline");
    btnOnline->setCheckable(true);
    btnOffline->setCheckable(true);
    btnOnline->setChecked(true);
    connect(btnOnline, &QPushButton::clicked, this, &MainWindow::onModeOnline);
    connect(btnOffline, &QPushButton::clicked, this, &MainWindow::onModeOffline);
    modeRow->addWidget(btnOnline);
    modeRow->addWidget(btnOffline);
    mainLayout->addLayout(modeRow);

    m_stack = new QStackedWidget(this);
    QWidget *onlinePage = new QWidget;
    QWidget *offlinePage = new QWidget;
    m_stack->addWidget(onlinePage);
    m_stack->addWidget(offlinePage);

    QVBoxLayout *onlineLayout = new QVBoxLayout(onlinePage);
    QTabWidget *onlineTabs = new QTabWidget;
    onlineTabs->addTab(new QWidget, tr("Receive (emit key)"));
    onlineTabs->addTab(new QWidget, tr("Send (capture & pay)"));
    onlineLayout->addWidget(onlineTabs);

    QWidget *recvTab = onlineTabs->widget(0);
    QVBoxLayout *recvLay = new QVBoxLayout(recvTab);
    recvLay->addWidget(new QLabel(tr("Emit ultrasound: header + your public key. Sender captures 2x and extracts key.")));
    QPushButton *btnEmit = new QPushButton(tr("Start emitting ultrasound"));
    connect(btnEmit, &QPushButton::clicked, this, &MainWindow::onOnlineReceiveEmit);
    recvLay->addWidget(btnEmit);
    recvLay->addStretch();

    QWidget *sendTab = onlineTabs->widget(1);
    QFormLayout *sendForm = new QFormLayout(sendTab);
    QLineEdit *leAmount = new QLineEdit;
    leAmount->setPlaceholderText(tr("Amount"));
    leAmount->setObjectName("onlineAmount");
    sendForm->addRow(tr("Amount:"), leAmount);
    QPushButton *btnCapture = new QPushButton(tr("Start listening (mic 2x)"));
    QPushButton *btnSubmit = new QPushButton(tr("Initiate transaction (will ask UPI PIN)"));
    btnSubmit->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #1f6feb, stop:1 #388bfd);");
    connect(btnCapture, &QPushButton::clicked, this, &MainWindow::onOnlineSendCapture);
    connect(btnSubmit, &QPushButton::clicked, this, &MainWindow::onOnlineSendSubmit);
    sendForm->addRow(btnCapture);
    sendForm->addRow(btnSubmit);
    QLabel *nonceLabel = new QLabel;
    nonceLabel->setObjectName("onlineNonceLabel");
    sendForm->addRow(tr("Nonce (timestamp):"), nonceLabel);

    QVBoxLayout *offlineLayout = new QVBoxLayout(offlinePage);
    QTabWidget *offlineTabs = new QTabWidget;
    offlineTabs->addTab(new QWidget, tr("Send (sign)"));
    offlineTabs->addTab(new QWidget, tr("Receive (verify & receipt)"));
    offlineLayout->addWidget(offlineTabs);

    QWidget *offSendTab = offlineTabs->widget(0);
    QFormLayout *offSendForm = new QFormLayout(offSendTab);
    offSendForm->addRow(new QLabel(tr("Cold wallet → cold wallet. Sign with digital signature. UPI PIN required to approve.")));
    QLineEdit *leOffSender = new QLineEdit;
    leOffSender->setPlaceholderText(tr("Sender ID / phone"));
    leOffSender->setObjectName("offlineSenderId");
    QLineEdit *leOffReceiver = new QLineEdit;
    leOffReceiver->setPlaceholderText(tr("Receiver ID / phone"));
    leOffReceiver->setObjectName("offlineReceiverId");
    QLineEdit *leOffAmount = new QLineEdit;
    leOffAmount->setPlaceholderText(tr("Amount"));
    leOffAmount->setObjectName("offlineAmount");
    offSendForm->addRow(tr("Sender:"), leOffSender);
    offSendForm->addRow(tr("Receiver:"), leOffReceiver);
    offSendForm->addRow(tr("Amount:"), leOffAmount);
    QPushButton *btnOffSign = new QPushButton(tr("Sign transaction (will ask UPI PIN)"));
    btnOffSign->setStyleSheet("background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #1f6feb, stop:1 #388bfd);");
    connect(btnOffSign, &QPushButton::clicked, this, &MainWindow::onOfflineSendSign);
    offSendForm->addRow(btnOffSign);

    QWidget *offRecvTab = offlineTabs->widget(1);
    QFormLayout *offRecvForm = new QFormLayout(offRecvTab);
    offRecvForm->addRow(new QLabel(tr("Verify sender signature; emit receipt. On sync, store on server. If verification fails, account is frozen.")));
    QPushButton *btnOffVerify = new QPushButton(tr("Verify & send receipt"));
    connect(btnOffVerify, &QPushButton::clicked, this, &MainWindow::onOfflineReceiveVerify);
    offRecvForm->addRow(btnOffVerify);

    mainLayout->addWidget(m_stack);

    QPushButton *btnHistory = new QPushButton(tr("Transaction history (online + offline)"));
    btnHistory->setObjectName("btnSecondary");
    connect(btnHistory, &QPushButton::clicked, this, &MainWindow::showHistory);
    mainLayout->addWidget(btnHistory);

    showOnlinePanel();
}

void MainWindow::onPhoneNumberChanged()
{
    QLineEdit *le = findChild<QLineEdit*>("phoneNumber");
    QLabel *lbl = findChild<QLabel*>("publicKeyLabel");
    if (!le || !lbl) return;
    QString phone = le->text().trimmed();
    if (phone.isEmpty()) {
        lbl->setText(tr("(Enter phone to generate)"));
        return;
    }
    QString pk = TransactionEngine::publicKeyHexFromPhoneNumber(phone);
    if (pk.isEmpty()) {
        lbl->setText(tr("(Invalid phone)"));
        return;
    }
    lbl->setText(pk.length() > 32 ? (pk.left(24) + "…" + pk.right(8)) : pk);
    lbl->setToolTip(pk);
}

void MainWindow::onModeOnline()
{
    findChild<QPushButton*>("btnOnline")->setChecked(true);
    findChild<QPushButton*>("btnOffline")->setChecked(false);
    m_stack->setCurrentIndex(0);
}

void MainWindow::onModeOffline()
{
    findChild<QPushButton*>("btnOnline")->setChecked(false);
    findChild<QPushButton*>("btnOffline")->setChecked(true);
    m_stack->setCurrentIndex(1);
}

void MainWindow::showOnlinePanel()
{
    m_stack->setCurrentIndex(0);
}

void MainWindow::showOfflinePanel()
{
    m_stack->setCurrentIndex(1);
}

QString MainWindow::currentNonce() const
{
    return TransactionEngine::getTimestampNonce();
}

void MainWindow::onOnlineReceiveEmit()
{
    QLineEdit *lePhone = findChild<QLineEdit*>("phoneNumber");
    QString phone = lePhone ? lePhone->text().trimmed() : QString();
    QByteArray pubKey = TransactionEngine::publicKeyFromPhoneNumber(phone);
    if (pubKey.isEmpty()) pubKey = QByteArray("FASTPAY_DEMO_KEY");
    QByteArray header("FASTPAY_ONLINE_V1");
    QByteArray payload = m_engine->buildOnlineEmitPayload(header, pubKey);
    m_ultrasound->startEmitting(payload);
    QMessageBox::information(this, tr("Online receive"), tr("Emitting ultrasound (header + public key). Sender captures 2x and extracts key."));
}

void MainWindow::onOnlineSendCapture()
{
    m_ultrasound->startListening();
    QLabel *nonceL = findChild<QLabel*>("onlineNonceLabel");
    if (nonceL) nonceL->setText(currentNonce());
    QMessageBox::information(this, tr("Online send"), tr("Listening on mic (buffer 2x). When header is found, key will be extracted."));
}

void MainWindow::onKeyReceivedFromMic(const QByteArray &key)
{
    Q_UNUSED(key);
    QMessageBox::information(this, tr("Key received"), tr("Public key extracted. Enter amount and tap \"Initiate transaction\" to pay with UPI PIN."));
}

void MainWindow::onOnlineSendSubmit()
{
    QLineEdit *amountEdit = findChild<QLineEdit*>("onlineAmount");
    QString amount = amountEdit ? amountEdit->text().trimmed() : QString();
    if (amount.isEmpty()) {
        QMessageBox::warning(this, tr("Amount required"), tr("Enter amount."));
        return;
    }
    QString pin = PinDialog::askPin(this, tr("Enter UPI PIN to approve"));
    if (pin.isEmpty()) return;
    QLineEdit *serverUrlEdit = findChild<QLineEdit*>("serverBaseUrl");
    QString serverUrl = serverUrlEdit ? serverUrlEdit->text().trimmed() : QString();
    if (!serverUrl.isEmpty()) {
        m_engine->setServerBaseUrl(serverUrl);
        m_engine->submitOnlineTransactionToServer("sender@fastpay", "receiver@fastpay", amount, pin);
        // Success/failure and amount clear handled by onOnlineTransactionCompleted / onOnlineTransactionFailed
    } else {
        QByteArray receiverKey("DEMO_KEY");
        bool ok = m_engine->submitOnlineTransaction("sender@fastpay", amount, receiverKey, pin);
        if (ok) {
            QMessageBox::information(this, tr("Online"), tr("Transaction initiated. Receiver gets notification. Nonce: %1").arg(currentNonce()));
            if (amountEdit) amountEdit->clear();
        } else
            QMessageBox::warning(this, tr("Error"), tr("Transaction failed. Check your UPI PIN."));
    }
}

void MainWindow::onOnlineTransactionCompleted(const QString &txId)
{
    QMessageBox::information(this, tr("Online"), tr("Transaction completed. Transaction ID: %1").arg(txId));
    QLineEdit *amountEdit = findChild<QLineEdit*>("onlineAmount");
    if (amountEdit) amountEdit->clear();
}

void MainWindow::onOnlineTransactionFailed(const QString &error)
{
    QMessageBox::warning(this, tr("Error"), error);
}

void MainWindow::onOfflineSendSign()
{
    QLineEdit *senderId = findChild<QLineEdit*>("offlineSenderId");
    QLineEdit *receiverId = findChild<QLineEdit*>("offlineReceiverId");
    QLineEdit *amount = findChild<QLineEdit*>("offlineAmount");
    QString s = senderId ? senderId->text().trimmed() : QString();
    QString r = receiverId ? receiverId->text().trimmed() : QString();
    QString a = amount ? amount->text().trimmed() : QString();
    if (s.isEmpty() || r.isEmpty() || a.isEmpty()) {
        QMessageBox::warning(this, tr("Offline send"), tr("Fill sender, receiver, amount."));
        return;
    }
    QString pin = PinDialog::askPin(this, tr("Enter UPI PIN to approve signing"));
    if (pin.isEmpty()) return;
    if (!m_engine->verifyPin(pin)) {
        QMessageBox::warning(this, tr("Wrong PIN"), tr("UPI PIN incorrect. Transaction not approved."));
        return;
    }
    QString nonce = currentNonce();
    QByteArray sig = m_engine->signOfflineTransaction(s, r, a, nonce, QByteArray());
    if (!sig.isEmpty())
        QMessageBox::information(this, tr("Offline send"), tr("Transaction signed. Nonce: %1. Send to receiver for verification and receipt.").arg(nonce));
}

void MainWindow::onOfflineReceiveVerify()
{
    QString message = "cold_sender@fastpay|cold_receiver@fastpay|50.00|" + currentNonce();
    QByteArray signature = m_engine->signOfflineTransaction("cold_sender@fastpay", "cold_receiver@fastpay", "50.00", currentNonce(), QByteArray());
    bool ok = m_engine->verifyOfflineTransaction(message, signature, QByteArray());
    if (ok) {
        m_engine->signReceipt(message, QByteArray());
        TransactionRecord rec;
        rec.id = QString::number(QDateTime::currentMSecsSinceEpoch());
        rec.type = "offline";
        rec.role = "receiver";
        rec.peerId = "cold_sender@fastpay";
        rec.amount = "50.00";
        rec.nonce = currentNonce();
        rec.status = "completed";
        rec.createdAt = QDateTime::currentDateTime();
        m_engine->submitOfflineWhenOnline(rec);
        QMessageBox::information(this, tr("Offline receive"), tr("Verified. Receipt emitted. When both come online, transaction will be stored on server."));
    } else {
        m_engine->freezeAccountOnVerificationFailure();
        QMessageBox::critical(this, tr("Verification failed"), tr("Transaction failed. Account frozen."));
    }
}

void MainWindow::onAccountFrozen()
{
    QMessageBox::critical(this, tr("Account frozen"), tr("Verification failed. Account is frozen."));
}

void MainWindow::showHistory()
{
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(tr("Transaction history (online + offline)"));
    dlg->setMinimumSize(500, 300);
    QVBoxLayout *lay = new QVBoxLayout(dlg);
    QTableWidget *table = new QTableWidget(0, 7);
    table->setHorizontalHeaderLabels({tr("ID"), tr("Type"), tr("Role"), tr("Peer"), tr("Amount"), tr("Nonce"), tr("Status")});
    QList<TransactionRecord> list = TransactionHistory::load();
    for (const TransactionRecord &r : list) {
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(r.id.left(8)));
        table->setItem(row, 1, new QTableWidgetItem(r.type));
        table->setItem(row, 2, new QTableWidgetItem(r.role));
        table->setItem(row, 3, new QTableWidgetItem(r.peerId));
        table->setItem(row, 4, new QTableWidgetItem(r.amount));
        table->setItem(row, 5, new QTableWidgetItem(r.nonce));
        table->setItem(row, 6, new QTableWidgetItem(r.status));
    }
    lay->addWidget(table);
    QPushButton *closeBtn = new QPushButton(tr("Close"));
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
    lay->addWidget(closeBtn);
    dlg->exec();
    dlg->deleteLater();
}
