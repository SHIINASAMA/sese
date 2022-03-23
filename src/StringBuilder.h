#pragma once
#include "AbstractStringBuffer.h"
#include "Config.h"

namespace sese {

    class API StringBuilder : public AbstractStringBuffer {
    public:
        explicit StringBuilder(size_t cap = STRING_BUFFER_SIZE_FACTOR) noexcept : AbstractStringBuffer(cap){
        }

        explicit StringBuilder(const char *str) noexcept : AbstractStringBuffer(str){
        }
    };
}// namespace sese