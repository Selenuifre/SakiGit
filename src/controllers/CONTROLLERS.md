# CONTROLLERS 层

## 模块作用

控制层。连接 UI 信号到 Service 调用，将 Service 返回的 domain 数据写入 Model，通知 UI 刷新。
Controller 是 `UI ↔ Service ↔ Model` 的枢纽：持有 `Service*`（执行业务）和 `Model*`（写入数据），
通过 Qt signal/slot 与 UI 交互，但不持有 UI 引用。

**文件：** `maincontroller.h`、`repositorycontroller.h`、`changescontroller.h`、
`historycontroller.h`、`branchcontroller.h`、`synccontroller.h`

---

## 暴露和调用的接口

### MainController — 总协调器

| 分类 | 接口 |
|---|---|
| **依赖注入（7 个 setter）** | `setRepositoryController` `setChangesController` `setHistoryController` `setBranchController` `setSyncController` `setGitService` `setSettingsService` |
| **操作** | `switchToRepository(path)` `openRepository(path)` `closeCurrentRepository()` `refreshAll()` |
| **查询** | `currentRepositoryPath()` `repositoryController()` `changesController()` `historyController()` `branchController()` `syncController()` |
| **信号** | `repositorySwitched(path)` `repositoryClosed()` `refreshCompleted()` `globalError(title, msg)` |
| **私有槽** | `onSubControllerError(op, msg)` — 汇总 5 个子控制器的 `errorOccurred` 并转发为 `globalError` |

### RepositoryController

| 分类 | 接口 |
|---|---|
| **构造注入** | `RepositoryService*` + `RepositoryListModel*` |
| **方法** | `openRepository(path)` `initRepository(path)` `cloneRepository(url,target)` `closeRepository(path)` `loadRecentRepositories()` `addRecentRepository(repo)` `removeRecentRepository(path)` `clearRecentRepositories()` `isGitRepository(path)` `repositoryListModel()` |
| **信号** | `repositoryOpened(repo)` `openFailed(path,err)` `repositoryInitialized(repo)` `repositoryCloned(repo)` `cloneFailed(url,err)` `recentRepositoriesLoaded()` `errorOccurred(op,err)` |

### ChangesController

| 分类 | 接口 |
|---|---|
| **构造注入** | `GitService*` + `FileChangeModel*` + `DiffLineModel*` |
| **方法** | `refreshChanges(path)` `loadDiff(path,file)` `stageFile/unstageFile(path,file)` `stageAllFiles/unstageAllFiles(path)` `commit(path,msg)` `discardChanges(path,file)`（预留） `fileChangeModel()` `diffLineModel()` `clear()` |
| **信号** | `changesRefreshed(success,err)` `diffLoaded(success,file,err)` `fileStaged(success,file,err)` `fileUnstaged(success,file,err)` `commitFinished(success,err)` `errorOccurred(op,err)` |

### HistoryController

| 分类 | 接口 |
|---|---|
| **构造注入** | `GitService*` + `CommitHistoryModel*` + `DiffLineModel*` |
| **方法** | `loadHistory(path,max=100)` `getCommitDetail(path,hash)` `loadCommitDiff(path,hash)` `filterByAuthor(author)` `searchByMessage(keyword)` `commitHistoryModel()` `diffLineModel()` `clear()` |
| **信号** | `historyLoaded(success,err)` `commitDiffLoaded(success,hash,err)` `errorOccurred(op,err)` |

### BranchController

| 分类 | 接口 |
|---|---|
| **构造注入** | `BranchService*` + `BranchListModel*` |
| **方法** | `loadBranches(path)` `createBranch(path,name)` `checkoutBranch(path,name)` `deleteBranch(path,name,force)` `mergeBranch(path,name)` `renameBranch(path,old,new)` `currentBranchName(path)` `branchListModel()` `clear()` |
| **信号** | `branchesLoaded(success,err)` `operationFinished(op,success,err)` `errorOccurred(op,err)` |

### SyncController

| 分类 | 接口 |
|---|---|
| **构造注入** | `SyncService*` |
| **方法** | `fetch(path,remote="origin")` `pull(path)` `push(path)` `push(path,remote,branch)` `listRemotes(path)` `addRemote(path,name,url)` `removeRemote(path,name)` |
| **信号** | `fetchFinished(success,err)` `pullFinished(success,err)` `pushFinished(success,err)` `remoteOperationFinished(op,success,err)` `errorOccurred(op,err)` |

### 调用方

- **ui/MainWindow** — 通过 `connect` 将 UI 信号绑定到 Controller 方法
- **app/Application** — 创建 Controller、注入 Service 和 Model、连接 UI 信号
- **app/AppContext** — 持有 Controller 的 `unique_ptr`

---

## 主要函数

### 统一调用模式（所有 Controller 共用）

```
1. Service 空指针检查 → 失败则返回 Result::failure + emit errorOccurred
2. 调用 Service 方法 → 返回 Result<T>
3. 失败 → emit errorOccurred(op, err) + emit xxxFinished(false, err)
4. 成功 → 写入 Model（setter）→ emit xxxFinished(true, "")
5. 写操作（stage/commit/checkout 等）成功后自动刷新关联 Model
```

### 关键调用链

- **仓库切换**：`MainController::switchToRepository(path)` → 同时调用 `ChangesController::refreshChanges` + `HistoryController::loadHistory` + `BranchController::loadBranches` → 发出 `repositorySwitched`
- **暂存文件**：`ChangesController::stageFile(path, file)` → `GitService::stageFile` → 成功后 `refreshChanges(path)` 自动刷新文件列表
- **创建分支**：`BranchController::createBranch(path, name)` → `BranchService::createBranch` → 成功后 `loadBranches(path)` 自动刷新分支列表
- **加载 Diff**：`ChangesController::loadDiff(path, file)` → `GitService::diff` → `DiffLineModel::setDiff(diff)`
- **提交历史 Diff**：`HistoryController::loadCommitDiff(path, hash)` → `GitService::rawDiff` → `Diff::fromUnifiedDiff` → `DiffLineModel::setDiff`
- **错误汇总**：5 个子 Controller 的 `errorOccurred` 全部连接到 `MainController::onSubControllerError` → 转发为 `globalError(title, msg)` → UI 显示

### `MainController::switchToRepository` 内部流程
```
1. m_currentRepoPath = repoPath
2. m_changesController->refreshChanges(repoPath)
3. m_historyController->loadHistory(repoPath)
4. m_branchController->loadBranches(repoPath)
5. emit repositorySwitched(repoPath)
```
