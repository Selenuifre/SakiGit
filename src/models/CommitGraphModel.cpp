#include "CommitGraphModel.h"
#include <climits>

static const QColor kPalette[] = {
    QColor("#ff6b6b"),QColor("#4ecdc4"),QColor("#45b7d1"),
    QColor("#f9ca24"),QColor("#ff9ff3"),QColor("#54a0ff"),
    QColor("#5f27cd"),QColor("#ff8a65"),QColor("#81c784"),
    QColor("#64b5f6"),QColor("#ba68c8"),QColor("#4db6ac"),
};
static constexpr int kPalSize = 12;

CommitGraphModel::CommitGraphModel(QObject* parent)
    : QObject(parent), m_totalColumns(0) {}

void CommitGraphModel::buildLayout(const std::vector<Commit>& commits,
                                    const std::vector<Branch>& branches,
                                    const std::vector<Tag>& tags,
                                    const QString& headHash)
{
    m_nodes.clear(); m_lines.clear(); m_branchColorCache.clear();
    const int N = static_cast<int>(commits.size());
    if (N == 0) { emit layoutChanged(); return; }

    // 1. hash → index
    QHash<QString,int> idx;
    for (int i=0;i<N;++i) idx.insert(commits[i].hash(),i);

    // 2. per-commit annotations
    QHash<QString,QStringList> brMap, tgMap;
    QHash<QString,bool> headMap;
    for (const auto& b:branches) if(!b.headCommitHash().isEmpty()) brMap[b.headCommitHash()].append(b.name());
    for (const auto& t:tags) if(t.isValid()) tgMap[t.targetHash()].append(t.name());
    if (!headHash.isEmpty()) headMap[headHash]=true;

    // 3. 为每个分支分配列：main=0，旁支左右交替
    struct BranchInfo { QString name; int col; };
    QList<BranchInfo> binfo;
    binfo.append({QString(), 0}); // placeholder for main
    int side=0;
    for (const auto& b:branches) {
        QString lo=b.name().toLower();
        if (lo==QStringLiteral("main")||lo==QStringLiteral("master")) {
            binfo[0]={b.name(), 0}; continue;
        }
        ++side;
        binfo.append({b.name(), (side%2==1)? -((side+1)/2) : (side/2)});
    }

    // 4. 追踪每个分支链：从 head 沿 first-parent 向下标记所有commit的primary分支
    // primary: commit → 它属于哪个分支（用于列分配）
    QHash<QString,int> primaryCol; // commit hash → column
    for (const auto& bi:binfo) {
        if (bi.name.isEmpty()) continue;
        QString cur; // 找该分支的 head commit hash
        for (const auto& b:branches) if(b.name()==bi.name){cur=b.headCommitHash();break;}
        int limit=0;
        while(!cur.isEmpty()&&idx.contains(cur)&&limit++<N) {
            int ci=idx[cur];
            if (!primaryCol.contains(cur)) primaryCol[cur]=bi.col; // 首次标记优先级最高（main先遍历）
            const auto& ps=commits[ci].parentHashes();
            cur=ps.isEmpty()?QString():ps.first(); // 只跟 first parent
        }
    }
    // 未标记的提交（无分支归属）默认归 main (col 0)
    for (int i=0;i<N;++i) if(!primaryCol.contains(commits[i].hash())) primaryCol[commits[i].hash()]=0;

    // 5. 分配行号（已在 log 的拓扑顺序中，index 即 row）
    std::vector<int> col(N,0);
    for (int i=0;i<N;++i) {
        int c = primaryCol.value(commits[i].hash(), 0);
        // merge 节点如果在 main 链上但有两个 parent 来自不同列，保留 main col
        col[i]=c;
    }
    // 统计总列数
    int minC=0,maxC=0;
    for (int c:col){minC=qMin(minC,c);maxC=qMax(maxC,c);}
    m_totalColumns=maxC-minC+1;

    // 6. 生成连线：每个commit→每个parent
    for (int i=0;i<N;++i) {
        for (const auto& ph:commits[i].parentHashes()) {
            auto it=idx.find(ph);
            if (it==idx.end()) continue;
            CommitGraphLine L;
            L.fromRow=i; L.fromColumn=col[i]; L.toRow=*it; L.toColumn=col[*it];
            // 颜色用子节点所在分支的颜色
            QString childBr=brMap.contains(commits[i].hash())?brMap[commits[i].hash()].first():QString();
            L.color=branchColor(childBr);
            m_lines.push_back(L);
        }
    }

    // 7. 构建节点
    m_nodes.reserve(N);
    for (int i=0;i<N;++i) {
        CommitGraphNode nd;
        nd.row=i; nd.column=col[i];
        nd.hash=commits[i].hash(); nd.shortHash=commits[i].shortHash(7);
        nd.summary=commits[i].summary(); nd.authorDate=commits[i].authorDate();
        nd.parentHashes=commits[i].parentHashes();
        if (brMap.contains(nd.hash)) nd.branchLabels=brMap[nd.hash];
        if (tgMap.contains(nd.hash)) nd.tagLabels=tgMap[nd.hash];
        if (headMap.contains(nd.hash)) nd.isHEAD=true;
        m_nodes.push_back(nd);
    }

    emit layoutChanged();
}

const CommitGraphNode* CommitGraphModel::nodeAt(int row) const {
    if (row<0||row>=static_cast<int>(m_nodes.size())) return nullptr;
    return &m_nodes[row];
}
QColor CommitGraphModel::branchColor(const QString& name) const {
    if (m_branchColorCache.contains(name)) return m_branchColorCache[name];
    QColor c=colorForIndex(m_branchColorCache.size());
    m_branchColorCache.insert(name,c); return c;
}
void CommitGraphModel::clear() {
    m_nodes.clear(); m_lines.clear(); m_branchColorCache.clear();
    m_totalColumns=0; emit layoutChanged();
}
QColor CommitGraphModel::colorForIndex(int i) const { return kPalette[i%kPalSize]; }
