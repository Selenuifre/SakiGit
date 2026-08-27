#ifndef RESULT_H
#define RESULT_H

#include "error.h"

#include <QString>
#include <QtGlobal>

#include <optional>
#include <utility>

template <typename T>
class Result
{
public:
    // 创建一个默认失败的结果对象
    Result()
        : m_success(false),
        m_error(Error::unknown(QStringLiteral("Operation failed.")))
    {
    }

    // 创建一个成功结果，并复制保存返回值
    static Result<T> success(const T& value)
    {
        Result<T> result;
        result.m_success = true;
        result.m_value = value;
        result.m_error = Error::none();
        return result;
    }

    // 创建一个成功结果，并移动保存返回值
    static Result<T> success(T&& value)
    {
        Result<T> result;
        result.m_success = true;
        result.m_value = std::move(value);
        result.m_error = Error::none();
        return result;
    }

    // 创建一个失败结果，并保存错误信息；保留该接口以兼容旧调用点
    static Result<T> failure(const QString& errorMessage)
    {
        return failure(Error::unknown(errorMessage));
    }

    // 创建一个失败结果，并保存结构化错误信息
    static Result<T> failure(const Error& error)
    {
        Result<T> result;
        result.m_success = false;
        result.m_error = error.isValid() ? error : Error::unknown(QStringLiteral("Operation failed."));
        return result;
    }

    // 判断操作是否成功
    bool isSuccess() const
    {
        return m_success;
    }

    // 判断操作是否失败
    bool isFailure() const
    {
        return !m_success;
    }

    // 判断结果中是否包含返回值
    bool hasValue() const
    {
        return m_value.has_value();
    }

    // 以只读方式获取返回值
    const T& value() const
    {
        Q_ASSERT(m_value.has_value());
        return *m_value;
    }

    // 以可修改方式获取返回值
    T& value()
    {
        Q_ASSERT(m_value.has_value());
        return *m_value;
    }

    // 获取返回值；如果没有值，则返回默认值
    T valueOr(const T& defaultValue) const
    {
        if (m_value.has_value()) {
            return *m_value;
        }

        return defaultValue;
    }

    // 获取失败时的错误信息
    QString errorMessage() const
    {
        return m_error.message();
    }

    // 获取失败时的结构化错误信息
    Error error() const
    {
        return m_error;
    }

private:
    bool m_success;
    std::optional<T> m_value;
    Error m_error;
};

template <>
class Result<void>
{
public:
    // 创建一个默认失败的无返回值结果对象
    Result()
        : m_success(false),
        m_error(Error::unknown(QStringLiteral("Operation failed.")))
    {
    }

    // 创建一个成功的无返回值结果
    static Result<void> success()
    {
        Result<void> result;
        result.m_success = true;
        result.m_error = Error::none();
        return result;
    }

    // 创建一个失败的无返回值结果，并保存错误信息；保留该接口以兼容旧调用点
    static Result<void> failure(const QString& errorMessage)
    {
        return failure(Error::unknown(errorMessage));
    }

    // 创建一个失败的无返回值结果，并保存结构化错误信息
    static Result<void> failure(const Error& error)
    {
        Result<void> result;
        result.m_success = false;
        result.m_error = error.isValid() ? error : Error::unknown(QStringLiteral("Operation failed."));
        return result;
    }

    // 判断操作是否成功
    bool isSuccess() const
    {
        return m_success;
    }

    // 判断操作是否失败
    bool isFailure() const
    {
        return !m_success;
    }

    // 获取失败时的错误信息
    QString errorMessage() const
    {
        return m_error.message();
    }

    // 获取失败时的结构化错误信息
    Error error() const
    {
        return m_error;
    }

private:
    bool m_success;
    Error m_error;
};

#endif // RESULT_H
