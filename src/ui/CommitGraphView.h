#ifndef COMMITGRAPHVIEW_H
#define COMMITGRAPHVIEW_H
#include <QWidget>
class CommitGraphModel;
class CommitHistoryModel;

class CommitGraphView : public QWidget {
    Q_OBJECT
public:
    explicit CommitGraphView(QWidget* parent=nullptr);
    QSize sizeHint() const override;
    void setModel(CommitGraphModel* m);
    void setCommitHistoryModel(CommitHistoryModel* m);
    CommitGraphModel* model() const { return m_model; }
    QPointF nodePt(int row, int col) const;
    void setSelectedHash(const QString& hash);
    void clearSelection();
signals:
    void commitNodeClicked(const QString& hash);
    void commitNodeRightClicked(const QString& hash);
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void leaveEvent(QEvent*) override;
private:
    int     rowAt(QPointF pt) const;
    void paintTracks(QPainter& p);
    void paintLines(QPainter& p);
    void paintNodes(QPainter& p);
    void paintLabels(QPainter& p);
    void paintText(QPainter& p);

    static constexpr double kRowH  = 60;
    static constexpr double kColW  = 55;
    static constexpr double kNodeR = 12;
    static constexpr double kLeft  = 400;
    static constexpr double kTop   = 36;

    CommitGraphModel* m_model = nullptr;
    CommitHistoryModel* m_historyModel = nullptr;
    QString m_selectedHash;
    int m_hoveredRow = -1;
    double m_scale = 1.0;
    QPointF m_offset;
    QPointF m_lastMouse;
    bool m_panning = false;
};
#endif
