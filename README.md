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
 - **Removing unused variables:** Variables denoted in grammar (e.g. `e:expression`) which are not used in any source oe error block are discarded.
 - **Removing unused captures:** Captures denoted in grammar, which are not used in any source block, error block or referenced (via `$n`) are discarded.
 - **Removing container nodes:** When alternation, sequence, etc. contains only one child node, the child can be attached directly to the containers parent.
 - **Removing comments:** This makes it simpler to implement the other optimizations. Also, most of the comments would not make much sense in the reorganized grammar.

## Usage:
    pegof [<options>] [--] [<input_file> ...]

### Basic options:
    -h/--help
        Show help (this text)
    -c/--conf FILE
        Use given configuration file
    -v/--verbose
        Verbose logging to stderr

### Input/output options:
    -f/--format
        Output formatted grammar (default)
    -a/--ast
        Output abstract syntax tree representation
    -d/--debug
        Output debug info (includes AST and formatted output)
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
    -O/--optimize
        Apply optimizations
    -C/--keep-captures
        Do not discard unused captures
    -V/--keep-variables
        Do not discard unused variables
    -n/--no-concat
        Do not concatenate adjacent string
    -N/--no-char-class
        Do not optimize character classes
    -l/--inline-limit N
        Maximum number of references rule can have and still
        be inlined (default 10)

## Configuration file

Configuration file can be specified on command line. This file can contain the same options as can be given
on command line, only without the leading dashes. Short version are supported as well, but not recommended,
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
