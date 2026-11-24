#!/bin/bash
# ============================================================================
# C++ Build Script for Student Result Management System
# Institution: Madhav Institute of Technology & Science, Gwalior
# ============================================================================
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

echo "---------------------------------------------------------"
echo " Building Student Result Management C++ Backend (C++17)  "
echo "---------------------------------------------------------"

cd "${SCRIPT_DIR}"

# Check for compiler
if command -v clang++ &> /dev/null; then
    CXX_COMPILER="clang++"
elif command -v g++ &> /dev/null; then
    CXX_COMPILER="g++"
else
    echo "Error: No suitable C++ compiler (clang++ or g++) found."
    exit 1
fi

echo "Using C++ Compiler: ${CXX_COMPILER}"

mkdir -p "${ROOT_DIR}/bin"

# Detect macOS SDK
SYSROOT_FLAG=""
if [[ "$(uname)" == "Darwin" ]]; then
    if [ -d "/Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk" ]; then
        SYSROOT_FLAG="-isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk"
    elif [ -d "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk" ]; then
        SYSROOT_FLAG="-isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
    elif command -v xcrun &> /dev/null; then
        SYSROOT_FLAG="-isysroot $(xcrun --show-sdk-path 2>/dev/null || echo '')"
    fi
fi

${CXX_COMPILER} -std=c++17 -O2 ${SYSROOT_FLAG} \
    -I"${SCRIPT_DIR}/include" \
    -I"${SCRIPT_DIR}" \
    main.cpp database.cpp auth.cpp student.cpp teacher.cpp subject.cpp marks.cpp result.cpp \
    -pthread \
    -o "${ROOT_DIR}/bin/student_result_server"

echo "---------------------------------------------------------"
echo " Build Succeeded!"
echo " Binary created: ${ROOT_DIR}/bin/student_result_server"
echo " To run server:  cd ${ROOT_DIR} && ./bin/student_result_server"
echo "---------------------------------------------------------"
