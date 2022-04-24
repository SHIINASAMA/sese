#include "sese/net/Socket.h"
#include "sese/Test.h"
#include "sese/record/Logger.h"
#include "sese/system/CpuInfo.h"

using sese::CpuInfoInitiateTask;
using sese::Initializer;
using sese::InitiateTask;
using sese::LoggerInitiateTask;
using sese::TestInitiateTask;

static Initializer Initializer;// NOLINT

InitiateTask::InitiateTask(std::string name) : name(std::move(name)) {
}

const std::string &sese::InitiateTask::getName() const {
    return name;
}

Initializer::Initializer() {
    loadTask(std::make_shared<LoggerInitiateTask>());
    loadTask(std::make_shared<TestInitiateTask>());
    loadTask(std::make_shared<CpuInfoInitiateTask>());
#ifdef _WIN32
    loadTask(std::make_shared<sese::SocketInitiateTask>());
#endif
}

Initializer::~Initializer() {
    /// 保证初始化器按顺序销毁
    InitiateTask::Ptr top = nullptr;
    while (!tasks.empty()) {
        top = tasks.top();
        unloadTask(top);
    }
}

void Initializer::loadTask(InitiateTask::Ptr &&task) noexcept {
    auto rt = task->init();
    if (rt != 0) {
        printf("Load failed: %32s Exit code %d", task->getName().c_str(), rt);
    } else {
        tasks.emplace(task);
    }
}

void Initializer::unloadTask(InitiateTask::Ptr &task) noexcept {
    auto rt = task->destroy();
    if (rt != 0) {
        printf("Unload failed: %32s Exit code %d", task->getName().c_str(), rt);
    }
    tasks.pop();
}
