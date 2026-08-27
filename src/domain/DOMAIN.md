# DOMAIN 层

## 模块作用

纯数据结构层。定义 Git 相关实体的数据类型、枚举常量以及解析/转换工具函数。
不包含任何业务逻辑，不调用 Service，不依赖 Qt Widget，仅依赖 Qt Core。

**文件：** `gittypes.h`（核心）、`branch.h`、`commit.h`、`diff.h`、`filechange.h`、`repository.h`

---

## 暴露和调用的接口

### GitTypes 命名空间（`gittypes.h`）—— 被所有 domain 类和上层引用

**8 个枚举（全部 `Q_DECLARE_METATYPE` 注册到 Qt 元类型）：**
- `ObjectType` — Commit / Tree / Blob / Tag
- `RefType` — LocalBranch / RemoteBranch / Tag / Head
- `RepositoryState` — Ready / Missing / Merging / Rebasing 等 10 个状态
- `RemoteProvider` — GitHub / GitLab / Bitbucket / Other
- `BranchType` — Local / Remote（`Branch` 类通过 using 复用）
- `FileStatus` — Added / Modified / Deleted / Renamed / Conflicted 等 10 个状态
- `StageState` — Unstaged / Staged / PartiallyStaged / Conflict
- `DiffLineType` — Context / Added / Removed / HunkHeader / NoNewline

**3 个常量：** `LocalBranchRefPrefix`、`RemoteBranchRefPrefix`、`TagRefPrefix`

**16 个 inline 工具函数：**
- 路径类：`normalizePath`、`cleanDiffPath`
- Hash 类：`shortHash`
- Ref 类：`shortRefName`、`refTypeFromName`、`remoteNameFromBranchName`、`localNameFromRemoteBranchName`
- 平台识别：`remoteProviderFromUrl`
- Porcelain 解析：`fileStatusFromPorcelain`、`stageStateFromPorcelain`
- Diff 解析：`diffLineTypeFromLine`
- 转字符串：`toString()` 对全部 8 个枚举各一个重载

### 各 domain 类对外接口

| 类 | 构造 | 核心 getter/setter | 判断方法 | 显示方法 | 运算符 |
|---|---|---|---|---|---|
| **Branch** | `()` `(name)` `(name,type)` | name/fullName/type/remoteName/upstreamName/aheadCount/behindCount/headCommitHash/lastCommitSummary/lastCommitDate | isLocal/isRemote/isCurrent/hasUpstream/isSynced/hasDiverged/hasUnpushedCommits/hasUnpulledCommits/isValid | displayName/syncStatusText/shortHeadCommitHash | `==` `!=`（比较 fullName+type 或 name+type） |
| **Commit** | `()` `(hash)` `(hash,summary)` | hash/summary/body/message/authorName/authorEmail/authorDate/committerName/committerEmail/committerDate/parentHashes/changedFiles | isValid/isMergeCommit/hasBody | displayTitle/displayAuthor/displayDate/shortHash | `==` `!=`（按 hash） |
| **FileChange** | `()` `(path)` `(path,status)` `(path,status,stage)` | path/oldPath/status/stageState/indexStatus/worktreeStatus | isStaged/isUnstaged/isPartiallyStaged/isConflict/isDeleted/isRenamed/isUntracked/hasOldPath/isValid | displayPath/statusText/stageStateText/porcelainCode | `==` `!=`（按 path+oldPath+status+stage） |
| **DiffLine** | `()` `(type,text)` `(type,text,old,new)` | type/text/oldLineNumber/newLineNumber | isAdded/isRemoved/isContext/isHunkHeader | displayPrefix | — |
| **DiffHunk** | `()` `(header)` | header/oldStart/oldLineCount/newStart/newLineCount/lines | isValid | addedLineCount/removedLineCount | — |
| **FileDiff** | `()` `(path)` | oldPath/newPath/binary/newFile/deletedFile/renamed/hunks | isValid/isBinary/isNewFile/isDeletedFile/isRenamed | displayPath/addedLineCount/removedLineCount | — |
| **Diff** | `()` `(rawText)` | rawText/files | isEmpty/fileCount/addedLineCount/removedLineCount | — | — |
| **Repository** | `()` `(path)` `(path,name)` | id/name/localPath/currentBranch/defaultBranch/remoteName/remoteUrl/provider/state/lastOpenedAt/favorite | isValid/hasRemote/isMissing/isGitHubRepository | displayName | `==` `!=`（按 id） |

### 调用方

- **services/** — GitService 解析命令行输出为 domain 对象，返回 `Result<T>`
- **models/** — 以 domain 对象为数据源，包装为 Qt Model
- **controllers/** — 接收 Service 返回的 domain 对象，传给 Model
- **ui/** — 通过 Qt Model 的 data() 间接读取 domain 对象字段

---

## 主要函数

### `gittypes.h` 中最重要的工具函数

| 函数 | 输入 | 输出 | 逻辑 |
|---|---|---|---|
| `shortRefName` | `"refs/remotes/origin/main"` | `"origin/main"` | 去掉 `refs/heads/` / `refs/remotes/` / `refs/tags/` 前缀 |
| `refTypeFromName` | `"refs/heads/main"` | `RefType::LocalBranch` | 前缀匹配判断 |
| `fileStatusFromPorcelain` | `"M "` / `"??"` / `"UU"` | `FileStatus::Modified` 等 | 按优先级解析 2 字符 porcelain 码 |
| `stageStateFromPorcelain` | `"M "` / `"MM"` | `StageState::Staged` 等 | 比较索引状态和文件状态 |
| `diffLineTypeFromLine` | `"+added"` / `"-removed"` | `DiffLineType::Added` 等 | 判断行首字符（排除 `+++`/`---`） |
| `toString(FileStatus)` | `FileStatus::Added` | `"Added"` | switch 映射 |

### 各 domain 类的核心方法

- **Branch::setFullName(ref)** — 自动解析类型（`refTypeFromName`）+ 短名
- **Branch::setAheadCount/BehindCount** — 负数自动截断为 0
- **Branch::syncStatusText** — 返回 `"N ahead, M behind"` / `"Synced"` / `"No upstream"`
- **Commit::setMessage(msg)** — 自动拆分为 summary（首行）和 body（其余）
- **Commit::shortHash(7)** — 截取前 7 位
- **Diff::fromUnifiedDiff(text)** — 完整的 unified diff 解析器，逐行解析 7 种标记
- **DiffHunk::parseHeader** — 正则解析 `@@ -oldStart,oldCount +newStart,newCount @@`
- **FileChange::setPath/setOldPath** — 自动调用 `GitTypes::normalizePath`
- **FileChange::porcelainCode** — 拼接 indexStatus + worktreeStatus
- **Repository::setLocalPath** — 自动生成 id 和 name
- **Repository::setRemoteUrl** — 自动推断 provider
