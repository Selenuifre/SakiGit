#ifndef COMMIT_H
#define COMMIT_H

#include <QDateTime>
#include <QMetaType>
#include <QString>
#include <QStringList>

#include "Operators.h"

class Commit : public EqualityOperators<Commit>
{
public:
    Commit();
    explicit Commit(const QString& hash);
    Commit(const QString& hash, const QString& summary);

    QString hash() const;
    void setHash(const QString& hash);

    QString shortHash(int length = 7) const;

    QString summary() const;
    void setSummary(const QString& summary);

    QString body() const;
    void setBody(const QString& body);

    QString message() const;
    void setMessage(const QString& message);

    QString authorName() const;
    void setAuthorName(const QString& authorName);

    QString authorEmail() const;
    void setAuthorEmail(const QString& authorEmail);

    QDateTime authorDate() const;
    void setAuthorDate(const QDateTime& authorDate);

    QString committerName() const;
    void setCommitterName(const QString& committerName);

    QString committerEmail() const;
    void setCommitterEmail(const QString& committerEmail);

    QDateTime committerDate() const;
    void setCommitterDate(const QDateTime& committerDate);

    QStringList parentHashes() const;
    void setParentHashes(const QStringList& parentHashes);
    void addParentHash(const QString& parentHash);

    QStringList changedFiles() const;
    void setChangedFiles(const QStringList& changedFiles);
    void addChangedFile(const QString& filePath);

    bool isValid() const;
    bool isMergeCommit() const;
    bool hasBody() const;

    QString displayTitle() const;
    QString displayAuthor() const;
    QString displayDate(const QString& format = QStringLiteral("yyyy-MM-dd HH:mm")) const;

    static QString summaryFromMessage(const QString& message);
    static QString bodyFromMessage(const QString& message);

    bool operator==(const Commit& other) const;

private:
    QString m_hash;

    QString m_summary;
    QString m_body;

    QString m_authorName;
    QString m_authorEmail;
    QDateTime m_authorDate;

    QString m_committerName;
    QString m_committerEmail;
    QDateTime m_committerDate;

    QStringList m_parentHashes;
    QStringList m_changedFiles;
};

Q_DECLARE_METATYPE(Commit)

#endif // COMMIT_H
