// ============================================================================
// CliMain.cpp —— CLI 入口组织文件
//
// 按二五计划阶段 7 的规划，此文件为 CLI 入口点。
//
// 实现说明：由于 Qt AUTOMOC 在多可执行文件目标共享头文件时的 CMake 限制，
// 当前采用单一可执行文件方案。CLI 模式由 main.cpp 检测命令行参数后，
// 委托给 CliApplication 处理。此文件作为 CLI 模块的入口组织文件存在，
// 保持与二五计划文件结构的一致性。
//
// 未来当构建系统支持独立的 headless 可执行文件时，可在此文件中实现
// 独立的 main() 函数，编译为 sakigit 可执行文件。
//
// 用法（通过 SakiGit 可执行文件）：
//   SakiGit commit-message --repo <path>
//   SakiGit review --repo <path> [--staged] [--strict]
// ============================================================================

#include "cli/CliApplication.h"

// cliMain 作为 CLI 的入口函数，由 main.cpp 在检测到 CLI 子命令时调用。
// 此设计使 CLI 模块保持独立，同时避免 CMake 多目标 AUTOMOC 冲突。
int cliMain(int argc, char* argv[])
{
    CliApplication cliApp(argc, argv);
    return cliApp.run();
}
