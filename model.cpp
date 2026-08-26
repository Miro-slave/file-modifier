#include "model.h"

#include <QFileDialog>
#include <QThread>
#include <QDir>
#include <QDirIterator>

#include <istream>

Model::Model()
        : m_run_policy(RunPolicy::SingleRun)
        , m_duplicate_policy(DuplicatePolicy::AddNumber)
        , m_deleting_policy(DeletingPolicy::NotDelete)
        , m_modifier(-1)
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
    m_file_filters = file_filters;
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

void Model::processSingleFile(const QString& input_file_path, const QString& output_file_path) {
    QFile input_file(input_file_path);
    QFile output_file(output_file_path);

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

        // transform full 8 byte chunks
        for (std::size_t i = 0, len = 0; i + 7 <= byte_count; i += 8) {
            input_file.read(reinterpret_cast<char*>(&chunk), 8);

            chunk ^= m_modifier;

            output_file.write(reinterpret_cast<const char*>(&chunk), 8);

            QString str;
        }

        // transform remaining bytes
        std::size_t remaining_bytes_count = byte_count % 8;

        input_file.read(reinterpret_cast<char*>(&chunk), 8);

        chunk ^= m_modifier;

        output_file.write(reinterpret_cast<const char*>(&chunk), remaining_bytes_count);
    }

    if (m_deleting_policy == DeletingPolicy::Delete) {
        input_file.remove();
    }
}

void Model::work() {
    QDirIterator it(m_input_directory_path, m_file_filters,  QDir::Files);

    while (it.hasNext()) {
        it.next();

        QString input_file_path = m_input_directory_path + "\\" + it.fileName();
        QString output_file_path =  m_output_directory_path + "\\" + it.fileName();

        m_mutex.lock();

        while (m_paused) {
            m_wait_condition.wait(&m_mutex);
        }

        if (m_terminated) {
            m_paused = false;
            m_terminated = false;

            m_mutex.unlock();

            emit finished();

            return;
        }

        processSingleFile(input_file_path, output_file_path);

        m_mutex.unlock();
    }

    m_paused = false;
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

    m_terminated = true;

    m_wait_condition.wakeAll();
}
