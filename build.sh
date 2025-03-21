#!/bin/zsh

SHARED_LIB=libomena.so
LIB_DIR=src
OBJECT_FILES=$LIB_DIR/build
COMPILER=$(which gcc || which clang || which tcc)

if [[ $? -ne 0 ]]; then
    print "Hey! you need a C compiler. Try installing GCC or Clang."
    exit 1
fi

mkdir -p $OBJECT_FILES  

for file in $(find $LIB_DIR -type f -name "*.c"); do
    obj_file=$OBJECT_FILES/$(basename "$file" .c).o
    $COMPILER -fPIC -c "$file" -o "$obj_file"
    if [[ $? -ne 0 ]]; then
        print "Compilation failed for $file"
        exit 1
    fi
done


#/* did they pass ? */
if [[ $? -eq 0 ]]; then
    $COMPILER -shared -o $LIB_DIR/$SHARED_LIB $OBJECT_FILES/*.o
    print "Shared library created at $LIB_DIR/$SHARED_LIB"
else
    print "Failed to create shared library."
    exit 1
fi

chmod +x ./run_test.sh
if [[ -f run_test.sh ]]; then
    ./run_test.sh
fi
