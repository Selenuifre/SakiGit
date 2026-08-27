#ifndef THREEWAYMERGEVIEWER_H
#define THREEWAYMERGEVIEWER_H

#include "domain/MergeFileContent.h"

#include <QWidget>

class QPlainTextEdit;
class QLabel;
class QSplitter;

// 三方合并查看器——并排显示 base / ours / theirs 三个版本
class ThreeWayMergeViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ThreeWayMergeViewer(QWidget* parent = nullptr);

    // 加载三方内容
    void loadContent(const MergeFileContent& content);

    // 清空
    void clear();

private:
    void setupUi();

    QPlainTextEdit* m_baseView;
    QPlainTextEdit* m_oursView;
    QPlainTextEdit* m_theirsView;
    QLabel* m_baseLabel;
    QLabel* m_oursLabel;
    QLabel* m_theirsLabel;
};

#endif // THREEWAYMERGEVIEWER_H
