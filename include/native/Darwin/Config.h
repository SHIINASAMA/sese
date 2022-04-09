/**
 * @file Config.h
 * @brief Darwin 平台配置文件
 * @author kaoru
 * @date 2022年3月28日
 */
#pragma once

#if defined(__x86_64__)
#define SESE_ARCH_X64
#elif defined(__aarch64__)
#define SESE_ARCH_ARM64
#else
#error only support x86_64 & arm64
#endif

/// 可导出类型标识符
#define API
/// pid_t 格式化占位符
#define PRIdPid "d"
/// 忽略大小写比较
#define strcmpi strcasecmp

/// Native Socket 类型
using socket_t = int32_t;