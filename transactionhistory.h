#ifndef TRANSACTIONHISTORY_H
#define TRANSACTIONHISTORY_H

#include "transactionengine.h"
#include <QList>

class TransactionHistory
{
public:
    static QList<TransactionRecord> load();
    static void append(const TransactionRecord &record);
    static void clear();
};

#endif // TRANSACTIONHISTORY_H
