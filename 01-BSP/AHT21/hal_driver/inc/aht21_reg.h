//
// Created by redmiX on 2025/11/13.
//

#ifndef RTOS_PROJECT_AHT21_REG_H
#define RTOS_PROJECT_AHT21_REG_H

#define AHT21_REG_I2C               0x38  // AHT21的IIC七位地址
#define AHT21_REG_READ_ADDR         0x71  // AHT21 I2C从机读地址（用于读取传感器数据）
#define AHT21_REG_WRITE_ADDR        0x70  // AHT21 I2C从机写地址（用于发送命令/参数）
#define AHT21_REG_MEASURE_CMD       0xAC  // AHT21启动测量的基础命令字（需配合参数使用）
#define AHT21_REG_MEASURE_CMD_ARFS1 0x33  // 测量命令参数1（触发单次测量/唤醒传感器）
#define AHT21_REG_MEASURE_CMD_ARFS2 0x00  // 测量命令参数2（保留位，默认置0）
#define AHT21_REG_SOFT_RESET        0xBA  // 软件复位命令

#endif //RTOS_PROJECT_AHT21_REG_H