// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WLRDATACONTROLCLIPBOARDINTERFACE_H
#define WLRDATACONTROLCLIPBOARDINTERFACE_H

#include <QtConcurrent>
#include <QMimeData>
#include <QThreadPool>

#include <atomic>
#include <memory>

#include "wlrdatacontrolmanagerintegration.h"
#include "wlrdatacontroldeviceintegration.h"
#include "wlrdatacontrolsourceintegration.h"

struct WaylandMimePayload {
    QString mimeType;
    QByteArray data;
};

struct WaylandMimeReadResult {
    quint64 readTaskId = 0;
    QList<WaylandMimePayload> payloads;
};

class WlrDataControlClipboardInterface : public QObject
{
    Q_OBJECT
public:
    explicit WlrDataControlClipboardInterface(QObject *parent = nullptr);

    // R/W interfaces
    const QMimeData *mimeData() const;
    void setMimeData(QMimeData *mimeData);

protected:
    bool managerReady() { return m_dcManager && m_dcManager->isActive(); }
    void refreshDataControlSourceDevice();

    void takeoverClipboardDataSource();

    quint64 beginReadTask();
    void cancelPendingRead();
    void requestReadTaskCancel();
    bool isCurrentReadTask(quint64 readTaskId) const;
    void completeRead(std::unique_ptr<QMimeData> mimeData);

protected slots:
    void onActiveChanged() { refreshDataControlSourceDevice(); }
    void onDataControlDeviceFinished();

    // Read clipboard, Stage 2
    void onNewSelection(WlrDataControlOfferIntegration *offer);
    // Read clipboard, read task finished
    void onReadTaskFinished();

    // Write clipboard, Stage 2
    void onSourceSend(QString mimeType, int fd);
    // Write clipboard (abort signal from compositor)
    void onSourceCancelled();

signals:
    // Notifies daemon the clipboard content changed
    void dataChanged();

private:
    // Wayland objects
    std::unique_ptr<WlrDataControlManagerIntegration> m_dcManager;
    std::unique_ptr<WlrDataControlSourceIntegration> m_dcSource;
    std::unique_ptr<WlrDataControlDeviceIntegration> m_dcDevice;

    // Current valid read task id. Finished tasks with older ids are ignored.
    quint64 m_activeReadTaskId = 0;
    std::shared_ptr<std::atomic_bool> m_readCancelFlag;
    QFuture<WaylandMimeReadResult> m_readTaskFuture;
    QFutureWatcher<WaylandMimeReadResult> m_readTaskWatcher;

    // Keep reader and writer off the global thread pool.
    QThreadPool m_readThreadPool;
    QThreadPool m_writeThreadPool;

    // Clipboard content read from / written to clipboard
    std::unique_ptr<QMimeData> m_mimeData;
};

#endif // WLRDATACONTROLCLIPBOARDINTERFACE_H
