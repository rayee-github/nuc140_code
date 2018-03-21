#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "NUC1xx.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvTIMER.h"
#include "Driver\DrvUART.h"
#include "LCD_Driver.h"
#include "Driver\DrvSPI.h"
#include "Driver\DrvI2C.h"
// pin32 GPB0/RX0 to another board's UART TX
// pin33 GPB1/TX0 to another board's UART RX
// E_FUNC_I2C1,SDA1:GPA-10,SCL1:GPA-11
// GPA-10/11 as SDA1/SCL1 pins.
//uart0 TX:GPB1 RX:GPB0
#define	STE_DATA_READ	DrvGPIO_set(E_IO_INPUT);
#define	STE_DATA_WRITE	DrvGPIO_set(E_IO_OUTPUT);
#define Rd(address)		((address<<1) | 0x01)
#define Wr(address)		((address<<1) & 0xFE)
#define Da(Data)		(Data & 0xFF)
/////////////////MPU9250
#define MPU   0x68
#define XG_OFFSET_H      0x13  // User-defined trim values for gyroscope
#define XG_OFFSET_L      0x14
#define YG_OFFSET_H      0x15
#define YG_OFFSET_L      0x16
#define ZG_OFFSET_H      0x17
#define ZG_OFFSET_L      0x18
#define SMPLRT_DIV       0x19
#define CONFIG           0x1A
#define GYRO_CONFIG      0x1B
#define ACCEL_CONFIG     0x1C
#define ACCEL_CONFIG2    0x1D
#define LP_ACCEL_ODR     0x1E   
#define WOM_THR          0x1F   
#define MOT_DUR          0x20  // Duration counter threshold for motion interrupt generation, 1 kHz rate, LSB = 1 ms
#define ZMOT_THR         0x21  // Zero-motion detection threshold bits [7:0]
#define ZRMOT_DUR        0x22  // Duration counter threshold for zero motion interrupt generation, 16 Hz rate, LSB = 64 ms
#define FIFO_EN          0x23
#define I2C_MST_CTRL     0x24   
#define I2C_SLV0_ADDR    0x25
#define I2C_SLV0_REG     0x26
#define I2C_SLV0_CTRL    0x27
#define I2C_SLV1_ADDR    0x28
#define I2C_SLV1_REG     0x29
#define I2C_SLV1_CTRL    0x2A
#define I2C_SLV2_ADDR    0x2B
#define I2C_SLV2_REG     0x2C
#define I2C_SLV2_CTRL    0x2D
#define I2C_SLV3_ADDR    0x2E
#define I2C_SLV3_REG     0x2F
#define I2C_SLV3_CTRL    0x30
#define I2C_SLV4_ADDR    0x31
#define I2C_SLV4_REG     0x32
#define I2C_SLV4_DO      0x33
#define I2C_SLV4_CTRL    0x34
#define I2C_SLV4_DI      0x35
#define I2C_MST_STATUS   0x36
#define INT_PIN_CFG      0x37
#define INT_ENABLE       0x38
#define DMP_INT_STATUS   0x39  // Check DMP interrupt
#define INT_STATUS       0x3A
#define ACCEL_XOUT_H     0x3B
#define ACCEL_XOUT_L     0x3C
#define ACCEL_YOUT_H     0x3D
#define ACCEL_YOUT_L     0x3E
#define ACCEL_ZOUT_H     0x3F
#define ACCEL_ZOUT_L     0x40
#define TEMP_OUT_H       0x41
#define TEMP_OUT_L       0x42
#define GYRO_XOUT_H      0x43
#define GYRO_XOUT_L      0x44
#define GYRO_YOUT_H      0x45
#define GYRO_YOUT_L      0x46
#define GYRO_ZOUT_H      0x47
#define GYRO_ZOUT_L      0x48
#define EXT_SENS_DATA_00 0x49
#define EXT_SENS_DATA_01 0x4A
#define EXT_SENS_DATA_02 0x4B
#define EXT_SENS_DATA_03 0x4C
#define EXT_SENS_DATA_04 0x4D
#define EXT_SENS_DATA_05 0x4E
#define EXT_SENS_DATA_06 0x4F
#define EXT_SENS_DATA_07 0x50
#define EXT_SENS_DATA_08 0x51
#define EXT_SENS_DATA_09 0x52
#define EXT_SENS_DATA_10 0x53
#define EXT_SENS_DATA_11 0x54
#define EXT_SENS_DATA_12 0x55
#define EXT_SENS_DATA_13 0x56
#define EXT_SENS_DATA_14 0x57
#define EXT_SENS_DATA_15 0x58
#define EXT_SENS_DATA_16 0x59
#define EXT_SENS_DATA_17 0x5A
#define EXT_SENS_DATA_18 0x5B
#define EXT_SENS_DATA_19 0x5C
#define EXT_SENS_DATA_20 0x5D
#define EXT_SENS_DATA_21 0x5E
#define EXT_SENS_DATA_22 0x5F
#define EXT_SENS_DATA_23 0x60
#define MOT_DETECT_STATUS 0x61
#define I2C_SLV0_DO      0x63
#define I2C_SLV1_DO      0x64
#define I2C_SLV2_DO      0x65
#define I2C_SLV3_DO      0x66
#define I2C_MST_DELAY_CTRL 0x67
#define SIGNAL_PATH_RESET  0x68
#define MOT_DETECT_CTRL  0x69
#define USER_CTRL        0x6A  // Bit 7 enable DMP, bit 3 reset DMP
#define PWR_MGMT_1       0x6B // Device defaults to the SLEEP mode
#define PWR_MGMT_2       0x6C
#define DMP_BANK         0x6D  // Activates a specific bank in the DMP
#define DMP_RW_PNT       0x6E  // Set read/write pointer to a specific start address in specified DMP bank
#define DMP_REG          0x6F  // Register in DMP from which to read or to which to write
#define DMP_REG_1        0x70
#define DMP_REG_2        0x71 
#define FIFO_COUNTH      0x72
#define FIFO_COUNTL      0x73
#define FIFO_R_W         0x74
#define WHO_AM_I_MPU9250 0x75 // Should return 0x71
#define XA_OFFSET_H      0x77
#define XA_OFFSET_L      0x78
#define YA_OFFSET_H      0x7A
#define YA_OFFSET_L      0x7B
#define ZA_OFFSET_H      0x7D
#define ZA_OFFSET_L      0x7E
/////////////////AK8963
#define AK   0x0C
#define AK8963_WHO_AM_I  0x00 // should return 0x48
#define AK8963_INFO      0x01
#define AK8963_ST1       0x02  // data ready status bit 0
#define AK8963_XOUT_L    0x03  // data
#define AK8963_XOUT_H    0x04
#define AK8963_YOUT_L    0x05
#define AK8963_YOUT_H    0x06
#define AK8963_ZOUT_L    0x07
#define AK8963_ZOUT_H    0x08
#define AK8963_ST2       0x09  // Data overflow bit 3 and data read error status bit 2
#define AK8963_CNTL      0x0A  // Power down (0000), single-measurement (0001), self-test (1000) and Fuse ROM (1111) modes on bits 3:0
#define AK8963_ASTC      0x0C  // Self test control
#define AK8963_I2CDIS    0x0F  // I2C disable
#define AK8963_ASAX      0x10  // Fuse ROM x-axis sensitivity adjustment value
#define AK8963_ASAY      0x11  // Fuse ROM y-axis sensitivity adjustment value
#define AK8963_ASAZ      0x12  // Fuse ROM z-axis sensitivity adjustment value
#define Kp 2.0f * 5.0f // these are the free parameters in the Mahony filter and fusion scheme, Kp for proportional feedback, Ki for integral
#define Ki 0.0f
void SPI1_Callback(uint32_t u32UserData);
uint8_t common = {0x00};
uint32_t datain[6] = {0xFF};
uint32_t dataout[6] = {0xFF};
uint32_t g_Spi1IntFlag;
int16_t accelCount[3];  // Stores the 16-bit signed accelerometer sensor output
int16_t gyroCount[3];   // Stores the 16-bit signed gyro sensor output
int16_t magCount[3];    // Stores the 16-bit signed magnetometer sensor output
float magCalibration[3] = {0, 0, 0}, magbias[3] = {0, 0, 0};
float gyroBias[3] = {0, 0, 0}, accelBias[3] = {0, 0, 0}; // Bias corrections for gyro and accelerometer

float pitch, yaw, roll;
float deltat = 0.01f;                             // integration interval for both filter schemes                  
float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};           // vector to hold quaternion
float eInt[3] = {0.0f, 0.0f, 0.0f};              // vector to hold integral error for Mahony method
float PI = 3.14159265358979323846f;
void DelayNs(int n)
{
	int i = 0;
	for(i=0;i<n;i++)__NOP();
}

void DrvGPIO_set(E_DRVGPIO_IO mode)
{
	volatile uint32_t u32Reg;
	u32Reg = (uint32_t)&GPIOA->PMD + (4*0x40);
	outpw(u32Reg, inpw(u32Reg) & ~(0x3<<(1*2)));
	if (mode == E_IO_OUTPUT)outpw(u32Reg, inpw(u32Reg) | (0x1<<(1*2)));
}

void  writeByte(uint8_t addr ,uint8_t reg , uint8_t data)
{
	uint8_t L,H;
	uint32_t i;
	int count = 0;
	//Open I2C1 and set clock = 50Kbps
	SystemCoreClock = DrvSYS_GetHCLKFreq(); 
	DrvI2C_Open(I2C_PORT1, 50000);
	//send i2c start
	DrvI2C_Ctrl(I2C_PORT1, 1, 0, 1, 1);	 	//set start
	while (I2C1->I2CON.SI == 0);				//poll si flag
	//send  addr command
	I2C1->I2CDAT = addr<<1;		    	//write  addr   to
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1);	   //clr si
	while( I2C1->I2CON.SI == 0 );			   //poll si flag
	//send reg
	I2C1->I2CDAT = reg;					//write  reg   to
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1); //clr si and set ack	
	while( I2C1->I2CON.SI == 0 );			//poll si flag
	//send data
	I2C1->I2CDAT = data;					//write data to 
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1); //clr si and set ack	
	while( I2C1->I2CON.SI == 0 );			//poll si flag
	//send i2c stop
	DrvI2C_Ctrl(I2C_PORT1, 0, 1, 1, 0);    //clr si and set stop
	while( I2C1->I2CON.STO){
		count++;
		if(count>1000){
			break;
		}
	}
	for(i=0;i<60;i++);
	DrvI2C_Close(I2C_PORT1);
}



uint8_t readByte(uint8_t addr , uint8_t data)
{
	uint8_t value;
	uint32_t i;
	int count = 0;
	//Open I2C1 and set clock = 50Kbps
	SystemCoreClock = DrvSYS_GetHCLKFreq(); 
	DrvI2C_Open(I2C_PORT1, 50000);
	//send i2c start
	DrvI2C_Ctrl(I2C_PORT1, 1, 0, 1, 1);	 	//set start
	while (I2C1->I2CON.SI == 0);				//poll si flag
	//send writer command
	I2C1->I2CDAT = addr<<1;
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1);	   //clr si
	while( I2C1->I2CON.SI == 0 );			   //poll si flag
	//send data
	I2C1->I2CDAT = data;					//write data to 
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1); //clr si and set ack	
	while( I2C1->I2CON.SI == 0 );			//poll si flag
	//send i2c start
	DrvI2C_Ctrl(I2C_PORT1, 1, 0, 1, 1);	 	//set start
	while (I2C1->I2CON.SI == 0);				//poll si flag
	//send writer command
	I2C1->I2CDAT = (addr<<1)+1;
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1);	   //clr si
	while( I2C1->I2CON.SI == 0 );			   //poll si flag
	//resive data
	I2C1->I2CDAT = 0XFF;
	DrvI2C_Ctrl(I2C_PORT1, 0, 0, 1, 1);    //clr si	
	while( I2C1->I2CON.SI == 0 );			   //poll si flag
	value= I2C1->I2CDAT;
	//send i2c stop
	DrvI2C_Ctrl(I2C_PORT1, 0, 1, 1, 0);    //clr si and set stop
	while( I2C1->I2CON.STO){
		count++;
		if(count>1000)  break;
	}
	for(i=0;i<60;i++);
	DrvI2C_Close(I2C_PORT1);
	return value;
}
volatile uint8_t comRbuf[16] ={0x30};
volatile uint16_t comRbytes = 0;
volatile uint16_t comRhead 	= 0;
volatile uint16_t comRtail 	= 0;

char TEXT1[16] = "TX: sending...  ";
char TEXT2[16] = "RX:             ";
char textout[30];

/*---------------------------------------------------------------------------------------------------------*/
/* UART Callback function                                                                           	   */
/*---------------------------------------------------------------------------------------------------------*/
void UART_INT_HANDLE(void)
{
	uint8_t i;
	uint8_t bInChar[1]= {0xFF};

	while(UART0->ISR.RDA_IF==1) 
	{
		DrvUART_Read(UART_PORT0,bInChar,1);	
		if(bInChar[0]=='a') // check if Buffer is full
		{
			DrvUART_Write(UART_PORT0,textout,strlen(textout));  
		}
	}
}
void initMPU9250()
{  
	uint8_t c;
	writeByte(MPU, PWR_MGMT_1, 0x00); 
	DrvSYS_Delay(100000);

	writeByte(MPU, PWR_MGMT_1, 0x01);  
	writeByte(MPU, CONFIG, 0x03);  

	writeByte(MPU, SMPLRT_DIV, 0x04);  
	c = readByte(MPU, GYRO_CONFIG); 
	c = c & ~0x02; 
	c = c & ~0x18; 
	writeByte(MPU, GYRO_CONFIG, c ); 
	c = readByte(MPU, ACCEL_CONFIG); 
	c = c & ~0x18; 
	writeByte(MPU, ACCEL_CONFIG, c); 
	c = readByte(MPU, ACCEL_CONFIG2); 
	c = c & ~0x0F; 
	c = c | 0x03;  
	writeByte(MPU, ACCEL_CONFIG2, c); 
	writeByte(MPU, INT_PIN_CFG, 0x22);    
	writeByte(MPU, INT_ENABLE, 0x01);  
}
void initAK8963(float * destination)
{
	// First extract the factory calibration for each magnetometer axis
	uint8_t rawData[3];  // x/y/z gyro calibration data stored here
	writeByte(AK, AK8963_CNTL, 0x00); // Power down magnetometer  
	DrvSYS_Delay(10000);
	writeByte(AK, AK8963_CNTL, 0x0F); // Enter Fuse ROM access mode
	DrvSYS_Delay(10000);
	rawData[0]=readByte(AK, AK8963_ASAX);  // Read the x-, y-, and z-axis calibration values
	rawData[1]=readByte(AK, AK8963_ASAY);
	rawData[2]=readByte(AK, AK8963_ASAZ);
	destination[0] =  (float)(rawData[0] - 128)/256.0f + 1.0f;   // Return x-axis sensitivity adjustment values, etc.
	destination[1] =  (float)(rawData[1] - 128)/256.0f + 1.0f;  
	destination[2] =  (float)(rawData[2] - 128)/256.0f + 1.0f; 			   
	writeByte(AK, AK8963_CNTL, 0x00); // Power down magnetometer  
	DrvSYS_Delay(10000);
	writeByte(AK, AK8963_CNTL, 0x06); // Set magnetometer data resolution and sample ODR
	DrvSYS_Delay(10000);
}

void resetMPU9250() {
	// reset device
	writeByte(MPU, PWR_MGMT_1, 0x80); 
}
void calibrateMPU9250(float * dest1, float * dest2)
{  
	uint8_t data[12]; 
	uint16_t ii, packet_count, fifo_count;
	int32_t gyro_bias[3] = {0, 0, 0}, accel_bias[3] = {0, 0, 0};
	uint32_t mask = 1uL; // Define mask for temperature compensation bit 0 of lower byte of accelerometer bias registers
	uint8_t mask_bit[3] = {0, 0, 0}; // Define array to hold mask bit for each accelerometer bias axis
	uint16_t  gyrosensitivity  = 131;   // = 131 LSB/degrees/sec
	uint16_t  accelsensitivity = 16384;  // = 16384 LSB/g
	int32_t accel_bias_reg[3] = {0, 0, 0}; // A place to hold the factory accelerometer trim biases
	writeByte(MPU, PWR_MGMT_1, 0x80); 
	DrvSYS_Delay(100000);

	writeByte(MPU, PWR_MGMT_1, 0x01);  
	writeByte(MPU, PWR_MGMT_2, 0x00); 
	DrvSYS_Delay(200000);

	writeByte(MPU, INT_ENABLE, 0x00);   // Disable all interrupts
	writeByte(MPU, FIFO_EN, 0x00);      // Disable FIFO
	writeByte(MPU, PWR_MGMT_1, 0x00);   // Turn on internal clock source
	writeByte(MPU, I2C_MST_CTRL, 0x00); // Disable I2C master
	writeByte(MPU, USER_CTRL, 0x00);    // Disable FIFO and I2C master modes
	writeByte(MPU, USER_CTRL, 0x0C);    // Reset FIFO and DMP
	DrvSYS_Delay(10000);

	// Configure MPU9250 gyro and accelerometer for bias calculation
	writeByte(MPU, CONFIG, 0x01);      // Set low-pass filter to 188 Hz
	writeByte(MPU, SMPLRT_DIV, 0x00);  // Set sample rate to 1 kHz
	writeByte(MPU, GYRO_CONFIG, 0x00);  // Set gyro full-scale to 250 degrees per second, maximum sensitivity
	writeByte(MPU, ACCEL_CONFIG, 0x00); // Set accelerometer full-scale to 2 g, maximum sensitivity

	// Configure FIFO to capture accelerometer and gyro data for bias calculation
	writeByte(MPU, USER_CTRL, 0x40);   // Enable FIFO  
	writeByte(MPU, FIFO_EN, 0x78);     // Enable gyro and accelerometer sensors for FIFO (max size 512 bytes in MPU-9250)
	DrvSYS_Delay(40000);

	// At end of sample accumulation, turn off FIFO sensor read
	writeByte(MPU, FIFO_EN, 0x00);        // Disable gyro and accelerometer sensors for FIFO
	data[0]=readByte(MPU, FIFO_COUNTH); // read FIFO sample count
	data[1]=readByte(MPU, FIFO_COUNTL); // read FIFO sample count
	fifo_count = ((uint16_t)data[0] << 8) | data[1];
	packet_count = fifo_count/12;// How many sets of full gyro and accelerometer data for averaging

	for (ii = 0; ii < packet_count; ii++) {
		int16_t accel_temp[3] = {0, 0, 0}, gyro_temp[3] = {0, 0, 0};
		data[0]=readByte(MPU,ACCEL_XOUT_H); 
		data[1]=readByte(MPU,ACCEL_XOUT_L);
		data[2]=readByte(MPU,ACCEL_YOUT_H);
		data[3]=readByte(MPU,ACCEL_YOUT_L);
		data[4]=readByte(MPU,ACCEL_ZOUT_H);
		data[5]=readByte(MPU,ACCEL_ZOUT_L);
		data[6]=readByte(MPU,GYRO_XOUT_H);
		data[7]=readByte(MPU,GYRO_XOUT_L);
		data[8]=readByte(MPU,GYRO_YOUT_H);
		data[9]=readByte(MPU,GYRO_YOUT_L);
		data[10]=readByte(MPU,GYRO_ZOUT_H);
		data[11]=readByte(MPU,GYRO_ZOUT_L);
		accel_temp[0] = (int16_t) (((int16_t)data[0] << 8) | data[1]  ) ;  // Form signed 16-bit integer for each sample in FIFO
		accel_temp[1] = (int16_t) (((int16_t)data[2] << 8) | data[3]  ) ;
		accel_temp[2] = (int16_t) (((int16_t)data[4] << 8) | data[5]  ) ;    
		gyro_temp[0]  = (int16_t) (((int16_t)data[6] << 8) | data[7]  ) ;
		gyro_temp[1]  = (int16_t) (((int16_t)data[8] << 8) | data[9]  ) ;
		gyro_temp[2]  = (int16_t) (((int16_t)data[10] << 8) | data[11]) ;

		accel_bias[0] += (int32_t) accel_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
		accel_bias[1] += (int32_t) accel_temp[1];
		accel_bias[2] += (int32_t) accel_temp[2];
		gyro_bias[0]  += (int32_t) gyro_temp[0];
		gyro_bias[1]  += (int32_t) gyro_temp[1];
		gyro_bias[2]  += (int32_t) gyro_temp[2];

	}
	accel_bias[0] /= (int32_t) packet_count; // Normalize sums to get average count biases
	accel_bias[1] /= (int32_t) packet_count;
	accel_bias[2] /= (int32_t) packet_count;
	gyro_bias[0]  /= (int32_t) packet_count;
	gyro_bias[1]  /= (int32_t) packet_count;
	gyro_bias[2]  /= (int32_t) packet_count;

	if(accel_bias[2] > 0L) {accel_bias[2] -= (int32_t) accelsensitivity;}  // Remove gravity from the z-axis accelerometer bias calculation
	else {accel_bias[2] += (int32_t) accelsensitivity;}

	// Construct the gyro biases for push to the hardware gyro bias registers, which are reset to zero upon device startup
	data[0] = (-gyro_bias[0]/4  >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
	data[1] = (-gyro_bias[0]/4)       & 0xFF; // Biases are additive, so change sign on calculated average gyro biases
	data[2] = (-gyro_bias[1]/4  >> 8) & 0xFF;
	data[3] = (-gyro_bias[1]/4)       & 0xFF;
	data[4] = (-gyro_bias[2]/4  >> 8) & 0xFF;
	data[5] = (-gyro_bias[2]/4)       & 0xFF;

	dest1[0] = (float) gyro_bias[0]/(float) gyrosensitivity; // construct gyro bias in deg/s for later manual subtraction
	dest1[1] = (float) gyro_bias[1]/(float) gyrosensitivity;
	dest1[2] = (float) gyro_bias[2]/(float) gyrosensitivity;


	data[0]=readByte(MPU, XA_OFFSET_H);
	data[1]=readByte(MPU, XA_OFFSET_L);
	accel_bias_reg[0] = (int16_t) ((int16_t)data[0] << 8) | data[1];
	data[0]=readByte(MPU, YA_OFFSET_H);
	data[1]=readByte(MPU, YA_OFFSET_L);
	accel_bias_reg[1] = (int16_t) ((int16_t)data[0] << 8) | data[1];
	data[0]=readByte(MPU, ZA_OFFSET_H);
	data[1]=readByte(MPU, ZA_OFFSET_L);
	accel_bias_reg[2] = (int16_t) ((int16_t)data[0] << 8) | data[1];



	for(ii = 0; ii < 3; ii++) {
		if(accel_bias_reg[ii] & mask) mask_bit[ii] = 0x01; // If temperature compensation bit is set, record that fact in mask_bit
	}

	// Construct total accelerometer bias, including calculated average accelerometer bias from above
	accel_bias_reg[0] -= (accel_bias[0]/8); // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
	accel_bias_reg[1] -= (accel_bias[1]/8);
	accel_bias_reg[2] -= (accel_bias[2]/8);

	data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
	data[1] = (accel_bias_reg[0])      & 0xFF;
	data[1] = data[1] | mask_bit[0]; // preserve temperature compensation bit when writing back to accelerometer bias registers
	data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
	data[3] = (accel_bias_reg[1])      & 0xFF;
	data[3] = data[3] | mask_bit[1]; // preserve temperature compensation bit when writing back to accelerometer bias registers
	data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
	data[5] = (accel_bias_reg[2])      & 0xFF;
	data[5] = data[5] | mask_bit[2]; // preserve temperature compensation bit when writing back to accelerometer bias registers

	dest2[0] = (float)accel_bias[0]/(float)accelsensitivity; 
	dest2[1] = (float)accel_bias[1]/(float)accelsensitivity;
	dest2[2] = (float)accel_bias[2]/(float)accelsensitivity;
}
void readAccelData(int16_t * destination)
{
	uint8_t rawData[6];  // x/y/z accel register data stored here
	rawData[0]=readByte(MPU, ACCEL_XOUT_H);  // Read the six raw data registers into data array
	rawData[1]=readByte(MPU, ACCEL_XOUT_L);
	rawData[2]=readByte(MPU, ACCEL_YOUT_H);
	rawData[3]=readByte(MPU, ACCEL_YOUT_L);
	rawData[4]=readByte(MPU, ACCEL_ZOUT_H);
	rawData[5]=readByte(MPU, ACCEL_ZOUT_L);
	destination[0] = rawData[0] *256 + rawData[1] ;  // Turn the MSB and LSB into a signed 16-bit value
	destination[1] = rawData[2] *256 + rawData[3] ;
	destination[2] = rawData[4] *256 + rawData[5] ;
}

void readGyroData(int16_t * destination)
{
	uint8_t rawData[6];  // x/y/z accel register data stored here
	rawData[0]=readByte(MPU, GYRO_XOUT_H);  // Read the six raw data registers into data array
	rawData[1]=readByte(MPU, GYRO_XOUT_L);
	rawData[2]=readByte(MPU, GYRO_YOUT_H);
	rawData[3]=readByte(MPU, GYRO_YOUT_L);
	rawData[4]=readByte(MPU, GYRO_ZOUT_H);
	rawData[5]=readByte(MPU, GYRO_ZOUT_L);
	destination[0] = rawData[0] <<8 | rawData[1] ;  // Turn the MSB and LSB into a signed 16-bit value
	destination[1] = rawData[2] <<8 | rawData[3] ;
	destination[2] = rawData[4] <<8 | rawData[5] ;
}

void readMagData(int16_t * destination)
{
	uint8_t rawData[7];  // x/y/z gyro register data, ST2 register stored here, must read ST2 at end of data acquisition
	uint8_t c; // End data read by reading ST2 register
	if(readByte(AK, AK8963_ST1) & 0x01) { // wait for magnetometer data ready bit to be set
		rawData[0]=readByte(AK, AK8963_XOUT_L);  // Read the six raw data and ST2 registers sequentially into data array
		rawData[1]=readByte(AK, AK8963_XOUT_H);
		rawData[2]=readByte(AK, AK8963_YOUT_L);
		rawData[3]=readByte(AK, AK8963_YOUT_H);
		rawData[4]=readByte(AK, AK8963_ZOUT_L);
		rawData[5]=readByte(AK, AK8963_ZOUT_H);
		rawData[6]=readByte(AK, AK8963_ST2);	
		c = rawData[6] ;
		if(!(c & 0x08)) { // Check if magnetic sensor overflow set, if not then report data
			destination[0] = rawData[1] <<8 | rawData[0] ;  // Turn the MSB and LSB into a signed 16-bit value
			destination[1] = rawData[3] <<8 | rawData[2] ;
			destination[2] = rawData[5] <<8 | rawData[4] ;
		}
	}
}

int16_t readTempData()
{
	uint8_t rawData[2];  // x/y/z gyro register data stored here
	rawData[0]=readByte(MPU, TEMP_OUT_H);  // Read the two raw data registers sequentially into data array 
	rawData[1]=readByte(MPU, TEMP_OUT_L);
	return rawData[0]*256+rawData[1];  // Turn the MSB and LSB into a 16-bit value
}

void MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz)
{
	float q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];   // short name local variable for readability
	float norm;
	float hx, hy, _2bx, _2bz;
	float s1, s2, s3, s4;
	float qDot1, qDot2, qDot3, qDot4;

	// Auxiliary variables to avoid repeated arithmetic
	float _2q1mx;
	float _2q1my;
	float _2q1mz;
	float _2q2mx;
	float _4bx;
	float _4bz;
	float _2q1 = 2.0f * q1;
	float _2q2 = 2.0f * q2;
	float _2q3 = 2.0f * q3;
	float _2q4 = 2.0f * q4;
	float _2q1q3 = 2.0f * q1 * q3;
	float _2q3q4 = 2.0f * q3 * q4;
	float q1q1 = q1 * q1;
	float q1q2 = q1 * q2;
	float q1q3 = q1 * q3;
	float q1q4 = q1 * q4;
	float q2q2 = q2 * q2;
	float q2q3 = q2 * q3;
	float q2q4 = q2 * q4;
	float q3q3 = q3 * q3;
	float q3q4 = q3 * q4;
	float q4q4 = q4 * q4;
	float beta = sqrt(3.0f / 4.0f) * PI* (60.0f / 180.0f);  // compute beta
	float zeta = sqrt(3.0f / 4.0f) * PI* (1.0f / 180.0f);  // compute zeta, the other free parameter in the Madgwick scheme usually set to a small or zero value
	// Normalise accelerometer measurement
	norm = sqrt(ax * ax + ay * ay + az * az);	 
	if (norm == 0.0f) return; // handle NaN
	norm = 1.0f/norm;
	ax *= norm;
	ay *= norm;
	az *= norm;
	// Normalise magnetometer measurement
	norm = sqrt(mx * mx + my * my + mz * mz);
	if (norm == 0.0f) return; // handle NaN
	norm = 1.0f/norm;
	mx *= norm;
	my *= norm;
	mz *= norm;
	// Reference direction of Earth's magnetic field
	_2q1mx = 2.0f * q1 * mx;
	_2q1my = 2.0f * q1 * my;
	_2q1mz = 2.0f * q1 * mz;
	_2q2mx = 2.0f * q2 * mx;
	hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
	hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
	_2bx = sqrt(hx * hx + hy * hy);
	_2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
	_4bx = 2.0f * _2bx;
	_4bz = 2.0f * _2bz;

	// Gradient decent algorithm corrective step
	s1 = -_2q3 * (2.0f * q2q4 - _2q1q3 - ax) + _2q2 * (2.0f * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
	s2 = _2q4 * (2.0f * q2q4 - _2q1q3 - ax) + _2q1 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q2 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + _2bz * q4 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
	s3 = -_2q1 * (2.0f * q2q4 - _2q1q3 - ax) + _2q4 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q3 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
	s4 = _2q2 * (2.0f * q2q4 - _2q1q3 - ax) + _2q3 * (2.0f * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);


	norm = sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);    // normalise step magnitude
	norm = 1.0f/norm;
	s1 *= norm;
	s2 *= norm;
	s3 *= norm;
	s4 *= norm;

	// Compute rate of change of quaternion
	qDot1 = 0.5f * (-q2 * gx - q3 * gy - q4 * gz) - beta * s1;
	qDot2 = 0.5f * (q1 * gx + q3 * gz - q4 * gy) - beta * s2;
	qDot3 = 0.5f * (q1 * gy - q2 * gz + q4 * gx) - beta * s3;
	qDot4 = 0.5f * (q1 * gz + q2 * gy - q3 * gx) - beta * s4;

	// Integrate to yield quaternion
	q1 += qDot1 * deltat;
	q2 += qDot2 * deltat;
	q3 += qDot3 * deltat;
	q4 += qDot4 * deltat;
	norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);    // normalise quaternion
	norm = 1.0f/norm;
	q[0] = q1 * norm;
	q[1] = q2 * norm;
	q[2] = q3 * norm;
	q[3] = q4 * norm; 

}

void MahonyQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz)
{
	float q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];   // short name local variable for readability
	float norm;
	float hx, hy, bx, bz;
	float vx, vy, vz, wx, wy, wz;
	float ex, ey, ez;
	float pa, pb, pc;

	// Auxiliary variables to avoid repeated arithmetic
	float q1q1 = q1 * q1;
	float q1q2 = q1 * q2;
	float q1q3 = q1 * q3;
	float q1q4 = q1 * q4;
	float q2q2 = q2 * q2;
	float q2q3 = q2 * q3;
	float q2q4 = q2 * q4;
	float q3q3 = q3 * q3;
	float q3q4 = q3 * q4;
	float q4q4 = q4 * q4;   

	// Normalise accelerometer measurement
	norm = sqrt(ax * ax + ay * ay + az * az);
	if (norm == 0.0f) return; // handle NaN
	norm = 1.0f / norm;        // use reciprocal for division
	ax *= norm;
	ay *= norm;
	az *= norm;

	// Normalise magnetometer measurement
	norm = sqrt(mx * mx + my * my + mz * mz);
	if (norm == 0.0f) return; // handle NaN
	norm = 1.0f / norm;        // use reciprocal for division
	mx *= norm;
	my *= norm;
	mz *= norm;

	// Reference direction of Earth's magnetic field
	hx = 2.0f * mx * (0.5f - q3q3 - q4q4) + 2.0f * my * (q2q3 - q1q4) + 2.0f * mz * (q2q4 + q1q3);
	hy = 2.0f * mx * (q2q3 + q1q4) + 2.0f * my * (0.5f - q2q2 - q4q4) + 2.0f * mz * (q3q4 - q1q2);
	bx = sqrt((hx * hx) + (hy * hy));
	bz = 2.0f * mx * (q2q4 - q1q3) + 2.0f * my * (q3q4 + q1q2) + 2.0f * mz * (0.5f - q2q2 - q3q3);

	// Estimated direction of gravity and magnetic field
	vx = 2.0f * (q2q4 - q1q3);
	vy = 2.0f * (q1q2 + q3q4);
	vz = q1q1 - q2q2 - q3q3 + q4q4;
	wx = 2.0f * bx * (0.5f - q3q3 - q4q4) + 2.0f * bz * (q2q4 - q1q3);
	wy = 2.0f * bx * (q2q3 - q1q4) + 2.0f * bz * (q1q2 + q3q4);
	wz = 2.0f * bx * (q1q3 + q2q4) + 2.0f * bz * (0.5f - q2q2 - q3q3);  

	// Error is cross product between estimated direction and measured direction of gravity
	ex = (ay * vz - az * vy) + (my * wz - mz * wy);
	ey = (az * vx - ax * vz) + (mz * wx - mx * wz);
	ez = (ax * vy - ay * vx) + (mx * wy - my * wx);
	if (Ki > 0.0f)
	{
		eInt[0] += ex;      // accumulate integral error
		eInt[1] += ey;
		eInt[2] += ez;
	}
	else
	{
		eInt[0] = 0.0f;     // prevent integral wind up
		eInt[1] = 0.0f;
		eInt[2] = 0.0f;
	}

	// Apply feedback terms
	gx = gx + Kp * ex + Ki * eInt[0];
	gy = gy + Kp * ey + Ki * eInt[1];
	gz = gz + Kp * ez + Ki * eInt[2];

	// Integrate rate of change of quaternion
	pa = q2;
	pb = q3;
	pc = q4;
	q1 = q1 + (-q2 * gx - q3 * gy - q4 * gz) * (0.5f * deltat);
	q2 = pa + (q1 * gx + pb * gz - pc * gy) * (0.5f * deltat);
	q3 = pb + (q1 * gy - pa * gz + pc * gx) * (0.5f * deltat);
	q4 = pc + (q1 * gz + pa * gy - pb * gx) * (0.5f * deltat);

	// Normalise quaternion
	norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);
	norm = 1.0f / norm;
	q[0] = q1 * norm;
	q[1] = q2 * norm;
	q[2] = q3 * norm;
	q[3] = q4 * norm;

}
int main (void)
{
	STR_UART_T sParam;
	uint8_t	test1,test2;
	int sign;
	float ax,ay,az,gx,gy,gz,hx,hy,hz;
	float temp;
	float Xh,Yh	;
	UNLOCKREG();
	DrvSYS_Open(48000000);
	DrvSYS_SetOscCtrl(E_SYS_XTL12M,1);
	DrvSYS_Delay(20000);/* Delay for Xtal stable */
	while(!SYSCLK->CLKSTATUS.XTL12M_STB);
	DrvSYS_SelectHCLKSource(0);
	LOCKREG();
	Initial_pannel();
	clr_all_pannal();
	DrvGPIO_InitFunction(E_FUNC_I2C1);
	DrvGPIO_InitFunction(E_FUNC_UART0);		

	/* UART Setting */
	sParam.u32BaudRate 		= 115200;
	sParam.u8cDataBits 		= DRVUART_DATABITS_8;
	sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
	sParam.u8cParity 		= DRVUART_PARITY_NONE;
	sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;

	/* Set UART Configuration */
	if(DrvUART_Open(UART_PORT0,&sParam) != E_SUCCESS);  

	DrvUART_EnableInt(UART_PORT0, DRVUART_RDAINT, UART_INT_HANDLE);  

	//while(1)
	{
		/*writeByte(0x68, 0x37,0x02);
		writeByte(0x68, 0x6A,0x01);
		writeByte(0x0c,0x0b,0x01);
		writeByte(0x0c,0x0a,0x02);*/

		resetMPU9250(); 
		calibrateMPU9250(gyroBias, accelBias); 
		initMPU9250();
		initAK8963(magCalibration);		//¤j·§1.2
		DrvSYS_Delay(300000);		

		while(1){
			writeByte(0x68, 0x37,0x02);
			writeByte(0x68, 0x6A,0x01);
			writeByte(0x0c,0x0b,0x01);
			writeByte(0x0c,0x0b,0x00);
			writeByte(0x0c,0x0a,0x02);
			
			Initial_pannel();
			clr_all_pannal();
			readAccelData(accelCount);
			readGyroData(gyroCount);
			readMagData(magCount);
			temp=(float)readTempData()/333.87f + 21.0f;
			ax = accelCount[0]*2.0/32768.0- accelBias[0];	  //accel  value
			ay = accelCount[1]*2.0/32768.0- accelBias[1]; 
			az = accelCount[2]*2.0/32768.0- accelBias[2];  	  
			gx = gyroCount[0]*250.0/32768.0-gyroBias[0];		 //gyro  value
			gy = gyroCount[1]*250.0/32768.0-gyroBias[1]; 
			gz = gyroCount[2]*250.0/32768.0-gyroBias[2]; 			
			hx = (float)magCount[0]*1.49939*magCalibration[0]+30.0f;	   //mag value
			hy = (float)magCount[1]*1.49939*magCalibration[1]-10.f; 
			hz = (float)magCount[2]*1.49939*magCalibration[2]+60.0f;	
			/*MahonyQuaternionUpdate(ax, ay, az, gx*PI/180.0f, gy*PI/180.0f, gz*PI/180.0f,  hx,  hy, hz);
			MadgwickQuaternionUpdate(ax, ay, az, gx*PI/180.0f, gy*PI/180.0f, gz*PI/180.0f,  hx,  hy, hz);
			yaw   = atan2(2.0f * (q[1] * q[2] + q[0] * q[3]), q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3]);//°¾Â\  
			pitch = -asin(2.0f * (q[1] * q[3] - q[0] * q[2]));														 //­Á¥õ
			roll  = atan2(2.0f * (q[0] * q[1] + q[2] * q[3]), q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3]);//Â½ºu
			pitch *= 180.0f / PI;
			yaw   *= 180.0f / PI; 
			yaw   -= 13.8f; // Declination at Danville, California is 13 degrees 48 minutes and 47 seconds on 2014-04-04
			roll  *= 180.0f / PI;		 */

			if(az>=0)
				sign=1;
			else
				sign=-1;
			roll = atan2(-ax,(sign * sqrt(az * az + 0.1 * ay * ay))); // mu = 0.1 and sign = +1 for positive az2 and -1 for negative
			pitch = atan2 (ay, sqrt(ax * ax + az * az));
			Xh = hx*cos(pitch) + hy*sin(roll)*sin(pitch) + hz*cos(roll)*sin(pitch);
			Yh = hy*cos(roll) - hz*sin(roll);
			yaw = 360-((atan2(Yh,Xh)+PI)*(180.0f/PI));   

			sprintf(TEXT2,"pitch:%f ",pitch);	
			print_lcd(0,TEXT2);	
			sprintf(TEXT2,"roll:%f ",roll);		  
			print_lcd(1,TEXT2);	
			sprintf(TEXT2,"yaw:%f",yaw);		  
			print_lcd(2,TEXT2);		
			sprintf(TEXT2,"%f %f",Xh,Yh);		  
			print_lcd(3,TEXT2);			 
			/*sprintf(TEXT2,"mx:%f ",hx);	
			print_lcd(0,TEXT2);	
			sprintf(TEXT2,"my:%f ",hy);		  
			print_lcd(1,TEXT2);	
			sprintf(TEXT2,"mz:%f",hz);		  
			print_lcd(2,TEXT2);		
			sprintf(TEXT2,"%f %f",Xh,Yh);	  
			print_lcd(3,TEXT2);	*/	

			/////////////////////////////////////////////  
			sprintf(TEXT2,"x%dy%dz%ds%dd",(int)(hx+180.0f),(int)(hy+180.0f),(int)(hz+180.0f),(int)yaw);		 			
			DrvUART_Write(UART_PORT0,TEXT2,strlen(TEXT2));  
		}
	}
}


