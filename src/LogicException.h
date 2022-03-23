#pragma once
#include "Exception.h"

namespace sese {

    class API LogicException : public Exception {
    public:
        explicit LogicException(const std::string &str = LOGIC_STRING) : Exception(str) {}
    };

}// namespace sese
