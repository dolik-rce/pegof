#include "checker.h"
#include "packcc_wrapper.h"
#include "utils.h"
#include "log.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include <stdio.h>
#include <unistd.h>
#include <cstdarg>

namespace fs = std::filesystem;

std::stringstream packcc_errors;

const std::string N_A = "N/A";

std::string Stats::compare(const Stats& s) const {
    #define PAD(X) left_pad((X > 0) ? std::to_string(X) : N_A, 8)
    #define PADP(X) left_pad(((s.X > 0) ? std::to_string(X * 100 / s.X) : N_A) + "%", 8)

    std::string result;
    result += "         |   lines  |   bytes  |   rules  |   terms  \n";
    result += "---------+----------+----------+----------+----------\n";
    result += "input    | " + PAD(s.lines) + " | " + PAD(s.bytes) + " | " + PAD(s.rules) + " | " + PAD(s.terms) + "\n";
    result += "output   | " + PAD(lines) + " | " + PAD(bytes) + " | " + PAD(rules) + " | " + PAD(terms) + "\n";
    result += "output % | " + PADP(lines) + " | " + PADP(bytes) + " | " + PADP(rules) + " | " + PADP(terms);
    return result;

    #undef PAD
    #undef PADP
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

bool Checker::call_packcc(const std::string& input, const std::string& output, std::string& errors) const {
    if (Config::get<bool>("skip-validation")) {
        log(2, "Skipping validation due to --skip-validation");
        return true;
    }

    log(2, "Processing %s with PackCC, storing output to %s.{h,c}", input.c_str(), output.c_str());

    // Call PackCC
    options_t opts = {};
    void *const ctx = create_context(input.c_str(), output.c_str(), &opts);
    bool result = parse(ctx) && generate(ctx);
    destroy_context(ctx);

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
    log(2, "Code has %ld bytes and %ld lines", code.size(), lines);
    log(2, "Grammar has %d rules and %d terms", rules, terms);
    return Stats(code.size(), lines, rules, terms);
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
