cmake_minimum_required(VERSION 3.16)
project(UltrasoundQmlCppDemo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 5.15 COMPONENTS Core Quick QuickControls2 REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/AppController.h src/AppController.cpp
    src/ImageManager.h src/ImageManager.cpp
    src/SonoParameters.h src/SonoParameters.cpp
    src/StateManager.h src/StateManager.cpp
    src/CineBuffer.h src/CineBuffer.cpp
    src/MarkManager.h src/MarkManager.cpp
    src/ExamManager.h src/ExamManager.cpp
    src/PresetManager.h src/PresetManager.cpp
    src/PeripheralManager.h src/PeripheralManager.cpp
    src/UltrasoundImageProvider.h src/UltrasoundImageProvider.cpp
    src/FrameGenerator.h src/FrameGenerator.cpp
    src/qml.qrc
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt5::Core Qt5::Quick Qt5::QuickControls2
)

target_compile_definitions(${PROJECT_NAME} PRIVATE QT_DEPRECATED_WARNINGS)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
