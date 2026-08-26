#ifndef FILEMODIFIER_H
#define FILEMODIFIER_H

#include <QString>
#include <QThread>

class FileModifier
{
public:
    // corresponds policy towards output files with identical names
    enum class DuplicatePolicy {
        Rewrite = 0,
        AddNumber,
    };

    // corresponds policy towards output files with identical names
    enum class DeletingPolicy {
        NotDelete = 0,
        Delete
    };

    FileModifier() = default;
    void setModifier(quint64 modifier);
    void setDuplicatePolicy(DuplicatePolicy duplicate_policy);
    void setDeletingPolicy(DeletingPolicy deleting_policy);
    void modify(const QString& input_file_path, const QString& output_file_path);
private:
    quint64 m_modifier;
    DuplicatePolicy m_duplicate_policy;
    DeletingPolicy m_deleting_policy;
};

#endif // FILEMODIFIER_H
