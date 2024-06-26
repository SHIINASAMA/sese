#include "sese/system/LibraryLoader.h"

using sese::system::LibraryObject;

LibraryObject::Ptr LibraryObject::create(const std::string &name) noexcept {
    auto handle = LoadLibraryA(name.c_str());
    if (handle == nullptr) {
        return nullptr;
    } else {
        return MAKE_SHARED_PRIVATE(LibraryObject, handle);
    }
}

LibraryObject::Ptr LibraryObject::createWithPath(const system::Path &path) noexcept {
    return LibraryObject::create(path.getNativePath());
}

LibraryObject::LibraryObject(Module module) noexcept {
    this->module = module;
}

const void *LibraryObject::findFunctionByName(const std::string &name) const {
    return reinterpret_cast<const void *>(GetProcAddress(module, name.c_str()));
}

LibraryObject::~LibraryObject() noexcept {
    FreeLibrary(module);
}