#include "i2c_bus.h"

/**
 * @brief 微秒级延时函数
 * 通过循环执行NOP指令实现精确延时。根据不同的编译器选择相应的内联汇编语法。
 * @param us 延时时间（单位：微秒）
 * @return 无返回值
 */
static void delay_us(uint32_t us) {
	volatile uint32_t count = us * 25;  // 保持原计算逻辑

	while (count--) {
		// 根据编译器切换内联汇编语法
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
		__asm volatile ("nop");   // Keil专用
#elif defined(__GNUC__) || defined(__clang__)
		__asm__ volatile ("nop"); // GCC/Clang专用
#else
#error "Unsupported compiler for inline asm!"
#endif
	}
}

/**
 * @brief 配置I2C数据线(SDA)为输出模式
 * 该函数将指定I2C总线的数据线引脚配置为开漏输出模式，
 * 并启用上拉电阻，用于I2C通信中的数据传输控制。
 * @param bus 指向I2C总线配置结构体的指针，包含SDA引脚和端口信息
 * @return 无返回值
 */
void SDA_Output_Mode(const i2c_bus_t *bus)
{
    /* 初始化GPIO结构体 */
    GPIO_InitTypeDef GPIO_Init_Structure = {0};
    GPIO_Init_Structure.Pin = bus->I2C_SDA_PIN;
    GPIO_Init_Structure.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_Init_Structure.Pull = GPIO_PULLUP;
    GPIO_Init_Structure.Speed = GPIO_SPEED_FREQ_HIGH;

    /* 配置SDA引脚为开漏输出模式 */
    HAL_GPIO_Init(bus->I2C_SDA_PORT, &GPIO_Init_Structure);
}

/**
 * @brief 配置I2C数据线(SDA)为输入模式
 * 将指定I2C总线的数据线引脚设置为输入模式，并启用上拉电阻。
 * @param bus 指向I2C总线配置结构体的指针，包含SDA引脚和端口信息
 * @return 无返回值
 */
void SDA_Input_Mode(const i2c_bus_t *bus)
{
    GPIO_InitTypeDef GPIO_Init_Structure = {0};
    GPIO_Init_Structure.Pin = bus->I2C_SDA_PIN;
    GPIO_Init_Structure.Mode = GPIO_MODE_INPUT;
    GPIO_Init_Structure.Pull = GPIO_PULLUP;
    GPIO_Init_Structure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(bus->I2C_SDA_PORT, &GPIO_Init_Structure);
}

/**
 * @brief 在SDA线上输出一个位
 * 控制SDA引脚输出高电平或低电平。
 * @param bus 指向I2C总线配置结构体的指针
 * @param val 要输出的值（非零表示高电平，0表示低电平）
 * @return 无返回值
 */
void SDA_Output(const i2c_bus_t *bus, uint16_t val)
{
    if ( val )
    {
        bus->I2C_SDA_PORT->BSRR |= bus->I2C_SDA_PIN;
    }
    else
    {
        bus->I2C_SDA_PORT->BSRR = (uint32_t)bus->I2C_SDA_PIN << 16U;
    }
}

/**
 * @brief 在SCL线上输出一个位
 * 控制SCL引脚输出高电平或低电平。
 * @param bus 指向I2C总线配置结构体的指针
 * @param val 要输出的值（非零表示高电平，0表示低电平）
 * @return 无返回值
 */
void SCL_Output(const i2c_bus_t *bus, uint16_t val)
{
    if ( val )
    {
        bus->I2C_SCL_PORT->BSRR |= bus->I2C_SCL_PIN;
    }
    else
    {
         bus->I2C_SCL_PORT->BSRR = (uint32_t)bus->I2C_SCL_PIN << 16U;
    }
}

/**
 * @brief 从SDA线读取一个位
 * 读取当前SDA引脚的状态并返回对应的逻辑值。
 * @param bus 指向I2C总线配置结构体的指针
 * @return 返回读取到的位（1 表示高电平，0 表示低电平）
 */
uint8_t SDA_Input(const i2c_bus_t *bus)
{
	if(HAL_GPIO_ReadPin(bus->I2C_SDA_PORT, bus->I2C_SDA_PIN) == GPIO_PIN_SET){
		return 1;
	}else{
		return 0;
	}
}

/**
 * @brief 发送I2C起始信号
 * 在SCL为高电平时拉低SDA，产生I2C起始条件。
 * @param bus 指向I2C总线配置结构体的指针
 * @return 无返回值
 */
void I2CStart(const i2c_bus_t *bus)
{
    SDA_Output(bus,1);
		delay_us(2);
    SCL_Output(bus,1);
		delay_us(1);
    SDA_Output(bus,0);
		delay_us(1);
    SCL_Output(bus,0);
		delay_us(1);
}

/**
 * @brief 发送I2C停止信号
 * 在SCL为高电平时释放SDA，产生I2C停止条件。
 * @param bus 指向I2C总线配置结构体的指针
 * @return 无返回值
 */
void I2CStop(const i2c_bus_t*bus)
{
    SCL_Output(bus,0);
		delay_us(2);
    SDA_Output(bus,0);
		delay_us(1);
    SCL_Output(bus,1);
		delay_us(1);
    SDA_Output(bus,1);
		delay_us(1);

}

/**
 * @brief 等待I2C ACK响应信号
 * 设置SDA为输入模式后检测是否接收到设备发出的ACK信号。
 * 若超时未收到则发送STOP信号并返回错误状态。
 * @param bus 指向I2C总线配置结构体的指针
 * @return 成功收到ACK返回SUCCESS，否则返回ERROR
 */
unsigned char I2CWaitAck(const i2c_bus_t *bus)
{
    unsigned short cErrTime = 5;

	// unsigned short cErrTime = 20;

	SDA_Input_Mode(bus);
    SCL_Output(bus,1);
    while(SDA_Input(bus))
    {
        cErrTime--;
				delay_us(1);
        if (0 == cErrTime)
        {
            SDA_Output_Mode(bus);
            I2CStop(bus);
            return ERROR;
        }
    }
    SDA_Output_Mode(bus);
    SCL_Output(bus,0);
		delay_us(2);
    return SUCCESS;
}

/**
 * @brief 发送I2C ACK信号
 * 主机在接收完一字节数据后主动发送ACK信号给从机。
 * @param bus 指向I2C总线配置结构体的指针
 * @return 无返回值
 */
void I2CSendAck(const i2c_bus_t *bus)
{
    SDA_Output(bus,0);
		delay_us(1);
    SCL_Output(bus,1);
		delay_us(1);
    SCL_Output(bus,0);
		delay_us(1);

}

/**
 * @brief 发送I2C NACK信号
 * 主机在接收完最后一个字节后发送NACK信号以终止通信。
 * @param bus 指向I2C总线配置结构体的指针
 * @return 无返回值
 */
void I2CSendNotAck(const i2c_bus_t *bus)
{
    SDA_Output(bus,1);
		delay_us(1);
    SCL_Output(bus,1);
		delay_us(1);
    SCL_Output(bus,0);
		delay_us(2);

}

/**
 * @brief 向I2C总线发送一个字节
 * 按照I2C协议逐位发送一个字节的数据，高位先发。
 * @param bus 指向I2C总线配置结构体的指针
 * @param cSendByte 待发送的字节数据
 * @return 无返回值
 */
void I2CSendByte(const i2c_bus_t *bus,unsigned char cSendByte)
{
    unsigned char  i = 8;
    while (i--)
    {
        SCL_Output(bus,0);
        delay_us(2);
        SDA_Output(bus, cSendByte & 0x80);
				delay_us(1);
        cSendByte += cSendByte;
				delay_us(1);
        SCL_Output(bus,1);
				delay_us(1);
    }
    SCL_Output(bus,0);
		delay_us(2);
}

/**
 * @brief 从I2C总线接收一个字节
 * 按照I2C协议逐位接收一个字节的数据，高位先收。
 * @param bus 指向I2C总线配置结构体的指针
 * @return 接收到的一个字节数据
 */
unsigned char I2CReceiveByte(const i2c_bus_t *bus)
{
    unsigned char i = 8;
    unsigned char cR_Byte = 0;
    SDA_Input_Mode(bus);
    while (i--)
    {
        cR_Byte += cR_Byte;
        SCL_Output(bus,0);
				delay_us(2);
        SCL_Output(bus,1);
				delay_us(1);
        cR_Byte |=  SDA_Input(bus);
    }
    SCL_Output(bus,0);
    SDA_Output_Mode(bus);
    return cR_Byte;
}

/**
 * @brief 写入单个字节到指定寄存器地址
 * 实现对目标器件某寄存器的一次性写操作。
 * @param bus 指向I2C总线配置结构体的指针
 * @param daddr 设备地址（7位）
 * @param reg 寄存器地址
 * @param data 待写入的数据
 * @return 成功返回0，失败返回1
 */
uint8_t I2C_Write_One_Byte(i2c_bus_t *bus, uint8_t daddr,uint8_t reg,uint8_t data)
{
  I2CStart(bus);

	I2CSendByte(bus,daddr<<1);
	if(I2CWaitAck(bus))	//等待应答
	{
		I2CStop(bus);
		return 1;
	}
	I2CSendByte(bus,reg);
	I2CWaitAck(bus);
	I2CSendByte(bus,data);
	I2CWaitAck(bus);
  I2CStop(bus);
	delay_us(1);
	return 0;
}

/**
 * @brief 批量写入多个字节到指定寄存器开始的位置
 * 实现对目标器件连续多个寄存器进行批量写操作。
 * @param bus 指向I2C总线配置结构体的指针
 * @param daddr 设备地址（7位）
 * @param reg 起始寄存器地址
 * @param length 数据长度（字节数）
 * @param buff 指向待写入数据缓冲区的指针
 * @return 成功返回0，失败返回1
 */
uint8_t I2C_Write_Multi_Byte(i2c_bus_t *bus, uint8_t daddr,uint8_t reg,uint8_t length,uint8_t buff[])
{
	unsigned char i;
  I2CStart(bus);

	I2CSendByte(bus,daddr<<1);
	if(I2CWaitAck(bus))
	{
		I2CStop(bus);
		return 1;
	}
	I2CSendByte(bus,reg);
	I2CWaitAck(bus);
	for(i=0;i<length;i++)
	{
		I2CSendByte(bus,buff[i]);
		I2CWaitAck(bus);
	}
  I2CStop(bus);
	delay_us(1);
	return 0;
}

/**
 * @brief 从指定寄存器读取一个字节数据
 * 实现对目标器件某一寄存器的一次性读操作。
 * @param bus 指向I2C总线配置结构体的指针
 * @param daddr 设备地址（7位）
 * @param reg 寄存器地址
 * @return 读取到的数据
 */
unsigned char I2C_Read_One_Byte(i2c_bus_t *bus, uint8_t daddr,uint8_t reg)
{
	unsigned char dat;
	I2CStart(bus);
	I2CSendByte(bus,daddr<<1);
	I2CWaitAck(bus);
	I2CSendByte(bus,reg);
	I2CWaitAck(bus);

	I2CStart(bus);
	I2CSendByte(bus,(daddr<<1)+1);
	I2CWaitAck(bus);
	dat = I2CReceiveByte(bus);
	I2CSendNotAck(bus);
	I2CStop(bus);
	return dat;
}

/**
 * @brief 从指定寄存器开始位置读取多个字节数据
 * 实现对目标器件连续多个寄存器进行批量读操作。
 * @param bus 指向I2C总线配置结构体的指针
 * @param daddr 设备地址（7位）
 * @param reg 起始寄存器地址
 * @param length 数据长度（字节数）
 * @param buff 指向存储读取结果的缓冲区指针
 * @return 成功返回0，失败返回1
 */
uint8_t I2C_Read_Multi_Byte(i2c_bus_t *bus, uint8_t daddr, uint8_t reg, uint8_t length, uint8_t buff[])
{
	unsigned char i;
	I2CStart(bus);
	I2CSendByte(bus,daddr<<1);
	if(I2CWaitAck(bus))
	{
		I2CStop(bus);
		return 1;
	}
	I2CSendByte(bus,reg);
	I2CWaitAck(bus);

	I2CStart(bus);
	I2CSendByte(bus,(daddr<<1)+1);
	I2CWaitAck(bus);
	for(i=0;i<length;i++)
	{
		buff[i] = I2CReceiveByte(bus);
		if(i<length-1)
		{I2CSendAck(bus);}
	}
	I2CSendNotAck(bus);
	I2CStop(bus);
	return 0;
}

/**
 * @brief 初始化I2C总线相关GPIO引脚
 * 对SDA与SCL两个引脚分别初始化为开漏输出模式并开启内部上拉电阻。
 * @param bus 指向I2C总线配置结构体的指针
 * @return 无返回值
 */
void I2CInit(const i2c_bus_t *bus)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

	__HAL_RCC_GPIOB_CLK_ENABLE();
		
    GPIO_InitStructure.Pin = bus->I2C_SDA_PIN ;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(bus->I2C_SDA_PORT, &GPIO_InitStructure);
		
	GPIO_InitStructure.Pin = bus->I2C_SCL_PIN ;
    HAL_GPIO_Init(bus->I2C_SCL_PORT, &GPIO_InitStructure);
}