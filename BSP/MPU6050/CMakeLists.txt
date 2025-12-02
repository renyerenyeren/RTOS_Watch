cmake_minimum_required(VERSION 3.10)

# 定义 MPU6050 驱动库（整合 driver 和 handler）
add_library(bsp_mpu6050 STATIC)

# 添加 MPU6050 模块的源文件
target_sources(bsp_mpu6050 PRIVATE
        # driver 目录源文件
        ${CMAKE_CURRENT_SOURCE_DIR}/hal_driver/src/mpu6050_driver.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hal_driver/src/circular_buffer.c
        # handler 目录源文件
        ${CMAKE_CURRENT_SOURCE_DIR}/handler/src/imu_handler.c
)

# 暴露 MPU6050 模块的头文件路径
target_include_directories(bsp_mpu6050 PUBLIC
        # driver 目录头文件
        ${CMAKE_CURRENT_SOURCE_DIR}/hal_driver/inc
        # handler 目录头文件
        ${CMAKE_CURRENT_SOURCE_DIR}/handler/inc
)

# 链接依赖库（根据实际需求添加，例如硬件驱动库）
target_link_libraries(bsp_mpu6050 PRIVATE
        STM32_Drivers
        user_debug
)