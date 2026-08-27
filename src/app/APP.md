# APP 层

## 模块作用

程序启动与整合层。创建全局对象（Logger、Service、Model、Controller、MainWindow），
初始化日志系统，连接 UI 信号到 Controller，运行 Qt 事件循环。

**文件：** `application.h`、`appcontext.h` + 根目录 `main.cpp`

---

## 暴露和调用的接口

### Application — 启动器（非 QObject，`final`）

| 分类 | 接口 |
|---|---|
| **构造** | `Application(QApplication&)` |
| **生命周期** | `initialize() → Result<void>` — 顺序执行元数据设置、Logger 初始化、Service 创建、MainWindow 创建、信号连接 |
| **运行** | `run() → int` — `showMainWindow()` + `QApplication::exec()` |
| **关闭** | `shutdown()` / `~Application()` — 保存窗口状态 → 销毁 Window/Service → sync Settings → 卸载 Logger |
| **访问器** | `settingsService()` `gitService()` `gitTaskRunner()` `mainWindow()` `lastError()` |
| **配置** | `updateGitExecutablePath(path)` — 同步更新 settings + gitService + gitTaskRunner |

### AppContext — 全局上下文（`QObject`）

| 分类 | 接口 |
|---|---|
| **生命周期** | `initialize()` — `createDefaultObjects()` → `applyGitExecutablePath()` → `loadRecentRepositories()`、`shutdown()` — 保存 → 清空 → 销毁 |
| **持有对象** | `SettingsService` `GitService` `GitTaskRunner` `RepositoryListModel` `FileChangeModel` `CommitHistoryModel`（全部 `unique_ptr`） |
| **仓库追踪** | `setCurrentRepository(repo)` `currentRepository()` `clearCurrentRepository()` `hasCurrentRepository()` `currentRepositoryPath()` |
| **最近仓库** | `loadRecentRepositories()` `saveRecentRepositories()` `addRecentRepository(repo)` `removeRecentRepository(path)` `clearRepositoryData()` |
| **MainWindow** | `setMainWindow(MainWindow*)`（非拥有指针） `mainWindow()` |
| **信号** | `initialized()` `shutdownRequested()` `mainWindowChanged(MainWindow*)` `currentRepositoryChanged(Repository)` `currentRepositoryCleared()` |

### main.cpp — 程序入口

| 分类 | 接口 |
|---|---|
| **双模式启动** | `Normal` — `QApplication` + `Application::run()`；`Test` — `QCoreApplication` + 模板函数 `runSingleTest<T>(name)` |
| **测试参数** | `--run-diffparsetest` `--run-gitcommandexecutortest` `--run-repositoryservicetest` `--run-branchservicetest` `--run-syncservicetest` `--run-settingsservicetest` `--run-filechangemodeltest` `--run-commithistorymodeltest` `--run-branchlistmodeltest` `--run-all-tests` `--help` |
| **测试模板** | `runSingleTest<T>(name)` — 构造 `T test` → `test.runAll(&err)` → 输出 PASS/FAIL；`runAndReport<T>(out,err,name)` — 用于 `runAllTests` 中逐个报告 |

---

## 主要函数

### `Application::initialize()` — 启动初始化全流程
```
1. configureApplicationMetadata()
   → QCoreApplication::setOrganizationName/ApplicationName/ApplicationVersion
2. initializeLogger()
   → Logger::instance() 配置控制台+文件输出（Debug 级别）
   → 日志路径 = <AppLocalDataLocation>/logs/sakigit.log
   → installQtMessageHandler()
3. initializeServices()
   → 创建 SettingsService(orgName, appName)
   → 创建 GitService + GitTaskRunner(this)
   → 从 settings 读取 gitExecutablePath 同步到两个服务
4. initializeMainWindow()
   → 创建 MainWindow
   → restoreMainWindowState() 恢复窗口几何和停靠状态
5. connectApplicationSignals()
   → QApplication::aboutToQuit → saveMainWindowState()
6. m_initialized = true → 记录日志 "Application initialized"
```

### `Application::run()` — 启动并进入事件循环
```
showMainWindow() → initialize()（幂等）→ m_mainWindow->show/raise/activateWindow
  → m_qtApplication.exec()（阻塞等待事件）
退出后：检查 lastError → 记录到 Logger
```

### `Application::shutdown()` — 有序关闭
```
saveMainWindowState() → 断开 aboutToQuit 连接
→ reset mainWindow → reset gitTaskRunner → reset gitService
→ sync() + reset settingsService
→ Logger::uninstallQtMessageHandler()
```

### `main.cpp` 中的测试运行器
```
detectRunMode(argv) → 匹配 --run-xxx 参数
  → 测试模式：创建 QCoreApplication → runSingleTest<T>(name)
  → 正常模式：创建 QApplication → Application(qtApp).run()
```
