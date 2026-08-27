#include "settingsservice.h"

#include <QDir>
#include <QStandardPaths>

namespace {
const char* KeyRecentRepositories = "repositories/recent";
const char* KeyMaxRecentRepositories = "repositories/maxRecent";
const char* KeyLastOpenedRepositoryPath = "repositories/lastOpenedPath";
const char* KeyDefaultClonePath = "repositories/defaultClonePath";
const char* KeyGitExecutablePath = "git/executablePath";
const char* KeyTheme = "ui/theme";
const char* KeyMainWindowGeometry = "ui/mainWindowGeometry";
const char* KeyMainWindowState = "ui/mainWindowState";
const char* KeyAIProvider = "ai/provider";
const char* KeyAIApiKey = "ai/apiKey";
const char* KeyAIModel = "ai/model";

const int DefaultMaxRecentRepositories = 20;
}

SettingsService::SettingsService(const QString& organizationName,
                                 const QString& applicationName)
    : m_organizationName(organizationName),
    m_applicationName(applicationName),
    m_settings(organizationName, applicationName)
{
}

QString SettingsService::organizationName() const
{
    return m_organizationName;
}

QString SettingsService::applicationName() const
{
    return m_applicationName;
}

void SettingsService::setRecentRepositories(const QList<Repository>& repositories)
{
    m_settings.beginWriteArray(QString::fromLatin1(KeyRecentRepositories));

    const int count = qMin(repositories.size(), maxRecentRepositories());

    for (int i = 0; i < count; ++i) {
        m_settings.setArrayIndex(i);
        writeRepository(repositories.at(i));
    }

    m_settings.endArray();
    m_settings.sync();
}

QList<Repository> SettingsService::recentRepositories() const
{
    QList<Repository> repositories;

    const int count = m_settings.beginReadArray(QString::fromLatin1(KeyRecentRepositories));

    for (int i = 0; i < count; ++i) {
        m_settings.setArrayIndex(i);

        Repository repository = readRepository();

        if (repository.isValid() || !repository.localPath().isEmpty()) {
            repositories.append(repository);
        }
    }

    m_settings.endArray();

    return repositories;
}

void SettingsService::addRecentRepository(const Repository& repository)
{
    if (repository.localPath().isEmpty()) {
        return;
    }

    QList<Repository> repositories = recentRepositories();
    const QString normalizedPath = Repository::resolveAbsolutePath(repository.localPath());

    for (int i = repositories.size() - 1; i >= 0; --i) {
        if (Repository::resolveAbsolutePath(repositories.at(i).localPath()) == normalizedPath) {
            repositories.removeAt(i);
        }
    }

    repositories.prepend(repository);

    while (repositories.size() > maxRecentRepositories()) {
        repositories.removeLast();
    }

    setRecentRepositories(repositories);
    setLastOpenedRepositoryPath(repository.localPath());
}

void SettingsService::removeRecentRepository(const QString& localPath)
{
    QList<Repository> repositories = recentRepositories();
    const QString normalizedPath = Repository::resolveAbsolutePath(localPath);

    for (int i = repositories.size() - 1; i >= 0; --i) {
        if (Repository::resolveAbsolutePath(repositories.at(i).localPath()) == normalizedPath) {
            repositories.removeAt(i);
        }
    }

    setRecentRepositories(repositories);
}

void SettingsService::clearRecentRepositories()
{
    m_settings.remove(QString::fromLatin1(KeyRecentRepositories));
    m_settings.sync();
}

int SettingsService::maxRecentRepositories() const
{
    return m_settings.value(QString::fromLatin1(KeyMaxRecentRepositories),
                            DefaultMaxRecentRepositories).toInt();
}

void SettingsService::setMaxRecentRepositories(int count)
{
    if (count <= 0) {
        count = DefaultMaxRecentRepositories;
    }

    m_settings.setValue(QString::fromLatin1(KeyMaxRecentRepositories), count);
    m_settings.sync();
}

QString SettingsService::lastOpenedRepositoryPath() const
{
    return m_settings.value(QString::fromLatin1(KeyLastOpenedRepositoryPath)).toString();
}

void SettingsService::setLastOpenedRepositoryPath(const QString& path)
{
    m_settings.setValue(QString::fromLatin1(KeyLastOpenedRepositoryPath),
                        Repository::resolveAbsolutePath(path));
    m_settings.sync();
}

QString SettingsService::defaultClonePath() const
{
    const QString savedPath = m_settings.value(QString::fromLatin1(KeyDefaultClonePath)).toString();

    if (!savedPath.trimmed().isEmpty()) {
        return QDir::cleanPath(savedPath);
    }

    const QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

    if (!documentsPath.isEmpty()) {
        return QDir::cleanPath(documentsPath);
    }

    return QDir::homePath();
}

void SettingsService::setDefaultClonePath(const QString& path)
{
    m_settings.setValue(QString::fromLatin1(KeyDefaultClonePath), QDir::cleanPath(path.trimmed()));
    m_settings.sync();
}

QString SettingsService::gitExecutablePath() const
{
    return m_settings.value(QString::fromLatin1(KeyGitExecutablePath),
                            QStringLiteral("git")).toString();
}

void SettingsService::setGitExecutablePath(const QString& path)
{
    const QString cleanPath = path.trimmed();

    if (cleanPath.isEmpty()) {
        m_settings.setValue(QString::fromLatin1(KeyGitExecutablePath), QStringLiteral("git"));
    } else {
        m_settings.setValue(QString::fromLatin1(KeyGitExecutablePath), cleanPath);
    }

    m_settings.sync();
}

SettingsService::Theme SettingsService::theme() const
{
    const QString savedTheme = m_settings.value(QString::fromLatin1(KeyTheme),
                                                QStringLiteral("system")).toString();

    return themeFromString(savedTheme);
}

void SettingsService::setTheme(Theme theme)
{
    m_settings.setValue(QString::fromLatin1(KeyTheme), themeToString(theme));
    m_settings.sync();
}

QByteArray SettingsService::mainWindowGeometry() const
{
    return m_settings.value(QString::fromLatin1(KeyMainWindowGeometry)).toByteArray();
}

void SettingsService::setMainWindowGeometry(const QByteArray& geometry)
{
    m_settings.setValue(QString::fromLatin1(KeyMainWindowGeometry), geometry);
    m_settings.sync();
}

QByteArray SettingsService::mainWindowState() const
{
    return m_settings.value(QString::fromLatin1(KeyMainWindowState)).toByteArray();
}

void SettingsService::setMainWindowState(const QByteArray& state)
{
    m_settings.setValue(QString::fromLatin1(KeyMainWindowState), state);
    m_settings.sync();
}

QVariant SettingsService::value(const QString& key, const QVariant& defaultValue) const
{
    return m_settings.value(key, defaultValue);
}

void SettingsService::setValue(const QString& key, const QVariant& value)
{
    m_settings.setValue(key, value);
    m_settings.sync();
}

bool SettingsService::contains(const QString& key) const
{
    return m_settings.contains(key);
}

void SettingsService::remove(const QString& key)
{
    m_settings.remove(key);
    m_settings.sync();
}

void SettingsService::sync()
{
    m_settings.sync();
}

void SettingsService::clear()
{
    m_settings.clear();
    m_settings.sync();
}

QString SettingsService::themeToString(Theme theme)
{
    switch (theme) {
    case Theme::Light:
        return QStringLiteral("light");
    case Theme::Dark:
        return QStringLiteral("dark");
    case Theme::System:
    default:
        return QStringLiteral("system");
    }
}

SettingsService::Theme SettingsService::themeFromString(const QString& value)
{
    const QString cleanValue = value.trimmed().toLower();

    if (cleanValue == QStringLiteral("light")) {
        return Theme::Light;
    }

    if (cleanValue == QStringLiteral("dark")) {
        return Theme::Dark;
    }

    return Theme::System;
}

// ============================================================================
// AI 配置（Phase 5）
// ============================================================================

QString SettingsService::aiProvider() const
{
    return m_settings.value(QString::fromLatin1(KeyAIProvider)).toString();
}

void SettingsService::setAIProvider(const QString& provider)
{
    m_settings.setValue(QString::fromLatin1(KeyAIProvider), provider.trimmed());
    m_settings.sync();
}

QString SettingsService::aiApiKey() const
{
    return m_settings.value(QString::fromLatin1(KeyAIApiKey)).toString();
}

void SettingsService::setAIApiKey(const QString& apiKey)
{
    m_settings.setValue(QString::fromLatin1(KeyAIApiKey), apiKey.trimmed());
    m_settings.sync();
}

QString SettingsService::aiModel() const
{
    return m_settings.value(QString::fromLatin1(KeyAIModel)).toString();
}

void SettingsService::setAIModel(const QString& model)
{
    m_settings.setValue(QString::fromLatin1(KeyAIModel), model.trimmed());
    m_settings.sync();
}

void SettingsService::writeRepository(const Repository& repository)
{
    m_settings.setValue(QStringLiteral("id"), repository.id());
    m_settings.setValue(QStringLiteral("name"), repository.name());
    m_settings.setValue(QStringLiteral("localPath"), repository.localPath());
    m_settings.setValue(QStringLiteral("currentBranch"), repository.currentBranch());
    m_settings.setValue(QStringLiteral("defaultBranch"), repository.defaultBranch());
    m_settings.setValue(QStringLiteral("remoteName"), repository.remoteName());
    m_settings.setValue(QStringLiteral("remoteUrl"), repository.remoteUrl());
    m_settings.setValue(QStringLiteral("provider"), GitTypes::toString(repository.provider()));
    m_settings.setValue(QStringLiteral("state"), GitTypes::toString(repository.state()));
    m_settings.setValue(QStringLiteral("lastOpenedAt"), repository.lastOpenedAt());
}

Repository SettingsService::readRepository() const
{
    Repository repository;

    repository.setId(m_settings.value(QStringLiteral("id")).toString());
    repository.setName(m_settings.value(QStringLiteral("name")).toString());
    repository.setLocalPath(m_settings.value(QStringLiteral("localPath")).toString());
    repository.setCurrentBranch(m_settings.value(QStringLiteral("currentBranch")).toString());
    repository.setDefaultBranch(m_settings.value(QStringLiteral("defaultBranch")).toString());
    repository.setRemoteName(m_settings.value(QStringLiteral("remoteName")).toString());
    repository.setRemoteUrl(m_settings.value(QStringLiteral("remoteUrl")).toString());
    repository.setLastOpenedAt(m_settings.value(QStringLiteral("lastOpenedAt")).toDateTime());

    const QString state = m_settings.value(QStringLiteral("state")).toString();

    if (state == QStringLiteral("Ready")) {
        repository.setState(GitTypes::RepositoryState::Ready);
    } else if (state == QStringLiteral("Missing")) {
        repository.setState(GitTypes::RepositoryState::Missing);
    } else if (state == QStringLiteral("NotGitRepository")) {
        repository.setState(GitTypes::RepositoryState::NotGitRepository);
    } else {
        repository.setState(GitTypes::RepositoryState::Unknown);
    }

    return repository;
}
