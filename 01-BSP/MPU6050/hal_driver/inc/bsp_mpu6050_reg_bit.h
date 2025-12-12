//
// Created by redmiX on 2025/11/23.
//

#ifndef RTOS_PROJECT_BSP_MPU6050_REG_BIT_H
#define RTOS_PROJECT_BSP_MPU6050_REG_BIT_

/* MPU_SELF_TEST_X_REG (0x0D) ************************************************/
// Read/Write
#define XA_TEST_BIT(x)     (x << 5)
#define XG_TEST_BIT(x)     (x << 0)

/* MPU_SELF_TEST_Y_REG (0x0E) ************************************************/
// Read/Write
#define YA_TEST_BIT(x)     (x << 5)
#define YG_TEST_BIT(x)     (x << 0)

/* MPU_SELF_TEST_Z_REG (0x0F) ************************************************/
// Read/Write
#define ZA_TEST_BIT(x)     (x << 5)
#define ZG_TEST_BIT(x)     (x << 0)

/* MPU_SELF_TEST_A_REG (0x10) ************************************************/
// Read/Write
#define XA_TEST_BIT1(x)    (x << 4)
#define YA_TEST_BIT1(x)    (x << 2)
#define ZA_TEST_BIT1(x)    (x << 0)

/* MPU_SAMPLE_RATE_REG (0x19) ************************************************/
// Read/Write
// Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
// where Gyroscope Output Rate = 8kHz when the DLPF is disabled (DLPF_CFG = 0 or 7),
// and 1kHz when the DLPF is enabled (DLPF_CFG = 1 or 2)
#define SMPLRT_DIV_BIT(x) (x << 0)

/* MPU_CFG_REG (0x1A) ********************************************************/
// Read/Write
#define DLPF_CFG_BIT(x)     (x << 0)
// --------------------------------------------------------------------
// DLPF_CFG     |     Accelerometer        |             Gyroscope     |
//              |     (Fs = 1kHz)          |                           |
//              |--------------|-----------|---------------------------|
//              | Bandwidth    | Delay     | Bandwidth | Delay | Fs    |
//              | (Hz)         | (ms)      | (Hz)      | (ms)  | (kHz) |
// --------------------------------------------------------------------
// 0            |  260          |   0      |    256    | 0.98  | 8     |
// 1            |  184          |   2      |    188    | 1.9   | 1     |
// 2            |  94           |   3      |    98     | 2.8   | 1     |
// 3            |  44           |   4.9    |    42     | 4.8   | 1     |
// 4            |  21           |   8.5    |    20     | 8.3   | 1     |
// 5            |  10           |   13.8   |    10     | 13.4  | 1     |
// 6            |  5            |   19.0   |    5      | 18.6  | 1     |
// 7            |  RESERVED                | RESERVED          | 8     |
// --------------------------------------------------------------------
#define EXT_SYNC_SET_BIT(x) (x << 3)

/* MPU_GYRO_CFG_REG (0x1B) ***************************************************/
// Read/Write
// 2-bit unsigned value. Selects the full scale range of gyroscopes
// 0 ± 250 °/s
// 1 ± 500 °/s
// 2 ± 1000 °/s
// 3 ± 2000 °/s
#define FS_SEL_BIT(x) (x << 3)
#define ZG_ST_BIT(x)  (x << 5)
#define YG_ST_BIT(x)  (x << 6)
#define XG_ST_BIT(x)  (x << 7)

/* MPU_ACCEL_CFG_REG (0x1C) **************************************************/
// Read/Write
// 2-bit unsigned value. Selects the full scale range of accelerometer
// 0 ± 2g
// 1 ± 4g
// 2 ± 8g
// 3 ± 16g
#define AFS_SEL_BIT(x) (x << 3)
#define ZA_ST_BIT(x)   (x << 5)
#define YA_ST_BIT(x)   (x << 6)
#define XA_ST_BIT(x)   (x << 7)

/* MPU_MOTION_DET_REG(0x1F) **************************************************/
// Read-write register. Specifies the Motion detection threshold
#define MOT_THR_BIT(X) (X << 0)

/* MPU_FIFO_EN_REG (0x23) ****************************************************/
// Read/Write
#define TEMP_FIFO_EN_BIT(x) (x << 7)
#define XG_FIFO_EN_BIT(x)   (x << 6)
#define YG_FIFO_EN_BIT(x)   (x << 5)
#define ZG_FIFO_EN_BIT(x)   (x << 4)
#define ACCEL_FIFO_EN_BIT(x)(x << 3)
#define SLV2_FIFO_EN_BIT(x) (x << 2)
#define SLV1_FIFO_EN_BIT(x) (x << 1)
#define SLV0_FIFO_EN_BIT(x) (x << 0)

/* MPU_I2C_MST_CTRL_REG (0x24) ***********************************************/
// Read/Write
#define MULT_MST_EN_BIT(x)  (x << 7)
#define WAIT_FOR_ES_BIT(x)  (x << 6)
#define SLV3_FIFO_EN_BIT(x) (x << 5)
#define I2C_MST_P_NSR_BIT(x)(x << 4)
#define I2C_MST_CLK_BIT(x)  (x << 0)

/* MPU_I2C_SLV0_ADDR_REG (0x25) **********************************************/
// Read/Write
#define I2C_SLV0_RW_BIT(x)  (x << 7)
#define I2C_SLV0_ADDR_BIT(x)(x << 0)

/* MPU_I2C_SLV0_REG_REG (0x26) ***********************************************/
// Read/Write
#define I2C_SLV0_REG_BIT(x) (x << 0)

/* MPU_I2C_SLV0_CTRL_REG (0x27) **********************************************/
// Read/Write
#define I2C_SLV0_EN_BIT(x)  (x << 7)
#define I2C_SLV0_BYTE_SW_BIT(x)(x << 6)
#define I2C_SLV0_REG_DIS_BIT(x)(x << 5)
#define I2C_SLV0_GRP_BIT(x) (x << 4)
#define I2C_SLV0_LEN_BIT(x) (x << 0)

/* MPU_I2C_SLV1_ADDR_REG (0x28) **********************************************/
// Read/Write
#define I2C_SLV1_RW_BIT(x)  (x << 7)
#define I2C_SLV1_ADDR_BIT(x)(x << 0)

/* MPU_I2C_SLV1_REG_REG (0x29) ***********************************************/
// Read/Write
#define I2C_SLV1_REG_BIT(x) (x << 0)

/* MPU_I2C_SLV1_CTRL_REG (0x2A) **********************************************/
// Read/Write
#define I2C_SLV1_EN_BIT(x)  (x << 7)
#define I2C_SLV1_BYTE_SW_BIT(x)(x << 6)
#define I2C_SLV1_REG_DIS_BIT(x)(x << 5)
#define I2C_SLV1_GRP_BIT(x) (x << 4)
#define I2C_SLV1_LEN_BIT(x) (x << 0)

/* MPU_I2C_SLV2_ADDR_REG (0x2B) **********************************************/
// Read/Write
#define I2C_SLV2_RW_BIT(x)  (x << 7)
#define I2C_SLV2_ADDR_BIT(x)(x << 0)

/* MPU_I2C_SLV2_REG_REG (0x2C) ***********************************************/
// Read/Write
#define I2C_SLV2_REG_BIT(x) (x << 0)

/* MPU_I2C_SLV2_CTRL_REG (0x2D) **********************************************/
// Read/Write
#define I2C_SLV2_EN_BIT(x)  (x << 7)
#define I2C_SLV2_BYTE_SW_BIT(x)(x << 6)
#define I2C_SLV2_REG_DIS_BIT(x)(x << 5)
#define I2C_SLV2_GRP_BIT(x) (x << 4)
#define I2C_SLV2_LEN_BIT(x) (x << 0)

/* MPU_I2C_SLV3_ADDR_REG (0x2E) **********************************************/
// Read/Write
#define I2C_SLV3_RW_BIT(x)  (x << 7)
#define I2C_SLV3_ADDR_BIT(x)(x << 0)

/* MPU_I2C_SLV3_REG_REG (0x2F) ***********************************************/
// Read/Write
#define I2C_SLV3_REG_BIT(x) (x << 0)

/* MPU_I2C_SLV3_CTRL_REG (0x30) **********************************************/
// Read/Write
#define I2C_SLV3_EN_BIT(x)  (x << 7)
#define I2C_SLV3_BYTE_SW_BIT(x)(x << 6)
#define I2C_SLV3_REG_DIS_BIT(x)(x << 5)
#define I2C_SLV3_GRP_BIT(x) (x << 4)
#define I2C_SLV3_LEN_BIT(x) (x << 0)

/* MPU_I2C_SLV4_ADDR_REG (0x31) **********************************************/
// Read/Write
#define I2C_SLV4_RW_BIT(x)  (x << 7)
#define I2C_SLV4_ADDR_BIT(x)(x << 0)

/* MPU_I2C_SLV4_REG_REG (0x32) ***********************************************/
// Read/Write
#define I2C_SLV4_REG_BIT(x) (x << 0)

/* MPU_I2C_SLV4_DO_REG (0x33) ************************************************/
// Read/Write
#define I2C_SLV4_DO_BIT(x)  (x << 0)

/* MPU_I2C_SLV4_CTRL_REG (0x34) **********************************************/
// Read/Write
#define I2C_SLV4_EN_BIT(x)  (x << 7)
#define I2C_SLV4_INT_EN_BIT(x)(x << 6)
#define I2C_SLV4_REG_DIS_BIT(x)(x << 5)
#define I2C_MST_DLY_BIT(x) (x << 0)

/* MPU_I2C_SLV4_DI_REG (0x35) ************************************************/
// Read/Write
#define I2C_SLV4_DI_BIT(x)  (x << 0)

/* MPU_I2C_MST_STATUS_REG (0x36) *********************************************/
// Read Only
#define PASS_THROUGH_BIT    (1 << 7)
#define I2C_SLV4_DONE_BIT   (1 << 6)
#define I2C_LOST_ARB_BIT    (1 << 5)
#define I2C_SLV4_NACK_BIT   (1 << 4)
#define I2C_SLV3_NACK_BIT   (1 << 3)
#define I2C_SLV2_NACK_BIT   (1 << 2)
#define I2C_SLV1_NACK_BIT   (1 << 1)
#define I2C_SLV0_NACK_BIT   (1 << 0)

/* MPU_INTBP_CFG_REG (0x37) **************************************************/
// Read-write register. INT Pin / Bypass Enable Configuration
// INT_LEVEL       When this bit is equal to 0, the logic level for the INT pin is active high.
//                 When this bit is equal to 1, the logic level for the INT pin is active low.
// INT_OPEN        When this bit is equal to 0, the INT pin is configured as push-pull.
//                 When this bit is equal to 1, the INT pin is configured as open drain.
// LATCH_INT_EN    When this bit is equal to 0, the INT pin emits a 50us long pulse.
//                 When this bit is equal to 1, the INT pin is held high until the interrupt is cleared.
// INT_RD_CLEAR    When this bit is equal to 0, interrupt status bits are cleared only by reading INT_STATUS
//                 When this bit is equal to 1, interrupt status bits are cleared on any read operation.
// FSYNC_INT_LEVEL When this bit is equal to 0, the logic level for the FSYNC pin (when used as
//                 an interrupt to the host processor) is active high.
//                 When this bit is equal to 1, the logic level for the FSYNC pin (when used as
//                 an interrupt to the host processor) is active low.
// FSYNC_INT_EN    When equal to 0, this bit disables the FSYNC pin from causing an interrupt to
//                 the host processor.
//                 When equal to 1, this bit enables the FSYNC pin to be used as an interrupt to
//                 the host processor.
#define I2C_BYPASS_EN_BIT(X)   (X << 1)
#define FSYNC_INT_EN_BIT(X)    (X << 2)
#define FSYNC_INT_LEVEL_BIT(X) (X << 3)
#define INT_RD_CLEAR_BIT(X)    (X << 4)
#define LATCH_INT_EN_BIT(X)    (X << 5)
#define INT_OPEN_BIT(X)        (X << 6)
#define INT_LEVEL_BIT(X)       (X << 7)


/* MPU_INT_EN_REG (0x38) *****************************************************/
// Read-write register. Enables the interrupt
// MOT_EN          When set to 1, this bit enables Motion detection to generate an interrupt.
// FIFO_OFLOW_EN   When set to 1, this bit enables a FIFO buffer overflow to generate an interrupt.
// I2C_MST_INT_EN  When set to 1, this bit enables any of the I2C Master interrupt sources to
//                 generate an interrupt.
// DATA_RDY_EN     When set to 1, this bit enables the Data Ready interrupt, which occurs each
//                 time a write operation to all of the sensor registers has been completed.
#define CLOSE_ALL              (0X00)
#define DATA_RDY_EN_BIT(x)      (x << 0)
#define I2C_MST_INT_EN_BIT(x)   (x << 3)
#define FIFO_OVERFLOW_EN_BIT(x) (x << 4)
#define MOT_EN_BIT(x)           (x << 6)

/* MPU_INT_STA_REG(0x3A) *****************************************************/
// Read-only register. Indicates the status of the interrupt
#define DATA_RDY_INT_BIT      (1 << 0)
#define I2C_MST_INT_BIT       (1 << 3)
#define FIFO_OVERFLOW_INT_BIT (1 << 4)
#define MOT_INT_BIT           (1 << 6)

/* MPU_ACCEL_XOUT_H_REG (0x3B) ***********************************************/
/* MPU_ACCEL_XOUT_L_REG (0x3C) ***********************************************/
/* MPU_ACCEL_YOUT_H_REG (0x3D) ***********************************************/
/* MPU_ACCEL_YOUT_L_REG (0x3E) ***********************************************/
/* MPU_ACCEL_ZOUT_H_REG (0x3F) ***********************************************/
/* MPU_ACCEL_ZOUT_L_REG (0x40) ***********************************************/
/* MPU_TEMP_OUT_H_REG (0x41) *************************************************/
/* MPU_TEMP_OUT_L_REG (0x42) *************************************************/
/* MPU_GYRO_XOUT_H_REG (0x43) ************************************************/
/* MPU_GYRO_XOUT_L_REG (0x44) ************************************************/
/* MPU_GYRO_YOUT_H_REG (0x45) ************************************************/
/* MPU_GYRO_YOUT_L_REG (0x46) ************************************************/
/* MPU_GYRO_ZOUT_H_REG (0x47) ************************************************/
/* MPU_GYRO_ZOUT_L_REG (0x48) ************************************************/

/* MPU_I2CMST_DELAY_REG(0x67) ************************************************/
// Read/Write
#define I2C_SELV0_DLY_EN_BIT(x) (x << 0)
#define I2C_SELV1_DLY_EN_BIT(x) (x << 1)
#define I2C_SELV2_DLY_EN_BIT(x) (x << 2)
#define I2C_SELV3_DLY_EN_BIT(x) (x << 3)
#define I2C_SELV4_DLY_EN_BIT(x) (x << 4)
#define DELAY_ES_SHADOW_BIT(x)  (x << 7)

/* MPU_SIGPATH_RST_REG(0x68) *************************************************/
// Write Only
#define TEMP_RESET_BIT(x)  (x << 0)
#define ACCEL_RESET_BIT(x) (x << 1)
#define GYRO_RESET_BIT(x)  (x << 2)

/* MPU_MDETECT_CTRL_REG(0x69) ************************************************/
// Read/Write
#define ACCEL_ON_DELAY_BIT(x) (x << 4)

/* MPU_USER_CTRL_REG(0x6A) ***************************************************/
// Read/Write
#define SIG_COND_RESET_BIT(x) (x << 0)
#define I2C_MST_RESET_BIT(x)  (x << 1)
#define FIFO_RESET_BIT(x)     (x << 2)
#define I2C_IF_DIS_BIT(x)     (x << 4)
#define I2C_MST_EN_BIT(x)     (x << 5)
#define FIFO_EN_BIT(x)        (x << 6)

/* MPU_PWR_MGMT1_REG (0x6B) **************************************************/
#define DEVICE_RESET_BIT(x)  (x << 7)
#define SLEEP_BIT(x)         (x << 6)
#define CYCLE_BIT(x)         (x << 5)
#define TEMP_DIS_BIT(x)      (x << 3)
// bit 0-2: CLKSEL 3-bit unsigned value. Specifies the clock source of the device.
// CLKSEL     Clock Source
// 0          Internal 8MHz oscillator
// 1          PLL with X axis gyroscope reference
// 2          PLL with Y axis gyroscope reference
// 3          PLL with Z axis gyroscope reference
// 4          PLL with external 32.768kHz reference
// 5          PLL with external 19.2MHz reference
// 6          Reserved
// 7          Stops the clock and keeps the timing generator in reset
#define CLKSEL_BIT(x)        (x << 0)

/* MPU_PWR_MGMT2_REG (0x6C) **************************************************/
// Read/Write
// LP_WAKE_CTRL  2-bit unsigned value.
//               Specifies the frequency of wake-ups during Accelerometer Only Low Power Mode.
// LP_WAKE_CTRL  Wake-up Frequency
//    0             1.25 Hz
//    1             5 Hz
//    2             20 Hz
//    3             40 Hz
// STBY_XA       When set to 1, this bit puts the X axis accelerometer into standby mode.
// STBY_YA       When set to 1, this bit puts the Y axis accelerometer into standby mode.
// STBY_ZA       When set to 1, this bit puts the Z axis accelerometer into standby mode.
// STBY_XG       When set to 1, this bit puts the X axis gyroscope into standby mode.
// STBY_YG       When set to 1, this bit puts the Y axis gyroscope into standby mode.
// STBY_ZG       When set to 1, this bit puts the Z axis gyroscope into standby mode
#define LP_WAKE_CTRL_BIT(x) (x << 6)
#define STBY_XA_BIT(x)      (x << 5)
#define STBY_YA_BIT(x)      (x << 4)
#define STBY_ZA_BIT(x)      (x << 3)
#define STBY_XG_BIT(x)      (x << 2)
#define STBY_YG_BIT(x)      (x << 1)
#define STBY_ZG_BIT(x)      (x << 0)

#endif //RTOS_PROJECT_BSP_MPU6050_REG_BIT_H