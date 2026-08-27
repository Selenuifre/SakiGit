# UI 层

## 模块作用

用户界面层。基于 Qt Widget 的主窗口、页面、对话框和辅助组件。
通过 `QAbstractItemModel*` 绑定数据（不依赖具体 Model 类），通过信号发射用户操作给 Controller。

**文件（10 个组件）：** `mainwindow.h`、`repositorysidebar.h`、`changespage.h`、`historypage.h`、
`branchespage.h`、`diffviewer.h`、`commitpanel.h`、`clonedialog.h`、`preferencesdialog.h`、`toastmanager.h`

---

## 暴露和调用的接口

### MainWindow（`QMainWindow`）

| 分类 | 接口 |
|---|---|
| **页面枚举** | `Page { ChangesPageIdx=0, HistoryPageIdx, BranchesPageIdx, SettingsPageIdx }` |
| **Model setter（5 个）** | `setRepositoryModel(QAbstractItemModel*)` `setFileChangeModel` `setDiffLineModel` `setCommitHistoryModel` `setBranchListModel` |
| **仓库信息** | `setCurrentRepository(name, path)` `currentRepositoryName/Path()` |
| **页面/状态** | `setCurrentPage(Page)` `currentPage()` `setStatusMessage(msg,timeout)` `setBusy(bool)` `isBusy()` |
| **信号（17 个）** | `openRepositoryRequested` `cloneRepositoryRequested` `refreshRequested` `fetchRequested` `pullRequested` `pushRequested` `commitRequested(msg)` `repositoryActivated(index)` `fileChangeActivated(index)` `commitActivated(index)` `removeRepositoryRequested(index)` `favoriteRepositoryRequested(index,bool)` `createBranchRequested(name)` `deleteBranchRequested(name)` `checkoutRequested(name)` `mergeRequested(name)` `stageRequested(index)` `unstageRequested(index)` `stageAllRequested` `unstageAllRequested` `pageChanged(int)` |
| **内部子控件** | `RepositorySidebar*` `ChangesPage*` `HistoryPage*` `BranchesPage*` `QTabBar*` `QStackedWidget*` |

### RepositorySidebar（`QWidget`）

| 分类 | 接口 |
|---|---|
| **数据** | `setRepositoryModel(QAbstractItemModel*)` `repositoryModel()` `proxyModel()` `listView()` |
| **选择/过滤** | `setCurrentRepositoryIndex` `currentRepositoryIndex` `setFilterText/filterText` `setEmptyText/emptyText` |
| **状态** | `setBusy(bool)` `isBusy()` |
| **信号（8 个）** | `openRepositoryRequested` `cloneRepositoryRequested` `refreshRequested` `repositoryActivated` `repositorySelectionChanged` `removeRepositoryRequested` `favoriteRepositoryRequested` `filterTextChanged` |

### ChangesPage（`QWidget`）

| 分类 | 接口 |
|---|---|
| **数据** | `setModel(QAbstractItemModel*)`（→ FileChangeModel） `setDiffModel(QAbstractItemModel*)`（→ DiffLineModel） |
| **子组件** | `diffViewer()` `commitPanel()` `listView()` |
| **信号（5 个）** | `fileActivated(index)` `stageRequested(index)` `unstageRequested(index)` `stageAllRequested()` `unstageAllRequested()` |

### HistoryPage（`QWidget`）

| 分类 | 接口 |
|---|---|
| **数据** | `setModel(QAbstractItemModel*)` `setDiffModel(QAbstractItemModel*)` `setCommitDetail(QString)` |
| **子组件** | `diffViewer()` `listView()` |
| **信号（2 个）** | `commitSelected(index)` `loadMoreRequested()` |

### BranchesPage（`QWidget`）

| 分类 | 接口 |
|---|---|
| **数据** | `setModel(QAbstractItemModel*)` `listView()` |
| **信号（5 个）** | `branchSelected(index)` `createBranchRequested(name)` `deleteBranchRequested(name)` `checkoutRequested(name)` `mergeRequested(name)` |

### DiffViewer（`QWidget`）

| 分类 | 接口 |
|---|---|
| **数据** | `setModel(QAbstractItemModel*)` `model()` `tableView()` `clear()` `setPlaceholderText(text)` |

### CommitPanel（`QWidget`）

| 分类 | 接口 |
|---|---|
| **数据** | `setMessage(text)` `message()` `clear()` `setCommitEnabled(bool)` `setReadOnly(bool)` |
| **信号（2 个）** | `commitRequested(message)` `messageChanged(text)` |

### CloneDialog（`QDialog`）

| 分类 | 接口 |
|---|---|
| **数据** | `setDefaultPath(path)` `setUrl/url()` `setTargetPath/targetPath()` |
| **信号** | `cloneRequested(url, targetPath)` |

### PreferencesDialog（`QDialog`）

| 分类 | 接口 |
|---|---|
| **数据** | `setDefaultRepositoryPath/get` `setGitUserName/get` `setGitUserEmail/get` |
| **信号** | `settingsChanged()` |

### ToastManager（`QObject`，静态方法）

| 分类 | 接口 |
|---|---|
| **显示消息** | `showMessage(parent, msg, timeout=3000)`（蓝色） `showError(parent, msg, timeout=5000)`（红色） `showSuccess(parent, msg, timeout=3000)`（绿色） |

### 调用方

- **app/Application** — `connectMainWindowSignals` 将 MainWindow 信号绑定到 Controller
- 所有 UI 组件通过 `QAbstractItemModel*` 绑定，不由外部直接操作

---

## 主要函数

### MainWindow 页面创建与信号转发
```
setupCentralArea():
  1. 创建 QTabBar（Changes/History/Branches/Settings）
  2. 创建 QStackedWidget：
     - ChangesPage → 转发 listView::activated → fileChangeActivated
                    转发 stage/unstage/stageAll/unstageAll 信号
                    转发 commitPanel::commitRequested → commitRequested
     - HistoryPage → 转发 listView::activated → commitActivated
     - BranchesPage → 转发 create/delete/checkout/merge 信号
     - SettingsPage（占位）
  3. RepositorySidebar → 转发 open/clone/refresh/repositoryActivated/remove/favorite 信号
```

### RepositorySidebar 过滤与右键菜单
```
setRepositoryModel(model):
  → m_proxyModel->setSourceModel(model)
  → 按 DisplayNameRole 不区分大小写过滤
  → 连接 model 信号到 updateEmptyState

showContextMenu(position):
  → 显示右键菜单：Open / Clone / Refresh / Favorite / Remove
  → Favorite 动作通过 data(FavoriteRole) 读取当前状态并翻转
```

### DiffViewer 表格绑定
```
setModel(model):
  → m_tableView->setModel(model)
  → 有数据 → 隐藏占位 → 显示表格 → 自动调整列宽
  → 无数据 → 显示占位文本
```

### ToastManager 自毁消息提示
```
showMessage(parent, msg, timeout):
  → 创建 QLabel 覆盖在 parent 上（底部居中）
  → 根据类型（error/success/default）设置不同背景色
  → QTimer::singleShot(timeout) → hide() + deleteLater()
```
