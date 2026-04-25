#!/usr/bin/env bash
set -Eeuo pipefail

# 用法：
#   ./run_asan_env.sh ./UltrasoundQmlCppDemo
#
# 注意：
#   这个脚本只是设置 ASAN/UBSAN 运行环境。
#   真正想用 AddressSanitizer，需要 CMake 编译时加入：
#   -fsanitize=address,undefined -fno-omit-frame-pointer -g

APP="${1:-./UltrasoundQmlCppDemo}"
LOG_DIR="${LOG_DIR:-./test_logs}"
STAMP="$(date +%Y%m%d_%H%M%S)"
OUT_DIR="${LOG_DIR}/asan_${STAMP}"
mkdir -p "${OUT_DIR}"

export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-xcb}"
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=1:abort_on_error=1:strict_string_checks=1:check_initialization_order=1}"
export UBSAN_OPTIONS="${UBSAN_OPTIONS:-print_stacktrace=1:halt_on_error=1}"

"${APP}" > "${OUT_DIR}/stdout.log" 2> "${OUT_DIR}/stderr.log"
