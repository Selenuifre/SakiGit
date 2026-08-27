#ifndef REMOTECONTROLLER_H
#define REMOTECONTROLLER_H

#include "infrastructure/result.h"

#include <QObject>
#include <QString>

class RemoteListModel;
class RemoteService;

class RemoteController : public QObject
{
    Q_OBJECT

public:
    explicit RemoteController(RemoteService* remoteService,
                              RemoteListModel* remoteListModel,
                              QObject* parent = nullptr);

    void loadRemotes(const QString& repoPath);
    Result<void> addRemote(const QString& repoPath,
                           const QString& name,
                           const QString& url);
    Result<void> removeRemote(const QString& repoPath,
                              const QString& name);
    Result<void> renameRemote(const QString& repoPath,
                              const QString& oldName,
                              const QString& newName);
    Result<void> setRemoteUrl(const QString& repoPath,
                              const QString& name,
                              const QString& url);

    RemoteListModel* remoteListModel() const;
    void clear();

signals:
    void remotesLoaded(bool success, const QString& errorMessage);
    void operationFinished(const QString& operation, bool success, const QString& errorMessage);
    void errorOccurred(const QString& operation, const QString& errorMessage);

private:
    RemoteService* m_remoteService;
    RemoteListModel* m_remoteListModel;
};

#endif // REMOTECONTROLLER_H
