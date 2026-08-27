#ifndef REMOTE_H
#define REMOTE_H

#include <QMetaType>
#include <QString>

class Remote
{
public:
    Remote();
    explicit Remote(const QString& name);
    Remote(const QString& name, const QString& url);

    QString name() const;
    void setName(const QString& name);

    QString url() const;
    void setUrl(const QString& url);

    QString pushUrl() const;
    void setPushUrl(const QString& pushUrl);

    bool isValid() const;

    QString displayName() const;
    QString displayUrl() const;

    bool operator==(const Remote& other) const;
    bool operator!=(const Remote& other) const;

private:
    QString m_name;
    QString m_url;
    QString m_pushUrl;
};

Q_DECLARE_METATYPE(Remote)

#endif // REMOTE_H
