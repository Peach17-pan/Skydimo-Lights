cmake_minimum_required(VERSION 3.10)
project(AppStateMonitor)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加源文件
add_executable(app_state_monitor
    main.cpp
    window_monitor.cpp
    app_classifier.cpp
    rule_engine.cpp
)

# 链接Windows库
target_link_libraries(app_state_monitor
    psapi
)

# 设置编译选项
if(MSVC)
    # MSVC编译器选项
    target_compile_options(app_state_monitor PRIVATE
        /W4           # 警告级别4
        /utf-8        # 使用UTF-8编码
    )
else()
    # GCC/Clang编译器选项
    target_compile_options(app_state_monitor PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

