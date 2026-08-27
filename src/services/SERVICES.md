# SERVICES 层

## 模块作用

业务逻辑层。封装 Git 操作，将 `GitCommandExecutor` 的原始命令行输出解析为 domain 对象，
以 `Result<T>` 返回给 Controller。Services 不持有 UI/Model/Controller 引用。

**文件：** `gitservice.h`（核心，35+ 方法）、`gittaskrunner.h`（异步）、`branchservice.h`、
`repositoryservice.h`、`settingsservice.h`、`syncservice.h`

---

## 暴露和调用的接口

### GitService — 核心业务服务

拥有 `GitCommandExecutor m_executor`（值组合），所有方法同步返回 `Result<T>`。

| 分类 | 方法 | 返回类型 | 对应 Git 命令 |
|---|---|---|---|
| **读取-仓库** | `openRepository(path)` `checkGitAvailable()` `isGitRepository(path)` | `Result<Repository>` `Result<void>` `Result<bool>` | `rev-parse` `--version` |
| **读取-状态** | `status(path)` `diff(path,file)` `rawDiff(path,file)` | `Result<QList<FileChange>>` `Result<Diff>` `Result<QString>` | `status --porcelain` `diff` |
| **读取-历史** | `commitHistory(path,max)` `branches(path)` `currentBranch(path)` | `Result<QList<Commit>>` `Result<QList<Branch>>` `Result<QString>` | `log` `for-each-ref` |
| **暂存/提交** | `stageFile` `unstageFile` `commit` | `Result<void>` | `add` `restore` `commit` |
| **同步** | `fetch` `pull` `push` | `Result<void>` | `fetch` `pull` `push` |
| **分支** | `createBranch` `checkoutBranch` `deleteBranch` `merge` `renameBranch` | `Result<void>` | `branch` `checkout` `merge` |
| **远程** | `listRemotes` `addRemote` `removeRemote` | `Result<QStringList>` `Result<void>` | `remote` |
| **仓库操作** | `init` `clone` | `Result<void>` | `init` `clone` |

### GitTaskRunner — 异步执行器（`QObject`，14 槽 + 18 信号）

拥有 `GitService m_service`（值组合）。槽函数将任务投递到 `QtConcurrent::run` 线程池，
完成后通过 `Qt::QueuedConnection` 回调主线程发信号。每个完成信号格式：`xxxFinished(bool success, ..., QString errorMessage)`。

| 槽函数 | 对应信号 |
|---|---|
| `checkGitAvailable()` | `gitAvailableChecked` |
| `openRepository(path)` | `repositoryOpened(success, path, Repository, err)` |
| `loadStatus(path)` | `statusLoaded(success, path, QList<FileChange>, err)` |
| `loadDiff(path,file)` | `diffLoaded(success, path, file, Diff, err)` |
| `loadRawDiff(path,file)` | `rawDiffLoaded(success, path, file, text, err)` |
| `loadCommitHistory(path,max)` | `commitHistoryLoaded(success, path, QList<Commit>, err)` |
| `loadBranches(path)` | `branchesLoaded(success, path, QList<Branch>, err)` |
| `loadCurrentBranch(path)` | `currentBranchLoaded(success, path, branchName, err)` |
| `stageFile/unstageFile/commit` | `fileStaged/fileUnstaged/commitFinished` |
| `fetch/pull/push` | `fetchFinished/pullFinished/pushFinished` |

### 门面 Service（委托 GitService，增加空指针保护）

| Service | 注入依赖 | 方法 |
|---|---|---|
| **BranchService** | `GitService*` | `listBranches` `currentBranch` `createBranch` `checkoutBranch` `deleteBranch` `mergeBranch` `renameBranch` |
| **RepositoryService** | `GitService*` + `SettingsService*` | `openRepository` `initRepository` `cloneRepository` `closeRepository` `currentRepository` `currentBranch` `isGitRepository` `recentRepositories` `addRecentRepository` `removeRecentRepository` |
| **SyncService** | `GitService*` | `fetch` `pull` `push`（2 重载） `listRemotes` `addRemote` `removeRemote` |
| **SettingsService** | `QSettings`（值组合） | `recentRepositories` `addRecentRepository` `removeRecentRepository` `lastOpenedRepositoryPath` `defaultClonePath` `gitExecutablePath` `theme` `mainWindowGeometry/State` `value/setValue/contains/remove/sync/clear` |

### 调用方

- **controllers/** — 所有 Controller 通过 `Service*` 指针调用
- **app/AppContext** — 创建和持有所有 Service 的 `unique_ptr`
- **app/Application** — 创建 `GitService` + `GitTaskRunner`

---

## 主要函数

### `GitService::openRepository(path)` — 打开仓库全流程
1. `Repository::normalizePath` 规范化路径
2. `m_executor.isRepository(cleanPath)` 验证是 git 仓库
3. `m_executor.repositoryRoot(cleanPath)` 获取根目录
4. `m_executor.currentBranch()` 读取当前分支（best-effort，失败忽略）
5. `readDefaultBranch()` 读取默认分支（解析 `origin/HEAD`）
6. `m_executor.remoteUrl("origin")` 读取远程 URL（best-effort）
7. 构造 `Repository(rootPath, Ready 状态)` → `Result<Repository>::success`

### `GitService::status(path)` — 解析文件状态
1. `m_executor.statusPorcelain(path)` → 获取 `git status --porcelain` 原始输出
2. `parseStatusOutput(raw)` — 逐行解析 2 字符 porcelain 码 → `FileStatus` + `StageState`，自动检测重命名（` -> ` 分隔符）
3. 返回 `QList<FileChange>`

### `GitService::branches(path)` — 解析分支列表
1. `m_executor.currentBranch(path)` → 获取当前分支名（best-effort）
2. `m_executor.run(["for-each-ref", "--sort=-committerdate", "refs/heads", "refs/remotes"])` → 获取所有分支
3. `parseBranchOutput(raw)` — 逐行解析 tab 分隔字段（refname/shortname/hash/subject/date/upstream），自动计算 ahead/behind

### `GitTaskRunner` 异步模式（所有 14 个槽共用）
1. `startTask(name)` → `m_activeTaskCount++` → `emit busyChanged`
2. 拷贝 `GitService service = m_service`（值拷贝，线程安全）
3. `QPointer<GitTaskRunner> self(this)` — 防止 Runner 被销毁后回调
4. `QtConcurrent::run` → 工作线程同步调用 `service.method()` → 检查 `self`
5. `QMetaObject::invokeMethod(self, Qt::QueuedConnection)` → 主线程回调 → 检查 `self` → emit 完成信号 → `finishTask(name)`

### `SettingsService` 回退链
- `defaultClonePath`：设置值 → `QStandardPaths::DocumentsLocation` → `QDir::homePath()`
- `gitExecutablePath`：设置值 → `"git"`
- `theme`：设置值 → `"system"` → `Theme::System`
- `maxRecentRepositories`：设置值 → `20`
