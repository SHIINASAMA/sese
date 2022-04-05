#include "Util.h"
#include "thread/Locker.h"
#include "thread/Thread.h"

#define FILTER_TEST_LOCKER "fLOCKER"

std::mutex mutex;

void proc() {
    auto thread = sese::Thread::getCurrentThread();
    auto *num = static_cast<uint32_t *>(thread->getArgument());
    sese::Locker locker(mutex);
    //    mutex.lock();
    for (uint32_t idx = 0; idx < 5000; idx++) {
        (*num)++;
    }
    //    mutex.unlock();
    ROOT_INFO(FILTER_TEST_LOCKER, "num = %d", *num)
}

int main() {
    uint32_t num = 0;
    sese::Thread thread1(proc, "sub1");
    thread1.setArgument(&num);
    sese::Thread thread2(proc, "sub2");
    thread2.setArgument(&num);

    thread1.start();
    thread2.start();
    thread1.join();
    thread2.join();

    ROOT_INFO(FILTER_TEST_LOCKER, "main thread term")
    return 0;
}