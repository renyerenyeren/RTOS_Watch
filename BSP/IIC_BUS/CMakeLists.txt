cmake_minimum_required(VERSION 3.10)

# 定义 IIC 驱动库（整合 driver 和 handler）
add_library(bsp_i2c STATIC)

# 添加 IIC 模块的源文件
target_sources(bsp_i2c PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/i2c_bus.c
)

# 暴露 IIC 模块的头文件路径
target_include_directories(bsp_i2c PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接依赖库（根据实际需求添加，例如硬件驱动库）
target_link_libraries(bsp_i2c PRIVATE
        STM32_Drivers
#        user_debug
)