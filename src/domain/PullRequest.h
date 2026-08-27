#ifndef PULLREQUEST_H
#define PULLREQUEST_H

#include "domain/CodeHostingPlatform.h"

#include <QDateTime>
#include <QMetaType>
#include <QString>

class PullRequest
{
public:
    PullRequest();

    int number() const;
    void setNumber(int number);

    QString title() const;
    void setTitle(const QString& title);

    QString body() const;
    void setBody(const QString& body);

    QString state() const;
    void setState(const QString& state);

    QString author() const;
    void setAuthor(const QString& author);

    QString head() const;
    void setHead(const QString& head);

    QString base() const;
    void setBase(const QString& base);

    QString url() const;
    void setUrl(const QString& url);

    // 平台原始标识（如 GitHub PR 编号对应的 API URL），用于内部追踪
    QString sourceId() const;
    void setSourceId(const QString& sourceId);

    CodeHostingPlatform platform() const;
    void setPlatform(CodeHostingPlatform platform);

    QDateTime createdAt() const;
    void setCreatedAt(const QDateTime& createdAt);

    QDateTime updatedAt() const;
    void setUpdatedAt(const QDateTime& updatedAt);

    bool isOpen() const;
    bool isMerged() const;
    bool isValid() const;

    QString displayTitle() const;
    QString displayState() const;
    QString platformDisplayName() const;    // 例如 "[GitHub] #42 Fix bug"

    bool operator==(const PullRequest& other) const;
    bool operator!=(const PullRequest& other) const;

private:
    int m_number;
    QString m_title;
    QString m_body;
    QString m_state;
    QString m_author;
    QString m_head;
    QString m_base;
    QString m_url;
    QString m_sourceId;
    CodeHostingPlatform m_platform;
    QDateTime m_createdAt;
    QDateTime m_updatedAt;
};

Q_DECLARE_METATYPE(PullRequest)

#endif // PULLREQUEST_H
