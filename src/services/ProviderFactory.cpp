#include "ProviderFactory.h"

#include "infrastructure/HttpClient.h"
#include "infrastructure/ICodeHostingProvider.h"
#include "services/providers/GitHubProvider.h"
#include "services/providers/GiteeProvider.h"
#include "services/providers/GitLabProvider.h"

#include <algorithm>

ProviderFactory::ProviderFactory(HttpClient* httpClient)
    : m_httpClient(httpClient)
{
}

void ProviderFactory::ensureBuiltinProviders()
{
    if (m_providers.empty()) {
        m_providers.push_back(std::make_unique<GitHubProvider>(m_httpClient));
        m_providers.push_back(std::make_unique<GiteeProvider>(m_httpClient));
        m_providers.push_back(std::make_unique<GitLabProvider>(m_httpClient));
    }
}

ICodeHostingProvider* ProviderFactory::provider(CodeHostingPlatform platform)
{
    ensureBuiltinProviders();

    for (auto& p : m_providers) {
        if (p->platform() == platform) {
            return p.get();
        }
    }

    return nullptr;
}

ICodeHostingProvider* ProviderFactory::providerForRemote(const QString& remoteUrl)
{
    ensureBuiltinProviders();

    for (auto& p : m_providers) {
        if (p->canHandleRemote(remoteUrl)) {
            return p.get();
        }
    }

    // 回退到 GitHub
    return defaultProvider();
}

ICodeHostingProvider* ProviderFactory::defaultProvider()
{
    return provider(CodeHostingPlatform::GitHub);
}

std::vector<ICodeHostingProvider*> ProviderFactory::allProviders()
{
    ensureBuiltinProviders();

    std::vector<ICodeHostingProvider*> result;
    for (const auto& p : m_providers) {
        result.push_back(p.get());
    }
    return result;
}

void ProviderFactory::registerProvider(std::unique_ptr<ICodeHostingProvider> provider)
{
    if (provider) {
        m_providers.push_back(std::move(provider));
    }
}
