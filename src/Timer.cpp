#include <sese/Timer.h>
#include <sese/Util.h>
#include <sese/thread/Locker.h>

using sese::Locker;
using sese::Thread;
using sese::Timer;

Timer::Timer() {
    thread = std::make_shared<Thread>([this] { loop(); }, "Timer");
    thread->start();
}

Timer::~Timer() {
    if(!isShutdown) {
        shutdown();
    }
}

void Timer::delay(std::function<void()> callback, uint8_t sec, uint8_t min, uint8_t hour, bool isRepeat) noexcept {
    Task task;
    task.sec = sec;
    task.min = min;
    task.hour = hour;
    makeTask(task, sec, min, hour);
    task.callback = std::move(callback);
    task.isRepeat = isRepeat;

    Locker locker(mutex);
    timingTasks[task.tSec].emplace_back(task);
}

void Timer::shutdown() {
    isShutdown = true;
    thread->join();
}

void Timer::execute(const std::function<void()> &taskCallback) {
    taskCallback();
}

void Timer::makeTask(Task &task, uint8_t sec, uint8_t min, uint8_t hour) {
    task.tSec = sec + currentSec;
    task.tMin = (min + currentMin) + task.tSec / 60;
    task.tHour = (hour + currentHour) + task.tMin / 60;
    task.tSec %= 60;
    task.tMin %= 60;
}
void Timer::loop() {
    while (!isShutdown) {
        sese::sleep(1);

        // 检查任务
        auto &currentTimingTask = timingTasks[currentSec];
        mutex.lock();
        if (!currentTimingTask.empty()) {
            for (auto itor = currentTimingTask.begin();
                 itor != currentTimingTask.end();) {
                if (itor->tHour == currentHour.load() && itor->tMin == currentMin.load()) {
                    execute(itor->callback);
                    if(itor->isRepeat) {
                        // 重复任务再次添加
                        auto task = *itor;
                        makeTask(task, task.sec, task.min, task.hour);
                        timingTasks[task.tSec].emplace_back(task);
                    }
                    itor = currentTimingTask.erase(itor);
                }
            }
        }
        mutex.unlock();

        // 更新时间
        currentSec == 59 ? currentSec = 0, currentMin++ : currentSec++;
        if(currentMin == 59) {
            currentMin = 0;
            currentHour++;
        }
    }
}