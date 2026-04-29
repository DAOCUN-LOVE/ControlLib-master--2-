#pragma once

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QTimer>
#include <memory>

#include "motor_data_manager.hpp"
#include "simple_chart.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onUpdateUi();
    void onClearData();
    void onSaveData();
    void onStartControl();
    void onStopControl();
    void onApplyParameters();
    void onTogglePlotPause();
    void onCurveVisibilityChanged();
    void onDataError(const QString &message);

private:
    void setupUI();
    void setupConnections();
    void initPlot();
    void updatePlotData(const std::vector<SpeedData> &data);
    void appendLogRows(const std::vector<SpeedData> &data);
    void trimLogLines();
    void updateStatus();
    QString formatLogLine(const SpeedData &data) const;

    SimpleChart *plot_ = nullptr;
    QTimer *update_timer_ = nullptr;

    QLineEdit *can_name_edit_ = nullptr;
    QSpinBox *motor_id_spinbox_ = nullptr;
    QDoubleSpinBox *target_speed_spinbox_ = nullptr;
    QDoubleSpinBox *kp_spinbox_ = nullptr;
    QDoubleSpinBox *ki_spinbox_ = nullptr;
    QDoubleSpinBox *kd_spinbox_ = nullptr;
    QCheckBox *show_target_checkbox_ = nullptr;
    QCheckBox *show_current_checkbox_ = nullptr;

    QLabel *status_label_ = nullptr;
    QTextEdit *log_text_ = nullptr;

    QPushButton *start_button_ = nullptr;
    QPushButton *stop_button_ = nullptr;
    QPushButton *apply_button_ = nullptr;
    QPushButton *pause_button_ = nullptr;
    QPushButton *clear_button_ = nullptr;
    QPushButton *save_button_ = nullptr;

    std::unique_ptr<MotorDataManager> data_manager_;
    bool plot_paused_ = false;
    size_t logged_count_ = 0;

    static constexpr int kMaxLogLines = 500;
};
