#ifndef STASHPAGE_H
#define STASHPAGE_H

#include <QWidget>

class QAbstractItemModel;
class QLineEdit;
class QListView;
class QPushButton;
class PageHeaderWidget;

class StashPage : public QWidget
{
    Q_OBJECT

public:
    explicit StashPage(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* model);
    QAbstractItemModel* model() const;
    QListView* listView() const;

signals:
    void stashSelected(const QModelIndex& index);
    void saveStashRequested(const QString& message);
    void applyStashRequested(int index);
    void dropStashRequested(int index);
    void showDiffRequested(int index);
    void refreshRequested();

private slots:
    void handleSaveClicked();
    void handleContextMenu(const QPoint& pos);

private:
    void setupUi();

    PageHeaderWidget* m_headerWidget;
    QListView* m_stashListView;
    QLineEdit* m_messageEdit;
    QPushButton* m_saveButton;
};

#endif // STASHPAGE_H
