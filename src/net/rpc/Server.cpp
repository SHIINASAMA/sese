#include <sese/net/rpc/Server.h>
#include <sese/config/json/JsonUtil.h>
#include <sese/Packaged2Stream.h>

#define BuiltinSetExitCode(code) exit->setDataAs<int64_t>(code)

using namespace sese;
using namespace sese::json;

void rpc::Server::serve(const IPAddress::Ptr &address, size_t threads) noexcept {
    tcpServer = TcpServer::create(address, threads, 0);

    tcpServer->loopWith([this](IOContext *context) {
        auto stream = std::make_shared<ClosablePackagedStream<IOContext>>(context);
        auto object = json::JsonUtil::deserialize(stream, 5);
        if (!object) return;// 序列化请求失败
        auto result = std::make_shared<json::ObjectData>();

        auto exit = std::make_shared<json::BasicData>();
        BuiltinSetExitCode(SESE_RPC_CODE_SUCCEED);
        result->set(SESE_RPC_TAG_EXIT_CODE, exit);

        auto version = std::make_shared<json::BasicData>();
        version->setDataAs<std::string>(SESE_RPC_VERSION_0_1);
        result->set(SESE_RPC_TAG_VERSION, version);

        // 1.版本确定
        std::string ver;
        auto verData = object->getDataAs<BasicData>(SESE_RPC_TAG_VERSION);
        if (nullptr == verData) {
            BuiltinSetExitCode(SESE_RPC_CODE_MISSING_REQUIRED_FIELDS);
            JsonUtil::serialize(result, stream);
            stream->close();
            return;
        } else {
            ver = verData->getDataAs<std::string>(SESE_RPC_VALUE_UNDEF);
        }

        if (SESE_RPC_VALUE_UNDEF == ver) {
            BuiltinSetExitCode(SESE_RPC_CODE_MISSING_REQUIRED_FIELDS);
            JsonUtil::serialize(result, stream);
            stream->close();
            return;
        } else if (SESE_RPC_VERSION_0_1 != ver) {
            BuiltinSetExitCode(SESE_RPC_CODE_NONSUPPORT_VERSION);
            JsonUtil::serialize(result, stream);
            stream->close();
            return;
        }

        // 2.获取并检查远程调用名称
        std::string name;
        Get4Server(name, object, result, std::string, SESE_RPC_TAG_NAME, SESE_RPC_VALUE_UNDEF);
        if (SESE_RPC_VALUE_UNDEF == name) {
            BuiltinSetExitCode(SESE_RPC_CODE_MISSING_REQUIRED_FIELDS);
            JsonUtil::serialize(result, stream);
            stream->close();
            return;
        }

        auto iterator = map.find(name);
        if (iterator == map.end()) {
            BuiltinSetExitCode(SESE_RPC_CODE_NO_EXIST_FUNC);
            JsonUtil::serialize(result, stream);
            stream->close();
            return;
        }

        // 3.获取并检查用户过程
        auto func = iterator->second;
        auto args = object->getDataAs<ObjectData>(SESE_RPC_TAG_ARGS);
        if (!args) {
            BuiltinSetExitCode(SESE_RPC_CODE_MISSING_REQUIRED_FIELDS);
            JsonUtil::serialize(result, stream);
            stream->close();
            return;
        }

        // 4.执行用户过程
        func(args, result);

        // 5.序列化
        JsonUtil::serialize(result, stream);
        stream->close();
    });
}

#undef BuiltinSetExitCode

void rpc::Server::enroll(const std::string &name, sese::rpc::Server::Func func) noexcept {
    this->map[name] = std::move(func);
}

void rpc::Server::shutdown() noexcept {
    tcpServer->shutdown();
}