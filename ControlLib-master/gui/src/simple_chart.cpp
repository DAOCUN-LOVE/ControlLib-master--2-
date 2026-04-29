#include "simple_chart.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <limits>

SimpleChart::SimpleChart(QWidget *parent)
    : QWidget(parent) {
    setMinimumSize(520, 340);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);
}

void SimpleChart::setData(int graphIndex, const QVector<double> &xData, const QVector<double> &yData) {
    ensureGraph(graphIndex);
    graphs_[graphIndex].xData = xData;
    graphs_[graphIndex].yData = yData;
    update();
}

void SimpleChart::setAxisLabel(const QString &xLabel, const QString &yLabel) {
    setAxisLabels(xLabel, yLabel, right_y_label_);
}

void SimpleChart::setAxisLabels(const QString &xLabel, const QString &leftYLabel, const QString &rightYLabel) {
    x_label_ = xLabel;
    left_y_label_ = leftYLabel;
    right_y_label_ = rightYLabel;
    update();
}

void SimpleChart::setLegendNames(const QStringList &names) {
    legend_names_ = names;
    update();
}

void SimpleChart::clear() {
    for (auto &graph : graphs_) {
        graph.xData.clear();
        graph.yData.clear();
    }
    update();
}

void SimpleChart::replot() {
    update();
}

void SimpleChart::setPenColor(int graphIndex, const QColor &color) {
    ensureGraph(graphIndex);
    graphs_[graphIndex].color = color;
    update();
}

void SimpleChart::setPenStyle(int graphIndex, Qt::PenStyle style) {
    ensureGraph(graphIndex);
    graphs_[graphIndex].style = style;
    update();
}

void SimpleChart::setGraphAxis(int graphIndex, Axis axis) {
    ensureGraph(graphIndex);
    graphs_[graphIndex].axis = axis;
    update();
}

void SimpleChart::setGraphVisible(int graphIndex, bool visible) {
    ensureGraph(graphIndex);
    graphs_[graphIndex].visible = visible;
    update();
}

void SimpleChart::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);

    const QRect chart_rect = plotRect();
    drawGrid(painter, chart_rect);
    drawAxes(painter, chart_rect);
    drawGraphs(painter, chart_rect);
    drawLegend(painter, chart_rect);
}

void SimpleChart::ensureGraph(int graphIndex) {
    if (graphIndex < 0) {
        return;
    }

    if (graphIndex >= static_cast<int>(graphs_.size())) {
        const int old_size = static_cast<int>(graphs_.size());
        graphs_.resize(graphIndex + 1);
        for (int i = old_size; i <= graphIndex; ++i) {
            graphs_[i].color = (i == 0) ? QColor(33, 113, 181)
                              : (i == 1) ? QColor(203, 24, 29)
                                         : QColor(35, 139, 69);
        }
    }
}

QRect SimpleChart::plotRect() const {
    const int w = std::max(80, width() - LEFT_MARGIN - RIGHT_MARGIN);
    const int h = std::max(80, height() - TOP_MARGIN - BOTTOM_MARGIN);
    return QRect(LEFT_MARGIN, TOP_MARGIN, w, h);
}

void SimpleChart::drawGrid(QPainter &painter, const QRect &rect) {
    painter.save();
    painter.setPen(QPen(QColor(220, 224, 230), 1, Qt::DashLine));

    for (int i = 0; i <= 10; ++i) {
        const double ratio = i / 10.0;
        const int x = rect.left() + static_cast<int>(ratio * rect.width());
        const int y = rect.top() + static_cast<int>(ratio * rect.height());
        painter.drawLine(x, rect.top(), x, rect.bottom());
        painter.drawLine(rect.left(), y, rect.right(), y);
    }

    painter.restore();
}

void SimpleChart::drawAxes(QPainter &painter, const QRect &rect) {
    const QRectF x_range = getXRange();
    const QRectF left_range = getDataRect(Axis::Left);
    const QRectF right_range = getDataRect(Axis::Right);

    painter.save();
    painter.setFont(QFont("Arial", 9));

    painter.setPen(QPen(Qt::black, 2));
    painter.drawLine(rect.left(), rect.bottom(), rect.right(), rect.bottom());
    painter.drawLine(rect.left(), rect.top(), rect.left(), rect.bottom());
    painter.drawLine(rect.right(), rect.top(), rect.right(), rect.bottom());

    painter.setPen(QPen(Qt::black, 1));
    for (int i = 0; i <= 5; ++i) {
        const double ratio = i / 5.0;
        const int x = rect.left() + static_cast<int>(ratio * rect.width());
        const int y = rect.bottom() - static_cast<int>(ratio * rect.height());

        const double x_value = x_range.left() + ratio * x_range.width();
        const double left_value = left_range.top() + ratio * left_range.height();
        const double right_value = right_range.top() + ratio * right_range.height();

        painter.drawLine(x, rect.bottom(), x, rect.bottom() + 5);
        painter.drawText(x - 28, rect.bottom() + 22, 56, 16, Qt::AlignCenter, formatNumber(x_value));

        painter.drawLine(rect.left() - 5, y, rect.left(), y);
        painter.drawText(4, y - 8, LEFT_MARGIN - 12, 16, Qt::AlignRight | Qt::AlignVCenter,
                         formatNumber(left_value));

        painter.drawLine(rect.right(), y, rect.right() + 5, y);
        painter.drawText(rect.right() + 9, y - 8, RIGHT_MARGIN - 12, 16,
                         Qt::AlignLeft | Qt::AlignVCenter, formatNumber(right_value));
    }

    painter.setPen(Qt::black);
    painter.drawText(rect.center().x() - 60, height() - 20, 120, 16, Qt::AlignCenter, x_label_);

    painter.save();
    painter.translate(18, rect.center().y() + 50);
    painter.rotate(-90);
    painter.drawText(0, 0, 120, 16, Qt::AlignCenter, left_y_label_);
    painter.restore();

    painter.save();
    painter.translate(width() - 18, rect.center().y() - 50);
    painter.rotate(90);
    painter.drawText(0, 0, 120, 16, Qt::AlignCenter, right_y_label_);
    painter.restore();

    painter.restore();
}

void SimpleChart::drawGraphs(QPainter &painter, const QRect &rect) {
    painter.save();
    painter.setClipRect(rect.adjusted(1, 1, -1, -1));

    for (const auto &graph : graphs_) {
        if (!graph.visible || graph.xData.isEmpty() || graph.yData.isEmpty()) {
            continue;
        }

        const QRectF data_rect = getDataRect(graph.axis);
        QPainterPath path;
        bool first = true;

        const int count = std::min(graph.xData.size(), graph.yData.size());
        for (int i = 0; i < count; ++i) {
            const QPointF screen_point = dataToScreen(QPointF(graph.xData[i], graph.yData[i]), data_rect, rect);
            if (first) {
                path.moveTo(screen_point);
                first = false;
            } else {
                path.lineTo(screen_point);
            }
        }

        painter.setPen(QPen(graph.color, 2, graph.style));
        painter.drawPath(path);
    }

    painter.restore();
}

void SimpleChart::drawLegend(QPainter &painter, const QRect &rect) {
    if (legend_names_.isEmpty()) {
        return;
    }

    painter.save();
    painter.setFont(QFont("Arial", 9));

    int x = rect.left() + LEGEND_MARGIN;
    int y = rect.top() + LEGEND_MARGIN;
    const int line_height = 20;

    for (int i = 0; i < legend_names_.size() && i < static_cast<int>(graphs_.size()); ++i) {
        if (!graphs_[i].visible) {
            continue;
        }

        painter.setPen(QPen(graphs_[i].color, 2, graphs_[i].style));
        painter.drawLine(x, y + 7, x + 22, y + 7);

        painter.setPen(Qt::black);
        painter.drawText(x + 28, y + 12, legend_names_[i]);
        y += line_height;
    }

    painter.restore();
}

QRectF SimpleChart::getXRange() const {
    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    bool has_data = false;

    for (const auto &graph : graphs_) {
        if (!graph.visible || graph.xData.isEmpty()) {
            continue;
        }

        for (double x : graph.xData) {
            min_x = std::min(min_x, x);
            max_x = std::max(max_x, x);
            has_data = true;
        }
    }

    if (!has_data) {
        return QRectF(0.0, 0.0, 10.0, 1.0);
    }

    if (max_x <= min_x) {
        max_x = min_x + 1.0;
    }

    const double center = (min_x + max_x) * 0.5 + x_pan_;
    const double span = std::max((max_x - min_x) / std::max(0.1, x_zoom_), 1e-6);
    return QRectF(center - span * 0.5, 0.0, span, 1.0);
}

QRectF SimpleChart::getDataRect(Axis axis) const {
    const QRectF x_range = getXRange();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::lowest();
    bool has_data = false;

    for (const auto &graph : graphs_) {
        if (!graph.visible || graph.axis != axis || graph.yData.isEmpty()) {
            continue;
        }

        for (double y : graph.yData) {
            min_y = std::min(min_y, y);
            max_y = std::max(max_y, y);
            has_data = true;
        }
    }

    if (!has_data) {
        min_y = 0.0;
        max_y = 1.0;
    }

    if (max_y <= min_y) {
        max_y = min_y + 1.0;
    }

    const double padding = std::max((max_y - min_y) * 0.08, 1e-3);
    min_y -= padding;
    max_y += padding;

    const double center = (min_y + max_y) * 0.5 + y_pan_;
    const double span = std::max((max_y - min_y) / std::max(0.1, y_zoom_), 1e-6);
    return QRectF(x_range.left(), center - span * 0.5, x_range.width(), span);
}

QPointF SimpleChart::dataToScreen(const QPointF &point, const QRectF &dataRect, const QRect &screenRect) const {
    const double ratio_x = (point.x() - dataRect.left()) / dataRect.width();
    const double ratio_y = (point.y() - dataRect.top()) / dataRect.height();

    const double screen_x = screenRect.left() + ratio_x * screenRect.width();
    const double screen_y = screenRect.bottom() - ratio_y * screenRect.height();
    return QPointF(screen_x, screen_y);
}

QString SimpleChart::formatNumber(double value) const {
    const double abs_value = std::abs(value);
    if (abs_value >= 1000.0 || (abs_value > 0.0 && abs_value < 0.01)) {
        return QString::number(value, 'e', 1);
    }

    if (abs_value >= 100.0) {
        return QString::number(value, 'f', 0);
    }

    if (abs_value >= 10.0) {
        return QString::number(value, 'f', 1);
    }

    return QString::number(value, 'f', 2);
}

void SimpleChart::mousePressEvent(QMouseEvent *event) {
    dragging_ = true;
    last_pos_ = event->pos();
}

void SimpleChart::mouseMoveEvent(QMouseEvent *event) {
    if (!dragging_) {
        return;
    }

    const QRect rect = plotRect();
    const QRectF x_range = getXRange();
    const QRectF left_range = getDataRect(Axis::Left);
    const QPoint delta = event->pos() - last_pos_;

    x_pan_ -= static_cast<double>(delta.x()) / std::max(1, rect.width()) * x_range.width();
    y_pan_ += static_cast<double>(delta.y()) / std::max(1, rect.height()) * left_range.height();
    last_pos_ = event->pos();
    update();
}

void SimpleChart::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event)
    dragging_ = false;
}

void SimpleChart::wheelEvent(QWheelEvent *event) {
    const double scale = std::pow(1.0015, event->angleDelta().y());
    x_zoom_ = std::clamp(x_zoom_ * scale, 0.1, 100.0);
    y_zoom_ = std::clamp(y_zoom_ * scale, 0.1, 100.0);
    update();
}
