# PegOF

[PEG](https://en.wikipedia.org/wiki/Parsing_expression_grammar) grammar optimizer and formatter.
Supports any grammar supported by [PackCC](https://github.com/arithy/packcc) parser generator.

## How it works

Pegof parses peg grammar from input file (using PackCC generated parser) and extracts it's AST.
Then, based on the command it either just prints it out in nice and consistent format or directly as AST.
It can also perform multi-pass optimization process that goes through the AST and tries to simplify it
as much as possible.

Currently implemented optimizations:
 - **Rule inlining:** Some simple rules can be inlined directly into rules that reference them. Reducing number of rules improves the speed of generated parser.
 - **String concatenation:** Join adjacent string nodes into one. E.g. `"A" "B"` becomes `"AB"`.
 - **Character class optimization:** Normalize character classes to avoid duplicities and use ranges where possible. E.g. `[ABCDEFX0-53-9X]` becomes `[0-9A-FX]`.
 - **Convert single character classes to strings:** The code generated for strings is simpler than that generated for character classes. So we can convert for example `[\n]` to `"\n"` or `[^ ]` to `!" "`.
 - **Simplify negation of character classes:** Negation of character classes can be written as negative character class (e.g. `![\n]` -> `[^\n]`).
 - **Removing double quantifications:** If a single term is quantified twice, it is always possible to convert this into a single potfix operator with equel meaning (e.g. `(X+)?` -> `X*`).
 - **Removing double negations:** Negation of negation can be ignored, because it results in the original term (e.g. `!(!TERM)` -> `TERM`).
 - **Removing unnecessary repeats:** Joins repeated rules to single quantity. E.g. "A A*" -> "A+", "B* B*" -> "B*" etc.
 - **Removing unused variables:** Variables denoted in grammar (e.g. `e:expression`) which are not used in any source oe error block are discarded.
 - **Removing unused captures:** Captures denoted in grammar, which are not used in any source block, error block or referenced (via `$n`) are discarded.
 - **Removing container nodes:** When alternation, sequence, etc. contains only one child node, the child can be attached directly to the containers parent.

## Usage:
    pegof [<options>] [--] [<input_file> ...]

### Basic options:
    -h/--help
        Show help (this text)
    -c/--conf FILE
        Use given configuration file
    -v/--verbose
        Verbose logging to stderr (repeat for even more verbose output)
    -d/--debug
        Output very verbose debug info, implies max verbosity

### Input/output options:
    -f/--format
        Output formatted grammar (default)
    -a/--ast
        Output abstract syntax tree representation
    -p/--packcc
        Output source files as if the grammar was passed to packcc
    -I/--inplace
        Modify the input files (only when formatting)
    -i/--input FILE
        Path to file with PEG grammar, multiple paths can be given
        Value "-" can be used to specify standard input
        Mainly useful for config file
        If no file or --input is given, read standard input.
    -o/--output FILE
        Output to file (should be repeated if there is more inputs)
        Value "-" can be used to specify standard output
        Must not be used together with --inplace.

### Formatting options:
    -q/--quotes single/double
        Switch between double and single quoted strings (defaults to double)
    -w/--wrap-limit N
        Wrap alternations with more than N sequences (default 1)

### Optimization options:
    -O/--optimize OPT[,...]
        Comma separated list of optimizations to apply
    -X/--exclude OPT[,...]
        Comma separated list of optimizations that should not be applied
    -l/--inline-limit N
        Minimum inlining score needed for rule to be inlined.
        Number between 0.0 (inline everything) and 1.0 (most conservative), default is 0.2,
        only applied when inlining is enabled

### Supported values for --optimize and --exclude options:
    all
    char-class-negation
    concat-strings
    double-negation
    double-quantification
    inline
    none
    normalize-char-class
    remove-group
    repeats
    single-char-class
    unused-capture
    unused-variable

## Configuration file

Configuration file can be specified on command line. This file can contain the same options as can be given
on command line, only without the leading dashes. Short versions are supported as well, but not recommended,
because config file should be easy to read. It is encouraged to keep one option per line, but any whitespace
character can be used as separator.

Following config file would result in the same behavior as if pegof was called without any arguments:
```
format
input -
output -
double-quotes
inline-limit 10
wrap-limit 1
```

## Known issues

### Speed

This application was written with readability and maintainability in mind. Speed of execution is not a focus.
Some of very big grammars (e.g. for [Kotlin language](https://github.com/universal-ctags/ctags/blob/master/peg/kotlin.peg))
can take up to 1s to process.

### Error recovery

If the input file contains invalid grammar, pegof only prints `Syntax error`, without any indication of what is wrong.
Workaround is to run the file through PackCC directly, this will produce much more informative message stating what
happened and also on which line and column in the input file to look. If the PackCC processes the file without error
then you've found an error in pegof and bug report would be appreciated.

## Unicode support

This tool is currently lacking proper unicode support in many places. It is planned to be added later.

## Building

Pegof uses cmake, so to build it just run:

```sh
mkdir build
cd build
cmake -DPACKCC=/path/to/packcc ..
make
make test # optional, but recommended
```

Building on non-linux platforms has not been tested and might require some modifications to the process.

## Acknowledgment

Big thanks go to [Arihiro Yoshida](https://github.com/arithy), author of [PackCC](https://github.com/arithy/packcc)
for maintaining the great and very useful tool and also for providing
[an example how to write a simple AST builder](https://github.com/arithy/packcc/tree/master/examples/ast-tinyc),
which was used as a base for pegof.
