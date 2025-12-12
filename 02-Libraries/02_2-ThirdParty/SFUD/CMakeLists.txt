cmake_minimum_required(VERSION 3.22)

# 定义 SFUD 驱动库
add_library(SfudLib STATIC)

# 添加 SFUD 模块的源文件
target_sources(SfudLib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sfud.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sfud_port.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/sfud_sfdp.c

)

# 暴露 SFUD 模块的头文件路径
target_include_directories(SfudLib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# 链接依赖库（根据实际需求添加，例如硬件驱动库）
target_link_libraries(SfudLib PRIVATE
        STM32_Drivers
)