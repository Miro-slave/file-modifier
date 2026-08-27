#ifndef FILEFILTER_H
#define FILEFILTER_H

#include <QList>
#include <QRegularExpression>

class FileFilter
{
public:
    FileFilter() = default;
    void setRegexes(const QList<QString>& regex_lines);
    QList<QString> filter(const QList<QString>& file_names);
private:
    QList<QRegularExpression> m_regexes;
};

#endif // FILEFILTER_H
