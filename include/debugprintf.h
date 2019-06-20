#ifndef DEBUGPRINTF_H
#define DEBUGPRINTF_H

// debug信息类型
typedef enum DebugType {
    debug_info,
    debug_error,
    debug_warn
}DebugType;

// 格式化输出debug信息
void debug_printf(DebugType type, const char *format, ...);

#endif // DEBUGPRINTF_H
