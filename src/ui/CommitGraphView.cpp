#include "CommitGraphView.h"
#include "models/CommitGraphModel.h"
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>
#include <cmath>

CommitGraphView::CommitGraphView(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true); setMinimumHeight(120);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}
QSize CommitGraphView::sizeHint() const {
    if (!m_model||m_model->totalRows()==0) return QSize(600,120);
    int h=int(m_model->totalRows()*kRowH+kTop+40);
    return QSize(qMax(1200,int(kLeft+m_model->totalColumns()*kColW+600)), qMax(120,h));
}
void CommitGraphView::setModel(CommitGraphModel* m) {
    if(m_model) disconnect(m_model,nullptr,this,nullptr);
    m_model=m;
    if(m_model) {
        connect(m_model,&CommitGraphModel::layoutChanged,this,[this](){
            resize(sizeHint()); updateGeometry(); update();
        });
        resize(sizeHint());
    }
    updateGeometry(); update();
}
void CommitGraphView::setCommitHistoryModel(CommitHistoryModel* m){m_historyModel=m;}

QPointF CommitGraphView::nodePt(int row,int col) const {
    return QPointF((kLeft+col*kColW)*m_scale+m_offset.x(), (kTop+row*kRowH)*m_scale+m_offset.y());
}
int CommitGraphView::rowAt(QPointF pt) const {
    if(!m_model) return -1;
    int best=-1; double bestDist=99999;
    for(int r=0;r<m_model->totalRows();++r){
        auto* nd=m_model->nodeAt(r); if(!nd) continue;
        QPointF np=nodePt(r,nd->column);
        double d=std::sqrt((pt.x()-np.x())*(pt.x()-np.x())+(pt.y()-np.y())*(pt.y()-np.y()));
        if(d<bestDist){bestDist=d; best=r;}
    }
    return (bestDist<kRowH*m_scale/2)?best:-1;
}

void CommitGraphView::paintEvent(QPaintEvent*) {
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing,true); p.fillRect(rect(),QColor(0x1E,0x1E,0x1E));
    if(!m_model||m_model->totalRows()==0) {
        p.setPen(QColor(0xAD,0xBA,0xC7));
        p.drawText(rect(),Qt::AlignCenter,QStringLiteral("No commit graph data"));
        return;
    }
    paintTracks(p);
    paintLines(p);
    paintNodes(p);
    paintLabels(p);
    paintText(p);
}

void CommitGraphView::paintTracks(QPainter& p) {
    QSet<int> done;
    for(int r=0;r<m_model->totalRows();++r) {
        auto* nd=m_model->nodeAt(r); if(!nd||done.contains(nd->column)) continue;
        done.insert(nd->column);
        QColor c=m_model->branchColor(nd->branchLabels.isEmpty()?QString():nd->branchLabels.first());
        c.setAlpha(10);
        double x=(kLeft+nd->column*kColW-kColW/2+2)*m_scale+m_offset.x();
        p.fillRect(QRectF(x,kTop*m_scale+m_offset.y(),(kColW-4)*m_scale,
                           m_model->totalRows()*kRowH*m_scale),c);
    }
}

void CommitGraphView::paintLines(QPainter& p) {
    QPen pen; pen.setWidthF(2.0*m_scale); pen.setCapStyle(Qt::RoundCap);
    for(const auto& L:m_model->lines()) {
        QPointF c=nodePt(L.fromRow,L.fromColumn), pa=nodePt(L.toRow,L.toColumn);
        QColor col=L.color; col.setAlphaF(0.50); pen.setColor(col); p.setPen(pen);

        if(L.fromColumn==L.toColumn) {
            p.drawLine(QPointF(c.x(),c.y()+kNodeR*m_scale),
                       QPointF(c.x(),pa.y()-kNodeR*m_scale));
        } else {
            p.drawLine(QPointF(c.x(),c.y()+kNodeR*m_scale),
                       QPointF(pa.x(),pa.y()-kNodeR*m_scale));
        }
        // 箭头沿斜线方向
        double dx=pa.x()-c.x(), dy=(pa.y()-kNodeR*m_scale)-(c.y()+kNodeR*m_scale);
        double len=std::sqrt(dx*dx+dy*dy); if(len<1) len=1;
        dx/=len; dy/=len;
        QPointF perp(-dy,dx);
        QColor ac=L.color; ac.setAlpha(0xFF);
        p.setBrush(ac); p.setPen(Qt::NoPen); double s=7*m_scale;
        QPointF tip(pa.x(),pa.y()-kNodeR*m_scale);
        QPolygonF tri;
        tri<<tip<<(tip-QPointF(dx*s*2,dy*s*2)+perp*s)
           <<(tip-QPointF(dx*s*2,dy*s*2)-perp*s);
        p.drawPolygon(tri);
    }
}

void CommitGraphView::paintNodes(QPainter& p) {
    for(int r=0;r<m_model->totalRows();++r) {
        auto* nd=m_model->nodeAt(r); if(!nd) continue;
        QPointF c=nodePt(r,nd->column); double rad=kNodeR*m_scale;
        QColor col(0x88,0x88,0x88);
        if(!nd->branchLabels.isEmpty()) col=m_model->branchColor(nd->branchLabels.first());
        if(nd->isHEAD){
            p.setBrush(Qt::NoBrush); p.setPen(QPen(QColor(0xCF,0x22,0x2E),3.0*m_scale));
            p.drawEllipse(c,rad+3*m_scale,rad+3*m_scale);
        }
        QRadialGradient g(c,rad);
        g.setColorAt(0,col.lighter(130)); g.setColorAt(0.7,col); g.setColorAt(1,col.darker(120));
        p.setBrush(g); p.setPen(QPen(Qt::white,2.0*m_scale));
        p.drawEllipse(c,rad,rad);
        if(r==m_hoveredRow){
            p.setBrush(Qt::NoBrush); p.setPen(QPen(col.lighter(150),3.0*m_scale));
            p.drawEllipse(c,rad+2*m_scale,rad+2*m_scale);
        }
        // Selection highlight
        if(!m_selectedHash.isEmpty() && nd->hash == m_selectedHash) {
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(0x53,0x9B,0xF5), 3.5*m_scale));
            p.drawEllipse(c, rad + 5*m_scale, rad + 5*m_scale);
        }
    }
}

void CommitGraphView::setSelectedHash(const QString& hash) {
    if (m_selectedHash == hash) return;
    m_selectedHash = hash;
    update();
}

void CommitGraphView::clearSelection() {
    if (m_selectedHash.isEmpty()) return;
    m_selectedHash.clear();
    update();
}

void CommitGraphView::paintLabels(QPainter& p) {
    QFont f(QStringLiteral("Cascadia Code,Consolas,SF Mono,Menlo,DejaVu Sans Mono,monospace"), int(10*m_scale)); f.setBold(true); p.setFont(f);
    QHash<QString,int> latestRow;
    for(int r=0;r<m_model->totalRows();++r){
        auto* nd=m_model->nodeAt(r); if(!nd) continue;
        for(auto& br:nd->branchLabels)
            if(!latestRow.contains(br)||r<latestRow[br]) latestRow[br]=r;
    }
    QHash<int,int> off;
    for(int r=0;r<m_model->totalRows();++r){
        auto* nd=m_model->nodeAt(r); if(!nd) continue;
        for(auto& br:nd->branchLabels){
            if(latestRow.value(br,-1)!=r) continue;
            QPointF c=nodePt(r,nd->column);
            QColor bg=m_model->branchColor(br); QColor solid=bg; solid.setAlpha(0xFF); bg.setAlphaF(0.30);
            bool isH=nd->isHEAD && nd->branchLabels.first()==br;
            QString t=isH?QStringLiteral("HEAD  ")+br:br;
            int& o=off[r]; o++;
            QFontMetrics fm(f); double tw=fm.horizontalAdvance(t)+12*m_scale, th=22*m_scale;
            bool leftSide=(nd->column<0);
            double lx=leftSide?(c.x()-kNodeR*m_scale-tw-4*m_scale):(c.x()+kNodeR*m_scale+4*m_scale);
            double ly=c.y()-th/2+(o-1)*(th+4*m_scale);
            p.setPen(Qt::NoPen); p.setBrush(bg);
            p.drawRoundedRect(QRectF(lx,ly,tw,th),6*m_scale,6*m_scale);
            QPolygonF tri;
            if(leftSide) tri<<QPointF(c.x()-kNodeR*m_scale-1*m_scale,c.y())
                <<QPointF(lx+tw,c.y()-4*m_scale)<<QPointF(lx+tw,c.y()+4*m_scale);
            else tri<<QPointF(c.x()+kNodeR*m_scale+1*m_scale,c.y())
                <<QPointF(lx,c.y()-4*m_scale)<<QPointF(lx,c.y()+4*m_scale);
            p.drawPolygon(tri);
            p.setPen(solid);
            p.drawText(QRectF(lx,ly,tw,th),Qt::AlignCenter,t);
            if(isH){
                double s=7*m_scale; QPointF tip(c.x(),c.y()-kNodeR*m_scale-s*0.3);
                p.setPen(Qt::NoPen); p.setBrush(QColor(0xCF,0x22,0x2E));
                QPolygonF htri; htri<<tip<<QPointF(tip.x()-s,tip.y()-s*1.5)<<QPointF(tip.x()+s,tip.y()-s*1.5);
                p.drawPolygon(htri);
                QFont f2(QStringLiteral("Cascadia Code,Consolas,SF Mono,Menlo,DejaVu Sans Mono,monospace"), int(8*m_scale)); p.setFont(f2);
                p.setPen(QColor(0xCF,0x22,0x2E));
                p.drawText(QRectF(tip.x()-28*m_scale,tip.y()-36*m_scale,56*m_scale,14*m_scale),
                            Qt::AlignCenter,QStringLiteral("HEAD"));
            }
        }
    }
    for(int r=0;r<m_model->totalRows();++r){
        auto* nd=m_model->nodeAt(r); if(!nd||nd->tagLabels.isEmpty()) continue;
        QPointF c=nodePt(r,nd->column);
        double ox=c.x()+kNodeR*m_scale+4*m_scale, oy=c.y()+kNodeR*m_scale+6*m_scale;
        for(auto& tg:nd->tagLabels){
            QFontMetrics fm(f); double tw=fm.horizontalAdvance(tg)+10*m_scale;
            p.setPen(Qt::NoPen); p.setBrush(QColor(0xFF,0xE0,0x82));
            p.drawRoundedRect(QRectF(ox,oy,tw,18*m_scale),4*m_scale,4*m_scale);
            p.setPen(QColor(0x79,0x55,0x48));
            p.drawText(QRectF(ox,oy,tw,18*m_scale),Qt::AlignCenter,tg);
            ox+=tw+4*m_scale;
        }
    }
}

void CommitGraphView::paintText(QPainter& p) {
    double cx=width()*0.45;
    double tx=(kLeft+qMax(m_model->totalColumns(),3)*kColW+kColW*2)*m_scale+m_offset.x();
    QFont f(QStringLiteral("Cascadia Code,Consolas,SF Mono,Menlo,DejaVu Sans Mono,monospace"), int(11*m_scale)); p.setFont(f);
    for(int r=0;r<m_model->totalRows();++r){
        auto* nd=m_model->nodeAt(r); if(!nd) continue;
        QPointF c=nodePt(r,nd->column);
        p.setPen(QColor(0x76,0x83,0x90));
        p.drawText(QRectF(tx,c.y()-12*m_scale,200*m_scale,16*m_scale),
                    Qt::AlignLeft|Qt::AlignVCenter, nd->shortHash);
        p.setPen(QColor(0xAD,0xBA,0xC7));
        QString msg=nd->summary.left(42); if(nd->summary.length()>42) msg+=QStringLiteral("...");
        p.drawText(QRectF(tx+80*m_scale,c.y()-12*m_scale,350*m_scale,16*m_scale),
                    Qt::AlignLeft|Qt::AlignVCenter, msg);
    }
}

void CommitGraphView::mousePressEvent(QMouseEvent* e) {
    if(e->button()==Qt::MiddleButton||(e->button()==Qt::LeftButton&&e->modifiers()&Qt::ControlModifier)){
        m_panning=true; m_lastMouse=e->pos(); setCursor(Qt::ClosedHandCursor); return;
    }
    if(!m_model) return QWidget::mousePressEvent(e);
    if(e->button()==Qt::LeftButton){
        int r=rowAt(e->pos()); auto* nd=m_model->nodeAt(r);
        if(nd) emit commitNodeClicked(nd->hash);
        else clearSelection();
    }else if(e->button()==Qt::RightButton){
        int r=rowAt(e->pos()); auto* nd=m_model->nodeAt(r);
        if(nd) emit commitNodeRightClicked(nd->hash);
    }
}
void CommitGraphView::mouseMoveEvent(QMouseEvent* e) {
    if(m_panning){QPointF d=e->pos()-m_lastMouse; m_offset+=d; m_lastMouse=e->pos(); update(); return;}
    int r=rowAt(e->pos()); if(r!=m_hoveredRow){m_hoveredRow=r; update();}
    auto* nd=m_model->nodeAt(r);
    if(nd) setToolTip(nd->hash+QStringLiteral("\n")+nd->summary); else setToolTip(QString());
}
void CommitGraphView::mouseReleaseEvent(QMouseEvent*){m_panning=false; setCursor(Qt::ArrowCursor);}
void CommitGraphView::wheelEvent(QWheelEvent* e) {
    if(e->modifiers()&Qt::ControlModifier) {
        double oldS=m_scale;
        m_scale=qBound(0.5,m_scale*(e->angleDelta().y()>0?1.1:0.9),2.0);
        // 缩放中心 = 鼠标位置
        QPointF mp=e->position();
        m_offset = mp - (mp - m_offset) * (m_scale / oldS);
        update();
    } else { QWidget::wheelEvent(e); }
}
void CommitGraphView::leaveEvent(QEvent*){m_hoveredRow=-1; update();}
