# [TKOM]
Interpreter for simple language based on Python.

# Compile

Use `make`. If you want to build without running tests previously,
use `make build-notest`.

# Run tests

For unittest use `make tests`. This require `boost` >= `1.59`.
For full test use `test.sh`

# Use
Interpreter read from stdin. Compiled version can be run by `./tkom.out`.

For example, code like:

```python
def function_name(arg1):
    x = arg1 / 3
    return x + arg1

function_name(23)
```

Output is:

    ./tkom.out < examples/example.py
    30
