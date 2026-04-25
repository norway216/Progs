# Architecture Notes

## Core flow

`PeripheralManager::toggleProbe()` -> `StateManager::postEvent("probeInserted")` -> `RealTimeB` -> `ImageManager::start()` keeps generating frames.

`ImageManager::onTick()` -> `FrameGenerator::generate()` -> `UltrasoundImageProvider::setImage()` -> QML `ImageViewport` refreshes `image://ultrasound/live`.

`ImageManager::onTick()` also pushes each frame into `CineBuffer`, which is used when the state enters `CinePlaying`.

## State flow

- `Idle`
- `RealTimeB`
- `FreezeB`
- `CinePlaying`
- `MeasureActive`
- `SavingImage`

This mirrors the report's major state paths while keeping the demo small enough to understand.

## Rendering route

This demo uses QML `Image` + `QQuickImageProvider`. It is simple and portable. On RK3568 production software, you would typically evaluate:

- QML SceneGraph with EGL/GLES
- `QQuickFramebufferObject`
- direct texture upload from image pipeline
- avoiding `QImage` CPU copies for high-FPS rendering
