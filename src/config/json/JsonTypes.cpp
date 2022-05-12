#include <sese/config/json/JsonTypes.h>

using namespace sese::json;

Data::Data(DataType type) : type(type) {}

BasicData::BasicData() noexcept
    : Data(DataType::Basic),
      _isNull(true) {}

BasicData::BasicData(std::string text) noexcept
    : Data(DataType::Basic),
      data(std::move(text)),
      _isNull(false) {}

void BasicData::setNull() noexcept {
    _isNull = true;
}

void BasicData::setNotNull(std::string newData) noexcept {
    data = std::move(newData);
    _isNull = false;
}

ObjectData::ObjectData() : Data(DataType::Object) {}

void ObjectData::set(const std::string &key, const Data::Ptr &data) {
    keyValueSet[key] = data;
}

Data::Ptr ObjectData::get(const std::string &key) const noexcept {
    auto res = keyValueSet.find(key);
    if(res == keyValueSet.end()) {
        return nullptr;
    } else {
        return res->second;
    }
}

ArrayData::ArrayData() : Data(DataType::Array) {}