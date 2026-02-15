#include "transactionhistory.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

static QString historyPath()
{
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    return base + "/transaction_history.ini";
}

QList<TransactionRecord> TransactionHistory::load()
{
    QList<TransactionRecord> list;
    QSettings s(historyPath(), QSettings::IniFormat);
    int size = s.beginReadArray("transactions");
    for (int i = 0; i < size; ++i) {
        s.setArrayIndex(i);
        TransactionRecord r;
        r.id = s.value("id").toString();
        r.type = s.value("type").toString();
        r.role = s.value("role").toString();
        r.peerId = s.value("peerId").toString();
        r.amount = s.value("amount").toString();
        r.nonce = s.value("nonce").toString();
        r.status = s.value("status").toString();
        r.createdAt = s.value("createdAt").toDateTime();
        list.append(r);
    }
    s.endArray();
    return list;
}

void TransactionHistory::append(const TransactionRecord &record)
{
    QSettings s(historyPath(), QSettings::IniFormat);
    int size = s.beginReadArray("transactions");
    s.endArray();
    s.beginWriteArray("transactions");
    for (int i = 0; i < size; ++i)
        s.setArrayIndex(i);
    s.setArrayIndex(size);
    s.setValue("id", record.id);
    s.setValue("type", record.type);
    s.setValue("role", record.role);
    s.setValue("peerId", record.peerId);
    s.setValue("amount", record.amount);
    s.setValue("nonce", record.nonce);
    s.setValue("status", record.status);
    s.setValue("createdAt", record.createdAt);
    s.endArray();
}

void TransactionHistory::clear()
{
    QSettings s(historyPath(), QSettings::IniFormat);
    s.remove("transactions");
}
