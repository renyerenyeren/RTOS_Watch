cmake_minimum_required(VERSION 3.10)

# 引入 aht21_adapter 子模块
add_subdirectory(03_1-AHT21)
# 引入 imu_adapter 子模块
add_subdirectory(03_2-MPU6050)
# 引入 debug_adapter 子模块
add_subdirectory(03_X-Debug)


# ------------------------------
# 定义 adapter 接口库（用于聚合所有 adapter 组件）
# ------------------------------
# 使用 INTERFACE 关键字，表明这是一个接口库
add_library(adapter INTERFACE)

# 链接模块（公开链接）
# 任何链接了 "adapter" 的目标，都会自动链接
target_link_libraries(adapter INTERFACE
        aht21_adapter
        imu_adapter
        debug_adapter
)

# 暴露 adapter 公共头文件路径（如果 adapter 目录下有顶层的公共头文件）
target_include_directories(adapter INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
)