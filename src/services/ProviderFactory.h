#ifndef PROVIDERFACTORY_H
#define PROVIDERFACTORY_H

#include "domain/CodeHostingPlatform.h"

#include <memory>
#include <vector>

class HttpClient;
class ICodeHostingProvider;

// 代码托管平台 Provider 工厂。
// 负责创建和管理所有平台的 Provider 实例。
// 核心方法：根据 git remote URL 自动识别平台并返回对应 Provider。
class ProviderFactory
{
public:
    explicit ProviderFactory(HttpClient* httpClient);

    // 根据平台枚举获取 Provider（若不存在则创建）
    ICodeHostingProvider* provider(CodeHostingPlatform platform);

    // 根据 git remote URL 自动识别并返回匹配的 Provider
    // 依次尝试所有已注册的 Provider 的 canHandleRemote()
    ICodeHostingProvider* providerForRemote(const QString& remoteUrl);

    // 获取默认 Provider（GitHub）
    ICodeHostingProvider* defaultProvider();

    // 返回所有已创建的 Provider（首次调用时自动创建内置 Provider）
    std::vector<ICodeHostingProvider*> allProviders();

    // 注册自定义 Provider（例如自托管 GitLab）
    void registerProvider(std::unique_ptr<ICodeHostingProvider> provider);

private:
    void ensureBuiltinProviders();

    HttpClient* m_httpClient;
    std::vector<std::unique_ptr<ICodeHostingProvider>> m_providers;
};

#endif // PROVIDERFACTORY_H
