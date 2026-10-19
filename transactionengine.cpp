#include "transactionengine.h"
#include "transactionhistory.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QCryptographicHash>
#include <QUuid>
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

TransactionEngine::TransactionEngine(QObject *parent) : QObject(parent)
{
    m_network = new QNetworkAccessManager(this);
}

QString TransactionEngine::getTimestampNonce()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}

QString TransactionEngine::generateTransactionId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool TransactionEngine::checkTransactionIdMatch(const QString &localTransactionId, const QString &serverTransactionId)
{
    if (localTransactionId.trimmed() != serverTransactionId.trimmed()) {
        freezeAccountOnVerificationFailure();
        return false;
    }
    return true;
}

static QString normalizedPhone(const QString &phone)
{
    QString digits = phone.trimmed();
    digits.remove(QRegularExpression(QStringLiteral("[^0-9]")));
    return digits;
}

QByteArray TransactionEngine::publicKeyFromPhoneNumber(const QString &phoneNumber)
{
    QString norm = normalizedPhone(phoneNumber);
    if (norm.isEmpty()) return QByteArray();
    QByteArray hash = QCryptographicHash::hash(norm.toUtf8(), QCryptographicHash::Sha256);
    return hash;
}

QString TransactionEngine::publicKeyHexFromPhoneNumber(const QString &phoneNumber)
{
    return QString::fromUtf8(publicKeyFromPhoneNumber(phoneNumber).toHex());
}

static QString authPath()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    return base + QStringLiteral("/auth.ini");
}

bool TransactionEngine::setPin(const QString &pin)
{
    QString p = pin.trimmed();
    if (p.length() < 4 || p.length() > 6) return false;
    QByteArray hash = QCryptographicHash::hash(p.toUtf8(), QCryptographicHash::Sha256);
    QSettings s(authPath(), QSettings::IniFormat);
    s.setValue(QStringLiteral("pin_hash"), hash.toHex());
    s.sync();
    return true;
}

bool TransactionEngine::verifyPin(const QString &pin) const
{
    QString p = pin.trimmed();
    if (p.isEmpty()) return false;
    QByteArray hash = QCryptographicHash::hash(p.toUtf8(), QCryptographicHash::Sha256);
    QSettings s(authPath(), QSettings::IniFormat);
    QString stored = s.value(QStringLiteral("pin_hash")).toString();
    return QString::fromUtf8(hash.toHex()) == stored;
}

bool TransactionEngine::hasPinSet() const
{
    QSettings s(authPath(), QSettings::IniFormat);
    return s.contains(QStringLiteral("pin_hash"));
}

bool TransactionEngine::changePin(const QString &oldPin, const QString &newPin)
{
    if (!verifyPin(oldPin)) return false;
    return setPin(newPin);
}

QByteArray TransactionEngine::buildOnlineEmitPayload(const QByteArray &headerIdentifier, const QByteArray &publicKeyPem)
{
    const int headerSize = 32;
    const int keyMaxSize = 2208;
    const int totalSize = headerSize + keyMaxSize;

    QByteArray header = headerIdentifier.left(headerSize);
    if (header.size() < headerSize)
        header.append(QByteArray(headerSize - header.size(), 0));

    QByteArray key = publicKeyPem.left(keyMaxSize);
    if (key.size() < keyMaxSize)
        key.append(QByteArray(keyMaxSize - key.size(), 0));

    return header + key;
}

QByteArray TransactionEngine::extractKeyFromMicBuffer(const QByteArray &micBuffer2x, const QByteArray &header)
{
    int idx = micBuffer2x.indexOf(header);
    if (idx < 0) return QByteArray();
    int keyStart = idx + header.size();
    const int keyMaxSize = 2208;
    if (keyStart + keyMaxSize > micBuffer2x.size()) return QByteArray();
    QByteArray key = micBuffer2x.mid(keyStart, keyMaxSize);
    int endMark = key.indexOf("-----END PUBLIC KEY-----");
    if (endMark >= 0)
        key = key.left(endMark + 24);
    return key.trimmed();
}

QByteArray TransactionEngine::signOfflineTransaction(const QString &senderId, const QString &receiverId,
                                                     const QString &amount, const QString &nonce,
                                                     const QByteArray &senderPrivateKeyPem)
{
    Q_UNUSED(senderPrivateKeyPem);
    QString message = senderId + "|" + receiverId + "|" + amount + "|" + nonce;
    return QCryptographicHash::hash(message.toUtf8(), QCryptographicHash::Sha256);
}

bool TransactionEngine::verifyOfflineTransaction(const QString &message, const QByteArray &signature,
                                                 const QByteArray &senderPublicKeyPem)
{
    Q_UNUSED(senderPublicKeyPem);
    QByteArray expected = QCryptographicHash::hash(message.toUtf8(), QCryptographicHash::Sha256);
    return expected == signature;
}

QByteArray TransactionEngine::signReceipt(const QString &originalMessage, const QByteArray &receiverPrivateKeyPem)
{
    Q_UNUSED(receiverPrivateKeyPem);
    QString receiptMessage = originalMessage + "|RECEIPT";
    return QCryptographicHash::hash(receiptMessage.toUtf8(), QCryptographicHash::Sha256);
}

void TransactionEngine::submitOfflineWhenOnline(const TransactionRecord &record)
{
    addToLocalHistory(record);
    emit historyUpdated();
}

void TransactionEngine::freezeAccountOnVerificationFailure()
{
    m_accountFrozen = true;
    emit accountFrozen();
}

QList<TransactionRecord> TransactionEngine::getLocalHistory() const
{
    return TransactionHistory::load();
}

void TransactionEngine::addToLocalHistory(const TransactionRecord &record)
{
    TransactionHistory::append(record);
    emit historyUpdated();
}

void TransactionEngine::setServerBaseUrl(const QString &baseUrl)
{
    m_serverBaseUrl = baseUrl.trimmed();
    if (m_serverBaseUrl.endsWith(QLatin1Char('/')))
        m_serverBaseUrl.chop(1);
}

void TransactionEngine::fetchDemoBankBalance(const QString &accountId)
{
    const QUrl baseUrl(m_serverBaseUrl);
    if (!baseUrl.isValid() || baseUrl.host().isEmpty()
        || (baseUrl.scheme() != QStringLiteral("http") && baseUrl.scheme() != QStringLiteral("https"))) {
        emit demoBankBalanceFailed(accountId, tr("Enter a valid local demo server URL."));
        return;
    }

    QUrl url = baseUrl;
    url.setPath(baseUrl.path() + QStringLiteral("/api/v1/demo/bank-accounts/") + accountId);
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    request.setTransferTimeout(5000);
    QNetworkReply *reply = m_network->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, accountId]() {
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray payload = reply->readAll();
        const QString networkError = reply->errorString();
        reply->deleteLater();

        const QJsonDocument document = QJsonDocument::fromJson(payload);
        if (status < 200 || status >= 300 || !document.isObject()) {
            QString detail = networkError;
            if (document.isObject())
                detail = document.object().value(QStringLiteral("detail")).toString(detail);
            emit demoBankBalanceFailed(accountId, detail);
            return;
        }

        const QJsonObject result = document.object();
        if (!result.value(QStringLiteral("demo")).toBool()) {
            emit demoBankBalanceFailed(accountId, tr("The server did not identify this as a demo balance."));
            return;
        }
        emit demoBankBalanceReceived(
            accountId,
            result.value(QStringLiteral("balance")).toString(),
            result.value(QStringLiteral("currency")).toString());
    });
}

void TransactionEngine::submitOnlineTransactionToServer(const QString &senderId, const QString &receiverId,
                                                       const QString &amount, const QString &pin)
{
    if (!verifyPin(pin)) {
        emit onlineTransactionFailed(tr("Wrong UPI PIN. Transaction not approved."));
        return;
    }
    if (m_onlineRequestPending) {
        emit onlineTransactionFailed(tr("A simulated payment is already being processed."));
        return;
    }
    if (m_serverBaseUrl.isEmpty()) {
        emit onlineTransactionFailed(tr("Server URL not set. Enter a local demo API URL."));
        return;
    }
    const QString nonce = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QJsonObject body;
    body.insert(QStringLiteral("sender_id"), senderId);
    body.insert(QStringLiteral("receiver_id"), receiverId);
    body.insert(QStringLiteral("amount"), amount);
    body.insert(QStringLiteral("nonce"), nonce);
    QByteArray json = QJsonDocument(body).toJson(QJsonDocument::Compact);

    QUrl url(m_serverBaseUrl + QStringLiteral("/api/v1/transactions/online"));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setHeader(QNetworkRequest::ContentLengthHeader, json.size());
    req.setTransferTimeout(5000);
    m_onlineRequestPending = true;
    QNetworkReply *reply = m_network->post(req, json);

    connect(reply, &QNetworkReply::finished, this, [this, reply, senderId, receiverId, amount, nonce]() {
        m_onlineRequestPending = false;
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const QByteArray errorPayload = reply->readAll();
            const QJsonDocument errorDocument = QJsonDocument::fromJson(errorPayload);
            QString serverError;
            if (errorDocument.isObject())
                serverError = errorDocument.object().value(QStringLiteral("detail")).toString();
            if (status == 403) {
                m_accountFrozen = true;
                emit accountFrozen();
                emit onlineTransactionFailed(tr("Account frozen. Contact support."));
            } else {
                emit onlineTransactionFailed(serverError.isEmpty()
                    ? tr("Network error: %1").arg(reply->errorString())
                    : serverError);
            }
            return;
        }
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) {
            emit onlineTransactionFailed(tr("Invalid server response."));
            return;
        }
        QJsonObject obj = doc.object();
        if (!obj.value(QStringLiteral("demo")).toBool()) {
            emit onlineTransactionFailed(tr("Server did not identify this as a simulated payment."));
            return;
        }
        QString txId = obj.value(QStringLiteral("tx_id")).toString();
        if (txId.isEmpty()) {
            emit onlineTransactionFailed(tr("Server did not return transaction ID."));
            return;
        }
        TransactionRecord rec;
        rec.id = txId;
        rec.type = QStringLiteral("online");
        rec.role = QStringLiteral("sender");
        rec.peerId = receiverId;
        rec.amount = amount;
        rec.nonce = nonce;
        rec.status = QStringLiteral("completed");
        rec.createdAt = QDateTime::currentDateTime();
        addToLocalHistory(rec);
        emit onlineTransactionCompleted(txId);
    });
}

void TransactionEngine::verifyTransactionIdWithServer(const QString &userId, const QString &transactionId,
                                                     const QString &senderId, const QString &receiverId,
                                                     const QString &nonce, const QString &amount)
{
    if (m_serverBaseUrl.isEmpty()) {
        return;
    }
    QJsonObject body;
    body.insert(QStringLiteral("user_id"), userId);
    body.insert(QStringLiteral("transaction_id"), transactionId);
    body.insert(QStringLiteral("sender_id"), senderId);
    body.insert(QStringLiteral("receiver_id"), receiverId);
    body.insert(QStringLiteral("nonce"), nonce);
    body.insert(QStringLiteral("amount"), amount);
    QByteArray json = QJsonDocument(body).toJson(QJsonDocument::Compact);

    QUrl url(m_serverBaseUrl + QStringLiteral("/api/v1/transactions/verify-id"));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    req.setHeader(QNetworkRequest::ContentLengthHeader, json.size());
    QNetworkReply *reply = m_network->post(req, json);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 403) {
            freezeAccountOnVerificationFailure();
        }
    });
}
