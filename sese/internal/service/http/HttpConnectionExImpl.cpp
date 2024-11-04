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

#include <sese/Config.h>
#include <sese/internal/service/http/HttpConnectionEx.h>

sese::internal::service::http::HttpConnectionExImpl::HttpConnectionExImpl(
        const std::shared_ptr<HttpServiceImpl> &service,
        asio::io_context &context,
        const sese::net::IPAddress::Ptr &addr,
        SharedSocket socket
)
    : HttpConnectionEx(service, context, addr),
      socket(std::move(socket)) {
}

void sese::internal::service::http::HttpConnectionExImpl::writeBlocks(const std::vector<asio::const_buffer> &buffers, const std::function<void(const asio::error_code &code)> &callback) {
    is_write = true;
    async_write(*this->socket, buffers, [conn = getPtr(), callback](const asio::error_code &error, size_t) {
        conn->is_write = false;
        callback(error);
    });
}

void sese::internal::service::http::HttpConnectionExImpl::writeBlock(const void *buffer, size_t size, const std::function<void(const asio::error_code &code)> &callback) {
    is_write = true;
    async_write(*this->socket, asio::buffer(buffer, size), [conn = getPtr(), callback](const asio::error_code &error, size_t) {
        conn->is_write = false;
        callback(error);
    });
}


void sese::internal::service::http::HttpConnectionExImpl::readBlock(char *buffer, size_t length, const std::function<void(const asio::error_code &code)> &callback) {
    is_read = true;
    async_read(
            *this->socket,
            asio::buffer(buffer, length),
            asio::transfer_at_least(1),
            [conn = shared_from_this(), buffer, length, callback](const asio::error_code &error, std::size_t read) {
                conn->is_read = false;
                if (error || read == length) {
                    callback(error);
                } else {
                    conn->readBlock(buffer + read, length - read, callback);
                }
            }
    );
}

void sese::internal::service::http::HttpConnectionExImpl::checkKeepalive() {
    // if (keepalive) {
    timer.async_wait([conn = getPtr()](const asio::error_code &error) {
        if (error == asio::error::operation_aborted) {
        } else {
            conn->socket->close();
        }
    });
    // }
}

sese::internal::service::http::HttpsConnectionExImpl::HttpsConnectionExImpl(
        const std::shared_ptr<HttpServiceImpl> &service,
        asio::io_context &context,
        const sese::net::IPAddress::Ptr &addr,
        SharedStream stream
)
    : HttpConnectionEx(service, context, addr),
      stream(std::move(stream)) {
}

void sese::internal::service::http::HttpsConnectionExImpl::writeBlocks(const std::vector<asio::const_buffer> &buffers, const std::function<void(const asio::error_code &code)> &callback) {
    is_write = true;
    async_write(*this->stream, buffers, [conn = getPtr(), callback](const asio::error_code &error, size_t) {
        conn->is_write = false;
        callback(error);
    });
}

void sese::internal::service::http::HttpsConnectionExImpl::writeBlock(const void *buffer, size_t size, const std::function<void(const asio::error_code &code)> &callback) {
    is_write = true;
    async_write(*this->stream, asio::buffer(buffer, size), [conn = getPtr(), callback](const asio::error_code &error, size_t) {
        conn->is_write = false;
        callback(error);
    });
}

void sese::internal::service::http::HttpsConnectionExImpl::readBlock(char *buffer, size_t length, const std::function<void(const asio::error_code &code)> &callback) {
    is_read = true;
    this->stream->async_read_some(asio::buffer(buffer, length), [conn = getPtr(), buffer, length, callback](const asio::error_code &error, size_t read) {
        conn->is_read = false;
        if (error || read == length) {
            callback(error);
        } else {
            conn->readBlock(buffer + read, length - read, callback);
        }
    });
}

void sese::internal::service::http::HttpsConnectionExImpl::checkKeepalive() {
    // if (keepalive) {
    timer.async_wait([conn = getPtr()](const asio::error_code &error) {
        if (error == asio::error::operation_aborted) {
        } else {
            conn->stream->lowest_layer().close();
        }
    });
    // }
}
