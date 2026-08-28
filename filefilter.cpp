#include "filefilter.h"

void FileFilter::setRegexes(const QList<QString>& regex_lines) {
    for (const QString& regex_line : regex_lines) {
        m_regexes.emplaceBack(regex_line);
    }
}

QList<QString> FileFilter::filter(const QList<QString>& file_names) {
    if (m_regexes.empty()) {
        return file_names;
    }

    QList<QString> result;

    for (const QString& file_name : file_names) {
        for (const auto& regex : m_regexes) {
            if (regex.match(file_name).hasMatch() || file_name.endsWith(regex.pattern(), Qt::CaseInsensitive)) {
                result.push_back(file_name);

                break;
            }
        }
    }

    return result;
}
