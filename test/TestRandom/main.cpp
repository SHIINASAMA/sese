#include "system/CpuInfo.h"
#include "util/Random.h"

using sese::CpuInfo;
using sese::Random;

int main() {
    for (int i = 0; i < 20; i++) {
        printf("%lld", Random::next());
    }
    return 0;
}