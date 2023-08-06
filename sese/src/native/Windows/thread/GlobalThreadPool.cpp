#include <sese/thread/GlobalThreadPool.h>

DWORD WINAPI sese::GlobalThreadPool::taskRunner1(LPVOID lpParam) {
    auto task1 = (Task1 *) lpParam;
    task1->function();
    delete task1;
    return 0;
}

void sese::GlobalThreadPool::postTask(const std::function<void()> &func) {
    auto task1 = new Task1;
    task1->function = func;
    QueueUserWorkItem(taskRunner1, task1, WT_EXECUTEDEFAULT);
}
