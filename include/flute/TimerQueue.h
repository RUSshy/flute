/*************************************************************************
 *
 * File Name:  TimerQueue.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/29
 *
 *************************************************************************/

#pragma once
#include <flute/noncopyable.h>
#include <flute/socket_types.h>

#include <cstdint>
#include <functional>
#include <map>
#include <vector>

namespace flute {

class Timer;
class EventLoop;

class TimerQueue : private noncopyable {
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    std::uint64_t schedule(std::function<void()>&& callback, std::int64_t delay, int loopCount);
    std::uint64_t schedule(const std::function<void()>& callback, std::int64_t delay, int loopCount);
    void cancel(std::uint64_t timerId);
    std::int64_t searchNearestTime();
    void handleTimerEvent();

private:
    EventLoop* m_loop;
    std::vector<Timer*> m_timerQueue;
    std::map<std::int64_t, Timer*> m_timerMap;

    void postTimerInLoop(Timer* timer);
    void cancelTimerInLoop(std::uint64_t timerId);
};

} // namespace flute