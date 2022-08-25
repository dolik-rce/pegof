# 0.3.2

- Separate threshold for terminal symbol inlining
- Print rule counts in debug and verbose mode
- Basic unicode support in character classes
- Convert single letter character classes to string (e.g. [^x] -> !"x")
- Remove double negations (e.g. `!(!TERM)` -> `TERM`)

# 0.3.1

- Fix reading files with unicode
- Add Kotlin parser to tests, so we have some complex, real-world grammar

# 0.3.0

- Support actions at the beggining of rule (fixes #2)
- Correct escaping in character classes (fixes #6)
- Test that all outputs are compatibile with packcc
- Updated uncrustify config

# 0.2.0

- Fix compiler warnings about string formatting (fixes #3)
- Bump minimal required version of cmake
- Add license file (fixes #5)
- Enforce C++-17 (fixes #1)
- Test directives with C directives

# 0.1.0

- First public release
