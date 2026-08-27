#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_model = new Model();
    m_model->setTimerDuration(ui->timer_time_edit->time().msecsSinceStartOfDay());

    m_working_thread = new QThread();

    m_model->moveToThread(m_working_thread);

    connect(m_working_thread, &QThread::started, m_model, &Model::work);
    connect(m_model, &Model::finished, m_working_thread, &QThread::quit);
    // connect(m_working_thread, &QThread::finished, m_working_thread, &QThread::deleteLater);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_working_thread->isRunning()) {
        m_model->stop();
        m_working_thread->quit();

        if (!m_working_thread->wait(3000)) {
            m_working_thread->terminate();
            m_working_thread->wait(3000);
        }

        event->accept();
    }
}

MainWindow::~MainWindow()
{
    if (m_working_thread->isRunning()) {
        m_model->stop();
        m_working_thread->quit();

        if (!m_working_thread->wait(3000)) {
            m_working_thread->terminate();
            m_working_thread->wait(3000);
        }
    }

    delete m_model;
    delete m_working_thread;
    delete ui;
}

void MainWindow::on_input_button_clicked()
{
    ui->options_stacked_widget->setCurrentIndex(0);
}


void MainWindow::on_output_button_clicked()
{
    ui->options_stacked_widget->setCurrentIndex(1);
}


void MainWindow::on_working_mode_button_clicked()
{
    ui->options_stacked_widget->setCurrentIndex(2);
}

void MainWindow::on_proccessing_button_clicked()
{
    ui->options_stacked_widget->setCurrentIndex(3);
}


void MainWindow::on_choose_input_folder_button_clicked()
{
    QFileDialog file_dialog(this, QString("Выберите папку"), m_dir.absolutePath());
    QString input_folder_name = file_dialog.getExistingDirectory();

    m_model->setInputFolder(input_folder_name);
    ui->input_folder_line_edit->setText(input_folder_name);
    ui->input_folder_line_edit->home(false);
}


void MainWindow::on_apply_masks_button_clicked()
{
    QList<QString> file_filters = ui->plainTextEdit->toPlainText().split('\n');
    m_model->setFileFilters(file_filters);
}


void MainWindow::on_choose_output_folder_button_clicked()
{
    QFileDialog file_dialog(this, QString("Выберите папку"), m_dir.absolutePath());
    QString output_folder_name = file_dialog.getExistingDirectory();

    m_model->setOutputFolder(output_folder_name);
    ui->output_folder_line_edit->setText(output_folder_name);
    ui->output_folder_line_edit->home(false);
}


void MainWindow::on_start_button_clicked()
{
    if (!m_working_thread->isRunning()) {
        m_working_thread->start();
    }
}

void MainWindow::on_rewrite_radio_button_clicked()
{
    m_model->setDuplicatePolicy(Model::DuplicatePolicy::Rewrite);
}


void MainWindow::on_add_number_radio_button_clicked()
{
    m_model->setDuplicatePolicy(Model::DuplicatePolicy::AddNumber);
}


void MainWindow::on_delete_input_files_radio_button_clicked()
{
    if (ui->delete_input_files_radio_button->isChecked()) {
        m_model->setDeletingPolicy(Model::DeletingPolicy::Delete);
    } else {
        m_model->setDeletingPolicy(Model::DeletingPolicy::NotDelete);
    }
}


void MainWindow::on_timer_check_box_clicked()
{
    if (ui->timer_check_box->isChecked()) {
        ui->set_time_label->setEnabled(true);
        ui->timer_time_edit->setEnabled(true);
    } else {
        ui->set_time_label->setDisabled(true);
        ui->timer_time_edit->setDisabled(true);
    }
}


void MainWindow::on_pause_button_clicked()
{
    if (m_working_thread->isRunning()) {
        m_model->pause();
    }
}

void MainWindow::on_resume_button_clicked()
{
    if (m_working_thread->isRunning()) {
        m_model->resume();
    }
}


void MainWindow::on_stop_button_clicked()
{
    if (m_working_thread->isRunning()) {
        m_model->stop();
    }
}


void MainWindow::on_timer_check_box_checkStateChanged(const Qt::CheckState &arg1)
{
    if (arg1 == Qt::CheckState::Checked) {
        m_model->setRunPolicy(Model::RunPolicy::WithTimer);
    } else {
        m_model->setRunPolicy(Model::RunPolicy::SingleRun);
    }
}


void MainWindow::on_timer_time_edit_userTimeChanged(const QTime &time)
{
    m_model->setTimerDuration(ui->timer_time_edit->time().msecsSinceStartOfDay());
}

