<h1 align="center">Sese Framework</h1>
<div align="center">
<img src="logo.svg" width="128" height="128" alt="logo"/>
</div>
<div align="center">
<img src="https://img.shields.io/static/v1?label=license&message=Apache-2.0&color=blue&logo=Apache" alt="license"/>
<img src="https://img.shields.io/static/v1?label=language&message=C%2B%2B%2017&color=blue&logo=cplusplus" alt="lang"/>
<img src="https://img.shields.io/static/v1?label=build%20system&message=CMake&color=blue&logo=cmake" alt="buildsystem"/>
<br>
<img alt="CodeFactor Status" src="https://www.codefactor.io/repository/github/libsese/sese-core/badge"/>
<img alt="GitHub Workflow Status (with event)" src="https://img.shields.io/github/actions/workflow/status/libsese/sese-core/windows-2022.yml?label=Windows&logo=windows">
<img alt="GitHub Workflow Status (with event)" src="https://img.shields.io/github/actions/workflow/status/libsese/sese-core/ubuntu-2204-apt.yml?label=Ubuntu&logo=ubuntu">
<img alt="GitHub Workflow Status (with event)" src="https://img.shields.io/github/actions/workflow/status/libsese/sese-core/macos-12-brew.yml?label=macOS&logo=apple">
<br>
</div>

## Intro

This is a cross-platform framework for developing fundamental components,
 used to some extent as a supplement to the standard library.
 It is positioned similarly to `Boost` and `folly` with respect to the standard library.
 The project uses C++ 17 standard and introduces vcpkg as a package manager to help us simplify dependency management issues.

## Demo

Builtin logger

```c++
#include <sese/record/Marco.h>
// ...
SESE_INFO("hello world");
SESE_WARN("error %s", err.what().c_str());
```

> 2024-05-15T15:54:48.296Z I main.cpp:8 Main:7116> hello world<br>
> 2024-05-15T15:54:48.296Z W main.cpp:9 Main:7116> error End of file

---

HTTP Controller

```c++
#include <sese/service/http/HttpServer_V3.h>
// ...
SESE_CTRL(MyController, std::mutex mutex{}; int times = 0) {
    SESE_INFO("LOADING MyController");
    SESE_URL(timers, RequestType::GET, "/times") {
        sese::Locker locker(mutex);
        times += 1;
        auto message = "timers = '" + std::to_string(this->times) + "'\n";
        resp.getBody().write(message.data(), message.length());
    };
    SESE_URL(say, RequestType::GET, "/say?<say>") {
        auto words = req.get("say");
        auto message = "you say '" + words + "'\n";
        resp.getBody().write(message.data(), message.length());
    };
    SESE_INFO("LOADED");
}
```

---

Cross-process communication

```c++
#include <sese/system/IPC.h>
#include <sese/record/Marco.h>
// ···
// server
auto channel = sese::system::IPCChannel::create("Test", 1024);
    while (true) {
        auto messages = channel->read(1);
        if (messages.empty()) {
            sese::sleep(1s);
            continue;
        }
        for (auto &&msg: messages) {
            SESE_INFO("recv %s", msg.getDataAsString().c_str());

            if (msg.getDataAsString() == "Exit") {
                goto end;
            }
        }
    }
end:
    return 0;
// ···
// client
auto channel = sese::system::IPCChannel::use("Test");
channel->write(1, "Hello");
channel->write(2, "Hi");
channel->write(1, "Exit");
```

## Build

### For Developers/Contributors

1. Configure the development environment

For Windows users, please install and config
the [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started).

For non-Windows users, vcpkg is likewise available. But at the same time, you can choose to install the dependencies
using the native system dependency management tool. We have provided several preset installation scripts.

- Ubuntu

```bash
sudo ./scripts/install_ubuntu_deps.sh
```

- Fedora

```bash
sudo ./scripts/install_fedora_deps.sh
```

- macOS

```bash
./scripts/install_darwin_deps.sh
```

2. Compilation options

If you have vcpkg configured, you can simply configure the dependencies by setting
the [toolchain file](https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration).

If you're using a system dependency management tool, you'll need to manually add the 'SESE_USE_NATIVE_MANAGER'
compilation option after pressing the corresponding dependency.

https://github.com/libsese/sese/blob/4cd74389d7105b71c632070c775a727be8ee413d/CMakeLists.txt#L8-L16

> [!TIP]
> Please refer to the latest [CMakeLists.txt](./CMakeLists.txt) file understands the corresponding functional options
> Or you can just use our default [CMakePresets.json](./CMakePresets.json), which will enable most of the options by
> default.

3. Compile

Configuring the finished compilation options only requires a regular build, such as:

```bash
cmake --build build/linux-debug -- -j 4
```

### For Normal Users

For common users, we recommend using vcpkg to import this dependency, you can refer to our
[template project](https://github.com/libsese/sese-template) to config your project()

> [!WARNING]
> Projects can also be installed on ordinary machines as normal projects, but this is not a recommended practice and
> cannot be supported.
> If you want to do this, you can refer to the 'Build' > 'For Developers/Contributors' section on dependency management
> tools.

The main job is to write the project's dependency configuration file, for example:

`vcpkg.json`

```json
{
  "dependencies": [
    "sese"
  ]
}
```

> [!IMPORTANT]
> Since the built-in baseline `14b91796a68c87bc8d5cb35911b39287ccb7bd95`, sese has been included in the built-in list. Before this,
> you needed to create an additional configuration file to import our [private registry](https://learn.microsoft.com/en-us/vcpkg/consume/git-registries).

`vcpkg-configuration.json`

```json
{
  "default-registry": {
    "kind": "git",
    "repository": "https://github.com/microsoft/vcpkg.git",
    "baseline": "c8696863d371ab7f46e213d8f5ca923c4aef2a00"
  },
  "registries": [
    {
      "kind": "git",
      "repository": "https://github.com/libsese/vcpkg-registry.git",
      "baseline": "73268778d5f796f188ca66f71536377b171ee37e",
      "packages": [
        "sese"
      ]
    }
  ]
}
```

If you're not using vcpkg, the above steps are unnecessary.

### Testing

We used googletest as our testing framework. Detailed information about our tests can be found in
 github actions, including the results of various platforms and linux test coverage.

| Platform | Entry | Unit Test | Coverage Test |
|--|--|--|--|
| Windows | [Unit Tests](https://github.com/libsese/sese/actions/workflows/windows-2022.yml) | ✅ |
| Linux | [Unit Tests](https://github.com/libsese/sese/actions/workflows/ubuntu-22.04-apt.yml) | ✅ | ✅ |
| macOS | [Unit Tests](https://github.com/libsese/sese/actions/workflows/macos-12-brew.yml) | ✅ |

1. Local Testing

- Services on Ubuntu workflow

If you need to run full tests locally, you may need to pay attention to the database side of the
 test, which requires some additional service and configuration support.

https://github.com/libsese/sese/blob/4cd74389d7105b71c632070c775a727be8ee413d/.github/workflows/ubuntu-22.04-apt.yml#L14-L34

https://github.com/libsese/sese/blob/4cd74389d7105b71c632070c775a727be8ee413d/.github/workflows/ubuntu-22.04-apt.yml#L45-L53

https://github.com/libsese/sese/blob/4cd74389d7105b71c632070c775a727be8ee413d/.github/workflows/ubuntu-22.04-apt.yml#L72-L75

- Deploy services through docker-compose

```bash
docker-compose up -d
sqlite3 build/db_test.db < scripts/sqlite_dump.sql
```

2. Generate coverage test reports

Gcovr needs to be installed, either by pip or using the System Package Manager.

```bash
mkdir -p build/coverage/html
gcovr --config gcovr-html.cfg
```

> [!NOTE]
> Generating coverage test data first requires setting some additional compilation options to
> generate test data such as gcov format, such as GCC compilation options:

https://github.com/libsese/sese/blob/4cd74389d7105b71c632070c775a727be8ee413d/.github/workflows/ubuntu-22.04-apt.yml#L75

## Documents

[Documents](https://libsese.github.io/sese/) will be updated automatically with the update of the main branch to making pages.

Document content is automatically generated from code comments, and the docs directory actually houses some of the resources needed to build the document.