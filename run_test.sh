#!/bin/bash

FLAGS="-Wall -Wextra"

#! This test script are predifinied to c source file in test dir and only compile the test
#! with prefix spec_* 
#! Test logic, based on return status

RED='\033[1;31m'
GREEN='\033[1;32m'    
RESET='\033[0m'    
UNDERLINE='\033[4m'    

BUILD_DIR="test/build"
mkdir -p "$BUILD_DIR"


TEST_SOURCES=$(find test -name "spec_*.c")


if [ -z "$TEST_SOURCES" ]; then
    echo -e "${RED}No test files found in ./test directory!${RESET}"
    exit 1
fi

echo -e "${UNDERLINE}               COMPILING TESTS                              ${RESET}\n"


#! move it to . 
cp src/libomena.so .

if [[ -f src/libomena.so ]]; then
    for test_file in $TEST_SOURCES; do
        test_name=$(basename "$test_file" .c)
        gcc "$test_file" -o "$BUILD_DIR/$test_name" $FLAGS -L. -lomena
        if [ $? -eq 0 ]; then
            echo -e "Compiled $test_name [ ${GREEN}OK${RESET} ]"
        else
            echo -e "Compilation failed for $test_name [ ${RED}${UNDERLINE}ERROR${RESET} ]"
            exit 1
        fi
    done
fi

echo -e "\n${UNDERLINE}               RUNNING TESTS                              ${RESET}\n"


for test_exec in "$BUILD_DIR"/spec_*; do
    if [ -x "$test_exec" ]; then
        echo -n "Running $(basename $test_exec)... "

        if "$test_exec" > /dev/null 2>&1; then
            echo -e "[ ${GREEN}${UNDERLINE}PASS${RESET} ]"
        else
            echo -e "[ ${RED}${UNDERLINE}FAIL${RESET} ]"
        fi
    fi
done

echo -e "${UNDERLINE}                                                           ${RESET}"
echo -e "${GREEN} All tests completed.${RESET}"
