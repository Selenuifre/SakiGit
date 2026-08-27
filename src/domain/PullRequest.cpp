#include "PullRequest.h"

PullRequest::PullRequest()
    : m_number(-1),
    m_state(QStringLiteral("unknown")),
    m_platform(CodeHostingPlatform::Unknown)
{
}

int PullRequest::number() const
{
    return m_number;
}

void PullRequest::setNumber(int number)
{
    m_number = number;
}

QString PullRequest::title() const
{
    return m_title;
}

void PullRequest::setTitle(const QString& title)
{
    m_title = title.trimmed();
}

QString PullRequest::body() const
{
    return m_body;
}

void PullRequest::setBody(const QString& body)
{
    m_body = body.trimmed();
}

QString PullRequest::state() const
{
    return m_state;
}

void PullRequest::setState(const QString& state)
{
    m_state = state.trimmed();
}

QString PullRequest::author() const
{
    return m_author;
}

void PullRequest::setAuthor(const QString& author)
{
    m_author = author.trimmed();
}

QString PullRequest::head() const
{
    return m_head;
}

void PullRequest::setHead(const QString& head)
{
    m_head = head.trimmed();
}

QString PullRequest::base() const
{
    return m_base;
}

void PullRequest::setBase(const QString& base)
{
    m_base = base.trimmed();
}

QString PullRequest::url() const
{
    return m_url;
}

void PullRequest::setUrl(const QString& url)
{
    m_url = url.trimmed();
}

QString PullRequest::sourceId() const
{
    return m_sourceId;
}

void PullRequest::setSourceId(const QString& sourceId)
{
    m_sourceId = sourceId.trimmed();
}

CodeHostingPlatform PullRequest::platform() const
{
    return m_platform;
}

void PullRequest::setPlatform(CodeHostingPlatform platform)
{
    m_platform = platform;
}

QDateTime PullRequest::createdAt() const
{
    return m_createdAt;
}

void PullRequest::setCreatedAt(const QDateTime& createdAt)
{
    m_createdAt = createdAt;
}

QDateTime PullRequest::updatedAt() const
{
    return m_updatedAt;
}

void PullRequest::setUpdatedAt(const QDateTime& updatedAt)
{
    m_updatedAt = updatedAt;
}

bool PullRequest::isOpen() const
{
    return m_state == QStringLiteral("open");
}

bool PullRequest::isMerged() const
{
    return m_state == QStringLiteral("merged");
}

bool PullRequest::isValid() const
{
    return m_number > 0;
}

QString PullRequest::displayTitle() const
{
    if (m_title.isEmpty()) {
        return QStringLiteral("Untitled PR");
    }

    return QStringLiteral("#%1 %2").arg(m_number).arg(m_title);
}

QString PullRequest::displayState() const
{
    if (m_state == QStringLiteral("open")) {
        return QStringLiteral("Open");
    }

    if (m_state == QStringLiteral("closed")) {
        return QStringLiteral("Closed");
    }

    if (m_state == QStringLiteral("merged")) {
        return QStringLiteral("Merged");
    }

    return m_state;
}

QString PullRequest::platformDisplayName() const
{
    return QStringLiteral("[%1] %2")
        .arg(codeHostingPlatformName(m_platform), displayTitle());
}

bool PullRequest::operator==(const PullRequest& other) const
{
    return m_number == other.m_number && m_platform == other.m_platform;
}

bool PullRequest::operator!=(const PullRequest& other) const
{
    return !(*this == other);
}
