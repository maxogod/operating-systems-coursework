#!/usr/bin/env bash

cd ./shell

cc tests/reflector.c -o tests/reflector
docker build -t test-shell-runner -f Dockerfile .

rm -rf test-to-run
ls ./tests/specs/ | grep $1 | sed -e 's/.yaml//g' | awk '{var="docker run --rm test-shell-runner ./tests/test-shell /shell/sh /shell/tests/reflector "$1" | grep -E \"PASS|FAIL\""; print var}' > ./test-to-run
chmod +x ./test-to-run
./test-to-run