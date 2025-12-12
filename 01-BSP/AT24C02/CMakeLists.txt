cmake_minimum_required(VERSION 3.10)

# 定义 AT24C02 驱动库（整合 driver 和 handler）
add_library(bsp_at24c02 STATIC)

# 添加 AT24C02 模块的源文件
target_sources(bsp_at24c02 PRIVATE
        # driver 目录源文件
        ${CMAKE_CURRENT_SOURCE_DIR}/hal_driver/src/at24c02_driver.c
        # handler 目录源文件
        ${CMAKE_CURRENT_SOURCE_DIR}/handler/src/e2prom_handler.c
)

# 暴露 AT24C02 模块的头文件路径
target_include_directories(bsp_at24c02 PUBLIC
        # driver 目录头文件
        ${CMAKE_CURRENT_SOURCE_DIR}/hal_driver/inc
        # handler 目录头文件
        ${CMAKE_CURRENT_SOURCE_DIR}/handler/inc
)

# 链接依赖库（根据实际需求添加，例如硬件驱动库）
target_link_libraries(bsp_at24c02 PRIVATE
        STM32_Drivers
        libraries
)