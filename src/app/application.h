#ifndef APPLICATION_H
#define APPLICATION_H

#include "infrastructure/result.h"

#include <QMetaObject>
#include <QString>

#include <memory>

class QApplication;
class AppContext;
class GitService;
class GitTaskRunner;
class MainWindow;
class Repository;
class SettingsService;

class Application final
{
public:
    // 使用外部创建的 QApplication 构造应用启动器
    explicit Application(QApplication& qtApplication);

    // 释放应用资源，并保存必要状态
    ~Application();

    // 禁止拷贝，避免重复持有同一批应用服务
    Application(const Application&) = delete;

    // 禁止赋值，避免重复持有同一批应用服务
    Application& operator=(const Application&) = delete;

    // 初始化应用元信息、日志、全局上下文和主窗口
    Result<void> initialize();

    // 显示主窗口；如果尚未初始化会先执行初始化
    Result<void> showMainWindow();

    // 显示主窗口并进入 Qt 事件循环
    int run();

    // 保存状态并关闭应用启动层持有的资源
    void shutdown();

    // 判断应用启动层是否已完成初始化
    bool isInitialized() const;

    // 返回底层 Qt 应用对象
    QApplication& qtApplication() const;

    // 返回配置服务；初始化前可能为空
    SettingsService* settingsService() const;

    // 返回同步 Git 服务；初始化前可能为空
    GitService* gitService() const;

    // 返回异步 Git 任务执行器；初始化前可能为空
    GitTaskRunner* gitTaskRunner() const;

    // 返回主窗口；初始化前可能为空
    MainWindow* mainWindow() const;

    // 返回全局依赖容器；初始化前可能为空
    AppContext* appContext() const;

    // 返回最近一次初始化或启动失败的错误信息
    QString lastError() const;

    // 更新 Git 可执行文件路径，并同步到相关服务
    void updateGitExecutablePath(const QString& gitExecutablePath);

private:
    // 设置 QApplication 的组织名、应用名等基础信息
    void configureApplicationMetadata();

    // 初始化日志输出和 Qt 消息处理器
    void initializeLogger();

    // 创建全局依赖容器，包含所有 Service 和 Model 实例
    void initializeAppContext();

    // 创建主窗口、恢复窗口状态并注册到 AppContext
    void initializeMainWindow();
    void bindMainWindowModels();
    void connectMainWindowSignals();
    void connectControllerSignals();
    QString currentRepositoryPath() const;
    void activateRepository(const Repository& repository);
    QString currentBranchNameForPath(const QString& repoPath, const QString& fallback = QString()) const;
    void refreshCurrentRepositoryHeader();
    bool ensureCurrentRepository(const QString& operation) const;
    void showInfo(const QString& message) const;
    void showError(const QString& title, const QString& message) const;

    // 从配置中恢复主窗口几何和停靠状态
    void restoreMainWindowState();

    // 将主窗口几何和停靠状态保存到配置中
    void saveMainWindowState();

    // 连接 QApplication 生命周期信号
    void connectApplicationSignals();

    // 保存最近一次错误信息
    void setLastError(const QString& errorMessage);

private:
    // 外部创建并传入的 Qt 应用对象
    QApplication& m_qtApplication;

    // 全局依赖容器，持有所有 Service 和 Model 的唯一实例
    std::unique_ptr<AppContext> m_appContext;

    // 应用主窗口
    std::unique_ptr<MainWindow> m_mainWindow;

    // 是否已经完成初始化
    bool m_initialized;

    // 是否已经安装 Qt 日志处理器
    bool m_loggerInstalled;

    // 最近一次错误信息
    QString m_lastError;

    // QApplication 退出信号连接，用于关闭时断开 lambda 捕获
    QMetaObject::Connection m_aboutToQuitConnection;
};

#endif // APPLICATION_H
