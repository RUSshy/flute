//
// Created by why on 2019/12/30.
//

#include <flute/EventLoop.h>
#include <flute/EventLoopGroup.h>
#include <flute/Logger.h>

#include <memory>

namespace flute {

EventLoopGroup::EventLoopGroup(std::size_t childLoopSize)
    : m_childLoopSize(childLoopSize), m_masterEventLoop(new EventLoop()), m_slaveEventLoops(), m_threadPool() {
    m_threadPool.start(childLoopSize);
    m_slaveEventLoops.reserve(childLoopSize);
    for (std::size_t i = 0; i < childLoopSize; ++i) {
        std::promise<EventLoop*> p;
        m_threadPool.execute([&p] {
            EventLoop loop;
            p.set_value(&loop);
            loop.dispatch();
        });
        auto loop = p.get_future().get();
        m_slaveEventLoops.emplace_back(loop);
    }
}

EventLoopGroup::~EventLoopGroup() {
    if (m_masterEventLoop) {
        delete m_masterEventLoop;
        m_masterEventLoop = nullptr;
    }
}

EventLoop* EventLoopGroup::chooseSlaveEventLoop(std::uint64_t hash) {
    auto size = static_cast<std::int64_t>(m_slaveEventLoops.size());
    if (size <= 0) {
        return m_masterEventLoop;
    }
    if ((size & -size) == size) {
        return m_slaveEventLoops[hash & (size - 1)];
    } else {
        return m_slaveEventLoops[hash % size];
    }
}

EventLoop* EventLoopGroup::getMasterEventLoop() { return m_masterEventLoop; }

void EventLoopGroup::dispatch() { m_masterEventLoop->dispatch(); }

void EventLoopGroup::shutdown() {
    for (auto& loop : m_slaveEventLoops) {
        loop->quit();
    }
    m_threadPool.shutdown();
    m_masterEventLoop->quit();
}

} // namespace flute