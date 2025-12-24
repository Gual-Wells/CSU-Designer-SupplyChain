#include "graphview.h"
#include <QDebug>
#include <QPainter>

GraphView::GraphView(QWidget *parent)
    : QGraphicsView(parent), isDragging(false), currentScale(1.0)
{
       setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

       // 设置抗锯齿和图像平滑
       setRenderHint(QPainter::Antialiasing);
       setRenderHint(QPainter::SmoothPixmapTransform);

       // 禁用滚动条
       setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
       setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

       // 禁用默认的拖动模式
       setDragMode(QGraphicsView::NoDrag);
}

void GraphView::mousePressEvent(QMouseEvent *event) {

    if (event->button() == Qt::RightButton) {  // **改为按住右键拖动画布**
        isDragging = true;
        lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);  // **更改鼠标样式**
        event->accept();

    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void GraphView::mouseMoveEvent(QMouseEvent *event) {
    if (isDragging) {
        QPoint delta = event->pos() - lastMousePos;
        lastMousePos = event->pos();
        // **平移画布**
        setTransformationAnchor(QGraphicsView::NoAnchor);
        double speedFactor = 1.0 / currentScale;  // **缩小时加快，放大时减慢**
        translate(delta.x() * speedFactor, delta.y() * speedFactor);
        event->accept();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void GraphView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {  // **右键释放**
        isDragging = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

// **滚轮缩放（带最小/最大缩放限制）**
void GraphView::wheelEvent(QWheelEvent *event) {
//    if (!isDragging) {  // **拖动时忽略缩放**
        double scaleFactor = 1.2;
        if (event->angleDelta().y() > 0) {
            // **放大**
            if (currentScale * scaleFactor <= maxScale) {
                scale(scaleFactor, scaleFactor);
                currentScale *= scaleFactor;
            }
        } else {
            // **缩小**
            if (currentScale / scaleFactor >= minScale) {
                scale(1.0 / scaleFactor, 1.0 / scaleFactor);
                currentScale /= scaleFactor;
            }
        }
        event->accept();
//    }
}
