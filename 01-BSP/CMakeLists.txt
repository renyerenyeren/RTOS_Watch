cmake_minimum_required(VERSION 3.10)

# 引入 AHT21 子模块
add_subdirectory(AHT21)
# 引入 MPU6050 子模块
add_subdirectory(MPU6050)

# ------------------------------
# 定义 BSP 接口库（用于聚合所有 BSP 组件）
# ------------------------------
# 使用 INTERFACE 关键字，表明这是一个接口库
add_library(bsp INTERFACE)

# 链接 LED 模块（公开链接）
# 任何链接了 "bsp" 的目标，都会自动链接
target_link_libraries(bsp INTERFACE
        bsp_aht21
        bsp_mpu6050
)

# 暴露 BSP 公共头文件路径（如果 BSP 目录下有顶层的公共头文件）
 target_include_directories(bsp INTERFACE
     ${CMAKE_CURRENT_SOURCE_DIR}/inc
 )