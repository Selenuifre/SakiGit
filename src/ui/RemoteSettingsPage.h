#ifndef REMOTESETTINGSPAGE_H
#define REMOTESETTINGSPAGE_H

#include <QWidget>

class QAbstractItemModel;
class QLineEdit;
class QListView;
class QPushButton;

class RemoteSettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit RemoteSettingsPage(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* model);
    QAbstractItemModel* model() const;
    QListView* listView() const;

signals:
    void addRequested(const QString& name, const QString& url);
    void removeRequested(const QString& name);
    void renameRequested(const QString& oldName, const QString& newName);
    void setUrlRequested(const QString& name, const QString& url);

private slots:
    void handleAddClicked();
    void handleContextMenu(const QPoint& pos);

private:
    void setupUi();

    QListView* m_remoteListView;
    QLineEdit* m_nameEdit;
    QLineEdit* m_urlEdit;
    QPushButton* m_addButton;
};

#endif // REMOTESETTINGSPAGE_H
