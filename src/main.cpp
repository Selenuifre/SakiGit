#include "app/application.h"
#include "infrastructure/logger.h"
#include <QApplication>
#include <QCoreApplication>
#include <QTextStream>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
const char* HelpArgument                   = "--help";

enum class RunMode {
    Normal,                  // 正常运行图形界面
    Cli,                     // CLI 模式（commit-message / review 子命令）
    Help,                    // 显示帮助信息
};

RunMode detectRunMode(const QStringList& arguments)
{
    // =========================================================================
    // CLI 子命令检测
    // =========================================================================
    if (arguments.size() >= 2) {
        const QString firstArg = arguments.at(1);
        if (!firstArg.startsWith(QLatin1Char('-'))) {
            return RunMode::Cli;
        }
    }

    if (arguments.contains(QString::fromLatin1(HelpArgument))) {
        return RunMode::Help;
    }

    return RunMode::Normal;
}

void printHelp()
{
    QTextStream out(stdout);
    out << "SakiGit - Git Repository Analysis Tool\n"
        << "\n"
        << "Usage: SakiGit [options]\n"
        << "       SakiGit <command> [command-options]\n"
        << "\n"
        << "Commands:\n"
        << "  commit-message    Generate AI commit message from staged changes\n"
        << "  review            AI code review on staged or working tree changes\n"
        << "\n"
        << "Options:\n"
        << "  --help                           Show this help message\n"
        << "\n"
        << "Without any test options, SakiGit launches the graphical interface.\n";
}

} // anonymous namespace

int main(int argc, char* argv[])
{
    // 在创建 QApplication 之前检查运行模式，
    // 测试模式使用 QCoreApplication（无需 GUI 支持），
    // 正常模式使用 QApplication（完整图形界面）。
    // 直接从 argv 构建参数列表（不能在 QApplication 创建前调用 arguments()）
    QStringList arguments;
    arguments.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        arguments.append(QString::fromLocal8Bit(argv[i]));
    }

    const RunMode mode = detectRunMode(arguments);

    // Windows GUI 子系统默认无控制台，CLI/Help/Test 模式需要附加到父控制台
#ifdef Q_OS_WIN
    if (mode != RunMode::Normal) {
        AttachConsole(ATTACH_PARENT_PROCESS);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif

    switch (mode) {

    // =========================================================================
    // Production modes
    // =========================================================================

    case RunMode::Help: {
        QCoreApplication coreApp(argc, argv);
        printHelp();
        return 0;
    }

    case RunMode::Cli: {
        extern int cliMain(int argc, char* argv[]);
        return cliMain(argc, argv);
    }



    case RunMode::Normal:
    default:
        break;
    }

    // Production: 启动图形界面
    {
        QApplication qtApplication(argc, argv);
        Application application(qtApplication);

        const int exitCode = application.run();

        if (exitCode != 0 && !application.lastError().isEmpty()) {
            Logger::instance().error(application.lastError(), QStringLiteral("Main"));
        }

        return exitCode;
    }
}
