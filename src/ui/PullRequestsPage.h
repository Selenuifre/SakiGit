#ifndef PULLREQUESTSPAGE_H
#define PULLREQUESTSPAGE_H

#include <QWidget>

class QAbstractItemModel;
class QLineEdit;
class QListView;
class QPushButton;
class QTextEdit;

class PullRequestsPage : public QWidget
{
    Q_OBJECT

public:
    explicit PullRequestsPage(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* model);
    QAbstractItemModel* model() const;
    QListView* listView() const;

signals:
    void createPullRequestRequested(const QString& title, const QString& body,
                                    const QString& head, const QString& base);
    void mergeRequested(int prNumber);
    void refreshRequested();

private slots:
    void handleCreateClicked();
    void handleContextMenu(const QPoint& pos);

private:
    void setupUi();

    QListView* m_prListView;
    QLineEdit* m_titleEdit;
    QLineEdit* m_headEdit;
    QLineEdit* m_baseEdit;
    QTextEdit* m_bodyEdit;
    QPushButton* m_createButton;
};

#endif // PULLREQUESTSPAGE_H
