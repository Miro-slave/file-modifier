#include "model.h"

#include <QFileDialog>
#include <QThread>
#include <QDir>
#include <QDirIterator>

#include <istream>

Model::Model()
        : m_run_policy(RunPolicy::SingleRun)
        , m_timer_duration(10000)
        , m_duplicate_policy(DuplicatePolicy::AddNumber)
        , m_deleting_policy(DeletingPolicy::NotDelete)
        , m_modifier(-1)
        , m_bytes_until_pause_check(1'000'000)
        , m_running(false)
        , m_paused(false)
        , m_terminated(false)
{}

void Model::setInputFolder(const QString& input_directory_path) {
    m_input_directory_path = input_directory_path;
}

void Model::setOutputFolder(const QString& output_directory_path) {
    m_output_directory_path = output_directory_path;
}

void Model::setFileFilters(const QList<QString>& file_filters) {
    m_file_filter.setRegexes(file_filters);
}

void Model::setRunPolicy(RunPolicy run_policy) {
    m_run_policy = run_policy;
}

void Model::setDuplicatePolicy(DuplicatePolicy duplicate_policy) {
    m_duplicate_policy = duplicate_policy;
}

void Model::setDeletingPolicy(DeletingPolicy deleting_policy) {
    m_deleting_policy = deleting_policy;
}

void Model::setModifier(quint64 modifier) {
    m_modifier = modifier;
}

void Model::setTimerDuration(unsigned long timer_duration) {
    m_timer_duration = timer_duration;
}

void Model::processSingleFile(const QString& input_file_path, const QString& output_file_path) {
    if (m_processed_file_paths.contains(input_file_path)) {
        return;
    }

    QFile input_file(input_file_path);
    QFile output_file(output_file_path);

    // change output file if needed
    if (m_duplicate_policy == DuplicatePolicy::AddNumber && QFileInfo::exists(output_file_path)) {
        std::size_t number = 1;
        QString new_otput_file_name;

        do {
            new_otput_file_name = QFileInfo(output_file_path).path() + "\\" +
                                  QFileInfo(output_file_path).baseName() +
                                  "_" + QString::number(number) +
                                  "." + QFileInfo(output_file_path).suffix();
            number++;
        } while(QFileInfo::exists(new_otput_file_name));

        output_file.setFileName(new_otput_file_name);
    }

    input_file.open(QIODevice::ReadOnly);
    output_file.open(QIODevice::WriteOnly);

    if (input_file.isOpen()) {
        std::size_t byte_count = input_file.size();

        quint64 chunk = 0;

        QByteArray byte_array;
        QByteArray byte_array2;

        // to check if model is paused
        quint64 pause_check_counter = 0;

        // transform full 8 byte chunks
        for (std::size_t i = 0, len = 0; i + 7 <= byte_count; i += 8) {
            // proccessing
            input_file.read(reinterpret_cast<char*>(&chunk), 8);

            chunk ^= m_modifier;

            output_file.write(reinterpret_cast<const char*>(&chunk), 8);

            QString str;

            // pause/termination check
            m_mutex.lock();

            while (m_paused) {
                m_wait_condition.wait(&m_mutex);
            }

            if (m_terminated) {
                m_mutex.unlock();

                output_file.remove();

                return;
            }

            m_mutex.unlock();
        }

        // transform remaining bytes
        std::size_t remaining_bytes_count = byte_count % 8;

        input_file.read(reinterpret_cast<char*>(&chunk), 8);

        chunk ^= m_modifier;

        output_file.write(reinterpret_cast<const char*>(&chunk), remaining_bytes_count);
    }

    if (m_deleting_policy == DeletingPolicy::Delete) {
        input_file.remove();
    } else {
        m_processed_file_paths.insert(input_file_path);
    }
}

void Model::processFiles() {
    m_running = true;

    QDir input_directory_directory(m_input_directory_path);
    QList<QString> file_names = input_directory_directory.entryList(QDir::Files);

    file_names = m_file_filter.filter(file_names);

    for (const QString& file_name : file_names) {
        QString input_file_path = m_input_directory_path + "\\" + file_name;
        QString output_file_path =  m_output_directory_path + "\\" + file_name;

        processSingleFile(input_file_path, output_file_path);

        if (m_terminated) {
            break;
        }
    }

    m_running = false;
    m_paused = false;
}

void Model::work() {
    m_terminated = false;
    m_running = false;
    m_paused = false;

    m_processed_file_paths.clear();

    if (m_run_policy == RunPolicy::SingleRun) {
        processFiles();
    } else if (m_run_policy == RunPolicy::WithTimer) {
        while (!m_terminated) {
            m_elapsed_timer.start();

            processFiles();

            unsigned long remaining_time = m_timer_duration - m_elapsed_timer.elapsed();

            if (remaining_time > 0 && !m_terminated) {
                while (remaining_time) {
                    unsigned long time_to_wait = qMin(remaining_time, 1000ul);
                    QThread::msleep(time_to_wait);
                    remaining_time -= time_to_wait;

                    if (m_terminated) {
                        break;
                    }
                }
            }
        }
    }

    m_terminated = false;

    emit finished();
}

void Model::pause() {
    QMutexLocker mutex_locker(&m_mutex);

    m_paused = true;

    m_wait_condition.wakeAll();
}

void Model::resume() {
    QMutexLocker mutex_locker(&m_mutex);

    m_paused = false;

    m_wait_condition.wakeAll();
}

void Model::stop() {
    QMutexLocker mutex_locker(&m_mutex);

    m_paused = false;
    m_terminated = true;

    m_wait_condition.wakeAll();
}
