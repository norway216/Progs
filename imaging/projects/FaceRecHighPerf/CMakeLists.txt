cmake_minimum_required(VERSION 3.10)
project(FaceRecHighPerfDemo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)
find_package(OpenCV REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    FaceRecognition.cpp
    FaceRecognition.h
    ThreadPool.h
    ObjectPool.h
)

target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick
    ${OpenCV_LIBS}
)