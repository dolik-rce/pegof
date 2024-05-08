#include "checker.h"
#include "packcc_wrapper.h"
#include "config.h"
#include "utils.h"
#include "log.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <chrono>

#include <stdio.h>
#include <unistd.h>
#include <cstdarg>
#include <cstring>

namespace fs = std::filesystem;

std::stringstream packcc_errors;

const std::string EMPTY = "";
const int COL_WIDTH = 10;

std::string Stats::compare(const Stats& s) const {
    std::string result;
    #define COL(X) ((X > 0) ? (" | " + left_pad(#X, COL_WIDTH)) : EMPTY)
    result += "        " + COL(lines) + COL(bytes) + COL(rules) + COL(terms) + COL(duration) + COL(memory) + "\n";
    #undef COL
    #define COL(X) ((X > 0) ? "-+-----------" : EMPTY)
    result += "--------" + COL(lines) + COL(bytes) + COL(rules) + COL(terms) + COL(duration) + COL(memory) + "\n";
    #undef COL
    #define COL(X) ((X > 0) ? (" | " + left_pad(std::to_string(X), COL_WIDTH)) : EMPTY)
    result += "input   " + COL(s.lines) + COL(s.bytes) + COL(s.rules) + COL(s.terms) + COL(s.duration) + COL(s.memory) + "\n";
    result += "output  " + COL(lines) + COL(bytes) + COL(rules) + COL(terms) + COL(duration) + COL(memory) + "\n";
    #undef COL
    #define COL(X) ((X > 0) ? (" | " + left_pad(std::to_string(X * 100 / s.X) + "%", COL_WIDTH)) : EMPTY)
    result += "output %" + COL(lines) + COL(bytes) + COL(rules) + COL(terms) + COL(duration) + COL(memory);
    #undef COL
    return result;
}

Checker::Checker() {
    fs::path tmp_dir = fs::temp_directory_path() / ("pegof_" + std::to_string(time(0)));
    output = (tmp_dir / "output").native();
    tmp = tmp_dir.native();
    fs::create_directory(tmp_dir);
}

Checker::~Checker() {
    fs::remove_all(tmp);
}


void Checker::set_input_file(const std::string& input) {
    input_file = input;
}

bool Checker::call_packcc(const std::string& input, const std::string& output, std::string& errors) const {
    if (Config::get<bool>("skip-validation")) {
        log(2, "Skipping validation due to --skip-validation");
        return true;
    }

    log(2, "Processing %s with PackCC, storing output to %s.{h,c}", input.c_str(), output.c_str());

    // Call PackCC
    std::vector<std::string> options = split(Config::get<std::string>("packcc-options"));
    options_t opts = {
        (contains(options, "ascii") || contains(options, "a")) ? TRUE : FALSE,
        (contains(options, "lines") || contains(options, "l")) ? TRUE : FALSE,
        (contains(options, "debug") || contains(options, "d")) ? TRUE : FALSE
    };

    string_array_t dirs;
    string_array__init(&dirs);
    for (auto dir: Config::get_all_imports_dirs(input_file)) {
        string_array__add(&dirs, dir.c_str(), dir.size());
    }

    void *const ctx = create_context(input.c_str(), output.c_str(), &dirs, &opts);
    bool result = parse(ctx) && generate(ctx);
    destroy_context(ctx);
    string_array__term(&dirs);

    // Collect errors
    errors = packcc_errors.str();
    packcc_errors.clear();

    return result;
}

bool Checker::packcc(const std::string& peg, const std::string& output) const {
    std::string err;
    fs::path input = fs::path(tmp) / "tmp.peg";
    write_file(input.native(), peg);
    return call_packcc(input.native(), output, err);
}

Stats Checker::stats(Grammar& g) const {
    std::string code = read_file(output + ".c");
    std::size_t lines = std::count(code.begin(), code.end(), '\n');
    int rules = g.find_all<Rule>().size();
    int terms = g.find_all<Term>().size();
    int duration = 0;
    int memory = 0;
    benchmark(duration, memory);
    log(2, "Code has %ld bytes and %ld lines", code.size(), lines);
    log(2, "Grammar has %d rules and %d terms", rules, terms);
    return Stats(code.size(), lines, rules, terms, duration, memory);
}

bool Checker::validate(const std::string& input) const {
    std::string errors;
    if (!call_packcc(input, output, errors)) {
        error("Failed to parse grammar by packcc:\n%s", errors.c_str());
    }
    return true;
}

bool Checker::validate(const std::string& filename, const std::string& content) const {
    if (filename.empty()) {
        return validate_string("stdin", content);
    } else {
        return validate_file(filename);
    }
}

bool Checker::validate_string(const std::string& filename, const std::string& peg) const {
    fs::path input = fs::path(tmp) / filename;
    write_file(input.native(), peg);
    return validate(input.native());
}

bool Checker::validate_file(const std::string& filename) const {
    return validate(filename);
}

void Checker::benchmark(int& duration, int& memory) const {
    std::string script = Config::get<std::string>("benchmark");
    if (script.empty()) {
        return;
    }

    int exit_code;
    log(1, "Setting up benchmark environment.");
    exit_code = system((script + " setup " + output).c_str());
    if (exit_code != 0) {
        error("Benchmark setup failed! (exit_code=%d)", exit_code);
    }

    std::string out = tmp + "/benchmark.out";
    std::string time;
    if (system("which /usr/bin/time 2> /dev/null > /dev/null") == 0) {
        time = "/usr/bin/time -f \"\\n%M\" ";
    }
    std::string cmd = time + script + " benchmark " + output + " > " + out + " 2>&1";

    log(1, "Running benchmark.");
    log(4, "Benchmark command: %s", cmd.c_str());
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    exit_code = system(cmd.c_str());
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    if (exit_code != 0) {
        error("%s\n", read_file(out).c_str());
        error("Benchmark script failed! (exit_code=%d)", exit_code);
    }

    log(1, "Tearing down benchmark environment.");
    exit_code = system((script + " teardown " + output).c_str());
    if (exit_code != 0) {
        error("Benchmark teardown failed! (exit_code=%d)", exit_code);
    }
    if (time.size()) {
        std::vector<std::string> lines = split(read_file(out), "\n");
        memory = stoi(lines.back());
    }
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
}

extern "C" {
int vfprintf_wrapped(FILE *stream, const char *format, va_list args) {
    int result;
    if (stream == stderr) {
        char buffer[10240];
        result = vsprintf(buffer, format, args);
        packcc_errors << buffer;
    } else {
        result = vfprintf (stream, format, args);
    }
    return result;
}

int fprintf_wrapped(FILE *stream, const char *format, ...) {
    va_list args;
    va_start (args, format);
    int result = vfprintf_wrapped(stream, format, args);
    va_end (args);
    return result;
}
}
