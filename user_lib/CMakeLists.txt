cmake_minimum_required(VERSION 3.10)

# 定义 user_lib 驱动库（模块名：user_lib）
add_library(user_lib STATIC)

# 添加 user_lib 模块的源文件
target_sources(user_lib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/DWT_delay.c
)

# 暴露头文件路径（关键：对外公开 inc 目录，其他模块可引用）
# PUBLIC 表示：自身模块 + 依赖该模块的外部模块都能访问该头文件目录
target_include_directories(user_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc  # 头文件所在目录（对外暴露的核心）
)

# 链接依赖库（当前模块依赖的库，根据实际需求调整）
# 注意：依赖库必须是已定义的（如 STM32_Drivers、bsp、user_debug）
target_link_libraries(user_lib PRIVATE
        STM32_Drivers  # 硬件驱动库
        bsp            # BSP 聚合库
        user_debug     # 调试库
)