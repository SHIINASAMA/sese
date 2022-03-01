#pragma once

#ifdef _WIN32
#define pid_t unsigned long

#ifdef WINDOWS_DLL
#define API __declspec(dllexport)
#else
#define API
#endif
#endif

#ifdef __linux__
#define API
#endif

// String Buffer 单元因子
#define STRING_BUFFER_SIZE_FACTOR 1024
// 主日志器名称
#define MASTER_LOGGER_NAME "ROOT"
// 日志输出缓存区大小
#define LOGGER_OUTPUT_BUFFER 1024
// 日志格式化器匹配格式
#define DEFAULT_PATTERN "%c %lv %tn:%th %f:%li> %m"
// 空格字符
#define SPACE_CHARS " \n\r\t\v\b"