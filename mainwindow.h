#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "model.h"

#include <QMainWindow>
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void on_input_button_clicked();

    void on_output_button_clicked();

    void on_working_mode_button_clicked();

    void on_proccessing_button_clicked();

    void on_choose_input_folder_button_clicked();

    void on_apply_masks_button_clicked();

    void on_choose_output_folder_button_clicked();

    void on_start_button_clicked();

    void on_rewrite_radio_button_clicked();

    void on_add_number_radio_button_clicked();

    void on_delete_input_files_radio_button_clicked();

    void on_timer_check_box_clicked();

    void on_pause_button_clicked();

    void on_resume_button_clicked();

    void on_stop_button_clicked();

    void on_timer_check_box_checkStateChanged(const Qt::CheckState &arg1);

    void on_timer_time_edit_userTimeChanged(const QTime &time);

private:
    QDir m_dir;
    Ui::MainWindow *ui;
    Model* m_model;
    QThread* m_working_thread;
};
#endif // MAINWINDOW_H
