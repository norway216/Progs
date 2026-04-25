# UltrasoundQmlCppDemo 自动化测试工具

## 1. 文件说明

- `run_smoke_test.sh`：基础冒烟测试，启动程序并观察是否崩溃。
- `run_gdb_backtrace.sh`：使用 gdb 运行程序，崩溃后自动导出完整调用栈。
- `test_ultrasound_app.py`：Python 自动化测试工具，采样 CPU、内存，解析 FPS 日志，并生成 JSON 报告。
- `run_asan_env.sh`：ASAN/UBSAN 运行环境脚本，配合带 sanitizer 的编译版本使用。

## 2. 推荐使用顺序

```bash
chmod +x run_smoke_test.sh run_gdb_backtrace.sh run_asan_env.sh test_ultrasound_app.py

# 第一步：基础运行测试
./run_smoke_test.sh ./UltrasoundQmlCppDemo

# 第二步：如果发生段错误，生成调用栈
./run_gdb_backtrace.sh ./UltrasoundQmlCppDemo

# 第三步：Python 自动化测试，生成 JSON 报告
python3 test_ultrasound_app.py --app ./UltrasoundQmlCppDemo --duration 20 --target-fps 60
```

## 3. 关于你当前的 Wayland 提示

你看到的：

```text
Warning: Ignoring XDG_SESSION_TYPE=wayland on Gnome. Use QT_QPA_PLATFORM=wayland to run on Wayland anyway.
段错误 (核心已转储)
```

前半句通常只是 Qt 的平台提示，不一定是崩溃原因。  
Qt5 + QML 在 Ubuntu/Gnome/Wayland 下建议先用：

```bash
export QT_QPA_PLATFORM=xcb
```

如果 xcb 也崩溃，再用 gdb 看具体 C++ 调用栈。

## 4. 如果要进一步定位段错误

建议用 Debug 模式重新编译：

```bash
cmake -S .. -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug -j$(nproc)
cd build-debug
../run_gdb_backtrace.sh ./UltrasoundQmlCppDemo
```

如果怀疑内存越界、野指针、重复释放，建议开启 ASAN：

```bash
cmake -S .. -B build-asan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -g" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"

cmake --build build-asan -j$(nproc)
cd build-asan
../run_asan_env.sh ./UltrasoundQmlCppDemo
```

## 5. FPS 日志要求

Python 脚本会自动解析以下格式：

```text
FPS: 60
fps=59.8
frame rate: 60.1
```

如果你的程序当前没有打印 FPS，可以在 C++/QML 渲染循环中每秒打印一次 FPS。
