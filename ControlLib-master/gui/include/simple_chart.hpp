#pragma once

#include <QObject>
#include <QColor>
#include <QPoint>
#include <QRectF>
#include <QStringList>
#include <QVector>
#include <QWidget>
#include <vector>

class SimpleChart : public QWidget {
    Q_OBJECT

public:
    enum class Axis {
        Left,
        Right
    };

    explicit SimpleChart(QWidget *parent = nullptr);

    void setData(int graphIndex, const QVector<double> &xData, const QVector<double> &yData);
    void setAxisLabel(const QString &xLabel, const QString &yLabel);
    void setAxisLabels(const QString &xLabel, const QString &leftYLabel, const QString &rightYLabel);
    void setLegendNames(const QStringList &names);
    void clear();
    void replot();

    void setPenColor(int graphIndex, const QColor &color);
    void setPenStyle(int graphIndex, Qt::PenStyle style);
    void setGraphAxis(int graphIndex, Axis axis);
    void setGraphVisible(int graphIndex, bool visible);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    struct Graph {
        QVector<double> xData;
        QVector<double> yData;
        QColor color = Qt::blue;
        Qt::PenStyle style = Qt::SolidLine;
        Axis axis = Axis::Left;
        bool visible = true;
    };

    void ensureGraph(int graphIndex);
    void drawGrid(QPainter &painter, const QRect &rect);
    void drawAxes(QPainter &painter, const QRect &rect);
    void drawGraphs(QPainter &painter, const QRect &rect);
    void drawLegend(QPainter &painter, const QRect &rect);

    QRect plotRect() const;
    QRectF getDataRect(Axis axis) const;
    QRectF getXRange() const;
    QPointF dataToScreen(const QPointF &point, const QRectF &dataRect, const QRect &screenRect) const;
    QString formatNumber(double value) const;

    std::vector<Graph> graphs_;
    QString x_label_ = "Time (s)";
    QString left_y_label_ = "Speed";
    QString right_y_label_ = "Current";
    QStringList legend_names_;

    double x_zoom_ = 1.0;
    double y_zoom_ = 1.0;
    double x_pan_ = 0.0;
    double y_pan_ = 0.0;
    bool dragging_ = false;
    QPoint last_pos_;

    static constexpr int LEFT_MARGIN = 76;
    static constexpr int RIGHT_MARGIN = 76;
    static constexpr int TOP_MARGIN = 18;
    static constexpr int BOTTOM_MARGIN = 54;
    static constexpr int LEGEND_MARGIN = 10;
};
