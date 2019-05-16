# [TKOM]
Interpreter for simple language based on Python.

# Compile

Use `make`. If you want to build without running tests previously,
use `make build-notest`.

# Run unittests

Use `make tests`. This require `boost` >= `1.59`

# Use
At this moment project is on parser state. Code given on standard input
is scanned, parsed and reproduced to source code (with some standard
convension, e.g. block indend is always two spaces, even if input was other).

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
    PARSING END
    def function_name(arg1):
      x = arg1 / 3
      return x + arg1
    function_name(23)
