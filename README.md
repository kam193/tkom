# [TKOM]
Interpreter for simple language based on Python.

# Compile

Use `make`. If you want to build without running tests previously,
use `make build-notest`.

# Run unittests

Use `make tests`. This require `boost` >= `1.59`

# Use
At this moment there is only a lexer feature. Compiled version get
code from stdin and print list of tokens with line, column (this is
the end of token) and token ID. It also try to print three possible
value of tokens: string, integer and double. This is only for feature
presentation.

Compiled version can be run by `./tkom.out`.

For example file like:

```python
def function_name(arg1):
    x = arg1 / 3
    return x + arg1

function_name(23)
```

Output is:

    ./tkom.out < examples/example.py
     LINE | COL | TOKEN | STR_VALUE | DOUBLE_VALUE | INT_VALUE
        1 |   3 |    37 |           | 0 | 0
        1 |  17 |    23 | function_name | 0 | 0
        1 |  18 |     2 |           | 0 | 0
        1 |  22 |    23 |      arg1 | 0 | 0
        1 |  23 |     3 |           | 0 | 0
        1 |  24 |     6 |           | 0 | 0
    <...and more>
