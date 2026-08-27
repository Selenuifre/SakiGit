# MODELS 层

## 模块作用

Qt Model/View 架构中的数据模型层。将 domain 对象包装为 `QAbstractTableModel` / `QAbstractListModel`，
供 UI 组件直接绑定。Model 不调用任何 Service，不持有 Controller 引用，不做 git 操作。
唯一上游是 Controller（调用 setter 写入数据），唯一下游是 Qt View（自动刷新）。

**文件：** `filechangemodel.h`、`difflinemodel.h`、`commithistorymodel.h`、`branchlistmodel.h`、`repositorylistmodel.h`

---

## 暴露和调用的接口

### FileChangeModel（`QAbstractTableModel`，3 列）

| 分类 | 接口 |
|---|---|
| **列定义** | `FilePathColumn` / `StatusColumn` / `StagedColumn` |
| **规范 Role** | `FilePathRole` `StatusRole` `StagedRole`（+ 17 个扩展 Role） |
| **规范方法** | `setChanges(vector)` `changeAt(row)` `filePathAt(row)` `clear()` |
| **扩展方法** | `setFileChanges/addFileChange/updateFileChange/upsertFileChange/removeFileChangeAt/removeFileChange` |
| **查询方法** | `fileChanges()` `isEmpty()` `count()` `fileChangeAt()` `fileChangeForPath()` `indexForPath()` `indexOfPath()` `containsPath()` |
| **筛选方法** | `stagedChanges()` `unstagedChanges()` `conflictedChanges()` `stagedCount()` `unstagedCount()` `conflictedCount()` |
| **信号** | `fileChangesChanged` `fileChangeAdded` `fileChangeUpdated` `fileChangeRemoved` |

### DiffLineModel（`QAbstractTableModel`，3 列）

| 分类 | 接口 |
|---|---|
| **列定义** | `OldLineNumberColumn` / `NewLineNumberColumn` / `ContentColumn` |
| **Role** | `OldLineNumberRole` `NewLineNumberRole` `ContentRole` `LineTypeRole` `LineTypeTextRole` |
| **方法** | `setDiff(const Diff&)` `setLines(vector)` `lineAt(row)` `clear()` |
| **视觉增强** | `ForegroundRole` 返回绿/红色、`BackgroundRole` 返回浅绿/浅红、`ToolTipRole` 返回类型文本 |

### CommitHistoryModel（`QAbstractTableModel`，4 列）

| 分类 | 接口 |
|---|---|
| **列定义** | `HashColumn` / `MessageColumn` / `AuthorColumn` / `DateColumn` |
| **规范 Role** | `HashRole` `ShortHashRole` `MessageRole` `AuthorRole` `DateRole`（+ 18 个扩展 Role） |
| **规范方法** | `setCommits(vector)` `appendCommits(vector)` `commitAt(row)` `clear()` |
| **扩展方法** | `addCommit/updateCommit/upsertCommit/removeCommitAt/removeCommit` |
| **查询方法** | `commits()` `isEmpty()` `count()` `commitForHash()` `indexForHash()` `indexOfHash()` `containsHash()` |
| **高级查询** | `mergeCommits()` `commitsTouchingFile()` `mergeCommitCount()` `latestCommit()` |
| **哈希匹配** | `hashMatches` 支持完整/短哈希双向匹配 |
| **信号** | `commitsChanged` `commitAdded` `commitUpdated` `commitRemoved` |

### BranchListModel（`QAbstractListModel`）

| 分类 | 接口 |
|---|---|
| **Role** | `NameRole` `IsCurrentRole` `IsRemoteRole` |
| **方法** | `setBranches(vector)` `branchAt(row)` `branchNameAt(row)` `clear()` |

### RepositoryListModel（`QAbstractListModel`）

| 分类 | 接口 |
|---|---|
| **规范 Role** | `NameRole` `PathRole` `CurrentBranchRole`（+ 14 个扩展 Role） |
| **方法** | `setRepositories(vector)` `addRepository` `removeRepository(path)` `repositoryAt(row)` `clear()` |
| **扩展方法** | `repositories()` `isEmpty()` `count()` `repositoryForPath()` `indexForPath()` `containsPath()` `updateRepository` `upsertRepository` `removeRepositoryAt` `setFavorite` |
| **信号** | `repositoriesChanged` `repositoryAdded` `repositoryUpdated` `repositoryRemoved` |

### 调用方

- **controllers/** — 所有 Controller 通过 `Model*` 指针调用 setter 写入数据
- **ui/** — 通过 `QAbstractItemModel*` 基类指针绑定到 `QListView` / `QTableView`，Qt 自动刷新

---

## 主要函数

### 数据加载流程（所有 Model 通用模式）
```
Controller::loadXxx() → Service::getXxx() → Result<QList<T>>
  → QList<T> → std::vector<T> 转换
  → Model::setXxx(vector) → beginResetModel/endResetModel
  → Qt 自动通知所有绑定的 View 刷新
```

### 各 Model 特有的核心方法

- **FileChangeModel::setFileChanges** — 整体替换，自动去重（按路径），过滤无效项
- **FileChangeModel::indexOfPath** — 路径匹配查找（反斜杠转正斜杠规范化后比较）
- **DiffLineModel::setDiff(Diff)** — 遍历 `diff.files()` → 每个文件的所有 `hunks` → 展开所有 `DiffLine` 行
- **CommitHistoryModel::appendCommits** — 批量追加（`beginInsertRows/endInsertRows`），避免整体重置
- **CommitHistoryModel::hashMatches** — 规范化（trim+toLower）后比较：完全相等 / startsWith 双向匹配
- **RepositoryListModel::setData** — 支持 `FavoriteRole` / `NameRole` / `CurrentBranchRole` / `DefaultBranchRole` / `RemoteNameRole` / `RemoteUrlRole` / `ProviderRole` / `StateRole` / `LastOpenedAtRole` 写入
