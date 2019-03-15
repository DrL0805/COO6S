#ifndef		SOFTWAREI2C_1_H
#define		SOFTWAREI2C_1_H


void I2C_1_UnInit(void);

void I2C_1_Init(void);

void I2C_1_Start(void);

void I2C_1_Stop(void);

void I2C_1_NoACK(void);

void I2C_1_ACK(void);

unsigned char I2C_1_ReadByte(void);

unsigned short I2C_1_WriteByte(unsigned char Data);

unsigned short I2C_1_WriteBytes(unsigned char* dataAddr, unsigned short length);

void I2C_1_ReadBytes(unsigned char *dataAddr, unsigned short length);

uint32_t I2C_1_DeviceWrite(unsigned char deviceAddr, unsigned char regAddr, unsigned char* data_Point, unsigned short length );

uint32_t I2C_1_DeviceRead(unsigned char deviceAddr, unsigned char regAddr, unsigned char* data_Point, unsigned short length);















#endif  //  SOFTWAREI2C_1_H
