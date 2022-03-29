#!/bin/bash
# build juno scp_ramfw

if [ "$2" == "verbose" ]; then
    VERBOSE="V=y"
fi

if [ "$1" == "verbose" ]; then
    VERBOSE="V=y"
fi

if [ "$1" == "generate" ]; then
   make -f Makefile.cmake PRODUCT=juno BUILD_SYSTEM=Make BUILD_TARGET=scp_ramfw generate $VERBOSE
elif [ "$1" == "cpp_only" ]; then
   make -f Makefile.cmake PRODUCT=juno BUILD_SYSTEM=Make BUILD_TARGET=scp_ramfw CPP_HALT=1 $VERBOSE
elif [ "$1" == "clean" ]; then
    make -f Makefile.cmake clean
elif [ "$1" == "help" ]; then
    echo "Options:-"
    echo "generate    - generate only"
    echo "cpp_only    - stop after C pre-processor to check macro generation"
    echo "clean       - clean build area"
    echo "<no option> - default to full build of juno scp_ramfw"
else     #default to full
   make -f Makefile.cmake PRODUCT=juno BUILD_SYSTEM=Make BUILD_TARGET=scp_ramfw $VERBOSE
fi

 
