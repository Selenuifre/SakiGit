#ifndef BASESERVICE_H
#define BASESERVICE_H

#include "infrastructure/ResultUtils.h"
#include "infrastructure/result.h"

class GitService;

// 所有依赖 GitService 的服务类的公共基类。
// 提供标准化的空指针守卫和错误消息常量。
class BaseService
{
public:
    explicit BaseService(GitService* gitService)
        : m_gitService(gitService)
    {
    }

    virtual ~BaseService() = default;

    // 检查 GitService 依赖是否已注入
    bool isInitialized() const { return m_gitService != nullptr; }

    // 返回底层 GitService 指针（供子类或外部调用者访问）
    GitService* gitService() const { return m_gitService; }

protected:
    GitService* m_gitService = nullptr;

    // 标准化的"服务未初始化"错误
    template <typename T>
    Result<T> serviceNotInitialized() const
    {
        return Result<T>::failure(QString::fromLatin1(ServiceError::NotInitialized));
    }
};

#endif // BASESERVICE_H
