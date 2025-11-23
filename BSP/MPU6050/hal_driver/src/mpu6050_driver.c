//
// Created by redmiX on 2025/11/21.
//
//******************************** Includes *********************************//
#include "mpu6050_driver.h"

#include "bsp_mpu6050_reg.h"
#include "bsp_mpu6050_reg_bit.h"
#include "elog.h"
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#define IIC_MEMADD_SIZE_8BIT 0x00000001U
#define TIME_OUT_MS          1000
#define MPU6050_NOT_INIT     0
#define MPU6050_INIT         1

#ifdef  LOG_TAG
#undef  LOG_TAG
#define LOG_TAG       "mpu6050_driver"
#else // else of LOG_TAG
#define LOG_TAG       "mpu6050_driver"
#endif // end of LOG_TAG
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
#define MPU6050_WRITE_REG(p_mpu_driver, reg, p_data, len)  \
    p_mpu_driver->p_i2c_driver_interface->pf_i2c_mem_write(\
    p_mpu_driver->p_i2c_driver_interface->hi2c,            \
    ((MPU_ADDR<<1) & 0xFF) | 0,                            \
    reg,                                                   \
    IIC_MEMADD_SIZE_8BIT,                                  \
    p_data,                                                \
    len,                                                   \
    TIME_OUT_MS)

#define MPU6050_READ_REG(p_mpu_driver, reg, p_data, len)   \
    p_mpu_driver->p_i2c_driver_interface->pf_i2c_mem_read( \
    p_mpu_driver->p_i2c_driver_interface->hi2c,            \
    ((MPU_ADDR<<1) & 0xFF) | 1,                            \
    reg,                                                   \
    IIC_MEMADD_SIZE_8BIT,                                  \
    p_data,                                                \
    len,                                                   \
    TIME_OUT_MS)

#define MPU_DEBUG
#ifdef  MPU_DEBUG
#define LOG_DEBUG(x,...)  log_d(x, ##__VA_ARGS__)
#define LOG_ERROR(x,...)  log_e(x, ##__VA_ARGS__)
#else
#define LOG_DEBUG(x,...)   ((void)0)
#define LOG_ERROR(x,...)   ((void)0)
#endif

#define NULL_CHECK(x, tag)                          do{\
if(NULL == x)                                          \
{                                                      \
LOG_ERROR(#x" is null ptr");                           \
goto tag;}                                             \
}while (0)
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//
static double g_gyro_scale = 131.0;   //* 陀螺仪的灵敏度
static double g_accel_scale = 16384.0;//* 加速度计的灵敏度
static uint8_t g_is_init_flag = MPU6050_NOT_INIT;
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//

/**
 * @brief 反初始化MPU驱动
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_deinit(bsp_mpu6050_driver_t *p_mpu6050)
{
    if (MPU6050_NOT_INIT == g_is_init_flag)
    {
        LOG_ERROR("mpu6050 not inited");
        return MPU6050_ERROR;
    }

    g_is_init_flag = MPU6050_NOT_INIT;
    return MPU6050_OK;
}

/**
 * @brief 使MPU进入睡眠模式
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_sleep(bsp_mpu6050_driver_t *p_mpu6050)
{
    g_is_init_flag = MPU6050_NOT_INIT;
    return MPU6050_OK;
}

/**
 * @brief 唤醒MPU6050
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_wakeup(bsp_mpu6050_driver_t *p_mpu6050)
{
    mpu6050_status_t ret = MPU6050_OK;
    uint8_t data = SLEEP_BIT(0);

    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_PWR_MGMT1_REG, &data, 1);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 write MPU_PWR_MGMT1_REG error");
        return ret;
    }
    return ret;
}

/**
 * @brief 设置陀螺仪满量程范围
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @param[in] fsr 陀螺仪满量程设置值
 *          | fsr |   量程范围   | 灵敏度 |
 *             0     正负250度/秒    131
 *             1     正负500度/秒    65.5
 *             2     正负1000度/秒   32.8
 *             3     正负2000度/秒   16.4
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_gyro_fsr(bsp_mpu6050_driver_t *p_mpu6050,
                                                uint8_t fsr)
{
    LOG_DEBUG("=======set gyro fsr=======");
    mpu6050_status_t ret = MPU6050_OK;
    fsr = FS_SEL_BIT(fsr);

    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_GYRO_CFG_REG, &fsr, 1);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 set gyro fsr error");
        return ret;
    }
    switch (fsr)
    {
        case 0: g_gyro_scale = 131.0;break;
        case 1: g_gyro_scale =  65.5;break;
        case 2: g_gyro_scale =  32.8;break;
        case 3: g_gyro_scale =  16.4;break;
        default:g_gyro_scale = 131.0;break;
    }
    return ret;
}

/**
 * @brief 设置加速度计满量程范围
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @param[in] fsr 加速度计满量程设置值
 *                  | fsr|  量程  | 灵敏度 |
 *                  |  0 |正负 2g | 16384 |
 *                  |  1 |正负 4g |  8192 |
 *                  |  2 |正负 8g |  4096 |
 *                  |  3 |正负16g |  2048 |
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_accel_fsr(bsp_mpu6050_driver_t *p_mpu6050,
                                                 uint8_t fsr)
{
    LOG_DEBUG("=======set accel fsr=======");
    mpu6050_status_t ret = MPU6050_OK;
    fsr = AFS_SEL_BIT(fsr);

    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_ACCEL_CFG_REG, &fsr, 1);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 set accel fsr error");
        return ret;
    }
    switch (fsr)
    {
        case 0: g_accel_scale = 16384;break;
        case 1: g_accel_scale =  8192;break;
        case 2: g_accel_scale =  4096;break;
        case 3: g_accel_scale =  2048;break;
        default:g_accel_scale = 16384;break;
    }
    return ret;
}

/**
 * @brief 设置低通滤波器
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @param[in] data 低通滤波器设置值
 *|     |      陀螺仪       |      加速度计     |         |
 *|data |带宽 (Hz)|延迟 (ms)|带宽 (Hz)|延迟 (ms)|陀螺仪输出率|
 *|  0	|  256	 | 0.98	  | 260	   | 0.98	 |  8 kHz  |
 *|  1	|  184	 |  2.9	  | 184	   |  2.9	 |  1 kHz  |
 *|  2	|   92	 |  3.9	  | 92	   |  3.9	 |  1 kHz  |
 *|  3	|   44	 |  4.9	  | 44	   |  4.9	 |  1 kHz  |
 *|  4	|   21	 |  6.1	  | 21	   |  6.1	 |  1 kHz  |
 *|  5	|   10	 |  8.5	  | 10	   |  8.5	 |  1 kHz  |
 *|  6	|    5	 | 13.2	  | 5	   | 13.2	 |  1 kHz  |
 *|  7	| 3600	 | 0.17	  | 44	   | 0.98	 |  8 kHz  |
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_lpf(bsp_mpu6050_driver_t *p_mpu6050,
                                                 uint8_t data)
{
    LOG_DEBUG("=======set lpf=======");
    mpu6050_status_t ret = MPU6050_OK;
    data = DLPF_CFG_BIT(data);

    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_CFG_REG, &data, 1);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 set dlpf error");
        return ret;
    }
    return ret;
}

/**
 * @brief 设置采样率，准确来说是SMPLRT_DIV
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @param[in] data 采样率分频器[SMPLRT_DIV]
 *            采样率 = 陀螺仪输出速率/（1+SMPLRT_DIV）
 * 当DLPF禁用时（DLPF_CFG=0或7），陀螺仪输出速率=8kHz；当DLPF启用时，陀螺仪输出率=1kHz
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_rate(bsp_mpu6050_driver_t *p_mpu6050,
                                                 uint8_t data)
{
    LOG_DEBUG("=======set SMPLRT=======");
    mpu6050_status_t ret = MPU6050_OK;
    data = SMPLRT_DIV_BIT(data);

    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_SAMPLE_RATE_REG, &data, 1);    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 set SMPLRT error");
        return ret;
    }
    return ret;
}

/**
 * @brief 设置中断使能
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @param[in] data 中断使能设置值
 *|data |    中断名称
 *|BIT0	| 控制数据准备中断
 *|BIT1	|    *预留*
 *|BIT2	|  	 *预留*
 *|BIT3	| I2C主机相关中断
 *|BIT4	| FIFO 溢出中断
 *|BIT5	|   控制静态
 *|BIT6	| 检测运动中断
 *|BIT7	| 自由落体中断
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_interrupt_enable(
                                                bsp_mpu6050_driver_t *p_mpu6050,
                                                uint8_t data)
{
    LOG_DEBUG("=======set interrupt_enable=======");
    mpu6050_status_t ret = MPU6050_OK;

    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_INT_EN_REG, &data, 1);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 set interrupt_enable error");
        return ret;
    }
    return ret;
}

/**
 * @brief 设置运动检测阈值
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @param[in] data 运动检测阈值设置值
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_motion_threshold(
                                                bsp_mpu6050_driver_t *p_mpu6050,
                                                uint8_t data)
{
    LOG_DEBUG("=======set motion_threshold=======");
    mpu6050_status_t ret = MPU6050_OK;

    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_MOTION_DET_REG, &data, 1);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 set motion_threshold error");
        return ret;
    }
    return ret;
}

/**
 * @brief 设置中断电平
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @param[in] data 中断电平设置值
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_INT_level(
                                                bsp_mpu6050_driver_t *p_mpu6050,
                                                uint8_t data)
{
    LOG_DEBUG("=======set INT_level=======");
    mpu6050_status_t ret = MPU6050_OK;

    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_INTBP_CFG_REG, &data, 1);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 set INT_level error");
        return ret;
    }
    return ret;
}

/**
 * @brief 设置用户控制寄存器
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @param[in] data 用户控制设置值
 * |---------------------------------------------------------------|
 * |  BIT7     |    BIT6       |   BIT5        |    BIT4    | BIT3 |
 * |-----------|---------------|---------------|------------|------|
 * |   /       |  FIFO_EN      |I2C_MST_EN     | I2C_IF_DIS |  /   |
 * |-----------|---------------|---------------|------------|------|
 * |-----------|---------------|---------------|------------|------|
 * |  BIT2     |    BIT1       |    BIT0       |            |      |
 * |-----------|---------------|---------------|------------|------|
 * |FIFO_RESET | I2C_MST_RESET |SIG_COND_RESET |            |      |
 * |---------------------------------------------------------------|
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_user_ctrl(
                                                bsp_mpu6050_driver_t *p_mpu6050,
                                                uint8_t data)
{
    LOG_DEBUG("=======set user_ctrl=======");
    mpu6050_status_t ret = MPU6050_OK;

    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_USER_CTRL_REG, &data, 1);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 set user_ctrl error");
        return ret;
    }
    return ret;
}

/**
 * @brief 设置电源管理1寄存器
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @param[in] data 电源管理1设置值
 *|------------------------------------ |
 *|BIT7：置位后所有传感器恢复默认值           |
 *|BIT6：置位后进入睡眠模式                 |
 *|BIT5：置位切没有进入睡眠模式则MPU进行循环模式|
 *|BIT4：保留                            |
 *|BIT3：置位禁止温度传感器                 |
 *|BIT[0~2]：配置时钟                     |
 *|-------------------------------------|
 *|BIT[0~2]的值      时钟        |
 *|----------------------------|
 *|    0          内部时钟       |
 *|    1        x轴陀螺仪锁相环   |
 *|    2        Y轴陀螺仪锁相环   |
 *|    3        z轴陀螺仪锁相环   |
 *|    4        外部32.768kHz   |
 *|    5        外部19.2MHz     |
 *|    6            保留        |
 *|    7        关闭所有时钟      |
 *|----------------------------|
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_pwr_mgmt1_reg(
                                                bsp_mpu6050_driver_t *p_mpu6050,
                                                uint8_t data)
{
    LOG_DEBUG("=======set pwr_mgmt1 reg=======");
    mpu6050_status_t ret = MPU6050_OK;

    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_PWR_MGMT1_REG, &data, 1);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 set pwr_mgmt1 reg error");
        return ret;
    }
    return ret;
}

/**
 * @brief 设置电源管理2寄存器
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @param[in] data 电源管理2设置值
 *          置位为休眠，清零为正常工作，
 * BIT7：x轴陀螺仪休眠   BIT4：x轴加速度计休眠
 * BIT6：y轴陀螺仪休眠   BIT3：y轴加速度计休眠
 * BIT5：z轴陀螺仪休眠   BIT2：z轴加速度计休眠
 *仅当传感器处于 “循环睡眠 - 唤醒 ” 模式有效
 * BIT[0,1]     唤醒周期
 *    0         1.25ms
 *    1          2.5ms
 *    2            5ms
 *    3           10ms
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_pwr_mgmt2_reg(
                                                bsp_mpu6050_driver_t *p_mpu6050,
                                                uint8_t data)
{
    LOG_DEBUG("=======set pwr_mgmt2 reg=======");
    mpu6050_status_t ret = MPU6050_OK;

    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_PWR_MGMT2_REG, &data, 1);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 set pwr_mgmt2 reg error");
        return ret;
    }
    return ret;
}

/**
 * @brief 设置FIFO使能寄存器
 * @param[in,out] p_mpuxxx MPU驱动结构体指针
 * @param[in] data FIFO使能设置值
 * Bit7	Bit6	Bit5	Bit4	Bit3	Bit2	Bit1	Bit0
 * 温度	陀螺仪X 陀螺仪Y  陀螺仪Z  加速度计  从设备 2  从设备1 从设备 0
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_set_fifo_en_reg(
                                                bsp_mpu6050_driver_t *p_mpu6050,
                                                uint8_t data)
{
    LOG_DEBUG("=======set pwr fifo_en reg=======");
    mpu6050_status_t ret = MPU6050_OK;

    ret = MPU6050_WRITE_REG(p_mpu6050, MPU_FIFO_EN_REG, &data, 1);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 set fifo_en reg error");
        return ret;
    }
    return ret;
}

/**
 * @brief 获取温度数据
 * @param[in] p_mpuxxx MPU驱动结构体指针
 * @param[out] p_data 温度数据输出指针
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_get_temperature(
                                                bsp_mpu6050_driver_t *p_mpu6050,
                                                mpu6050_data_t *p_data)
{
    LOG_DEBUG("=======get temperature=======");
    mpu6050_status_t ret = MPU6050_OK;
    uint8_t data[2] = {0};
    int16_t temp = 0;

    ret = MPU6050_READ_REG(p_mpu6050, MPU_TEMP_OUTH_REG, data, 2);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 get temperature error");
        return ret;
    }

    temp = ((int16_t)(data[0] << 8) | data[1]);
    p_data->temperature = ((float)temp / 340.0f + 36.53f);
    return ret;
}

/**
 * @brief 获取加速度计数据
 * @param[in] p_mpuxxx MPU驱动结构体指针
 * @param[out] p_data 加速度计数据输出指针
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_get_accel(bsp_mpu6050_driver_t *p_mpu6050,
                                            mpu6050_data_t *p_data)
{
    LOG_DEBUG("=======get accel=======");
    mpu6050_status_t ret = MPU6050_OK;
    uint8_t data[6] = {0};

    ret = MPU6050_READ_REG(p_mpu6050, MPU_ACCEL_XOUTH_REG, data, 6);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 get accel error");
        return ret;
    }

    p_data->accel_x_raw = (int16_t)(data[0]<<8 | data[1]);
    p_data->accel_y_raw = (int16_t)(data[2]<<8 | data[3]);
    p_data->accel_z_raw = (int16_t)(data[4]<<8 | data[5]);

    p_data->ax = (double)(p_data->accel_x_raw/g_accel_scale);
    p_data->ay = (double)(p_data->accel_y_raw/g_accel_scale);
    p_data->az = (double)(p_data->accel_z_raw/g_accel_scale);
    return ret;
}

/**
 * @brief 获取陀螺仪数据
 * @param[in] p_mpuxxx MPU驱动结构体指针
 * @param[out] p_data 陀螺仪数据输出指针
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_get_gyro(bsp_mpu6050_driver_t *p_mpu6050,
                                            mpu6050_data_t *p_data)
{
    LOG_DEBUG("=======get gyro=======");
    mpu6050_status_t ret = MPU6050_OK;
    uint8_t data[6] = {0};

    ret = MPU6050_READ_REG(p_mpu6050, MPU_GYRO_XOUTH_REG, data, 6);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 get gyro error");
        return ret;
    }

    p_data->gyro_x_raw = (int16_t)(data[0]<<8 | data[1]);
    p_data->gyro_y_raw = (int16_t)(data[2]<<8 | data[3]);
    p_data->gyro_z_raw = (int16_t)(data[4]<<8 | data[5]);

    p_data->gx = (double)(p_data->gyro_x_raw/g_gyro_scale);
    p_data->gy = (double)(p_data->gyro_y_raw/g_gyro_scale);
    p_data->gz = (double)(p_data->gyro_z_raw/g_gyro_scale);
    return ret;
}

/**
 * @brief 获取所有传感器数据
 * @param[in] p_mpuxxx MPU驱动结构体指针
 * @param[out] p_data 所有传感器数据输出指针
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_get_all_data(bsp_mpu6050_driver_t *p_mpu6050,
                                            mpu6050_data_t *p_data)
{
    LOG_DEBUG("=======get all data=======");
    mpu6050_status_t ret = MPU6050_OK;
    uint8_t data[14] = {0};
    int16_t temp = 0;

    ret = MPU6050_READ_REG(p_mpu6050, MPU_ACCEL_XOUTH_REG, data, 14);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 get all data error");
        return ret;
    }

    p_data->accel_x_raw = (int16_t)(data[0]<<8 | data[1]);
    p_data->accel_y_raw = (int16_t)(data[2]<<8 | data[3]);
    p_data->accel_z_raw = (int16_t)(data[4]<<8 | data[5]);

    p_data->ax = (double)(p_data->accel_x_raw/g_accel_scale);
    p_data->ay = (double)(p_data->accel_y_raw/g_accel_scale);
    p_data->az = (double)(p_data->accel_z_raw/g_accel_scale);

    temp = ((int16_t)(data[6] << 8) | data[7]);
    p_data->temperature = ((float)temp / 340.0f + 36.53f);

    p_data->gyro_x_raw = (int16_t)(data[8]<<8 | data[9]);
    p_data->gyro_y_raw = (int16_t)(data[10]<<8 | data[11]);
    p_data->gyro_z_raw = (int16_t)(data[12]<<8 | data[13]);

    p_data->gx = (double)(p_data->gyro_x_raw/g_gyro_scale);
    p_data->gy = (double)(p_data->gyro_y_raw/g_gyro_scale);
    p_data->gz = (double)(p_data->gyro_z_raw/g_gyro_scale);
    return ret;
}

/**
 * @brief 获取中断状态寄存器值
 * @param[in] p_mpuxxx MPU驱动结构体指针
 * @param[out] p_data 中断状态寄存器值输出指针
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_get_interrupt_status_reg(
                                        bsp_mpu6050_driver_t *p_mpu6050,
                                        uint8_t *p_data)
{
    LOG_DEBUG("=======get interrupt_status reg=======");
    mpu6050_status_t ret = MPU6050_OK;

    ret = MPU6050_READ_REG(p_mpu6050, MPU_INT_STA_REG, p_data, 1);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 get interrupt_status reg error");
        return ret;
    }
}

/**
 * @brief 读取FIFO数据包
 * @param[in] p_mpuxxx MPU驱动结构体指针
 * @param[out] p_data FIFO数据输出指针
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_read_fifo_packet(
                                        bsp_mpu6050_driver_t *p_mpu6050,
                                        mpu6050_data_t *p_data)
{
    LOG_DEBUG("=======read fifo packet=======");
    mpu6050_status_t ret = MPU6050_OK;
    uint16_t fifo_count = 0;     // FIFO中当前可用的字节总数
    uint16_t fifo_pack_count = 0;// FIFO中包含的完整数据包数量
    uint8_t fifo_buffer[12] = {0};

    ret = MPU6050_READ_REG(p_mpu6050, MPU_FIFO_CNTH_REG, fifo_buffer, 2);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu6050 read_fifo_CNTH error");
        return ret;
    }
    fifo_count = (fifo_buffer[0]<<8) | fifo_buffer[1];
    LOG_DEBUG("mpu6050 read fifo cnt is [%u]",fifo_count);

    if (fifo_count >= 12)
    {
        fifo_pack_count = fifo_count / 12;
        for (uint16_t i = 0; i < fifo_pack_count; i++)
        {
            ret = MPU6050_READ_REG(p_mpu6050, MPU_FIFO_RW_REG, fifo_buffer, 2);
            if (MPU6050_OK != ret)
            {
                LOG_ERROR("mpu6050 fifo read error");
                return ret;
            }
            //拼接加速度数据
            p_data[i].accel_x_raw=(int16_t)(fifo_buffer[0]<<8)|fifo_buffer[1];
            p_data[i].accel_y_raw=(int16_t)(fifo_buffer[2]<<8)|fifo_buffer[3];
            p_data[i].accel_z_raw=(int16_t)(fifo_buffer[4]<<8)|fifo_buffer[5];
            //拼接陀螺仪数据
            p_data[i].gyro_x_raw=(int16_t)(fifo_buffer[6]<<8)|fifo_buffer[7];
            p_data[i].gyro_y_raw=(int16_t)(fifo_buffer[8]<<8)|fifo_buffer[9];
            p_data[i].gyro_z_raw=(int16_t)(fifo_buffer[10]<<8)|fifo_buffer[11];

            //转化为实际物理值
            p_data[i].ax=(double)p_data[i].accel_x_raw/g_accel_scale;
            p_data[i].ay=(double)p_data[i].accel_y_raw/g_accel_scale;
            p_data[i].az=(double)p_data[i].accel_z_raw/g_accel_scale;

            p_data[i].gx=(double)p_data[i].gyro_x_raw/g_gyro_scale;
            p_data[i].gy=(double)p_data[i].gyro_y_raw/g_gyro_scale;
            p_data[i].gz=(double)p_data[i].gyro_z_raw/g_gyro_scale;
        }
    }
    return ret;
}

/**
 * @brief 在ISR中读取FIFO数据
 * @param[in] p_mpuxxx MPU驱动结构体指针
 * @param[out] p_data FIFO数据输出指针
 * @return 执行状态
 */
static mpu6050_status_t mpu_driver_read_fifo_isr_occur(
                                        bsp_mpu6050_driver_t *p_mpu6050,
                                        mpu6050_data_t *p_data)
{
    mpu6050_status_t ret = MPU6050_OK;
    //** can not be used, because the fifo data is not correct**//

    // ret = MPUXXX_WRITE_REG(p_mpuxxx,MPU_MOTION_DET_REG, &data,1);
    // if (MPUxxx_OK!= ret)
    // {
    //     LOG_ERROR("read_fifo_isr set is ng");
    //     return ret;
    // }
    return ret;
}

mpu6050_status_t bsp_mpu6050_driver_inst(
    bsp_mpu6050_driver_t       *p_mpu6050_driver,
    mpu_i2c_driver_interface_t *p_i2c_driver_interface,
#ifdef OS_SUPPORTING
    yield_interface_t          *p_yield_interface,
    os_interface_t             *p_os_interfece,
#endif /* End of OS_SUPPORTING */
    delay_interface_t          *p_delay_interface,
    timebase_interface_t       *p_timebase_interface,
    void (*callback_register)    (void (*callback)(void *, void *)),
    void (*callback_register_dma)(void (*callback)(void *, void *))
#ifdef OS_SUPPORTING
    ,void *queue_handle,
    void *semaphore_handle,
    void *notify_handle
#endif /* End of OS_SUPPORTING */
                                 )
{
    mpu6050_status_t ret = MPU6050_OK;
    /****************************** 检查参数 ********************************/
    NULL_CHECK(p_mpu6050_driver,       mpu_driver_inst_null);
    NULL_CHECK(p_i2c_driver_interface, mpu_driver_inst_null);
#ifdef OS_SUPPORTING
    NULL_CHECK(p_yield_interface, mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece,    mpu_driver_inst_null);
#endif /* End of OS_SUPPORTING */
    NULL_CHECK(p_delay_interface,     mpu_driver_inst_null);
    NULL_CHECK(p_timebase_interface,  mpu_driver_inst_null);
    NULL_CHECK(callback_register,     mpu_driver_inst_null);
    NULL_CHECK(callback_register_dma, mpu_driver_inst_null);
#ifdef OS_SUPPORTING
    NULL_CHECK(queue_handle,     mpu_driver_inst_null);
    NULL_CHECK(semaphore_handle, mpu_driver_inst_null);
    NULL_CHECK(notify_handle,    mpu_driver_inst_null);
#endif /* End of OS_SUPPORTING */
    /****************************** 检查IIC参数 *****************************/
    NULL_CHECK(p_i2c_driver_interface->pf_i2c_init,      mpu_driver_inst_null);
    NULL_CHECK(p_i2c_driver_interface->pf_i2c_deinit,    mpu_driver_inst_null);
    NULL_CHECK(p_i2c_driver_interface->pf_i2c_mem_write, mpu_driver_inst_null);
    NULL_CHECK(p_i2c_driver_interface->pf_i2c_mem_read,  mpu_driver_inst_null);
    NULL_CHECK(p_i2c_driver_interface->pf_i2c_mem_read_dma,
                                                         mpu_driver_inst_null);
    /** 实例化i2c接口 */
    p_mpu6050_driver->p_i2c_driver_interface = p_i2c_driver_interface;
#ifdef OS_SUPPORTING
    /*************************** 检查OS延时参数 *******************************/
    NULL_CHECK(p_yield_interface->pf_rtos_yield, mpu_driver_inst_null);
    /** 实例化os延时接口 */
    p_mpu6050_driver->p_yield_interface = p_yield_interface;
    /**************************** 检查OS参数 ********************************/
    NULL_CHECK(p_os_interfece->os_queue_create,          mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_queue_put,             mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_queue_put_isr,         mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_queue_get,             mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_queue_delete,          mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_create_mutex,mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_delete_mutex,mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_lock_mutex,  mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_unlock_mutex,mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_create_binary,
                                                         mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_delete_binary,
                                                         mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_wait_binary, mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_signal_binary,
                                                         mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_signal_binary_isr,
                                                         mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_signal_notify_isr,
                                                         mpu_driver_inst_null);
    NULL_CHECK(p_os_interfece->os_semaphore_wait_notify, mpu_driver_inst_null);
    /** 实例化os操作接口接口 */
    p_mpu6050_driver->p_os_interface = p_os_interfece;
#endif /* End of OS_SUPPORTING */
    /**************************** 检查阻塞延时 ********************************/
    NULL_CHECK(p_delay_interface->pf_delay_init,         mpu_driver_inst_null);
    NULL_CHECK(p_delay_interface->pf_delay_us,           mpu_driver_inst_null);
    NULL_CHECK(p_delay_interface->pf_delay_ms,           mpu_driver_inst_null);
    /** 实例化阻塞延时接口 */
    p_mpu6050_driver->p_delay_interface = p_delay_interface;
    /*************************** 时基获取接口 *********************************/
    NULL_CHECK(p_timebase_interface->pf_get_tick_count,  mpu_driver_inst_null);
    /** 实例化获取时基接口 */
    p_mpu6050_driver->p_timebase_interface = p_timebase_interface;

    /** 实例化对外提供的接口 */
    p_mpu6050_driver->pf_deinit =                             mpu_driver_deinit;
    p_mpu6050_driver->pf_sleep =                               mpu_driver_sleep;
    p_mpu6050_driver->pf_wakeup =                             mpu_driver_wakeup;
    p_mpu6050_driver->pf_set_gyro_fsr =                 mpu_driver_set_gyro_fsr;
    p_mpu6050_driver->pf_set_accel_fsr =               mpu_driver_set_accel_fsr;
    p_mpu6050_driver->pf_set_lpf =                           mpu_driver_set_lpf;
    p_mpu6050_driver->pf_set_rate =                         mpu_driver_set_rate;
    p_mpu6050_driver->pf_set_interrupt_enable = mpu_driver_set_interrupt_enable;
    p_mpu6050_driver->pf_set_motion_threshold = mpu_driver_set_motion_threshold;
    p_mpu6050_driver->pf_set_INT_level =               mpu_driver_set_INT_level;
    p_mpu6050_driver->pf_set_user_ctrl =               mpu_driver_set_user_ctrl;
    p_mpu6050_driver->pf_set_pwr_mgmt1_reg =       mpu_driver_set_pwr_mgmt1_reg;
    p_mpu6050_driver->pf_set_pwr_mgmt2_reg =       mpu_driver_set_pwr_mgmt2_reg;
    p_mpu6050_driver->pf_set_fifo_en_reg =           mpu_driver_set_fifo_en_reg;
    p_mpu6050_driver->pf_get_temperature =           mpu_driver_get_temperature;
    p_mpu6050_driver->pf_get_accel =                       mpu_driver_get_accel;
    p_mpu6050_driver->pf_get_gyro =                         mpu_driver_get_gyro;
    p_mpu6050_driver->pf_get_all_data =                 mpu_driver_get_all_data;
    p_mpu6050_driver->pf_get_interrupt_status_reg =
                                            mpu_driver_get_interrupt_status_reg;
    p_mpu6050_driver->pf_read_fifo_packet =         mpu_driver_read_fifo_packet;
    p_mpu6050_driver->pf_read_fifo_isr_occur =   mpu_driver_read_fifo_isr_occur;

    p_mpu6050_driver->queue_handle = queue_handle;
    p_mpu6050_driver->semaphore_binary_handle = semaphore_handle;
    p_mpu6050_driver->notify_handle = notify_handle;

    ret = bsp_mpu6050_driver_init(p_mpu6050_driver);
    if (MPU6050_OK != ret)
    {
        LOG_ERROR("mpu init error");
        return MPU6050_ERROR;
    }

    callback_register    (int_interrupt_callback);
    callback_register_dma(dma_interrupt_callback);

mpu_driver_inst_null:
    {
        LOG_ERROR("bsp_mpu6050_driver inst");
        return MPU6050_ERRORPARAMETER;
    }
}