#ifndef STASH_H
#define STASH_H

#include <QDateTime>
#include <QMetaType>
#include <QString>

#include "Operators.h"

class Stash : public EqualityOperators<Stash>
{
public:
    Stash();
    explicit Stash(int index);
    Stash(int index, const QString& message);

    int index() const;
    void setIndex(int index);

    QString message() const;
    void setMessage(const QString& message);

    QString branchName() const;
    void setBranchName(const QString& branchName);

    QString commitHash() const;
    void setCommitHash(const QString& commitHash);

    QString shortCommitHash(int length = 7) const;

    QDateTime date() const;
    void setDate(const QDateTime& date);

    bool isValid() const;

    QString displayName() const;
    QString displayBranch() const;

    bool operator==(const Stash& other) const;

private:
    int m_index;
    QString m_message;
    QString m_branchName;
    QString m_commitHash;
    QDateTime m_date;
};

Q_DECLARE_METATYPE(Stash)

#endif // STASH_H
