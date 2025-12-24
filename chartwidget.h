#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QtCharts>
#include <Eigen/Dense>
#include <vector>
#include <algorithm>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>
#include <QFont>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QMouseEvent>
#include <QHash>
#include "population_and_order.h"

using namespace QtCharts;
using namespace Eigen;

// 全局变量声明
extern double choose;
extern population* survive;

// 自定义动态标签类
class DynamicLabel : public QGraphicsTextItem {
public:
    DynamicLabel(const QString& text, QPointF logicalPos, QChart* chart, QPointF offset, QGraphicsItem* parent = nullptr);

    void updatePosition();

private:
    QPointF m_logicalPos;
    QChart* m_chart;
    QPointF m_offset;
};

// 自定义 QChartView 类
class CustomChartView : public QChartView {
    Q_OBJECT
public:
    CustomChartView(QChart* chart, QWidget* parent = nullptr);

    void addDynamicLabel(DynamicLabel* label, const QPointF& point);
    DynamicLabel* getLabelAt(const QPointF& point);
    void resetChartView();

signals:
    void chartUpdated();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void updateLabels();

private:
    QHash<QPointF, DynamicLabel*> m_labels;
    bool m_isDragging;
    QPoint m_lastMousePos;
    QRectF m_initialSceneRect;
    QPointF m_initialCenter;
    qreal m_initialZoom;
};

// 全局函数声明
double complexModel(double x, const VectorXd& params);

// 复杂模型残差函数
struct ComplexResidual {
    const QVector<population>& data;
    ComplexResidual(const QVector<population>& d) : data(d) {}
    bool operator()(const VectorXd& params, VectorXd& residuals) const {
        for (int i = 0; i < data.size(); ++i) {
            double predicted = complexModel(data[i].pricefit, params);
            residuals(i) = data[i].timefit - predicted;
        }
        return true;
    }
};

// 柱状图窗口类
class HistogramWidget : public QWidget {
    Q_OBJECT
public:
    HistogramWidget(const QList<QPair<int, int>>& orderedCounts, QWidget* parent = nullptr);
    void setupHistogram(const QList<QPair<int, int>>& orderedCounts);
    void updateLabels(); // 新增：更新标签位置
protected:

    void resizeEvent(QResizeEvent* event) override; // 新增：响应窗口大小变化

private:
    QChart* m_chart;
    QMap<int, double> m_sqrtCounts; // 存储开方后的值
    QMap<int, DynamicLabel*> m_labels; // 存储动态标签
};

// ChartWidget 类声明
class ChartWidget : public QWidget {
    Q_OBJECT
public:
    ChartWidget(QVector<population> elite, QWidget* parent = nullptr);

private slots:
    void togglePointLabel(const QPointF& point);
    void showChooseDialog();
    void resetChart();
    void onChartUpdated();
    void computeSelectionHistogram();

private:
    void setupChart();
    void updateReferenceLine();
    void updateAuxiliaryLines();

    double simpleModel(double x, double a);
    double simpleError(double a, const QVector<population>& data);
    double fitSimpleModel(const QVector<population>& data);
    VectorXd initializeComplexParams(const QVector<population>& data);
    VectorXd fitComplexModel(const QVector<population>& data, double maxX);
    QLineSeries* fitInverseProportional(const QVector<population>& data, double x_start, double x_end, double maxX, double maxY);
    QLineSeries* drawReferenceLine(QVector<population>& data, double maxX, double maxY);

    QVector<population> m_elite;
    QVector<population> m_sortedElite;
    QMap<int, population> m_eliteMap;
    double m_xMin, m_xMax, m_yMin, m_yMax;
    QChart* m_chart;
    CustomChartView* m_view;
    QScatterSeries* m_scatter;
    QScatterSeries* m_selectedPoint;
    QLineSeries* m_refLine;
    QVector<QLineSeries*> m_auxiliaryLines;
    bool m_isChooseSet; // 新增：标志是否已设置 choose 值
};

// 声明 showEliteChartWithFit 函数
ChartWidget* showEliteChartWithFit(QVector<population>& elite);

// 为 QPointF 实现 qHash 函数
inline uint qHash(const QPointF& key, uint seed = 0) {
    return qHash(key.x(), seed) ^ qHash(key.y(), seed + 1);
}

#endif // CHARTWIDGET_H




