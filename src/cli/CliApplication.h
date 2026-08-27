#ifndef CLIAPPLICATION_H
#define CLIAPPLICATION_H

#include <QCoreApplication>
#include <QString>

#include <memory>

class AppContext;
class CliCommandParser;

// CLI 应用启动器（Headless 模式）。
// 职责：
//   1. 初始化 QCoreApplication（不需要 GUI）
//   2. 创建 AppContext 并初始化必需的服务
//   3. 注册所有子命令到 CliCommandParser
//   4. 解析命令行参数并分发到对应子命令
//   5. 输出结果到 stdout/stderr，返回退出码
class CliApplication
{
public:
    CliApplication(int argc, char* argv[]);
    ~CliApplication();

    // 禁止拷贝
    CliApplication(const CliApplication&) = delete;
    CliApplication& operator=(const CliApplication&) = delete;

    // 执行 CLI 工作流，返回退出码
    int run();

    // 返回 AppContext（供测试使用）
    AppContext* appContext() const;

private:
    // 初始化上下文中的服务
    void initializeServices();

    // 注册所有子命令
    void registerCommands();

    QCoreApplication m_app; // 必须先于其他成员构造

    std::unique_ptr<AppContext> m_context;
    std::unique_ptr<CliCommandParser> m_parser;
};

#endif // CLIAPPLICATION_H
