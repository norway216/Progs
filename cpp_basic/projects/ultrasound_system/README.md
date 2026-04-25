# Ultrasound QML + C++17 Demo

这是根据 `xunit_functional_analysis.xlsx` 中的功能汇总报告整理出的一个可运行超声软件 Demo。它不是医用级算法实现，而是一个工程骨架：UI 使用 QML，业务逻辑使用 C++17，模拟报告中的启动、探头切换、扫描模式切换、冻结/解冻、Cine 回放、参数预设、测量标注、患者登记、DICOM/PACS、USB/打印等链路。

## 1. 技术栈

- C++17
- Qt 5.15.x
- QML / Qt Quick Controls 2
- CMake 3.16+
- 推荐平台：Ubuntu / Debian / RK3568 Linux 均可移植

## 2. 对应报告中的模块

| 报告模块 | Demo 类 | 说明 |
|---|---|---|
| app/program | `AppController`, `StateManager` | 主控、状态机、日志聚合 |
| imaging | `ImageManager`, `FrameGenerator`, `UltrasoundImageProvider` | 模拟实时成像、模式切换、冻结、渲染帧供应 |
| interface/imaging/buffer | `CineBuffer` | 电影回放循环缓存 |
| preset | `PresetManager`, `SonoParameters` | 探头/应用预设和参数批量更新 |
| mark | `MarkManager` | 距离测量、体标/注释结果 |
| exam | `ExamManager` | 患者登记、保存、PACS 发送模拟 |
| peripheral | `PeripheralManager` | 探头插拔、USB、打印模拟 |
| QML UI | `qml/main.qml`, `qml/components/*` | 主界面、按钮、参数面板、CineBar、图像视窗 |

## 3. 编译运行

```bash
sudo apt update
sudo apt install -y build-essential cmake qtbase5-dev qtdeclarative5-dev qml-module-qtquick-controls2 qml-module-qtquick-layouts qml-module-qtquick2

cd UltrasoundQmlCppDemo
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/UltrasoundQmlCppDemo
```

## 4. 操作说明

1. 点击 `Insert Probe`，模拟探头插入，状态从 `Idle` 进入 `RealTimeB`。
2. 点击 `B/M/Color/PW/Triplex` 等按钮，触发 `ImageManager::switchScanMode()`，模拟 Updater 切换。
3. 点击 `Freeze` / `Unfreeze`，模拟冻结/解冻链路。
4. 冻结后点击 `Play Cine`，拖动 Cine 滑条查看缓存帧。
5. 调节 Depth/Gain/Dynamic Range/Frequency，触发 `SonoParameters` 更新并刷新成像。
6. 点击 `ABD/CARD/VAS`，模拟预设切换。
7. 点击 `Distance` 和 `Simulate Distance Measurement`，模拟测量链路。
8. 点击 `Patient` 录入患者，点击 `Save` / `PACS` 模拟归档和 DICOM 传输。

## 5. 设计重点

- QML 只做 UI 和交互绑定，不直接写业务状态。
- C++ 侧集中管理状态、参数、帧缓存、成像模拟、测量和检查业务。
- `QQuickImageProvider` 用于把 C++ 生成的 `QImage` 推给 QML 图像视窗。
- `CineBuffer` 采用容量限制，避免无限增长。
- `StateManager::postEvent()` 模拟报告中的 SCXML 状态事件流。
- `ImageManager::switchScanMode()` 模拟报告中的 `freeze -> updater切换 -> context更新 -> unfreeze` 链路，并用 120ms 延迟模拟异步切换。

## 6. 后续可扩展方向

- 将 `FrameGenerator` 替换为真实 RF/Beamformer/Scan Conversion 数据源。
- 将 `QImageProvider` 替换为 `QQuickFramebufferObject` 或 OpenGL Texture 渲染，减少 CPU 拷贝。
- 将 `CineBuffer` 改为无锁环形队列或双缓冲结构。
- 将 `StateManager` 替换为 Qt SCXML 或自定义状态表。
- 将 `ExamManager` 对接 SQLite/DICOM/DCMTK/PACS。
- 将 `PeripheralManager` 对接真实 GPIO/I2C/SPI/USB/打印机。
