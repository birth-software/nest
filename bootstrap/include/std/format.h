#pragma once

#include <std/base.h>

EXPORT String format_string(String buffer, const char* format, ...);
EXPORT String format_string_va(String buffer, const char* format, va_list args);
