#include "filemodifier.h"

#include <QFile>
#include <QFileDialog>
#include <QFileInfo>

void FileModifier::setModifier(quint64 modifier) {
    m_modifier = modifier;
    m_duplicate_policy = DuplicatePolicy::AddNumber;
    m_deleting_policy = DeletingPolicy::NotDelete;
}

void FileModifier::setDuplicatePolicy(DuplicatePolicy duplicate_policy) {
    m_duplicate_policy = duplicate_policy;
}

void FileModifier::setDeletingPolicy(DeletingPolicy deleting_policy) {
    m_deleting_policy = deleting_policy;
}

void FileModifier::modify(const QString& input_file_path, const QString& output_file_path) {
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
