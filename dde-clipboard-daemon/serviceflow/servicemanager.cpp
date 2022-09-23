// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "servicemanager.h"
#include "servicetool.h"

#include <QThreadPool>

// ### Hidden for using
class ServiceFlow
{
public:
    ServiceFlow(CommandService *service);
    ~ServiceFlow();

    QList<CommandService *> services() const;
    void start();
    void appendFlowMessage(CommandMessage *msg);
    bool isStarted() const;

private:
    void initServiceCommands();

    void setupFlow();
    void setupHeaderFlow();

private:
    CommandService *m_root;
    Transporter *m_rootTrans;
    QList<Transporter *> m_transporters;
    QList<Extractor *> m_extractors;
    QMap<CommandService *, QList<Command *>> m_serviceCommands;
    QThreadPool *m_threadPool;
};

ServiceFlow::ServiceFlow(CommandService *service)
    : m_root(service)
    , m_rootTrans(new Transporter)
    , m_threadPool(new QThreadPool)
{
    initServiceCommands();
    setupFlow();
}

ServiceFlow::~ServiceFlow()
{
    for (auto machines : m_serviceCommands) {
        for (auto machine : qAsConst(machines)) {
            machine->exit();
        }
    }

    m_rootTrans->release();
    for (auto trans : m_transporters)
        trans->release();

    m_threadPool->waitForDone();

    delete m_rootTrans;
    qDeleteAll(m_transporters);
    qDeleteAll(m_extractors);
    delete m_threadPool;

}

QList<CommandService *> ServiceFlow::services() const
{
    return m_serviceCommands.keys();
}

void ServiceFlow::start()
{
    QList<CommandService *> stores = { m_root };
    CommandService *header = nullptr;
    int priority = 0;
    while (!stores.isEmpty()) {
        int n = stores.size();
        for (int i = 0; i < n; ++i) {
            header = stores.takeFirst();
            for (auto machine : m_serviceCommands.value(header)) {
                if (machine->state() != Command::Preparing)
                    continue;
                m_threadPool->start(machine, priority);
            }
            if (auto nextService = header->nextService())
                stores.append(nextService);
        }
        priority++;
    }

}

void ServiceFlow::appendFlowMessage(CommandMessage *msg)
{
    m_rootTrans->putMessage(msg);
}

bool ServiceFlow::isStarted() const
{
    return m_threadPool->activeThreadCount() > 0;
}

void ServiceFlow::initServiceCommands()
{
    m_serviceCommands.clear();
    QList<CommandService *> headers = { m_root };
    CommandService *header = nullptr;
    while (!headers.isEmpty()) {
        header = headers.takeFirst();
        do {
            if (m_serviceCommands.contains(header))
                break;
            m_serviceCommands.insert(header, header->commands());
        } while (false);

        if (auto nextService = header->nextService())
            headers.append(nextService);
    }

    // 根据服务中命令的个数，动态创建线程池的数量
    int accu = 0;
    for (auto ms : m_serviceCommands.values())
        accu += ms.count();
    m_threadPool->setMaxThreadCount(accu);
}

void ServiceFlow::setupFlow()
{
    setupHeaderFlow();
    QList<CommandService*> headers = { m_root };
    CommandService *header = nullptr;

    while (!headers.isEmpty()) {
        header = headers.takeFirst();
        auto next = header->nextService();
        if (!next)
            continue;
        Transporter *inTrans = new Transporter();
        m_transporters.append(inTrans);

        Extractor *exOnIn = new Extractor(inTrans);
        exOnIn->setDirection(Extractor::CommandToTransporter);
        m_extractors.append(exOnIn);

        for (auto machine : m_serviceCommands.value(header))
            machine->installMessageExtractor(exOnIn);

        Transporter *trans = inTrans;
        Extractor *exOnOut = new Extractor(trans);
        exOnOut->setDirection(Extractor::TransporterToCommand);
        m_extractors.append(exOnOut);

        for (auto machine : m_serviceCommands.value(next))
            machine->installMessageExtractor(exOnOut);

        headers.append(next);
    }
}

void ServiceFlow::setupHeaderFlow()
{
    Extractor *rootExtra = new Extractor(m_rootTrans, Extractor::TransporterToCommand);
    m_extractors.append(rootExtra);
    for (auto machine : m_serviceCommands.value(m_root))
        machine->installMessageExtractor(rootExtra);
}


CommandServiceManager::CommandServiceManager()
    : m_maxServiceCount(0)
{

}

CommandServiceManager::~CommandServiceManager()
{
    for (auto service: m_rootServices.keys())
        unregisterService(service);
}

void CommandServiceManager::registerService(CommandService *root)
{
    Q_ASSERT(root);
    if (isServiceRegistered(root)) {
        qWarning("Already register this service.");
        return;
    }

    ServiceFlow *service = new ServiceFlow(root);
    m_rootServices.insert(root, service);
}

void CommandServiceManager::unregisterService(CommandService *root)
{
    if (!m_rootServices.contains(root))
        return;
    auto const &flow = m_rootServices.take(root);
    delete flow;

    if (m_waitingServices.isEmpty())
        return;

    auto service = m_waitingServices.takeFirst();
    start(service);
}

bool CommandServiceManager::isServiceRegistered(CommandService *service)
{
    if (m_rootServices.contains(service))
        return true;

    for (auto root : m_rootServices) {
        auto const &services = root->services();
        bool contains = std::any_of(services.cbegin(), services.cend(),
                                    [=](const CommandService *s) {
            return service == s;
        });

        if (!contains)
            continue;
        return contains;
    }
    return false;
}

int CommandServiceManager::registeredServicesCount() const
{
    return m_rootServices.count();
}

int CommandServiceManager::activeServiceFlowsCount() const
{
    return std::accumulate(m_rootServices.cbegin(), m_rootServices.cend(), 0,
                           [](int prev, ServiceFlow *flow) {
        if (flow->isStarted()) {
            prev++;
        }

        return prev;
    });
}

void CommandServiceManager::setMaxServiceCount(int maxCount)
{
    m_maxServiceCount = maxCount;
}

int CommandServiceManager::maxServiceCount() const
{
    return m_maxServiceCount;
}

void CommandServiceManager::start(CommandService *service)
{
    if (activeServiceFlowsCount() > m_maxServiceCount && m_maxServiceCount != 0) {
        if (!m_waitingServices.contains(service))
            m_waitingServices.append(service);
        return;
    }

    if (CommandService *root = findRootService(service)) {
        ServiceFlow *flow = m_rootServices.value(root);
        if (!flow->isStarted())
            flow->start();
    }
}

void CommandServiceManager::appendFlowMessage(CommandMessage *message, CommandService *service)
{
    // 只能从 header 中输入消息流
    Q_ASSERT(message && service);

    if (!isServiceRegistered(service)) {
        qWarning("Regiseter this service first.");
        return;
    }

    CommandService *root = findRootService(service);
    if (!root)
        return;

    ServiceFlow *flow = m_rootServices.value(root);
    flow->appendFlowMessage(message);
}

CommandService *CommandServiceManager::findRootService(CommandService *service)
{
    for (auto root : m_rootServices) {
        auto const &services = root->services();
        auto const &it = std::find_if(services.begin(), services.end(),
                                      [=](const CommandService *s) {
            return service == s;
        });

        if (it != services.end())
            return m_rootServices.key(root);
    }

    return nullptr;
}
