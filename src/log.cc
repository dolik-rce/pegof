#include "log.h"

#include <chrono>
#include <string>

DebugIndent::DebugIndent() {
    DebugIndent::inc();
}

DebugIndent::~DebugIndent() {
    DebugIndent::dec();
}

int DebugIndent::indent(int increment) {
    static int indent = 0;
    indent += increment;
    return indent;
}

void DebugIndent::inc() {
    indent(1);
}

void DebugIndent::dec() {
    indent(-1);
}

void DebugIndent::print() {
    fprintf(stderr, "%*s", indent() * 2, "");
}

void print_timestamp() {
    static bool timestamp_config = Config::get<bool>("timestamp");
    if (timestamp_config) {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        int micros = (std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000).count();
        time_t time = std::chrono::system_clock::to_time_t(now);
        char timestamp[20];
        std::strftime(timestamp, 20, "%F %T", std::localtime(&time));
        fprintf(stderr, "%s.%06d ", timestamp, micros);
    }
}
