#!/usr/bin/env bash
set -Eeuo pipefail

# 用法：
#   ./run_gdb_backtrace.sh ./UltrasoundQmlCppDemo
# 作用：
#   在 gdb 下运行程序，崩溃时自动导出完整调用栈，重点用于定位段错误。

APP="${1:-./UltrasoundQmlCppDemo}"
LOG_DIR="${LOG_DIR:-./test_logs}"
STAMP="$(date +%Y%m%d_%H%M%S)"
OUT_DIR="${LOG_DIR}/gdb_${STAMP}"
mkdir -p "${OUT_DIR}"

if [[ ! -x "${APP}" ]]; then
    echo "[FAIL] 程序不存在或不可执行: ${APP}"
    exit 2
fi

if ! command -v gdb >/dev/null 2>&1; then
    echo "[FAIL] 未安装 gdb。Ubuntu/Debian 可执行：sudo apt install gdb"
    exit 3
fi

export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-xcb}"
export QSG_INFO="${QSG_INFO:-1}"
export QT_DEBUG_PLUGINS="${QT_DEBUG_PLUGINS:-0}"

echo "[INFO] 使用 gdb 运行: ${APP}"
echo "[INFO] 日志目录: ${OUT_DIR}"

gdb -q -batch \
    -ex "set pagination off" \
    -ex "set confirm off" \
    -ex "run" \
    -ex "echo \n===== BACKTRACE FULL =====\n" \
    -ex "bt full" \
    -ex "echo \n===== THREADS BACKTRACE FULL =====\n" \
    -ex "thread apply all bt full" \
    -ex "echo \n===== REGISTERS =====\n" \
    -ex "info registers" \
    --args "${APP}" \
    > "${OUT_DIR}/gdb_backtrace.txt" \
    2>&1 || true

echo "[DONE] GDB 调用栈已生成: ${OUT_DIR}/gdb_backtrace.txt"
