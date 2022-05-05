#include <sese/net/TcpServer.h>
#include <sese/system/CpuInfo.h>

#pragma warning(disable : 4267)

using sese::CpuInfo;
using sese::IOContext;
using sese::IPAddress;
using sese::Socket;
using sese::TcpServer;
using sese::Thread;

IOContext::IOContext() noexcept {
    event = WSACreateEvent();
    overlapped.hEvent = event;
    operationType = OperationType::Null;
}

IOContext::~IOContext() noexcept {
    WSACloseEvent(event);
}

int64_t IOContext::send(const void *buffer, size_t size) noexcept {
    DWORD nBytes = size;
    DWORD dwFlags = 0;
    // 实际传输字节数
    DWORD cbTransfer;

    wsaBuf.buf = (char *) buffer;
    wsaBuf.len = size;
    operationType = OperationType::Send;
    int32_t nRet = WSASend(socket->getRawSocket(),
                           &wsaBuf,
                           1,
                           &nBytes,
                           dwFlags,
                           &overlapped,
                           nullptr);

    if (nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
        return -1;
    }

    WSAWaitForMultipleEvents(1, &event, FALSE, WSA_INFINITE, FALSE);
    WSAGetOverlappedResult(socket->getRawSocket(), &overlapped, &cbTransfer, FALSE, &dwFlags);
    WSAResetEvent(event);
    return cbTransfer;
}

int32_t IOContext::shutdown(Socket::ShutdownMode mode) const noexcept {
    return socket->shutdown(mode);
}

int64_t IOContext::recv(void *buffer, size_t size) noexcept {
    DWORD nBytes = size;
    DWORD dwFlags = 0;
    // 实际传输字节数
    DWORD cbTransfer;

    wsaBuf.buf = (char *) buffer;
    wsaBuf.len = size;
    operationType = OperationType::Recv;
    int32_t nRet = WSARecv(socket->getRawSocket(),
                           &wsaBuf,
                           1,
                           &nBytes,
                           &dwFlags,
                           &overlapped,
                           nullptr);

    if (nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
        return -1;
    }

    WSAWaitForMultipleEvents(1, &event, FALSE, WSA_INFINITE, FALSE);
    WSAGetOverlappedResult(socket->getRawSocket(), &overlapped, &cbTransfer, FALSE, &dwFlags);
    WSAResetEvent(event);
    return cbTransfer;
}

void IOContext::close() const noexcept {
    socket->close();
}

const IPAddress::Ptr &IOContext::getClientAddress() const noexcept {
    return reinterpret_cast<const IPAddress::Ptr &>(socket->getAddress());
}

bool TcpServer::init(const IPAddress::Ptr &ipAddress, size_t threads) noexcept {
    // IOCP 要求使用 WSASocket 创建 Socket 文件描述符
    auto family = ipAddress->getRawAddress()->sa_family;
    SOCKET serverSocket = WSASocketW(family, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (SOCKET_ERROR == serverSocket) {
        return false;
    }

    socket = std::make_shared<Socket>(serverSocket, ipAddress);
    if (!socket->setNonblocking(true)) {
        return false;
    }

    if (SOCKET_ERROR == socket->bind(ipAddress)) {
        socket->close();
        return false;
    }

    if (SOCKET_ERROR == socket->listen(SERVER_MAX_CONNECTION)) {
        socket->close();
        return false;
    }

    // 创建 IOCP
    size_t cores = CpuInfo::getLogicProcessors();
    if (threads == 0) threads = 1;
    // 如果选用线程小于等于逻辑核心数，则按一比一分配
    if (threads <= cores) {
        cores = threads;
    }
    this->threads = cores;
    iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, cores);
    if (INVALID_HANDLE_VALUE == iocpHandle) {
        socket->close();
        return false;
    }

    for (int32_t i = 0; i < cores; i++) {
        // 这里的线程由 IOCP 负责退出
        Thread th([this] { workerProc4WindowsIOCP(); },
                  "WindowsIOCPWorkerThread" + std::to_string(i));
    }
    threadPool = std::make_shared<ThreadPool>("TcpServer", threads);
    isShutdown = false;
    return true;
}

void TcpServer::workerProc4WindowsIOCP() {
    IOContext *ioContext = nullptr;
    DWORD dwContextBufferSize = 0;
    void *lpCompletionKey;
    while (true) {
        BOOL rt = GetQueuedCompletionStatus(
                iocpHandle,
                &dwContextBufferSize,
                (PULONG_PTR) &lpCompletionKey,
                (LPOVERLAPPED *) &ioContext,
                INFINITE);

        if (dwContextBufferSize == -1) {
            break;
        }

        if (!rt) {
            ioContext->socket->close();
            delete ioContext;
            continue;
        }

        int32_t nRet;
        DWORD nBytes = ioContext->wsaBuf.len;
        DWORD dwFlags = 0;
        switch (ioContext->operationType) {
            case OperationType::Send:
                nRet = WSASend(
                        ioContext->socket->getRawSocket(),
                        &(ioContext->wsaBuf),
                        1,
                        &nBytes,
                        dwFlags,
                        &(ioContext->overlapped),
                        nullptr);
                if (nRet == SOCKET_ERROR && ERROR_IO_PENDING != WSAGetLastError()) {
                    ioContext->socket->close();
                    delete ioContext;
                    continue;
                }
                break;
            case OperationType::Recv:
                nRet = WSARecv(
                        ioContext->socket->getRawSocket(),
                        &ioContext->wsaBuf,
                        1,
                        &nBytes,
                        &dwFlags,
                        &(ioContext->overlapped),
                        nullptr);
                if (nRet == SOCKET_ERROR && ERROR_IO_PENDING != WSAGetLastError()) {
                    ioContext->socket->close();
                    delete ioContext;
                    continue;
                }
                break;
            case OperationType::Null:
                ioContext->socket->close();
                delete ioContext;
                break;
        }
    }
}

void TcpServer::shutdown() {
    // 停止 loopWith
    isShutdown = true;

    // 停止 WindowsIOCPWorkerThread
    for (int32_t i = 0; i < threads; i++) {
        PostQueuedCompletionStatus(iocpHandle, 0, (DWORD) -1, nullptr);
    }

    // 停止线程池
    threadPool->shutdown();

    CloseHandle(iocpHandle);
    socket->close();
}

void TcpServer::loopWith(const std::function<void(IOContext *)> &handler) {
    while (!isShutdown) {
        auto client = socket->accept();
        if (client == nullptr) {
            continue;
        }

        if (!client->setNonblocking(true)) {
            client->close();
            continue;
        }

        if (CreateIoCompletionPort((HANDLE) client->getRawSocket(), iocpHandle, 0, 0) == nullptr) {
            client->close();
            continue;
        }

        auto ioContext = new IOContext;
        ioContext->socket = client;
        threadPool->postTask([handler, ioContext]() {
            handler(ioContext);
            delete ioContext;
        });
    }
}