#!/usr/bin/env bash
set -Eeuo pipefail

# 用法：
#   ./run_smoke_test.sh ./UltrasoundQmlCppDemo
# 可选环境变量：
#   DURATION=20 QT_QPA_PLATFORM=xcb ./run_smoke_test.sh ./UltrasoundQmlCppDemo
#   LOG_DIR=./test_logs ./run_smoke_test.sh ./UltrasoundQmlCppDemo

APP="${1:-./UltrasoundQmlCppDemo}"
DURATION="${DURATION:-20}"
LOG_DIR="${LOG_DIR:-./test_logs}"
STAMP="$(date +%Y%m%d_%H%M%S)"
OUT_DIR="${LOG_DIR}/run_${STAMP}"

mkdir -p "${OUT_DIR}"

if [[ ! -x "${APP}" ]]; then
    echo "[FAIL] 程序不存在或不可执行: ${APP}"
    echo "建议：chmod +x ${APP}"
    exit 2
fi

# 建议优先用 xcb。你当前日志里 Wayland 提示不是崩溃主因，但 Qt5/QML 在 Ubuntu + Gnome Wayland 下更容易出现兼容问题。
export QT_QPA_PLATFORM="${QT_QPA_PLATFORM:-xcb}"

# Qt/QML/SceneGraph 调试信息
export QSG_INFO="${QSG_INFO:-1}"
export QT_DEBUG_PLUGINS="${QT_DEBUG_PLUGINS:-0}"
export QT_LOGGING_RULES="${QT_LOGGING_RULES:-qt.scenegraph.general=true;qt.qml.warning=true}"

# 开 core dump
ulimit -c unlimited || true

echo "[INFO] APP=${APP}" | tee "${OUT_DIR}/summary.txt"
echo "[INFO] DURATION=${DURATION}s" | tee -a "${OUT_DIR}/summary.txt"
echo "[INFO] QT_QPA_PLATFORM=${QT_QPA_PLATFORM}" | tee -a "${OUT_DIR}/summary.txt"
echo "[INFO] OUT_DIR=${OUT_DIR}" | tee -a "${OUT_DIR}/summary.txt"

{
    echo "===== date ====="
    date
    echo
    echo "===== uname ====="
    uname -a
    echo
    echo "===== os-release ====="
    cat /etc/os-release 2>/dev/null || true
    echo
    echo "===== env: QT/QML/QSG ====="
    env | grep -E '^(QT|QML|QSG|DISPLAY|WAYLAND|XDG_SESSION_TYPE)=' | sort || true
    echo
    echo "===== ldd ${APP} ====="
    ldd "${APP}" || true
} > "${OUT_DIR}/system_info.txt" 2>&1

echo "[INFO] 启动程序并观察 ${DURATION}s..."
set +e
timeout --preserve-status "${DURATION}" "${APP}" \
    > "${OUT_DIR}/stdout.log" \
    2> "${OUT_DIR}/stderr.log"
RC=$?
set -e

echo "[INFO] return_code=${RC}" | tee -a "${OUT_DIR}/summary.txt"

if [[ "${RC}" -eq 124 || "${RC}" -eq 143 ]]; then
    echo "[PASS] 程序在 ${DURATION}s 内没有崩溃，已由 timeout 正常结束。" | tee -a "${OUT_DIR}/summary.txt"
    exit 0
elif [[ "${RC}" -eq 139 ]]; then
    echo "[FAIL] 程序发生段错误 SIGSEGV，return_code=139。" | tee -a "${OUT_DIR}/summary.txt"
    echo "[INFO] 请继续执行: ./run_gdb_backtrace.sh ${APP}" | tee -a "${OUT_DIR}/summary.txt"
    exit 139
elif [[ "${RC}" -ne 0 ]]; then
    echo "[FAIL] 程序异常退出，return_code=${RC}。" | tee -a "${OUT_DIR}/summary.txt"
    exit "${RC}"
else
    echo "[PASS] 程序正常退出。" | tee -a "${OUT_DIR}/summary.txt"
    exit 0
fi
