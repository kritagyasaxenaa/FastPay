#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

class TransactionEngine;
class UltrasoundHelper;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onModeOnline();
    void onModeOffline();
    void onOnlineReceiveEmit();
    void onOnlineSendCapture();
    void onOnlineSendSubmit();
    void onOfflineSendSign();
    void onOfflineReceiveVerify();
    void onKeyReceivedFromMic(const QByteArray &key);
    void onAccountFrozen();
    void onOnlineTransactionCompleted(const QString &txId);
    void onOnlineTransactionFailed(const QString &error);
    void showHistory();
    void onPhoneNumberChanged();

private:
    void setupUi();
    void applyStyleSheet();
    void showOnlinePanel();
    void showOfflinePanel();
    void ensurePinSet();
    QString currentNonce() const;

    QStackedWidget *m_stack = nullptr;
    TransactionEngine *m_engine = nullptr;
    UltrasoundHelper *m_ultrasound = nullptr;
};

#endif // MAINWINDOW_H
