#
# Arm SCP/MCP Software
# Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

STEPS=4

llvm_version=$1

SCRIPT_NAME=$(basename "$0")
TMP_FOLDER=/tmp/"${SCRIPT_NAME}_data"
LOG_OUTPUT_FILE=${TMP_FOLDER}/out.log

# helper functions
error() {
    >&2 echo "error: $@"
    echo "error: $@" >> "$LOG_OUTPUT_FILE"
}

log() {
    echo -n "$@" >> "$LOG_OUTPUT_FILE"
}

logln() {
    echo "$@" >> "$LOG_OUTPUT_FILE"
}

lindent() {
    log "  $@"
}

lindentln() {
    logln "  $@"
}

println() {
    logln "$@"
    echo "$@"
}

indentln() {
    lindentln "$@"
    echo "  $@"
}

indent() {
    lindent "$@"
    echo -n "  $@"
}

input() {
    read -p "$1" "$2"
    logln "$1$$2"
}

step() {
    STEP=$1
    shift
    println "[$STEP/$STEPS] $@"
}

eexit() {
    >&2 echo
    >&2 echo "error: please find the complete logs in ${LOG_OUTPUT_FILE}. Aborting..."
    exit 1
}

cmd() {
    logln "$ $@"
    eval "$@" >> "$LOG_OUTPUT_FILE" 2>&1 || { echo "error!"; eexit; }
}

cmdp() {
    logln "$1"
    echo -n "  $1..."
    shift
    cmd "$@"
    echo "ok!"
}

echo "Setting up temporary directory at ${TMP_FOLDER}"
mkdir -p "$TMP_FOLDER"
echo -n "" > "$LOG_OUTPUT_FILE"


# Step 1 : obtaining the GNU Arm Embedded Toolchain sysroot for later...

step 1 "Obtaining the sysroot of the GNU Arm Embedded Toolchain..."
SYSROOT=""

if [[ -n $GCC_TOOLCHAIN ]]; then
    lindentln "\$GCC_TOOLCHAIN was found set, using it..."
    SYSROOT=${GCC_TOOLCHAIN}/arm-none-eabi
elif command -v arm-none-eabi-gcc &> /dev/null; then
    lindentln "arm-none-eabi-gcc was found! Fetching the sysroot..."
    SYSROOT=$(arm-none-eabi-gcc -print-sysroot)
else
    indentln "Could not find the GNU Arm Embedded Toolchain!"
    indentln
    indentln "Please enter the path to the GNU Arm Embedded Toolchain:"
    input "> " GCC_TOOLCHAIN

    SYSROOT=${GCC_TOOLCHAIN}/arm-none-eabi
fi

if [[ ! -d $SYSROOT ]]; then
    error "${GCC_TOOLCHAIN} is not a valid GNU Arm Embedded Toolchain path!"
    eexit
fi

step 1 "Toolchain sysroot found at: ${SYSROOT}"

# Step 2: retrieve LLVM
step 2 "Installing the LLVM ${llvm_version} toolchain..."
lindentln "Checking if clang-${llvm_version} is present in the path..."
if command -v clang-${llvm_version} &> /dev/null; then
    indentln "LLVM {llvm_version} is already installed! Skipping..."
else
    cmdp "Downloading the LLVM installation script" wget https://apt.llvm.org/llvm.sh -O $TMP_FOLDER/install-llvm.sh
    cmd chmod +x $TMP_FOLDER/install-llvm.sh
    cmdp "Downloading and installing LLVM ${llvm_version} (this will take a while)" $TMP_FOLDER/install-llvm.sh ${llvm_version}
fi

# Step 3: Retrieving Compiler-RT source code
step 3 "Installing the Compiler-RT Arm builtins..."
cmdp "Downloading the source code" wget https://github.com/llvm/llvm-project/releases/download/llvmorg-${llvm_version}.0.1/compiler-rt-${llvm_version}.0.1.src.tar.xz -O ${TMP_FOLDER}/rt-src.tar.xz
cmdp "Extracting the source code" tar -xf ${TMP_FOLDER}/rt-src.tar.xz -C ${TMP_FOLDER}
cmd cd ${TMP_FOLDER}/compiler-rt-${llvm_version}.0.1.src

BAREMETAL_PATH=$(clang-${llvm_version} -print-resource-dir)/lib/baremetal
cmd mkdir -p $BAREMETAL_PATH

for ARCH in armv7m armv7em armv8m.main armv8.1m.main; do
    cmdp "Preparing Compiler-RT builtins for target ${ARCH}" cmake -GNinja \
        -DLLVM_CONFIG_PATH=$(command -v llvm-config-${llvm_version}) \
        -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
        -DCOMPILER_RT_OS_DIR="baremetal" \
        -DCOMPILER_RT_BUILD_BUILTINS=ON \
        -DCOMPILER_RT_BUILD_SANITIZERS=OFF \
        -DCOMPILER_RT_BUILD_XRAY=OFF \
        -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
        -DCOMPILER_RT_BUILD_PROFILE=OFF \
        -DCMAKE_C_COMPILER=$(command -v clang-${llvm_version}) \
        -DCMAKE_C_COMPILER_TARGET="${ARCH}-none-eabi" \
        -DCMAKE_ASM_COMPILER_TARGET="${ARCH}-none-eabi" \
        -DCMAKE_AR=$(command -v llvm-ar-${llvm_version}) \
        -DCMAKE_NM=$(command -v llvm-nm-${llvm_version}) \
        -DCMAKE_RANLIB=$(command -v llvm-ranlib-${llvm_version}) \
        -DCOMPILER_RT_BAREMETAL_BUILD=ON \
        -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
        -DCOMPILER_RT_INCLUDE_TESTS=OFF \
        -DCMAKE_C_FLAGS=\"-march=${ARCH} -mthumb -mfpu=none -mfloat-abi=soft \
            -I${SYSROOT}/include\" \
        -DCMAKE_ASM_FLAGS=\"-march=${ARCH} -mthumb -mfpu=none -mfloat-abi=soft \
            -I${SYSROOT}/include\"
    cmdp "Building builtins for target ${ARCH}" ninja builtins
    cmdp "Installing the builtins" \
        cp ./lib/baremetal/libclang_rt.builtins-arm.a \
           ${BAREMETAL_PATH}/libclang_rt.builtins-${ARCH}.a
done

echo
rm -rf ${TMP_FOLDER}
