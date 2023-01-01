#include "sese/system/LibraryLoader.h"
#include "sese/record/LogHelper.h"
#include "sese/util/Test.h"

using sese::record::LogHelper;
using sese::LibraryLoader;
using sese::LibraryObject;
using Func = double(double);

sese::record::LogHelper helper("fLIB_LOADER"); // NOLINT

int main() {
#ifdef _WIN32
    const char *libName = "NTDLL.DLL";
#elif __linux__
    const char *libName = "libstdc++.so.6";
#elif __APPLE__
    const char *libName = "libstdc++.6.dylib";
#endif
    helper.info("loading lib \"%s\"", libName);
    auto object = LibraryLoader::open(libName);
    assert(helper, object != nullptr, -1);
    auto sin = (Func *)object->findFunctionByName("sin");
    assert(helper, sin != nullptr, -2);

    auto a = sin(1.0f);
    auto b = sin(0.0f);
    helper.info("sin(1.0f) = %f", a);
    helper.info("sin(0.0f) = %f", b);

    return 0;
}