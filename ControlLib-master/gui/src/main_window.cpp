#include "main_window.hpp"

#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextStream>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("GM6020 单电机 PID 控制与数据监视"));
    resize(1280, 820);

    data_manager_ = std::make_unique<MotorDataManager>(this);

    setupUI();
    setupConnections();
    initPlot();
    updateStatus();
}

MainWindow::~MainWindow() {
    if (update_timer_) {
        update_timer_->stop();
    }
}

void MainWindow::setupUI() {
    QWidget *central_widget = new QWidget(this);
    setCentralWidget(central_widget);

    QVBoxLayout *root_layout = new QVBoxLayout(central_widget);
    QHBoxLayout *top_layout = new QHBoxLayout();

    plot_ = new SimpleChart(this);
    top_layout->addWidget(plot_, 3);

    QWidget *control_panel = new QWidget(this);
    control_panel->setMinimumWidth(300);
    control_panel->setMaximumWidth(360);
    QVBoxLayout *panel_layout = new QVBoxLayout(control_panel);

    QGroupBox *motor_group = new QGroupBox(QStringLiteral("电机连接"), this);
    QFormLayout *motor_layout = new QFormLayout(motor_group);
    can_name_edit_ = new QLineEdit(QStringLiteral("can0"), this);
    motor_id_spinbox_ = new QSpinBox(this);
    motor_id_spinbox_->setRange(1, 8);
    motor_id_spinbox_->setValue(1);
    motor_layout->addRow(QStringLiteral("CAN 设备"), can_name_edit_);
    motor_layout->addRow(QStringLiteral("GM6020 ID"), motor_id_spinbox_);
    panel_layout->addWidget(motor_group);

    QGroupBox *pid_group = new QGroupBox(QStringLiteral("目标与 PID"), this);
    QFormLayout *pid_layout = new QFormLayout(pid_group);

    target_speed_spinbox_ = new QDoubleSpinBox(this);
    target_speed_spinbox_->setRange(-1000.0, 1000.0);
    target_speed_spinbox_->setDecimals(3);
    target_speed_spinbox_->setSingleStep(1.0);
    target_speed_spinbox_->setValue(0.0);
    target_speed_spinbox_->setSuffix(QStringLiteral(" rad/s"));

    kp_spinbox_ = new QDoubleSpinBox(this);
    kp_spinbox_->setRange(0.0, 100000.0);
    kp_spinbox_->setDecimals(4);
    kp_spinbox_->setSingleStep(0.1);
    kp_spinbox_->setValue(100.0);

    ki_spinbox_ = new QDoubleSpinBox(this);
    ki_spinbox_->setRange(0.0, 100000.0);
    ki_spinbox_->setDecimals(4);
    ki_spinbox_->setSingleStep(0.01);
    ki_spinbox_->setValue(0.0);

    kd_spinbox_ = new QDoubleSpinBox(this);
    kd_spinbox_->setRange(0.0, 100000.0);
    kd_spinbox_->setDecimals(4);
    kd_spinbox_->setSingleStep(0.01);
    kd_spinbox_->setValue(0.0);

    apply_button_ = new QPushButton(QStringLiteral("应用参数"), this);

    pid_layout->addRow(QStringLiteral("目标速度"), target_speed_spinbox_);
    pid_layout->addRow(QStringLiteral("Kp"), kp_spinbox_);
    pid_layout->addRow(QStringLiteral("Ki"), ki_spinbox_);
    pid_layout->addRow(QStringLiteral("Kd"), kd_spinbox_);
    pid_layout->addRow(apply_button_);
    panel_layout->addWidget(pid_group);

    QGroupBox *curve_group = new QGroupBox(QStringLiteral("曲线显示"), this);
    QVBoxLayout *curve_layout = new QVBoxLayout(curve_group);
    show_target_checkbox_ = new QCheckBox(QStringLiteral("叠加目标速度"), this);
    show_target_checkbox_->setChecked(true);
    show_current_checkbox_ = new QCheckBox(QStringLiteral("显示实际电流"), this);
    show_current_checkbox_->setChecked(true);
    curve_layout->addWidget(show_target_checkbox_);
    curve_layout->addWidget(show_current_checkbox_);
    panel_layout->addWidget(curve_group);

    QGroupBox *status_group = new QGroupBox(QStringLiteral("状态"), this);
    QVBoxLayout *status_layout = new QVBoxLayout(status_group);
    status_label_ = new QLabel(this);
    status_label_->setWordWrap(true);
    status_layout->addWidget(status_label_);
    panel_layout->addWidget(status_group);

    QGroupBox *button_group = new QGroupBox(QStringLiteral("控制"), this);
    QVBoxLayout *button_layout = new QVBoxLayout(button_group);
    start_button_ = new QPushButton(QStringLiteral("开始控制"), this);
    stop_button_ = new QPushButton(QStringLiteral("停止控制"), this);
    stop_button_->setEnabled(false);
    pause_button_ = new QPushButton(QStringLiteral("暂停绘图"), this);
    clear_button_ = new QPushButton(QStringLiteral("清除图表和日志"), this);
    save_button_ = new QPushButton(QStringLiteral("保存历史 CSV"), this);

    button_layout->addWidget(start_button_);
    button_layout->addWidget(stop_button_);
    button_layout->addWidget(pause_button_);
    button_layout->addWidget(clear_button_);
    button_layout->addWidget(save_button_);
    panel_layout->addWidget(button_group);
    panel_layout->addStretch();

    top_layout->addWidget(control_panel, 1);
    root_layout->addLayout(top_layout, 1);

    QGroupBox *log_group = new QGroupBox(QStringLiteral("原始 motordata"), this);
    QVBoxLayout *log_layout = new QVBoxLayout(log_group);
    log_text_ = new QTextEdit(this);
    log_text_->setReadOnly(true);
    log_text_->setLineWrapMode(QTextEdit::NoWrap);
    log_text_->setMinimumHeight(170);
    log_text_->setFontFamily(QStringLiteral("monospace"));
    log_layout->addWidget(log_text_);
    root_layout->addWidget(log_group);
}

void MainWindow::setupConnections() {
    update_timer_ = new QTimer(this);
    update_timer_->setInterval(100);
    connect(update_timer_, &QTimer::timeout, this, &MainWindow::onUpdateUi);

    connect(start_button_, &QPushButton::clicked, this, &MainWindow::onStartControl);
    connect(stop_button_, &QPushButton::clicked, this, &MainWindow::onStopControl);
    connect(apply_button_, &QPushButton::clicked, this, &MainWindow::onApplyParameters);
    connect(pause_button_, &QPushButton::clicked, this, &MainWindow::onTogglePlotPause);
    connect(clear_button_, &QPushButton::clicked, this, &MainWindow::onClearData);
    connect(save_button_, &QPushButton::clicked, this, &MainWindow::onSaveData);
    connect(show_target_checkbox_, &QCheckBox::stateChanged, this, &MainWindow::onCurveVisibilityChanged);
    connect(show_current_checkbox_, &QCheckBox::stateChanged, this, &MainWindow::onCurveVisibilityChanged);
    connect(data_manager_.get(), &MotorDataManager::error, this, &MainWindow::onDataError);
}

void MainWindow::initPlot() {
    plot_->setAxisLabels(QStringLiteral("Time (s)"),
                         QStringLiteral("Speed (rad/s)"),
                         QStringLiteral("Current (A)"));
    plot_->setLegendNames({
        QStringLiteral("Actual speed"),
        QStringLiteral("Target speed"),
        QStringLiteral("Actual current")
    });

    plot_->setGraphAxis(0, SimpleChart::Axis::Left);
    plot_->setPenColor(0, QColor(33, 113, 181));
    plot_->setPenStyle(0, Qt::SolidLine);

    plot_->setGraphAxis(1, SimpleChart::Axis::Left);
    plot_->setPenColor(1, QColor(203, 24, 29));
    plot_->setPenStyle(1, Qt::DashLine);

    plot_->setGraphAxis(2, SimpleChart::Axis::Right);
    plot_->setPenColor(2, QColor(35, 139, 69));
    plot_->setPenStyle(2, Qt::SolidLine);

    onCurveVisibilityChanged();
}

void MainWindow::onStartControl() {
    onApplyParameters();

    const bool started = data_manager_->start(can_name_edit_->text(), motor_id_spinbox_->value());
    if (!started) {
        return;
    }

    update_timer_->start();
    start_button_->setEnabled(false);
    stop_button_->setEnabled(true);
    can_name_edit_->setEnabled(false);
    motor_id_spinbox_->setEnabled(false);
    updateStatus();
}

void MainWindow::onStopControl() {
    data_manager_->stop();
    onUpdateUi();
    update_timer_->stop();

    start_button_->setEnabled(true);
    stop_button_->setEnabled(false);
    updateStatus();
}

void MainWindow::onApplyParameters() {
    data_manager_->applyControlParameters(
        static_cast<float>(target_speed_spinbox_->value()),
        static_cast<float>(kp_spinbox_->value()),
        static_cast<float>(ki_spinbox_->value()),
        static_cast<float>(kd_spinbox_->value()));
    updateStatus();
}

void MainWindow::onTogglePlotPause() {
    plot_paused_ = !plot_paused_;
    pause_button_->setText(plot_paused_ ? QStringLiteral("继续绘图") : QStringLiteral("暂停绘图"));

    if (!plot_paused_) {
        updatePlotData(data_manager_->getMonitor()->getDataSnapshot());
    }

    updateStatus();
}

void MainWindow::onCurveVisibilityChanged() {
    plot_->setGraphVisible(0, true);
    plot_->setGraphVisible(1, show_target_checkbox_->isChecked());
    plot_->setGraphVisible(2, show_current_checkbox_->isChecked());

    if (!plot_paused_) {
        updatePlotData(data_manager_->getMonitor()->getDataSnapshot());
    }
}

void MainWindow::onUpdateUi() {
    const auto data = data_manager_->getMonitor()->getDataSnapshot();
    appendLogRows(data);

    if (!plot_paused_) {
        updatePlotData(data);
    }

    updateStatus();
}

void MainWindow::updatePlotData(const std::vector<SpeedData> &data) {
    QVector<double> times;
    QVector<double> actual_speeds;
    QVector<double> target_speeds;
    QVector<double> actual_currents;

    times.reserve(static_cast<int>(data.size()));
    actual_speeds.reserve(static_cast<int>(data.size()));
    target_speeds.reserve(static_cast<int>(data.size()));
    actual_currents.reserve(static_cast<int>(data.size()));

    for (const auto &d : data) {
        times.push_back(d.timestamp);
        actual_speeds.push_back(d.actual_speed);
        target_speeds.push_back(d.target_speed);
        actual_currents.push_back(d.actual_current);
    }

    plot_->setData(0, times, actual_speeds);
    plot_->setData(1, times, target_speeds);
    plot_->setData(2, times, actual_currents);
    plot_->replot();
}

void MainWindow::appendLogRows(const std::vector<SpeedData> &data) {
    if (logged_count_ > data.size()) {
        logged_count_ = 0;
    }

    for (size_t i = logged_count_; i < data.size(); ++i) {
        log_text_->append(formatLogLine(data[i]));
    }
    logged_count_ = data.size();

    trimLogLines();
    QTextCursor cursor = log_text_->textCursor();
    cursor.movePosition(QTextCursor::End);
    log_text_->setTextCursor(cursor);
}

void MainWindow::trimLogLines() {
    QTextDocument *doc = log_text_->document();
    while (doc->blockCount() > kMaxLogLines) {
        QTextCursor cursor(doc->firstBlock());
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.removeSelectedText();
        cursor.deleteChar();
    }
}

QString MainWindow::formatLogLine(const SpeedData &data) const {
    return QStringLiteral("t=%1s id=%2 target=%3rad/s speed=%4rad/s current=%5A raw_current=%6 cmd=%7 ecd=%8 rpm=%9 temp=%10C kp=%11 ki=%12 kd=%13")
        .arg(data.timestamp, 0, 'f', 3)
        .arg(data.motor_id)
        .arg(data.target_speed, 0, 'f', 3)
        .arg(data.actual_speed, 0, 'f', 3)
        .arg(data.actual_current, 0, 'f', 3)
        .arg(data.raw_current)
        .arg(data.command_current)
        .arg(data.encoder)
        .arg(data.speed_rpm)
        .arg(static_cast<int>(data.temperature))
        .arg(data.kp, 0, 'f', 4)
        .arg(data.ki, 0, 'f', 4)
        .arg(data.kd, 0, 'f', 4);
}

void MainWindow::updateStatus() {
    SpeedData latest;
    const bool has_latest = data_manager_->getMonitor()->getLatestData(latest);
    const auto params = data_manager_->controlParameters();

    QString state = data_manager_->isRunning()
        ? QStringLiteral("控制中")
        : QStringLiteral("未运行");
    if (plot_paused_) {
        state += QStringLiteral("，绘图已暂停");
    }

    QString text = QStringLiteral("%1\n点数: %2\n目标: %3 rad/s\nPID: Kp=%4 Ki=%5 Kd=%6")
        .arg(state)
        .arg(data_manager_->getMonitor()->size())
        .arg(params.target_speed, 0, 'f', 3)
        .arg(params.kp, 0, 'f', 4)
        .arg(params.ki, 0, 'f', 4)
        .arg(params.kd, 0, 'f', 4);

    if (has_latest) {
        text += QStringLiteral("\n实际速度: %1 rad/s\n实际电流: %2 A\n命令电流: %3")
            .arg(latest.actual_speed, 0, 'f', 3)
            .arg(latest.actual_current, 0, 'f', 3)
            .arg(latest.command_current);
    }

    status_label_->setText(text);
}

void MainWindow::onClearData() {
    data_manager_->getMonitor()->clear();
    logged_count_ = 0;
    log_text_->clear();
    plot_->clear();
    updateStatus();
}

void MainWindow::onSaveData() {
    const QString filename = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("保存历史数据"),
        QStringLiteral("gm6020_history_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        QStringLiteral("CSV Files (*.csv)"));

    if (filename.isEmpty()) {
        return;
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, QStringLiteral("保存失败"), file.errorString());
        return;
    }

    QTextStream stream(&file);
    stream << "time_s,motor_id,target_speed_rad_s,actual_speed_rad_s,error_rad_s,"
              "actual_current_a,raw_current,command_current,encoder,speed_rpm,temperature_c,kp,ki,kd\n";

    const auto data = data_manager_->getMonitor()->getDataSnapshot();
    for (const auto &d : data) {
        stream << QString::number(d.timestamp, 'f', 6) << ','
               << d.motor_id << ','
               << QString::number(d.target_speed, 'f', 6) << ','
               << QString::number(d.actual_speed, 'f', 6) << ','
               << QString::number(d.error, 'f', 6) << ','
               << QString::number(d.actual_current, 'f', 6) << ','
               << d.raw_current << ','
               << d.command_current << ','
               << d.encoder << ','
               << d.speed_rpm << ','
               << static_cast<int>(d.temperature) << ','
               << QString::number(d.kp, 'f', 6) << ','
               << QString::number(d.ki, 'f', 6) << ','
               << QString::number(d.kd, 'f', 6) << '\n';
    }

    status_label_->setText(QStringLiteral("已保存: %1").arg(filename));
}

void MainWindow::onDataError(const QString &message) {
    status_label_->setText(message);
    QMessageBox::warning(this, QStringLiteral("运行错误"), message);
}
