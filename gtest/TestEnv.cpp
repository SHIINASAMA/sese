#include <sese/system/Environment.h>
#include <sese/record/LogHelper.h>

#include <gtest/gtest.h>

using sese::system::Environment;
using sese::record::LogHelper;

TEST(TestEnv, _0) {
    LogHelper::i("build date: %s", Environment::getBuildDate());
    LogHelper::i("build date time: %s", Environment::getBuildDateTime());
    LogHelper::i("os type: %s", Environment::getOperateSystemType());
    LogHelper::i("version: %s.%s.%s", Environment::getMajorVersion(), Environment::getMinorVersion(), Environment::getPatchVersion());
    LogHelper::i("git info: %s-%s", Environment::getRepoBranch(), Environment::getRepoHash());
}