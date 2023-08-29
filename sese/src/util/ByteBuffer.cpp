#include "sese/util/ByteBuffer.h"
#include "sese/thread/Locker.h"

// GCOVR_EXCL_START

int64_t sese::ByteBuffer::read(void *buffer, size_t len) {
    Locker locker(mutex);
    return AbstractByteBuffer::read(buffer, len);
}

int64_t sese::ByteBuffer::write(const void *buffer, size_t needWrite) {
    Locker locker(mutex);
    return AbstractByteBuffer::write(buffer, needWrite);
}

int64_t sese::ByteBuffer::peek(void *buffer, size_t len) {
    Locker locker(mutex);
    return AbstractByteBuffer::peek(buffer, len);
}

sese::ByteBuffer::ByteBuffer(size_t baseSize, size_t factor) : AbstractByteBuffer(baseSize, factor) {
}

void sese::ByteBuffer::resetPos() {
    Locker locker(mutex);
    AbstractByteBuffer::resetPos();
}

size_t sese::ByteBuffer::getLength() {
    Locker locker(mutex);
    return AbstractByteBuffer::getLength();
}

size_t sese::ByteBuffer::getCapacity() {
    Locker locker(mutex);
    return AbstractByteBuffer::getCapacity();
}

size_t sese::ByteBuffer::freeCapacity() {
    Locker locker(mutex);
    return AbstractByteBuffer::freeCapacity();
}

// size_t sese::ByteBuffer::getCurrentReadPos() {
//     Locker locker(mutex);
//     return AbstractByteBuffer::getCurrentReadPos();
// }
//
// size_t sese::ByteBuffer::getCurrentWritePos() {
//     Locker locker(mutex);
//     return AbstractByteBuffer::getCurrentWritePos();
// }

int64_t sese::ByteBuffer::trunc(size_t needRead) {
    Locker locker(mutex);
    return AbstractByteBuffer::trunc(needRead);
}

// GCOVR_EXCL_STOP
