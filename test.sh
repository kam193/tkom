#!/bin/bash

LIST="1 2"

for e in $LIST; do
    ./tkom.out < "tests/in/test$e.in" > outtmp
    if cmp -s -- "tests/out/test$e.out" "outtmp"; then
        echo "Test $e passed"
    else
        echo "Test $e FAILD"
    fi
done
rm outtmp
