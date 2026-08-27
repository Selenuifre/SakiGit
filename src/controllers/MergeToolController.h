#ifndef MERGETOOLCONTROLLER_H
#define MERGETOOLCONTROLLER_H

#include "domain/MergeFileContent.h"
#include "infrastructure/result.h"

#include "BaseController.h"
#include <QString>

class MergeLineModel;
class MergeToolService;

class MergeToolController : public BaseController
{
    Q_OBJECT

public:
    explicit MergeToolController(MergeToolService* mergeToolService,
                                  MergeLineModel* mergeLineModel,
                                  QObject* parent = nullptr);

    // 加载三方内容
    void loadThreeWayContent(const QString& repoPath, const QString& filePath);

    // 保存用户解决的内容
    Result<void> saveResolvedContent(const QString& repoPath,
                                      const QString& filePath,
                                      const QString& content);

    MergeLineModel* mergeLineModel() const;
    void clear();

signals:
    void contentLoaded(const MergeFileContent& content);
    void contentLoadFailed(const QString& errorMessage);
    void contentSaved(const QString& filePath);

private:
    MergeToolService* m_mergeToolService;
    MergeLineModel* m_mergeLineModel;
};

#endif // MERGETOOLCONTROLLER_H
