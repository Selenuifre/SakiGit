#include "CliApplication.h"

#include "CliCommandParser.h"
#include "GenerateCommitCommand.h"
#include "ReviewCommand.h"
#include "app/AppContext.h"
#include "services/CommitMessageService.h"
#include "services/CodeReviewService.h"
#include "services/SettingsService.h"

#include <QTextStream>

CliApplication::CliApplication(int argc, char* argv[])
    : m_app(argc, argv)
{
}

CliApplication::~CliApplication()
{
    if (m_context) {
        m_context->shutdown();
    }
}

int CliApplication::run()
{
    // 初始化服务和命令
    initializeServices();
    registerCommands();

    // 解析命令行参数（使用成员 m_app 而非静态方法，避免 QApplication 实例警告）
    const QStringList args = m_app.arguments();
    const auto parseResult = m_parser->parse(args);

    // 处理帮助请求
    if (parseResult.showHelp) {
        QTextStream out(stdout);
        if (!parseResult.helpCommandName.isEmpty()) {
            out << m_parser->commandHelp(parseResult.helpCommandName);
        } else {
            out << m_parser->globalHelp();
        }
        return 0;
    }

    // 处理解析失败
    if (!parseResult.ok) {
        QTextStream err(stderr);
        err << parseResult.errorMessage << "\n\n";
        err << m_parser->globalHelp();
        return CliExitCode::GeneralError;
    }

    // 执行命令
    const CliResult result = parseResult.command->run(parseResult.commandArgs);

    // 输出结果
    if (!result.stdOut.isEmpty()) {
        QTextStream out(stdout);
        out << result.stdOut;
        // 如果不是以换行结尾，添加换行
        if (!result.stdOut.endsWith('\n')) {
            out << "\n";
        }
    }

    if (!result.stdErr.isEmpty()) {
        QTextStream err(stderr);
        err << result.stdErr;
        if (!result.stdErr.endsWith('\n')) {
            err << "\n";
        }
    }

    return result.exitCode;
}

AppContext* CliApplication::appContext() const
{
    return m_context.get();
}

void CliApplication::initializeServices()
{
    // 设置应用元信息（与 GUI Application::configureApplicationMetadata() 一致）
    QCoreApplication::setOrganizationName(QStringLiteral("SakiGit"));
    QCoreApplication::setApplicationName(QStringLiteral("SakiGit"));

    m_context = std::make_unique<AppContext>();
    m_context->initialize();
}

void CliApplication::registerCommands()
{
    m_parser = std::make_unique<CliCommandParser>();

    // 注册 commit-message 命令
    m_parser->registerCommand(
        std::make_unique<GenerateCommitCommand>(
            m_context->commitMessageService(),
            m_context->settingsService()));

    // 注册 review 命令
    m_parser->registerCommand(
        std::make_unique<ReviewCommand>(
            m_context->codeReviewService(),
            m_context->settingsService()));
}
