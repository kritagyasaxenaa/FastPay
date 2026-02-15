#ifndef TRANSACTIONENGINE_H
#define TRANSACTIONENGINE_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QDateTime>

class QNetworkAccessManager;

struct TransactionRecord {
    QString id;
    QString type;       // "online" | "offline"
    QString role;       // "sender" | "receiver"
    QString peerId;
    QString amount;
    QString nonce;      // timestamp (date + time)
    QString status;     // "pending" | "completed" | "failed" | "frozen"
    QDateTime createdAt;
};

class TransactionEngine : public QObject
{
    Q_OBJECT
public:
    explicit TransactionEngine(QObject *parent = nullptr);

    // Nonce = timestamp (date + time) for all transactions
    static QString getTimestampNonce();

    // --- Phone number → public key (hash of normalized phone) ---
    static QByteArray publicKeyFromPhoneNumber(const QString &phoneNumber);
    static QString publicKeyHexFromPhoneNumber(const QString &phoneNumber);

    // --- UPI-style PIN: set once, then verify to approve/verify transactions ---
    bool setPin(const QString &pin);
    bool verifyPin(const QString &pin) const;
    bool hasPinSet() const;
    bool changePin(const QString &oldPin, const QString &newPin);

    // --- Transaction ID: generate (client) and verify match; on mismatch report account freezed ---
    static QString generateTransactionId();
    bool checkTransactionIdMatch(const QString &localTransactionId, const QString &serverTransactionId);
    void setServerBaseUrl(const QString &baseUrl);
    QString serverBaseUrl() const { return m_serverBaseUrl; }
    void submitOnlineTransactionToServer(const QString &senderId, const QString &receiverId,
                                         const QString &amount, const QString &pin);
    void verifyTransactionIdWithServer(const QString &userId, const QString &transactionId,
                                       const QString &senderId, const QString &receiverId,
                                       const QString &nonce, const QString &amount);

    // --- Online: receiver emits ultrasound (header + public key); sender captures 2x, extracts key, pays with PIN ---
    QByteArray buildOnlineEmitPayload(const QByteArray &headerIdentifier, const QByteArray &publicKeyPem);
    QByteArray extractKeyFromMicBuffer(const QByteArray &micBuffer2x, const QByteArray &header);
    bool submitOnlineTransaction(const QString &senderUpiId, const QString &amount,
                                 const QByteArray &receiverPublicKeyPem, const QString &pin);

    // --- Offline: cold wallet to cold wallet; sender signs, receiver verifies and sends receipt; sync when online ---
    QByteArray signOfflineTransaction(const QString &senderId, const QString &receiverId,
                                      const QString &amount, const QString &nonce,
                                      const QByteArray &senderPrivateKeyPem);
    bool verifyOfflineTransaction(const QString &message, const QByteArray &signature,
                                  const QByteArray &senderPublicKeyPem);
    QByteArray signReceipt(const QString &originalMessage, const QByteArray &receiverPrivateKeyPem);
    void submitOfflineWhenOnline(const TransactionRecord &record);
    void freezeAccountOnVerificationFailure();

    // History (stored online and offline)
    QList<TransactionRecord> getLocalHistory() const;
    void addToLocalHistory(const TransactionRecord &record);
    bool isAccountFrozen() const { return m_accountFrozen; }

signals:
    void onlineTransactionCompleted(const QString &txId);
    void onlineTransactionFailed(const QString &error);
    void offlineTransactionVerified(const QString &txId);
    void offlineVerificationFailed();
    void accountFrozen();
    void historyUpdated();

private:
    bool m_accountFrozen = false;
    QList<TransactionRecord> m_localHistory;
    QString m_serverBaseUrl;
    QNetworkAccessManager *m_network = nullptr;
};

#endif // TRANSACTIONENGINE_H
