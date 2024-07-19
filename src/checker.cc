#include "checker.h"

#include "config.h"
#include "log.h"
#include "packcc_wrapper.h"
#include "utils.h"

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <unistd.h>

std::stringstream packcc_errors;

const std::string EMPTY = "";
const int COL_WIDTH = 10;
const int BUFFER_SIZE = 10240;

#define CH(X) ((X > 0) ? (" | " + left_pad(#X, COL_WIDTH)) : EMPTY)
#define CS(X) ((X > 0) ? "-+-----------" : EMPTY)
#define CN(X) ((X > 0) ? (" | " + left_pad(std::to_string(X), COL_WIDTH)) : EMPTY)
#define CP(X) ((X > 0) ? (" | " + left_pad(std::to_string(X * 100 / s.X) + "%", COL_WIDTH)) : EMPTY)

std::string Stats::compare(const Stats& s) const {
    std::string result;
    result += "        " + CH(lines) + CH(bytes) + CH(rules) + CH(terms) + CH(duration) + CH(memory) + "\n";
    result += "--------" + CS(lines) + CS(bytes) + CS(rules) + CS(terms) + CS(duration) + CS(memory) + "\n";
    result += "input   " + CN(s.lines) + CN(s.bytes) + CN(s.rules) + CN(s.terms) + CN(s.duration) + CN(s.memory) + "\n";
    result += "output  " + CN(lines) + CN(bytes) + CN(rules) + CN(terms) + CN(duration) + CN(memory) + "\n";
    result += "output %" + CP(lines) + CP(bytes) + CP(rules) + CP(terms) + CP(duration) + CP(memory);
    return result;
}

#undef CH
#undef CS
#undef CN
#undef CP

Checker::Checker() {
    output = TempDir::get("output");
}

Checker::~Checker() {}

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
    pcc_array_init(&dirs);
    for (auto dir: Config::get_all_imports_dirs(input_file)) {
        pcc_array_add(&dirs, dir.c_str(), dir.size());
    }

    bool result = pcc_process(input.c_str(), output.c_str(), &dirs, &opts);
    pcc_array_term(&dirs);

    // Collect errors
    errors = packcc_errors.str();
    packcc_errors.clear();

    return result;
}

bool Checker::packcc(const std::string& peg, const std::string& output) const {
    std::string err;
    std::string input = TempDir::get("tmp.peg");
    write_file(input, peg);
    return call_packcc(input, output, err);
}

Stats Checker::stats(Grammar& g) const {
    std::string code = read_file(output + ".c");
    std::size_t lines = std::count(code.begin(), code.end(), '\n');
    int rules = g.find_children<Rule>().size();
    int terms = g.find_children<Term>().size();
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
        error(PARSING_ERROR, "Failed to parse grammar by packcc:\n%s", errors.c_str());
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
    std::string input = TempDir::get(filename);
    write_file(input, peg);
    return validate(input);
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
        error(SCRIPT_ERROR, "Benchmark setup failed! (exit_code=%d)", exit_code);
    }

    std::string out = TempDir::get("benchmark.out");
    std::string time;
    if (system("which /usr/bin/time 2> /dev/null > /dev/null") == 0) {
        time = "/usr/bin/time -f \"\n%M\" ";
    }
    std::string cmd = time + script + " benchmark " + output + " > " + out + " 2>&1";

    log(1, "Running benchmark.");
    log(4, "Benchmark command: %s", cmd.c_str());
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    exit_code = system(cmd.c_str());
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    if (exit_code != 0) {
        error(SCRIPT_ERROR, "%s\nBenchmark script failed! (exit_code=%d)", read_file(out).c_str(), exit_code);
    }

    log(1, "Tearing down benchmark environment.");
    exit_code = system((script + " teardown " + output).c_str());
    if (exit_code != 0) {
        error(SCRIPT_ERROR, "Benchmark teardown failed! (exit_code=%d)", exit_code);
    }
    if (time.size()) {
        std::vector<std::string> lines = split(read_file(out), "\n");
        memory = stoi(lines.back());
    }
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
}

extern "C" {
    int vfprintf_wrapped(FILE* stream, const char* format, va_list args) {
        int result;
        if (stream == stderr) {
            char buffer[BUFFER_SIZE];
            result = vsnprintf(buffer, BUFFER_SIZE, format, args);
            packcc_errors << buffer;
            if (result >= BUFFER_SIZE) {
                packcc_errors << "...";
            }
        } else {
            result = vfprintf(stream, format, args);
        }
        return result;
    }

    int fprintf_wrapped(FILE* stream, const char* format, ...) {
        va_list args;
        va_start(args, format);
        int result = vfprintf_wrapped(stream, format, args);
        va_end(args);
        return result;
    }
}
