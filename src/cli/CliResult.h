#ifndef CLIRESULT_H
#define CLIRESULT_H

#include <QString>

// CLI 命令执行结果。
// 统一封装退出码、标准输出和错误输出。
// 退出码约定：
//   0 = 成功
//   1 = 一般错误（参数无效、repo 路径不存在、无 staged diff 等）
//   2 = AI 服务错误（网络超时、API 返回错误等）
//   3 = AI 未配置（SettingsService 中缺少 API Key）
struct CliResult
{
    int     exitCode = 0;     // 0 = 成功，非 0 = 失败
    QString stdOut;           // 标准输出（JSON 格式，供脚本解析）
    QString stdErr;           // 错误输出（人类可读）

    // 创建成功结果
    static CliResult success(const QString& output = QString())
    {
        CliResult r;
        r.exitCode = 0;
        r.stdOut = output;
        return r;
    }

    // 创建失败结果
    static CliResult failure(int code, const QString& error)
    {
        CliResult r;
        r.exitCode = code;
        r.stdErr = error;
        return r;
    }
};

// 退出码常量
namespace CliExitCode {
    constexpr int Success        = 0;
    constexpr int GeneralError   = 1;  // 参数/路径/diff 为空等
    constexpr int AIServiceError = 2;  // AI 网络/API 错误
    constexpr int AINotConfigured = 3; // AI 未配置
}

#endif // CLIRESULT_H
