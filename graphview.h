#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>

class GraphView : public QGraphicsView {
    Q_OBJECT

public:
    explicit GraphView(QWidget *parent = nullptr);
    bool isDrag(){return isDragging;};

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;  // **添加滚轮事件**

private:
    bool isDragging;     // **是否正在拖动画布**
    QPoint lastMousePos; // **鼠标上一次的位置**
    double currentScale; // **当前缩放比例**
    const double minScale = 0.1;  // **最小缩放**
    const double maxScale = 5.0;  // **最大缩放**
};

#endif // GRAPHVIEW_H
