#ifndef TAG_H
#define TAG_H

#include <QMetaType>
#include <QString>

// Tag 数据类——轻量级标签（lightweight tag）或附注标签（annotated tag）的引用信息。
// 用于 CommitGraphView 中在对应提交节点旁显示 tag 标注。
class Tag
{
public:
    Tag() = default;

    // 标签名称（如 "v1.0"、"release-2025"）
    QString name() const { return m_name; }
    void setName(const QString& name) { m_name = name.trimmed(); }

    // 标签指向的提交哈希
    QString targetHash() const { return m_targetHash; }
    void setTargetHash(const QString& hash) { m_targetHash = hash.trimmed(); }

    // 是否为有效标签
    bool isValid() const { return !m_name.isEmpty() && !m_targetHash.isEmpty(); }

private:
    QString m_name;
    QString m_targetHash;
};

Q_DECLARE_METATYPE(Tag)

#endif // TAG_H
