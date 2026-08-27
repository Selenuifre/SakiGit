# INFRASTRUCTURE 层

## 模块作用

基础工具层。提供整个项目共用的底层能力：统一返回值模型（`Result<T>`）、结构化错误（`Error`）、
线程安全日志系统（`Logger` 单例）、以及 Git 子进程封装（`GitCommandExecutor`）。
位于架构最底层，不依赖 domain/services/models/controllers/ui 中任何代码。

**文件：** `error.h`、`result.h`、`logger.h`、`gitcommandexecutor.h`

---

## 暴露和调用的接口

### Error 类

| 分类 | 接口 |
|---|---|
| **错误码枚举** | `Code { None, Unknown, InvalidArgument, NotFound, PermissionDenied, ProcessFailed, Timeout, GitError, IOError, ConfigurationError }` |
| **构造** | `Error()` `Error(code, msg)` `Error(code, msg, detail)` |
| **静态工厂** | `none()` `unknown(msg)` `invalidArgument(msg)` `notFound(msg)` `gitError(msg)` |
| **访问器** | `code()` `message()` `detail()` `context()` `contextValue(key)` `isValid()` `isEmpty()` |
| **修改器** | `setDetail(str)` `setContextValue(k,v)`  `withContextValue(k,v) const`（链式调用） |
| **诊断** | `toString()` → `[CodeName] message Detail: ... Context: k=v,...` |

### Result\<T\> 模板

| 分类 | 接口 |
|---|---|
| **静态工厂** | `Result<T>::success(val)` `Result<T>::failure(str)` `Result<T>::failure(Error)` |
| **访问器** | `isSuccess()` `isFailure()` `hasValue()` `value()` `valueOr(def)` `errorMessage()` `error()` |
| **特化** | `Result<void>` — 无 `hasValue/value/valueOr`，`success()` 无参数 |

### Logger 单例

| 分类 | 接口 |
|---|---|
| **获取实例** | `static Logger& instance()`（Meyers Singleton） |
| **日志级别** | `enum Level { Debug=0, Info, Warning, Error }` |
| **日志输出** | `debug/info/warning/error(msg, category)` → 均调用 `log(level, msg, category)` |
| **配置** | `setLogFilePath/logFilePath` `setMinimumLevel/minimumLevel` `setConsoleEnabled/FileEnabled` |
| **Qt 集成** | `installQtMessageHandler()` `uninstallQtMessageHandler()`（将 `qDebug/qWarning` 路由到 Logger） |
| **线程安全** | 所有读写通过 `QMutexLocker` 保护 `m_mutex` |

### GitCommandExecutor 类

| 分类 | 接口 |
|---|---|
| **命令结果** | `struct CommandResult { int exitCode; QString standardOutput; QString standardError; bool isSuccess(); QString message(); }` |
| **配置** | `gitExecutablePath()` / `setGitExecutablePath(path)` 、`timeoutMs()`/`setTimeoutMs(ms)` |
| **核心方法** | `Result<CommandResult> run(arguments, workingDirectory="")` — 启动 `QProcess` 执行 git 命令 |
| **高层方法** | `checkGitAvailable()` → `git --version`、`isRepository(path)` → `rev-parse --is-inside-work-tree`、`currentBranch(path)` → `branch --show-current`、`repositoryRoot(path)` → `rev-parse --show-toplevel`、`remoteUrl(path,remote)` → `remote get-url`、`statusPorcelain(path)` → `status --porcelain`、`diff(path,file)` → `diff`、`log(path,maxCount)` → `log --date=iso-strict --pretty=...`、`stageFile/unstageFile/commit` |

### 调用方

- `services/GitService` — 值组合 `GitCommandExecutor`，调用 `run()` 并将结果转为 domain 对象
- `services/GitTaskRunner` — 值组合 `GitService`，在 `QtConcurrent::run` 中异步调用
- `app/Application` — 初始化 Logger、设置日志文件路径
- 所有层 — 使用 `Result<T>` 作为统一返回值、使用 `Error` 构造错误信息

---

## 主要函数

### `GitCommandExecutor::run(arguments, workingDirectory)`
1. 创建 `QProcess`，设置工作目录（`QDir::cleanPath`）
2. `process.start(m_gitExecutablePath, arguments)`
3. `waitForStarted(timeout)` 失败 → 返回 `Result::failure(Error::ProcessFailed)`（含 context: executable, arguments, workingDirectory）
4. `waitForFinished(timeout)` 失败 → kill 进程 → 返回 `Result::failure(Error::Timeout)`
5. 成功 → 读取 `exitCode`、`stdout`(`fromLocal8Bit`)、`stderr`(`fromLocal8Bit`) → 构造 `CommandResult`

### `Logger::log(level, message, category)`
1. `shouldLog(level)` 检查级别 → false 直接返回
2. `formatMessage` 拼接 `[yyyy-MM-dd HH:mm:ss.zzz] [LEVEL] [category] message`
3. `writeLine` — 若 `m_consoleEnabled` → 输出 `stderr`；若 `m_fileEnabled` → 追加写入日志文件

### `Error::withContextValue(key, value)`
返回 `Error` 副本并在副本上调用 `setContextValue`。支持链式：`Error::gitError("msg").withContextValue("k1","v1").withContextValue("k2","v2")`

### `Result<T>::failure(Error)`
若传入的 `Error` 无效（`isValid()==false`），自动 fallback 为 `Error::unknown("Operation failed.")`，确保失败结果始终携带有效错误信息。
