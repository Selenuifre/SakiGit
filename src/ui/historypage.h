#ifndef HISTORYPAGE_H
#define HISTORYPAGE_H

#include <QWidget>

class QAbstractItemModel;
class QListView;
class QPlainTextEdit;
class QPushButton;
class QScrollArea;
class QSplitter;
class QStackedWidget;
class CommitGraphModel;
class CommitGraphView;
class DiffViewer;

class HistoryPage : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryPage(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* model);
    QAbstractItemModel* model() const;

    void setDiffModel(QAbstractItemModel* model);
    void setGraphModel(CommitGraphModel* model);

    void setCommitDetail(const QString& detail);
    void setChangedFiles(const QStringList& files);

    DiffViewer* diffViewer() const;
    CommitGraphView* graphView() const;
    QListView* listView() const;

signals:
    void commitSelected(const QModelIndex& index);
    void loadMoreRequested();

    void checkoutCommitRequested(const QString& hash);
    void createBranchAtCommitRequested(const QString& hash);
    void resetToCommitRequested(const QString& hash, const QString& mode);
    void copyHashRequested(const QString& hash);

    void fileInCommitClicked(const QString& commitHash, const QString& filePath);

private slots:
    void handleGraphContextMenu(const QString& hash);
    void handleCommitClicked(const QModelIndex& index);
    void toggleRightPanel();

private:
    void setupUi();
    void centerGraphOnCommit(const QString& hash);

    QListView* m_commitListView;
    QPlainTextEdit* m_commitDetailView;
    QSplitter* m_mainSplitter;
    QStackedWidget* m_rightStack;       // graph | diff
    QScrollArea* m_graphScrollArea;
    DiffViewer* m_diffViewer;
    CommitGraphView* m_graphView;
    QListView* m_changedFilesList;
    QMenu* m_contextMenu = nullptr;
    QString m_currentCommitHash;
    bool m_showingGraph = true;
};

#endif // HISTORYPAGE_H
