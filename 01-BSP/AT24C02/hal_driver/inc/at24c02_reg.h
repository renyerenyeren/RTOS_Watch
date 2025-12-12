//
// Created by redmiX on 2025/11/25.
//

#ifndef RTOS_PROJECT_AT24C02_REG_H
#define RTOS_PROJECT_AT24C02_REG_H

#define AT24C02_SLAVE_ADDR_WRITE  0xA0    // 从机写地址（A0/A1/A2接地）
#define AT24C02_SLAVE_ADDR_READ   0xA1    // 从机读地址
#define AT24C02_MAX_ADDR          0xFF    // 最大内存地址（256字节）

// AT24C02 硬件参数
#define AT24C02_TOTAL_BYTES       256
#define AT24C02_TOTAL_PAGES       32      // 总页数（0~31页）
#define AT24C02_PAGE_BYTES        8       // 每页字节数（0~7字节）
#define AT24C02_PAGE_SHIFT        3       // 页地址左移位数（2^3=8，对应每页8字节）
#define AT24C02_MEM_ADDR(PAGE_NUM, BYTE_NUM) \
((uint8_t)(((PAGE_NUM) << AT24C02_PAGE_SHIFT) | ((BYTE_NUM) & 0x07)))

#endif //RTOS_PROJECT_AT24C02_REG_H