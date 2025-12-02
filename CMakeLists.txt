cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME RTOS_project)


# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})


# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# ------------------------------
# 新增：引入 BSP 模块
# ------------------------------
add_subdirectory(BSP)
# ------------------------------
# 新增：引入 System 模块
# ------------------------------
add_subdirectory(System)
# ------------------------------
# 新增：引入 user_lib 模块
# ------------------------------
add_subdirectory(user_lib)
# ------------------------------
# 新增：引入 Debug 模块
# ------------------------------
add_subdirectory(Debug)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
        ./task/unpack/unpack_task.c
        ./task/log/log_task.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
        ./task/unpack
        ./task/log
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

target_link_libraries(FreeRTOS PUBLIC SEGGER)
# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
        # ------------------------------
        # 新增：链接 BSP 库
        # ------------------------------
        bsp
        # ------------------------------
        # 新增：链接 System 库
        # ------------------------------
        system_adaption
        # ------------------------------
        # 新增：链接 user_debug 库
        # ------------------------------
        user_debug
        # ------------------------------
        # 新增：链接 user_lib 库
        # ------------------------------
        user_lib
)
