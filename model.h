#ifndef MODEL_H
#define MODEL_H

#include "filefilter.h"

#include <QMutex>
#include <QString>
#include <QElapsedTimer>
#include <QSet>
#include <QWaitCondition>

class Model : public QObject
{
    Q_OBJECT
public:
    // corresponds policy towards working mode
    enum class RunPolicy {
        SingleRun = 0,
        WithTimer,
    };

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

    Model();
    void setInputFolder(const QString& input_directory_path);
    void setOutputFolder(const QString& output_directory_path);
    void setFileFilters(const QList<QString>& file_filters);
    void setRunPolicy(RunPolicy run_policy);
    void setDuplicatePolicy(DuplicatePolicy duplicate_policy);
    void setDeletingPolicy(DeletingPolicy deleting_policy);
    void setModifier(quint64 modifier);
    void setTimerDuration(unsigned long timer_duration);
    void setBytesUntilPauseCheck(quint64 bytes_until_pause_check);
    quint64 totalBytesToProcess();

private:
    void processSingleFile(const QString& input_file_path, const QString& output_file_path);

public slots:
    void work();
    void pause();
    void resume();
    void stop();
    void processFiles();
    quint64 processedBytes();

signals:
    void progressUpdate();
    void finished();

private:
    QString m_input_directory_path;
    QString m_output_directory_path;

    QSet<QString> m_processed_file_paths;

    FileFilter m_file_filter;
    QList<QString> m_file_filters;

    RunPolicy m_run_policy;

    QElapsedTimer m_elapsed_timer;
    unsigned long m_timer_duration;

    quint64 m_processed_bytes;
    quint64 m_bytes_until_pause_check;

    // for binary operation
    quint64 m_modifier;

    DuplicatePolicy m_duplicate_policy;
    DeletingPolicy m_deleting_policy;

    QMutex m_mutex;
    QWaitCondition m_wait_condition;

    bool m_running;
    bool m_paused;
    bool m_terminated;
};

#endif // MODEL_H
