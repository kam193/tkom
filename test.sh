#!/bin/bash

LIST="1 2 3 4 5 6 7 8 9 10 11 12 13 14 15"

make build

for e in $LIST; do
    ./tkom.out < "tests/in/test$e.in" > outtmp
    if cmp -s -- "tests/out/test$e.out" "outtmp"; then
        echo "Test $e passed"
    else
        echo "Test $e FAILD"
    fi
done
rm outtmp
