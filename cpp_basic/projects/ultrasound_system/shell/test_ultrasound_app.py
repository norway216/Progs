#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
UltrasoundQmlCppDemo 自动化验证工具

功能：
1. 启动程序并监控是否崩溃
2. 记录 stdout/stderr
3. 采样 CPU / RSS 内存
4. 解析日志中的 FPS 字段，例如：
   FPS: 60
   fps=59.8
   frame rate: 60.1
5. 输出 JSON 报告

用法：
    python3 test_ultrasound_app.py --app ./UltrasoundQmlCppDemo --duration 20
"""

import argparse
import json
import os
import re
import signal
import subprocess
import sys
import time
from pathlib import Path
from datetime import datetime

FPS_RE = re.compile(r"(?:FPS|fps|frame\s*rate)\s*[:=]\s*([0-9]+(?:\.[0-9]+)?)")

def read_proc_stat(pid: int):
    """
    返回简单 CPU tick 信息。
    Linux /proc/[pid]/stat:
    utime 第14项，stime 第15项。
    """
    try:
        text = Path(f"/proc/{pid}/stat").read_text()
        parts = text.split()
        utime = int(parts[13])
        stime = int(parts[14])
        return utime + stime
    except Exception:
        return None

def read_rss_kb(pid: int):
    try:
        for line in Path(f"/proc/{pid}/status").read_text().splitlines():
            if line.startswith("VmRSS:"):
                return int(line.split()[1])
    except Exception:
        return None
    return None

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--app", default="./UltrasoundQmlCppDemo", help="可执行程序路径")
    parser.add_argument("--duration", type=float, default=20.0, help="运行观察时长，单位秒")
    parser.add_argument("--log-dir", default="./test_logs", help="日志输出目录")
    parser.add_argument("--qt-platform", default="xcb", help="Qt 平台插件，建议 xcb；也可尝试 wayland/offscreen")
    parser.add_argument("--target-fps", type=float, default=60.0, help="目标 FPS")
    parser.add_argument("--fps-tolerance", type=float, default=5.0, help="FPS 允许低于目标的容差，例如 5 表示 >=55 通过")
    args = parser.parse_args()

    app = Path(args.app)
    if not app.exists():
        print(f"[FAIL] 程序不存在: {app}", file=sys.stderr)
        return 2

    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    out_dir = Path(args.log_dir) / f"py_run_{stamp}"
    out_dir.mkdir(parents=True, exist_ok=True)

    stdout_path = out_dir / "stdout.log"
    stderr_path = out_dir / "stderr.log"
    report_path = out_dir / "report.json"

    env = os.environ.copy()
    env.setdefault("QT_QPA_PLATFORM", args.qt_platform)
    env.setdefault("QSG_INFO", "1")
    env.setdefault("QT_DEBUG_PLUGINS", "0")
    env.setdefault("QT_LOGGING_RULES", "qt.scenegraph.general=true;qt.qml.warning=true")

    print(f"[INFO] 启动: {app}")
    print(f"[INFO] 日志目录: {out_dir}")
    print(f"[INFO] QT_QPA_PLATFORM={env.get('QT_QPA_PLATFORM')}")

    with stdout_path.open("wb") as out, stderr_path.open("wb") as err:
        proc = subprocess.Popen(
            [str(app)],
            stdout=out,
            stderr=err,
            env=env,
            cwd=str(app.parent if app.parent != Path("") else Path.cwd()),
        )

        cpu_samples = []
        rss_samples = []
        last_ticks = read_proc_stat(proc.pid)
        last_time = time.time()

        deadline = time.time() + args.duration
        crashed = False
        exit_code = None

        while time.time() < deadline:
            exit_code = proc.poll()
            if exit_code is not None:
                crashed = exit_code != 0
                break

            now = time.time()
            ticks = read_proc_stat(proc.pid)
            rss = read_rss_kb(proc.pid)

            if ticks is not None and last_ticks is not None:
                dt = now - last_time
                if dt > 0:
                    clk_tck = os.sysconf(os.sysconf_names["SC_CLK_TCK"])
                    cpu_percent = ((ticks - last_ticks) / clk_tck) / dt * 100.0
                    cpu_samples.append(cpu_percent)

            if rss is not None:
                rss_samples.append(rss)

            last_ticks = ticks
            last_time = now
            time.sleep(1.0)

        if proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=3)

        exit_code = proc.returncode

    stdout_text = stdout_path.read_text(errors="ignore") if stdout_path.exists() else ""
    stderr_text = stderr_path.read_text(errors="ignore") if stderr_path.exists() else ""
    all_log = stdout_text + "\n" + stderr_text

    fps_values = [float(m.group(1)) for m in FPS_RE.finditer(all_log)]
    avg_fps = sum(fps_values) / len(fps_values) if fps_values else None
    min_fps = min(fps_values) if fps_values else None

    # Linux 下：段错误常见返回码是 -11 或 139，取决于启动方式。
    segfault = exit_code in (-signal.SIGSEGV, 139)
    passed_no_crash = (exit_code == 0) or (exit_code in (-signal.SIGTERM, 143))
    fps_pass = None
    if avg_fps is not None:
        fps_pass = avg_fps >= (args.target_fps - args.fps_tolerance)

    report = {
        "app": str(app),
        "duration_sec": args.duration,
        "qt_platform": env.get("QT_QPA_PLATFORM"),
        "exit_code": exit_code,
        "segfault": segfault,
        "passed_no_crash": passed_no_crash and not segfault,
        "cpu_percent_avg": round(sum(cpu_samples) / len(cpu_samples), 2) if cpu_samples else None,
        "cpu_percent_max": round(max(cpu_samples), 2) if cpu_samples else None,
        "rss_mb_avg": round((sum(rss_samples) / len(rss_samples)) / 1024, 2) if rss_samples else None,
        "rss_mb_max": round(max(rss_samples) / 1024, 2) if rss_samples else None,
        "fps_values_count": len(fps_values),
        "fps_avg": round(avg_fps, 2) if avg_fps is not None else None,
        "fps_min": round(min_fps, 2) if min_fps is not None else None,
        "target_fps": args.target_fps,
        "fps_pass": fps_pass,
        "stdout_log": str(stdout_path),
        "stderr_log": str(stderr_path),
    }

    report_path.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8")

    print(json.dumps(report, ensure_ascii=False, indent=2))
    print(f"[DONE] 报告已生成: {report_path}")

    if segfault:
        print("[FAIL] 检测到段错误。建议继续执行 run_gdb_backtrace.sh 获取完整调用栈。")
        return 139
    if not report["passed_no_crash"]:
        print("[FAIL] 程序异常退出。")
        return 1
    if fps_pass is False:
        print("[WARN] 程序未达到目标 FPS。")
        return 4
    print("[PASS] 基础稳定性测试通过。")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
