# PegOF

Peg grammar optimizer and formatter. Supports any grammar supported by [PackCC](https://github.com/arithy/packcc)
parser generator.

## Usage:

```
pegof [command] <file>

Available commands:

    --format (default)      Formats the input file and prints to stdout.
    --ast                   Prints abstract syntax tree of input file.
    --optimize              Optimizes the input and prints it to stdout.
    --optimize-ast          Optimizes the input and prints its abstract syntax tree.
    --debug                 Performs all of the above and adds verbose messages about the processing.
```

## How it works

Pegof parses peg grammar from input file (using PackCC generated parser) and extracts it's AST.
Then, based on the command it either just prints it out in nice and consistent format or directly as AST.
It can also perform multi-pass optimization process that goes through the AST and tries to simplify it
as much as possible.

Currently implemented optimizations:
 - **Rule inlining:** Some simple rules can be inlined directly into rules that reference them. Reducing number of rules improves the speed of generated parser.
 - **String concatenation:** Join adjacent string nodes into one. E.g. `"A" "B"` becomes `"AB"`.
 - **Removing container nodes:** When alternation, sequence, etc. contains only one child node, the child can be attached directly to the containers parent.
 - **Removing comments:** This makes it simpler to implement the other optimizations. Also, most of the comments would not make much sense in the reorganized grammar.

## Known issues

### Speed

This application was written with readability and maintainability in mind. Speed of execution is not a focus.
Some of very big grammars (e.g. for [Kotlin language](https://github.com/universal-ctags/ctags/blob/master/peg/kotlin.peg))
can take up to 1s to process.

### Configurability

There is currently no way to configure the formatting and optimization behavior. For example strings are always printed
with double-quotes, indentation uses spaces and inlining is always limited to rules with 10 references or less.
This might change in future. Let the author know in issue, if you use this tool and if you would benefit
from configuration support.

### Error recovery

If the input file contains invalid grammar, pegof only prints `Syntax error`, without any indication of what is wrong.
Workaround is to run the file through PackCC directly, this will produce much more informative message stating what
happened and also on which line and column in the input file to look. If the PackCC processes the file without error
then you've found an error in pegof and bug report would be appreciated.

## Building

Pegof uses cmake, so to build it just run:

```sh
mkdir build
cd build
cmake -DPACKCC=/path/to/packcc ..
make
```

Building on non-linux platforms has not been tested and might require modifications to the process.

## Acknowledgment

Big thanks go to [Arihiro Yoshida](https://github.com/arithy), author of [PackCC](https://github.com/arithy/packcc)
for maintaining the great and very useful tool and also for providing
[an example how to write a simple AST builder](https://github.com/arithy/packcc/tree/master/examples/ast-tinyc),
which was used as a base for pegof.
