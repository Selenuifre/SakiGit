#ifndef SETTINGSSERVICE_H
#define SETTINGSSERVICE_H

#include "domain/repository.h"

#include <QByteArray>
#include <QList>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

class SettingsService
{
public:
    // 应用主题类型
    enum class Theme {
        System,
        Light,
        Dark
    };

public:
    // 使用组织名和应用名创建配置服务
    explicit SettingsService(const QString& organizationName = QStringLiteral("GitDeskQt"),
                             const QString& applicationName = QStringLiteral("GitDeskQt"));

    // 返回组织名
    QString organizationName() const;

    // 返回应用名
    QString applicationName() const;

    // 保存最近打开的仓库列表
    void setRecentRepositories(const QList<Repository>& repositories);

    // 读取最近打开的仓库列表
    QList<Repository> recentRepositories() const;

    // 添加一个最近打开的仓库
    void addRecentRepository(const Repository& repository);

    // 从最近仓库列表中移除指定路径的仓库
    void removeRecentRepository(const QString& localPath);

    // 清空最近打开的仓库列表
    void clearRecentRepositories();

    // 返回最近仓库最大保存数量
    int maxRecentRepositories() const;

    // 设置最近仓库最大保存数量
    void setMaxRecentRepositories(int count);

    // 返回最后一次打开的仓库路径
    QString lastOpenedRepositoryPath() const;

    // 保存最后一次打开的仓库路径
    void setLastOpenedRepositoryPath(const QString& path);

    // 返回默认克隆路径
    QString defaultClonePath() const;

    // 设置默认克隆路径
    void setDefaultClonePath(const QString& path);

    // 返回 Git 可执行文件路径
    QString gitExecutablePath() const;

    // 设置 Git 可执行文件路径
    void setGitExecutablePath(const QString& path);

    // 返回当前主题设置
    Theme theme() const;

    // 设置当前主题
    void setTheme(Theme theme);

    // 返回主窗口几何信息
    QByteArray mainWindowGeometry() const;

    // 保存主窗口几何信息
    void setMainWindowGeometry(const QByteArray& geometry);

    // 返回主窗口状态信息
    QByteArray mainWindowState() const;

    // 保存主窗口状态信息
    void setMainWindowState(const QByteArray& state);

    // 返回指定 key 的配置值
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;

    // 设置指定 key 的配置值
    void setValue(const QString& key, const QVariant& value);

    // 判断指定 key 是否存在
    bool contains(const QString& key) const;

    // 删除指定 key 的配置值
    void remove(const QString& key);

    // 立即把配置同步到磁盘
    void sync();

    // 清空所有配置
    void clear();

    // 将主题枚举转换成字符串
    static QString themeToString(Theme theme);

    // 将字符串转换成主题枚举
    static Theme themeFromString(const QString& value);

    // --- AI 配置（Phase 5） ---

    // 返回 AI 提供商（如 "openai", "anthropic", "custom"）
    QString aiProvider() const;
    void setAIProvider(const QString& provider);

    // 返回 AI API 密钥
    QString aiApiKey() const;
    void setAIApiKey(const QString& apiKey);

    // 返回 AI 模型名称（如 "gpt-4o", "gpt-4o-mini"）
    QString aiModel() const;
    void setAIModel(const QString& model);

private:
    // 保存 Repository 对象到 QSettings 当前数组项
    void writeRepository(const Repository& repository);

    // 从 QSettings 当前数组项读取 Repository 对象
    Repository readRepository() const;

private:
    // 组织名，用于区分配置所属组织
    QString m_organizationName;

    // 应用名，用于区分配置所属应用
    QString m_applicationName;

    // Qt 配置读写对象
    mutable QSettings m_settings;
};

#endif // SETTINGSSERVICE_H
