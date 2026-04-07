cmake_minimum_required(VERSION 3.10)

# 定义 st7789_adapter 驱动库
add_library(st7789_adapter STATIC)

# 添加 st7789_adapter 模块的源文件
target_sources(st7789_adapter PRIVATE
        st7789_system_adaption.c
)

# 暴露头文件路径（关键：对外公开 inc 目录，其他模块可引用）
target_include_directories(st7789_adapter PUBLIC
        .
)

# 链接依赖库（当前模块依赖的库，根据实际需求调整）
target_link_libraries(st7789_adapter PRIVATE
        STM32_Drivers  # 硬件驱动库
        bsp
        libraries
)