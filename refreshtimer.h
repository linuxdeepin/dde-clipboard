#ifndef REFRESHTIMER_H
#define REFRESHTIMER_H
#include <QObject>
#include <QTimer>
#include <QDateTime>

/*!
 * \~chinese \class RefreshTimer
 * \~chinese \brief 每900ms刷新一次timeLabel上的时间信息
 */
class RefreshTimer : public QObject
{
    Q_OBJECT
public :
    static RefreshTimer *instance()
    {
        static RefreshTimer *timer = new RefreshTimer;
        return timer;
    }
private:
    RefreshTimer()
    {
        start();
    }

    void start()
    {
        timer.start(900);
        connect(&timer, &QTimer::timeout, [ = ] {
            if (QDateTime::currentDateTime().toString("hhmmss") == "000000")
            {
                Q_EMIT forceRefresh();
            }
        });
    }

Q_SIGNALS:
    void forceRefresh();

private:
    QTimer timer;
};

#endif // REFRESHTIMER_H
