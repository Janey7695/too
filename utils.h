#ifndef _UTILS_H_
#define _UTILS_H_

#include <time.h>

#define DEBUG_LOG
#define COLOR_RED_NUM 31
#define COLOR_GREEN_NUM 32
#define COLOR_YELLOW_NUM 33
#define COLOR_NORMAL_NUM 0

#define LOG_PLAIN(color, level, message, ...)                       \
    {                                                               \
        char tss[80] = "";                                          \
        get_current_timestamp(tss);                                 \
        printf("%s - \033[%dm [%s] \033[0m : ", tss, color, level); \
        printf(message, ##__VA_ARGS__);                             \
        printf("\r\n");                                             \
    }
#define LOG_WARN(message, ...) LOG_PLAIN(COLOR_YELLOW_NUM, "warning", message, ##__VA_ARGS__)
#define LOG_ERROR(message, ...) LOG_PLAIN(COLOR_RED_NUM, "error", message, ##__VA_ARGS__)
#define LOG_NORMAL(message, ...) LOG_PLAIN(COLOR_NORMAL_NUM, "log", message, ##__VA_ARGS__)
#define LOG_SUCCESS(message, ...) LOG_PLAIN(COLOR_GREEN_NUM, "ok", message, ##__VA_ARGS__)

#ifdef DEBUG_LOG
#define DEBUG_PLAIN(color, level, message, ...)                       \
    {                                                               \
        char tss[80] = "";                                          \
        get_current_timestamp(tss);                                 \
        printf("%s - \033[%dm [%s] \033[0m : ", tss, color, level); \
        printf(message, ##__VA_ARGS__);                             \
        printf("\r\n");                                             \
    }
#else
#define DEBUG_PLAIN(color, level, message, ...)          {char* tss = NULL;}            
#endif
#define DEBUG_WARN(message, ...) DEBUG_PLAIN(COLOR_YELLOW_NUM, "warning", message, ##__VA_ARGS__)
#define DEBUG_ERROR(message, ...) DEBUG_PLAIN(COLOR_RED_NUM, "error", message, ##__VA_ARGS__)
#define DEBUG_NORMAL(message, ...) DEBUG_PLAIN(COLOR_NORMAL_NUM, "log", message, ##__VA_ARGS__)
#define DEBUG_SUCCESS(message, ...) DEBUG_PLAIN(COLOR_GREEN_NUM, "ok", message, ##__VA_ARGS__)

#define tOO_MALLOC(type,length)  (type *)malloc(sizeof(type)*length)


void get_current_timestamp(char *tss);

#endif
