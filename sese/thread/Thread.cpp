// Copyright 2024 libsese
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <utility>

#include "sese/thread/Thread.h"
#include "sese/Config.h"

#ifdef __linux__
static sese::pid_t getTid() noexcept {
    return syscall(__NR_gettid); // NOLINT
}
#endif

#ifdef _WIN32
static sese::tid_t getTid() noexcept {
    return GetCurrentThreadId(); // NOLINT
}
#endif

#ifdef __APPLE__
#include <unistd.h>
#include <pthread/pthread.h>
static sese::tid_t getTid() noexcept {
    uint64_t id;
    pthread_threadid_np(nullptr, &id);
    return id;
}
#endif

namespace sese {

thread_local std::shared_ptr<sese::Thread::RuntimeData> current_data = nullptr;

tid_t Thread::main_id = 0;

int32_t ThreadInitiateTask::init() noexcept {
    Thread::setCurrentThreadAsMain();
    return 0;
}

int32_t ThreadInitiateTask::destroy() noexcept {
    return 0;
}

tid_t Thread::getCurrentThreadId() noexcept {
    return current_data ? current_data->id : ::getTid();
}

const char *Thread::getCurrentThreadName() noexcept {
    auto tid = ::getTid();
    if (tid == Thread::main_id) {
        return THREAD_MAIN_NAME;
    }
    return current_data ? current_data->name.c_str() : THREAD_DEFAULT_NAME;
}

void Thread::setCurrentThreadAsMain() noexcept {
    if (Thread::main_id == 0) {
        Thread::main_id = ::getTid();
    }
}

tid_t Thread::getMainThreadId() noexcept {
    return Thread::main_id;
}

Thread::RuntimeData *Thread::getCurrentThreadData() noexcept {
    return current_data.get();
}

Thread::Thread(const std::function<void()> &function, const std::string &name) {
    this->data = std::make_shared<Thread::RuntimeData>(); // GCOVR_EXCL_LINE
    this->data->name = name;                              // GCOVR_EXCL_LINE
    this->data->function = function;                      // GCOVR_EXCL_LINE
}

Thread::Thread(Thread &&thread) noexcept {
    this->data = std::move(thread.data); // GCOVR_EXCL_LINE
}

Thread::~Thread() {
    if (this->data->th.joinable()) {
        this->data->th.join();
    }
}

void Thread::start() const {
    // clang-format off
    this->data->th = std::thread([this] { run(this->data); }); // GCOVR_EXCL_LINE
    // clang-format on
}

void Thread::join() const {
    this->data->th.join();
}

void Thread::run(std::shared_ptr<RuntimeData> data) {
    current_data = std::move(data);
    current_data->id = ::getTid();

    current_data->function();
}

bool Thread::joinable() const {
    return this->data->th.joinable();
}

void Thread::detach() const {
    this->data->th.detach();
}

Thread &Thread::operator=(Thread &&thread) noexcept {
    this->data = std::move(thread.data);
    return *this;
}


} // namespace sese