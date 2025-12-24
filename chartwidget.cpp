#include "chartwidget.h"
#include "choosedialog.h"
#include "result.h"

DynamicLabel::DynamicLabel(const QString& text, QPointF logicalPos, QChart* chart, QPointF offset, QGraphicsItem* parent)
    : QGraphicsTextItem(text, parent), m_logicalPos(logicalPos), m_chart(chart), m_offset(offset) {
    updatePosition();
    setVisible(true);
}

void DynamicLabel::updatePosition() {
    QPointF pixelPos = m_chart->mapToPosition(m_logicalPos);
    if (!pixelPos.isNull() && std::isfinite(pixelPos.x()) && std::isfinite(pixelPos.y())) {
        QValueAxis* axisY = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Vertical).first());
        if (axisY) {
            QBarCategoryAxis* axisX = qobject_cast<QBarCategoryAxis*>(m_chart->axes(Qt::Horizontal).first());
            if (axisX) {
                int index = static_cast<int>(m_logicalPos.x());
                QStringList categories = axisX->categories();
                if (index >= 0 && index < categories.size()) {
                    qreal xMin = m_chart->mapToPosition(QPointF(index - 0.5, 0)).x();
                    qreal xMax = m_chart->mapToPosition(QPointF(index + 0.5, 0)).x();
                    qreal centerX = (xMin + xMax) / 2.0;
                    pixelPos.setX(centerX);
                }
            }
            QFontMetrics fm(font());
            qreal textWidth = fm.horizontalAdvance(toPlainText());
            setPos(pixelPos + m_offset - QPointF(textWidth / 2.0, 0));
            setVisible(true);
        } else {
            setVisible(false);
        }
    } else {
        qDebug() << "Invalid position for logicalPos:" << m_logicalPos;
        setVisible(false);
    }
}

CustomChartView::CustomChartView(QChart* chart, QWidget* parent)
    : QChartView(chart, parent), m_isDragging(false), m_initialZoom(1.0) {
    setRenderHint(QPainter::Antialiasing);
    setRubberBand(QChartView::RectangleRubberBand);
    QValueAxis* axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
    QValueAxis* axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    if (axisX && axisY) {
        m_initialSceneRect = QRectF(
            axisX->min(), axisY->min(),
            axisX->max() - axisX->min(),
            axisY->max() - axisY->min()
        );
        m_initialCenter = QPointF(
            (axisX->min() + axisX->max()) / 2.0,
            (axisY->min() + axisY->max()) / 2.0
        );
        m_initialZoom = 1.0;
        qDebug() << "Initial center:" << m_initialCenter << "Initial rect:" << m_initialSceneRect;
    }
}

void CustomChartView::addDynamicLabel(DynamicLabel* label, const QPointF& point) {
    m_labels[point] = label;
}

DynamicLabel* CustomChartView::getLabelAt(const QPointF& point) {
    if (m_labels.contains(point)) {
        return m_labels[point];
    }
    return nullptr;
}

void CustomChartView::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() > 0) {
        chart()->zoomIn();
    } else {
        chart()->zoomOut();
    }
    updateLabels();
    emit chartUpdated();
    event->accept();
}

void CustomChartView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        m_isDragging = true;
        m_lastMousePos = event->pos();
        setRubberBand(QChartView::NoRubberBand);
    }
}

void CustomChartView::mouseMoveEvent(QMouseEvent* event) {
    if (m_isDragging && (event->buttons() & Qt::RightButton)) {
        QPointF delta = event->pos() - m_lastMousePos;
        chart()->scroll(-delta.x(), delta.y());
        m_lastMousePos = event->pos();
        updateLabels();
        emit chartUpdated();
    }
    event->accept();
}

void CustomChartView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        m_isDragging = false;
        setRubberBand(QChartView::RectangleRubberBand);
    }
}

void CustomChartView::updateLabels() {
    for (DynamicLabel* label : m_labels.values()) {
        label->updatePosition();
    }
}

void CustomChartView::resetChartView() {
    QValueAxis* axisX = qobject_cast<QValueAxis*>(chart()->axes(Qt::Horizontal).first());
    QValueAxis* axisY = qobject_cast<QValueAxis*>(chart()->axes(Qt::Vertical).first());
    if (axisX && axisY) {
        chart()->zoomReset();
        axisX->setRange(m_initialSceneRect.left(), m_initialSceneRect.right());
        axisY->setRange(m_initialSceneRect.bottom(), m_initialSceneRect.top());
        for (DynamicLabel* label : m_labels.values()) {
            label->updatePosition();
        }
    }
    emit chartUpdated();
}

void CustomChartView::resizeEvent(QResizeEvent* event) {
    QChartView::resizeEvent(event);
    updateLabels();
}

double complexModel(double x, const VectorXd& params) {
    double a = params(0), b = params(1), c = params(2);
    return a / (x + b) + c;
}

HistogramWidget::HistogramWidget(const QList<QPair<int, int>>& orderedCounts, QWidget* parent)
    : QWidget(parent) {
    setupHistogram(orderedCounts);
}

void HistogramWidget::setupHistogram(const QList<QPair<int, int>>& orderedCounts) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    m_chart = new QChart();
    m_chart->setTitle("精英点被选择次数柱状图");
    QBarSeries* series = new QBarSeries();
    QBarSet* barSet = new QBarSet("被选择次数");
    QStringList categories;
    double maxSqrtCount = 0.0;
    for (int i = 0; i < orderedCounts.size(); ++i) {
        int eliteIndex = orderedCounts[i].first;
        int count = orderedCounts[i].second;
        double sqrtCount = std::sqrt(static_cast<double>(count));
        m_sqrtCounts[eliteIndex] = sqrtCount;
        *barSet << sqrtCount;
        categories << QString("%1").arg(eliteIndex);
        maxSqrtCount = std::max(maxSqrtCount, sqrtCount);
    }
    series->append(barSet);
    m_chart->addSeries(series);
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    axisX->setTitleText("精英点");
    m_chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("被选择次数");
    axisY->setRange(0, maxSqrtCount * 1.1);
    QList<qreal> ticks;
    QList<int> originalValues = {0, 1, 4, 9, 25, 100};
    for (int val : originalValues) {
        ticks << std::sqrt(static_cast<qreal>(val));
    }
    axisY->setTickType(QValueAxis::TicksFixed);
    axisY->setTickAnchor(0);
    if (!ticks.isEmpty()) {
        axisY->setTickInterval(ticks.last() / (ticks.size() - 1));
    }
    axisY->setLabelFormat("%d");
    axisY->setLabelsVisible(false);
    connect(axisY, &QValueAxis::rangeChanged, this, [=](qreal min, qreal max) {
        Q_UNUSED(min);
        Q_UNUSED(max);
        axisY->setTickCount(ticks.size());
    });
    m_chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    QChartView* chartView = new QChartView(m_chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setRubberBand(QChartView::NoRubberBand);
    chartView->setInteractive(false);
    QTimer::singleShot(0, this, [=]() {
        if (!chartView->scene()) return;
        for (int i = 0; i < orderedCounts.size(); ++i) {
            int eliteIndex = orderedCounts[i].first;
            int count = orderedCounts[i].second;
            if (count == 0) continue;
            double sqrtCount = m_sqrtCounts[eliteIndex];
            QPointF logicalPos(i, sqrtCount);
            DynamicLabel* label = new DynamicLabel(
                QString::number(count),
                logicalPos,
                m_chart,
                QPointF(0, -50)
            );
            chartView->scene()->addItem(label);
            m_labels[eliteIndex] = label;
        }
    });
    layout->addWidget(chartView);
    setLayout(layout);
}

void HistogramWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateLabels();
}

void HistogramWidget::updateLabels() {
    for (DynamicLabel* label : m_labels.values()) {
        label->updatePosition();
    }
}

ChartWidget::ChartWidget(QVector<population> elite, QWidget* parent)
    : QWidget(parent), m_elite(elite), m_refLine(nullptr), m_selectedPoint(nullptr), m_isChooseSet(false) {
    setupChart();
}

void ChartWidget::togglePointLabel(const QPointF& point) {
    DynamicLabel* label = m_view->getLabelAt(point);
    if (label) {
        label->setVisible(!label->isVisible());
    }
}

void ChartWidget::showChooseDialog() {
    ChooseDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        choose = dialog.getChooseValue();
        m_isChooseSet = true;
        updateReferenceLine();
    }
}

void ChartWidget::resetChart() {
    m_view->resetChartView();
}

void ChartWidget::onChartUpdated() {
    updateAuxiliaryLines();
    if (m_isChooseSet) {
        updateReferenceLine();
    }
}

void ChartWidget::computeSelectionHistogram() {
    if (m_elite.isEmpty()) {
        qDebug() << "Error: m_elite is empty, cannot compute histogram.";
        return;
    }
    QMap<int, int> selectionCounts;
    for (int i = 1; i <= m_elite.size(); ++i) {
        selectionCounts[i] = 0;
    }
    double originalChoose = choose;
    for (double c = 0.01; c <= 0.99; c += 0.01) {
        choose = c;
        QLineSeries* tempLine = drawReferenceLine(m_elite, m_xMax, m_yMax);
        if (tempLine) {
            delete tempLine;
            if (survive) {
                for (int i = 1; i <= m_elite.size(); ++i) {
                    if (&m_elite[i-1] == survive) {
                        selectionCounts[i]++;
                        break;
                    }
                }
            }
        }
    }
    choose = originalChoose;
    QList<QPair<int, int>> orderedCounts;
    for (int i = 0; i < m_sortedElite.size(); ++i) {
        int eliteIndex = i + 1;
        orderedCounts.append({eliteIndex, selectionCounts[eliteIndex]});
    }
    HistogramWidget* histogramWidget = new HistogramWidget(orderedCounts);
    histogramWidget->setWindowTitle("精英点被选择次数柱状图");
    histogramWidget->resize(800, 600);
    histogramWidget->show();
}

void ChartWidget::setupChart() {
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    if (m_elite.isEmpty()) {
        qDebug() << "Error: m_elite is empty in setupChart.";
        QLabel* errorLabel = new QLabel("错误：精英数据为空，无法绘制图表。", this);
        errorLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(errorLabel);
        setLayout(mainLayout);
        return;
    }
    m_sortedElite = m_elite;
    std::sort(m_sortedElite.begin(), m_sortedElite.end(),
              [](const population& a, const population& b) { return a.pricefit < b.pricefit; });
    for (int i = 0; i < m_elite.size(); ++i) {
        m_eliteMap[i + 1] = m_elite[i];
    }
    double minX = m_sortedElite.front().pricefit;
    double maxX = m_sortedElite.back().pricefit;
    if (!std::isfinite(minX) || !std::isfinite(maxX)) {
        qDebug() << "Error: Invalid minX or maxX - minX:" << minX << ", maxX:" << maxX;
        QLabel* errorLabel = new QLabel("错误：数据中包含非法值，无法绘制图表。", this);
        errorLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(errorLabel);
        setLayout(mainLayout);
        return;
    }
    double x_range = maxX - minX;
    double A_x = 0.4 * x_range;
    m_xMin = minX - A_x;
    m_xMax = maxX + A_x;
    double minY = std::min_element(m_sortedElite.begin(), m_sortedElite.end(),
                                   [](const population& a, const population& b) { return a.timefit < b.timefit; })->timefit;
    double maxY = std::max_element(m_sortedElite.begin(), m_sortedElite.end(),
                                   [](const population& a, const population& b) { return a.timefit < b.timefit; })->timefit;
    if (!std::isfinite(minY) || !std::isfinite(maxY)) {
        qDebug() << "Error: Invalid minY or maxY - minY:" << minY << ", maxY:" << maxY;
        QLabel* errorLabel = new QLabel("错误：数据中包含非法值，无法绘制图表。", this);
        errorLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(errorLabel);
        setLayout(mainLayout);
        return;
    }
    double y_range = maxY - minY;
    double A_y = 0.4 * y_range;
    m_yMin = minY - A_y;
    m_yMax = maxY + A_y;
    m_scatter = new QScatterSeries();
    m_scatter->setName("精英点");
    m_scatter->setMarkerSize(15.0);
    m_scatter->setColor(Qt::black);
    for (const auto& p : m_sortedElite) {
        if (!std::isfinite(p.pricefit) || !std::isfinite(p.timefit)) {
            qDebug() << "Error: Invalid scatter point - pricefit:" << p.pricefit << ", timefit:" << p.timefit;
            delete m_scatter;
            m_scatter = nullptr;
            QLabel* errorLabel = new QLabel("错误：数据中包含非法值，无法绘制图表。", this);
            errorLabel->setAlignment(Qt::AlignCenter);
            mainLayout->addWidget(errorLabel);
            setLayout(mainLayout);
            return;
        }
        m_scatter->append(p.pricefit, p.timefit);
    }
    m_selectedPoint = new QScatterSeries();
    m_selectedPoint->setName("选中点");
    m_selectedPoint->setMarkerSize(15.0);
    m_selectedPoint->setColor(Qt::red);
    double fit_x_start = minX - std::max(0.1 * x_range, 1.0);
    if (minX > 0) {
        fit_x_start = std::min(fit_x_start, minX * 0.9);
    } else if (minX < 0) {
        fit_x_start = minX - std::max(0.1 * std::abs(minX), 1.0);
    }
    qDebug() << "动态 x_start:" << fit_x_start << ", x_end:" << m_xMax << ", 数据范围: [" << minX << ", " << maxX << "]";
    QLineSeries* fitted = fitInverseProportional(m_sortedElite, fit_x_start, m_xMax, maxX, maxY);
    if (!fitted) {
        qDebug() << "Error: Failed to fit inverse proportional curve.";
        delete m_scatter;
        m_scatter = nullptr;
        delete m_selectedPoint;
        m_selectedPoint = nullptr;
        QLabel* errorLabel = new QLabel("错误：无法拟合曲线。", this);
        errorLabel->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(errorLabel);
        setLayout(mainLayout);
        return;
    }
    fitted->setPen(QPen(Qt::blue, 2));
    m_chart = new QChart();
    m_chart->addSeries(m_scatter);
    m_chart->addSeries(m_selectedPoint);
    m_chart->addSeries(fitted);
    QValueAxis* axisX = new QValueAxis();
    QValueAxis* axisY = new QValueAxis();
    axisX->setLabelFormat("%.3f");
    axisY->setLabelFormat("%.3f");
    axisX->setTitleText("价格");
    axisY->setTitleText("时间");
    axisX->setGridLineVisible(true);
    axisY->setGridLineVisible(true);
    axisX->setTickCount(8);
    axisY->setTickCount(8);
    QFont labelFont;
    labelFont.setPointSize(10);
    axisX->setLabelsFont(labelFont);
    axisY->setLabelsFont(labelFont);
    axisX->setRange(m_xMin, m_xMax);
    axisY->setRange(m_yMin, m_yMax);
    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_scatter->attachAxis(axisX);
    m_scatter->attachAxis(axisY);
    m_selectedPoint->attachAxis(axisX);
    m_selectedPoint->attachAxis(axisY);
    fitted->attachAxis(axisX);
    fitted->attachAxis(axisY);
    updateAuxiliaryLines();
    m_chart->setTitle("帕累托前沿与拟合曲线");
    m_chart->setMargins(QMargins(10, 30, 10, 10));
    m_view = new CustomChartView(m_chart);
    QTimer::singleShot(0, this, [this]() {
        if (!m_view->scene()) return;
        int index = 1;
        for (const auto& p : m_sortedElite) {
            DynamicLabel* pointLabel = new DynamicLabel(
                QString("%1号 (%2, %3)").arg(index).arg(p.pricefit, 0, 'f', 3).arg(p.timefit, 0, 'f', 3),
                QPointF(p.pricefit, p.timefit),
                m_chart,
                QPointF(120, -50)
            );
            m_view->scene()->addItem(pointLabel);
            m_view->addDynamicLabel(pointLabel, QPointF(p.pricefit, p.timefit));
            index++;
        }
    });
    mainLayout->addWidget(m_view, 1);
    QVBoxLayout* rightLayout = new QVBoxLayout();

    // 添加 PSO 参数标签
    QLabel* psoLabel = new QLabel(this);
    psoLabel->setText(QString("收敛时PSO全局最优概率：%1\n个体最优概率：%2\n惯性概率：%3")
                      .arg(control_guy.a, 0, 'f', 3)
                      .arg(control_guy.b, 0, 'f', 3)
                      .arg(control_guy.c, 0, 'f', 3));
    // 确保字体与按钮一致
    QPushButton* tempButton = new QPushButton("临时按钮", this); // 创建临时按钮以获取默认字体
    QFont labelabc = tempButton->font(); // 使用 labelabcVFont labelabc = tempButton->font(); // 使用 labelabc 替代 labelFont
    delete tempButton; // 删除临时按钮
    psoLabel->setFont(labelabc);
    psoLabel->setAlignment(Qt::AlignLeft);
    psoLabel->setWordWrap(true); // 允许自动换行
    rightLayout->addWidget(psoLabel);


    // 添加第一个空行
    rightLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    // 添加收敛迭代次数标签
    QLabel* iterLabel = new QLabel(this);
    iterLabel->setText(QString("收敛迭代次数：%1").arg(hi_time));
    iterLabel->setFont(labelabc);
    iterLabel->setAlignment(Qt::AlignLeft);
    iterLabel->setWordWrap(true);
    rightLayout->addWidget(iterLabel);

    // 添加第二个空行
    rightLayout->addSpacerItem(new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

    QPushButton* setChooseButton = new QPushButton("设置 Choose 值", this);
    QPushButton* resetButton = new QPushButton("重置图表", this);
    QPushButton* histogramButton = new QPushButton("显示选择次数柱状图", this);
    rightLayout->addWidget(setChooseButton);
    rightLayout->addWidget(resetButton);
    rightLayout->addWidget(histogramButton);
    rightLayout->addStretch();
    mainLayout->addLayout(rightLayout);
    setLayout(mainLayout);
    connect(m_scatter, &QScatterSeries::clicked, this, &ChartWidget::togglePointLabel);
    connect(setChooseButton, &QPushButton::clicked, this, &ChartWidget::showChooseDialog);
    connect(resetButton, &QPushButton::clicked, this, &ChartWidget::resetChart);
    connect(histogramButton, &QPushButton::clicked, this, &ChartWidget::computeSelectionHistogram);
    connect(m_view, &CustomChartView::chartUpdated, this, &ChartWidget::onChartUpdated);
}

void ChartWidget::updateAuxiliaryLines() {
    for (QLineSeries* line : m_auxiliaryLines) {
        m_chart->removeSeries(line);
        delete line;
    }
    m_auxiliaryLines.clear();
    QValueAxis* axisX = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Horizontal).first());
    QValueAxis* axisY = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Vertical).first());
    if (!axisX || !axisY) {
        qDebug() << "Error: Axes not found in updateAuxiliaryLines.";
        return;
    }
    double xMin = axisX->min();
    double yMin = axisY->min();
    QList<QColor> colors = {
        QColor(200, 0, 0),
        QColor(0, 200, 0),
        QColor(0, 0, 200),
        QColor(200, 0, 200),
        QColor(0, 200, 200),
        QColor(200, 200, 0),
        QColor(150, 200, 0),
        QColor(0, 150, 200),
        QColor(200, 150, 0),
        QColor(150, 0, 200)
    };
    int colorIndex = 0;
    for (const auto& p : m_sortedElite) {
        QColor lineColor = colors[colorIndex % colors.size()];
        colorIndex++;
        QLineSeries* horizontalLine = new QLineSeries();
        horizontalLine->append(xMin, p.timefit);
        horizontalLine->append(p.pricefit, p.timefit);
        QPen pen(lineColor, 2, Qt::DashLine);
        horizontalLine->setPen(pen);
        m_chart->addSeries(horizontalLine);
        horizontalLine->attachAxis(axisX);
        horizontalLine->attachAxis(axisY);
        m_auxiliaryLines.append(horizontalLine);
        QLineSeries* verticalLine = new QLineSeries();
        verticalLine->append(p.pricefit, p.timefit);
        verticalLine->append(p.pricefit, yMin);
        verticalLine->setPen(pen);
        m_chart->addSeries(verticalLine);
        verticalLine->attachAxis(axisX);
        verticalLine->attachAxis(axisY);
        m_auxiliaryLines.append(verticalLine);
    }
}

void ChartWidget::updateReferenceLine() {
    if (!m_isChooseSet) {
        return;
    }
    if (m_elite.isEmpty()) {
        qDebug() << "Error: m_elite is empty, cannot draw reference line.";
        return;
    }
    for (const auto& p : m_elite) {
        if (!std::isfinite(p.pricefit) || !std::isfinite(p.timefit)) {
            qDebug() << "Error: Invalid data in m_elite - pricefit:" << p.pricefit << ", timefit:" << p.timefit;
            return;
        }
    }
    m_scatter->clear();
    for (const auto& p : m_sortedElite) {
        m_scatter->append(p.pricefit, p.timefit);
    }
    m_selectedPoint->clear();
    if (m_refLine) {
        m_chart->removeSeries(m_refLine);
        delete m_refLine;
        m_refLine = nullptr;
    }
    m_refLine = drawReferenceLine(m_elite, m_xMax, m_yMax);
    if (m_refLine) {
        QValueAxis* axisX = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Horizontal).first());
        QValueAxis* axisY = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Vertical).first());
        if (!axisX || !axisY) {
            qDebug() << "Error: Chart axes are not available.";
            delete m_refLine;
            m_refLine = nullptr;
            return;
        }
        m_chart->addSeries(m_refLine);
        m_refLine->attachAxis(axisX);
        m_refLine->attachAxis(axisY);
        if (survive) {
            m_selectedPoint->append(survive->pricefit, survive->timefit);
        }
        qDebug() << "Reference line drawn successfully.";
    } else {
        qDebug() << "Error: Failed to create reference line.";
    }
}

double ChartWidget::simpleModel(double x, double a) {
    return a / x;
}

double ChartWidget::simpleError(double a, const QVector<population>& data) {
    double error = 0.0;
    for (const auto& p : data) {
        double predicted = simpleModel(p.pricefit, a);
        double diff = p.timefit - predicted;
        error += diff * diff;
    }
    return error;
}

double ChartWidget::fitSimpleModel(const QVector<population>& data) {
    double sum_xy = 0.0, sum_x2 = 0.0;
    for (const auto& p : data) {
        double x = p.pricefit, y = p.timefit;
        sum_xy += x * y;
        sum_x2 += x * x;
    }
    double a = sum_xy / sum_x2;
    return std::max(a, 0.0);
}

VectorXd ChartWidget::initializeComplexParams(const QVector<population>& data) {
    VectorXd params(3);
    std::vector<double> y_values;
    for (const auto& p : data) y_values.push_back(p.timefit);
    std::sort(y_values.begin(), y_values.end());
    double c = std::max(y_values.front(), 0.0);
    double min_x = data.front().pricefit, max_x = data.back().pricefit;
    double avg_y = std::accumulate(y_values.begin(), y_values.end(), 0.0) / y_values.size();
    double a = (max_x - min_x) * (avg_y - c);
    double b = min_x / 2.0;
    params << a, b, c;
    return params;
}

VectorXd ChartWidget::fitComplexModel(const QVector<population>& data, double maxX) {
    VectorXd params = initializeComplexParams(data);
    const int maxIter = 500;
    double lambda = 0.001;
    const double tol = 1e-8;
    for (int iter = 0; iter < maxIter; ++iter) {
        VectorXd residuals(data.size());
        MatrixXd jacobian(data.size(), 3);
        const double eps = 1e-6;
        ComplexResidual residual(data);
        residual(params, residuals);
        double error = residuals.squaredNorm();
        for (int j = 0; j < 3; ++j) {
            VectorXd params_eps = params;
            params_eps(j) += eps;
            VectorXd residuals_eps(data.size());
            residual(params_eps, residuals_eps);
            jacobian.col(j) = (residuals_eps - residuals) / eps;
        }
        MatrixXd H = jacobian.transpose() * jacobian + lambda * MatrixXd::Identity(3, 3);
        VectorXd g = jacobian.transpose() * residuals;
        VectorXd delta = H.ldlt().solve(-g);
        VectorXd params_new = params + delta;
        VectorXd residuals_new(data.size());
        residual(params_new, residuals_new);
        double error_new = residuals_new.squaredNorm();
        if (error_new < error) {
            params = params_new;
            lambda /= 10.0;
            if (delta.norm() < tol) break;
        } else {
            lambda *= 10.0;
        }
    }
    return params;
}

QLineSeries* ChartWidget::fitInverseProportional(const QVector<population>& data, double x_start, double x_end, double maxX, double maxY) {
    QVector<population> sorted_data = data;
    std::sort(sorted_data.begin(), sorted_data.end(),
              [](const population& a, const population& b) { return a.pricefit < b.pricefit; });
    double a_simple = fitSimpleModel(sorted_data);
    double simple_error = simpleError(a_simple, sorted_data);
    VectorXd params_complex = fitComplexModel(sorted_data, maxX);
    VectorXd residuals_complex(sorted_data.size());
    ComplexResidual residual(sorted_data);
    residual(params_complex, residuals_complex);
    double complex_error = residuals_complex.squaredNorm();
    QLineSeries* curve = new QLineSeries();
    curve->setName(simple_error < complex_error ? "简单反比例拟合" : "复杂反比例拟合");
    int resolution = 1000;
    double min_x = sorted_data.front().pricefit;
    double x_range = maxX - min_x;
    double initial_step = std::max(x_range / resolution, 1e-6);
    if (simple_error < complex_error) {
        if (x_start <= 0) x_start = 1e-6;
        if (x_start < min_x) x_start = min_x;
        double step = initial_step;
        qDebug() << "简单模型: x_start =" << x_start << ", x_end =" << x_end << ", step =" << step;
        double x = x_start;
        int points_added = 0;
        while (x <= x_end && points_added < resolution) {
            if (x <= 0) {
                x += step;
                continue;
            }
            double y = simpleModel(x, a_simple);
            if (std::isfinite(y)) {
                curve->append(x, y);
                if (points_added == 0) qDebug() << "简单模型第一个点: x =" << x << ", y =" << y;
                points_added++;
            }
            x += step;
            step *= 1.01;
        }
    } else {
        double b = params_complex(1);
        double singularity_x = -b;
        if (x_start <= singularity_x) x_start = singularity_x + 1e-6;
        if (x_start < min_x) x_start = min_x;
        double step = initial_step;
        qDebug() << "复杂模型: b =" << b << ", 奇点 x =" << singularity_x << ", x_start =" << x_start << ", x_end =" << x_end << ", step =" << step;
        double x = x_start;
        int points_added = 0;
        while (x <= x_end && points_added < resolution) {
            if (x <= singularity_x) {
                x += step;
                continue;
            }
            double y = complexModel(x, params_complex);
            if (std::isfinite(y)) {
                curve->append(x, y);
                if (points_added == 0) qDebug() << "复杂模型第一个点: x =" << x << ", y =" << y;
                points_added++;
            }
            x += step;
            step *= 1.01;
        }
    }
    return curve;
}

QLineSeries* ChartWidget::drawReferenceLine(QVector<population>& data, double maxX, double maxY) {
    if (data.isEmpty()) {
        qDebug() << "drawReferenceLine: Data is empty.";
        return nullptr;
    }
    if (!std::isfinite(maxX) || !std::isfinite(maxY) || maxX <= 0 || maxY <= 0) {
        qDebug() << "Error: Invalid maxX or maxY - maxX:" << maxX << ", maxY:" << maxY;
        return nullptr;
    }
    QValueAxis* axisX = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Horizontal).first());
    QValueAxis* axisY = qobject_cast<QValueAxis*>(m_chart->axes(Qt::Vertical).first());
    if (!axisX || !axisY) {
        qDebug() << "Error: Axes not found in drawReferenceLine.";
        return nullptr;
    }
    double x_min = axisX->min();
    double x_max = axisX->max();
    double k;
    if (choose > 0.99) choose = 0.99;
    if (choose < 0.01) choose = 0.01;
    double x_temp = maxX * (1.0 - choose);
    double y_temp = maxY * choose;
    if (x_temp <= 0) {
        qDebug() << "Error: x_temp is zero or negative - x_temp:" << x_temp << ", choose:" << choose;
        return nullptr;
    }
    k = y_temp / x_temp;
    if (!std::isfinite(k)) {
        qDebug() << "Error: k is not finite - k:" << k;
        return nullptr;
    }
    qDebug() << "choose:" << choose << "x_temp:" << x_temp << "y_temp:" << y_temp << "k:" << k;
    double min_distance = std::numeric_limits<double>::max();
    double x_nearest = 0.0, y_nearest = 0.0;
    int nearest_index = -1;
    for (int i = 0; i < data.size(); ++i) {
        double x = data[i].pricefit, y = data[i].timefit;
        if (!std::isfinite(x) || !std::isfinite(y)) {
            qDebug() << "Error: Invalid data point - pricefit:" << x << ", timefit:" << y;
            return nullptr;
        }
        double distance = std::abs(y + k * x) / std::sqrt(1 + k * k);
        if (distance < min_distance) {
            min_distance = distance;
            x_nearest = x;
            y_nearest = y;
            nearest_index = i;
        }
    }
    if (nearest_index == -1) {
        qDebug() << "Error: No nearest point found.";
        return nullptr;
    }
    survive = &data[nearest_index];
    qDebug() << "Survive point: pricefit=" << survive->pricefit << ", timefit:" << survive->timefit;
    QLineSeries* line = new QLineSeries();
    line->setName("参考直线");
    line->setPen(QPen(Qt::red, 2));
    double y_start = -k * (x_min - x_nearest) + y_nearest;
    double y_end = -k * (x_max - x_nearest) + y_nearest;
    if (!std::isfinite(y_start) || !std::isfinite(y_end)) {
        qDebug() << "Error: Invalid y_start or y_end - y_start:" << y_start << ", y_end:" << y_end;
        delete line;
        return nullptr;
    }
    line->append(x_min, y_start);
    line->append(x_max, y_end);
    return line;
}

ChartWidget* showEliteChartWithFit(QVector<population>& elite) {
    ChartWidget* widget = new ChartWidget(elite);
    widget->setWindowTitle("帕累托前沿与拟合曲线");
    widget->resize(1200, 800);
    widget->show();
    return widget;
}
