#!/bin/bash
# build juno fw

TARGETS="all"
OPERATION="full"
VERBOSE=
MODE=
VALID_TARGETS="scp_ramfw scp_romfw scp_romfw_bypass"

print_help() {
    echo "CMake builder for juno"
    echo "Defaults to all targets, full build"
    echo "options:"
    echo "-v : verbose build output"
    echo "-t : select specific target - one of ${VALID_TARGETS}"
    echo "-d : build debug mode"
    echo "-o : operation is one of:"
    echo "       generate - generate only"
    echo "       cpp_only - stop after C pre-processor to check macro generation"
    echo "       clean    - clean build area"
    exit 1
}

check_target () {
    for valid in ${VALID_TARGETS}
    do
        if [ "$1" == "${valid}" ]; then
            return
        fi
    done
    echo "Invalid target $1"
    print_help
}

while getopts o:t:vdh flag
do
    case "${flag}" in
        o) OPERATION=${OPTARG};;
        t) TARGETS=${OPTARG};;
        v) VERBOSE="V=y";;
        d) MODE="MODE=debug";;
        h) print_help;;
    esac
done

if [ "${TARGETS}" == "all" ]; then
    TARGETS="${VALID_TARGETS}"
fi

for target in ${TARGETS} 
do
    echo "Building: Op=${OPERATION}, target=${target}, Verbose=${VERBOSE}"
    check_target ${target}
    if [ "${OPERATION}" == "generate" ]; then
        make -f Makefile.cmake PRODUCT=juno BUILD_SYSTEM=Make BUILD_TARGET=${target} ${MODE} generate $VERBOSE
    elif [  "${OPERATION}" == "cpp_only"  ]; then
        make -f Makefile.cmake PRODUCT=juno BUILD_SYSTEM=Make BUILD_TARGET=${target} ${MODE} CPP_HALT=1 $VERBOSE
    elif [  "${OPERATION}" == "clean" ]; then
        make -f Makefile.cmake clean
        # clean removes the full build dir - only do once
        break
    elif [  "${OPERATION}" == "full" ]; then
        make -f Makefile.cmake PRODUCT=juno BUILD_SYSTEM=Make ${MODE} BUILD_TARGET=${target} $VERBOSE
    else
        echo "Invalid Operation specified"
        print_help
    fi
done

exit 0

        
        
    
