cmake_minimum_required(VERSION 3.10)

# 定义 ST7789 驱动库
add_library(bsp_st7789 STATIC)

# 添加 ST7789 模块的源文件
target_sources(bsp_st7789 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/st7789_driver.c
)

# 暴露 ST7789 模块的头文件路径
target_include_directories(bsp_st7789 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# 链接依赖库（根据实际需求添加，例如硬件驱动库）
target_link_libraries(bsp_st7789 PRIVATE

)