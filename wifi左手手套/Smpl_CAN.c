// pin32 GPB0/RX0 to another board's UART TX
// pin33 GPB1/TX0 to another board's UART RX
// E_FUNC_I2C1,SDA1:GPA-10,SCL1:GPA-11
//PA10 pin10    PA11 pin9
//PA7 pin78  ~ PA0 pin71
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "NUC1xx.h"
#include "Driver\DrvGPIO.h"
#include "Driver\DrvSYS.h"
#include "Driver\DrvTIMER.h"
#include "Driver\DrvUART.h"
#include "Driver\DrvADC.h"
#include "LCD_Driver.h"
#include "Driver\DrvI2C.h"
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
int16_t accelCount[3];  // Stores the 16-bit signed accelerometer sensor output
float gyroBias[3] = {0, 0, 0};// Bias corrections for gyro and accelerometer
float accelBias[3] = {0, 0, 0}; 

void ADC_initialize (void)
{
	DrvADC_Open(ADC_SINGLE_END,ADC_SINGLE_CYCLE_OP,0x17,EXTERNAL_12MHZ,0x9);   //  0x17 adc0,1,2,4   00010111
	DrvSYS_Delay(100); 
}

static int32_t ADC_average (int32_t *ADC_value)  
{
	int32_t value[9] = {0};
	uint8_t i,j,k;
	int32_t value_max;
	for (i=0; i<9; i++)
	{
		value[i] = *ADC_value++;
	}
	for (j=0; j<9; j++)
	{
		for (k=0; k<9-j; k++) 
		{
			if(k != 8)
			{
				if (value[k] > value[k+1]) 
				{
					value_max = value[k];
					value[k] = value[k+1]; 
					value[k+1] = value_max;
				}
			}
		}
	}
	return (value[4]);
}

static int32_t get_ADC_value(uint8_t ADC_channel_number)
{
	uint8_t i;
	int32_t ADC_calibration[9]={0}; 
	for (i=0; i<9; i++)
	{
		DrvADC_StartConvert(); 
		while(!DrvADC_IsConversionDone()) 
		{
		}
		ADC_calibration[i] = DrvADC_GetConversionData(ADC_channel_number); 
		DrvADC_StopConvert(); 
	}
	return (ADC_average(ADC_calibration)); 
}

void InitHCLK12M(void)
{
	UNLOCKREG();
	//External 4~24 MHz High Speed Crystal Enable (writeprotectionbit)
	SYSCLK->PWRCON.XTL12M_EN = 1;
	//HCLK clock source select (write-protection bits)
	//000 = Clock source from external 12 MHz
	SYSCLK->CLKSEL0.HCLK_S = 0;
	LOCKREG();
}

////////////////////////////////////I2C³¡¤À/////////////////////////

void  writeByte(uint8_t addr ,uint8_t reg , uint8_t data)
{
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

void delay_time(int time)		     // 1 secend
{
	int i=0;
	for(i=0;i<time*3;i++)
	{
		DrvSYS_Delay(330000);
	}
}

void Init_LED()
{
	DrvGPIO_Open(E_GPC, 8, E_IO_INPUT);  //button
	DrvGPIO_Open(E_GPC, 0, E_IO_OUTPUT); //wifi_init
	DrvGPIO_Open(E_GPC, 1, E_IO_OUTPUT);
	DrvGPIO_Open(E_GPC, 2, E_IO_OUTPUT);
	
	DrvGPIO_ClrBit(E_GPC, 0);     //wifi_init
	DrvGPIO_ClrBit(E_GPC, 1);
	DrvGPIO_ClrBit(E_GPC, 2);
}

int main (void)
{
	STR_UART_T sParam;     
	int command;
	int32_t adc1=0,adc2=0,adc3=0,adc4=0;
	float ax,ay,az;
	char textout[30];
	
	//Initial 12M and set HCLK=12MHz
	InitHCLK12M();
	//Initial ADC
	ADC_initialize();
	Init_LED();
	UNLOCKREG();
	DrvSYS_Open(48000000);
	DrvSYS_SetOscCtrl(E_SYS_XTL12M,1);
	DrvSYS_Delay(20000);/* Delay for Xtal stable */
	while(!SYSCLK->CLKSTATUS.XTL12M_STB);
	DrvSYS_SelectHCLKSource(0);
	LOCKREG();
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
	//wifi
  delay_time(10);	
	DrvUART_Write(UART_PORT0,"AT+CIPSTART=\"TCP\",\"192.168.4.1\",8087\r\n",39);
	delay_time(1);
	
	resetMPU9250();
	calibrateMPU9250(gyroBias, accelBias);
	initMPU9250();
	DrvSYS_Delay(300000);
	DrvGPIO_SetBit(E_GPC, 0);     //finish init LED 
	while(1)
	{
		writeByte(0x68, 0x37,0x02);
		writeByte(0x68, 0x6A,0x01);
		DrvSYS_Delay(20000);	
		readAccelData(accelCount);
		ax = accelCount[0]*2.0/32768.0- accelBias[0];	  //accel  value
		ay = accelCount[1]*2.0/32768.0- accelBias[1]; 
		az = accelCount[2]*2.0/32768.0- accelBias[2];
		adc1=get_ADC_value(0);
		adc2=get_ADC_value(1);   
		adc3=get_ADC_value(2);   
		adc4=get_ADC_value(4);

		//hand action
		if(ax>=0.7f)      //right
			command=1;
		else if(ax<=-0.7f)    //left
			command=2;
		else if(ay>=0.5f)       //up
			command=3;	
		else if(ay<=-0.5f)		//down
			command=4;
		else 
			command=0;
		
		if(adc1>1500 || adc2>1500 || adc3>1500 || adc3>1500)
		{
			DrvGPIO_ClrBit(E_GPC, 2);
			//finger action
			/*if(adc1<=2810 && adc2<=2800 && adc3<=2900 && adc4<=2900)
			{
				command=5;           //all finger
			}
			else if(adc1<=2810 && adc2<=2800)    //finger3,4
			{
				command=8;
			}	
			else if(adc3<=2900 && adc4<=2900)    //finger1,2
			{
				command=9;
			}	
			else if(adc3<=2900)    //finger1
			{
				command=6;
			}
			else if(adc4<=2900)    //finger2
			{
				command=7;
			}*/
			if(adc1<=2810 && adc2<=2800 && adc3<=2900)
			{
				command=5;           //all finger
			}
		}
		else
		{
			DrvGPIO_SetBit(E_GPC, 2);
		}
		//No action
		if(adc1>2810 && adc2>2800 && adc3>2900 && adc4>2900 && ax<0.7 && ax>-0.7 && ay<0.5 && ay>-0.5)
		{
			command=0;
		}
		//WiFi send data
		if(DrvGPIO_GetBit(E_GPC, 8)==0)
		{
			/*sprintf(textout,"a%d\r\n",get_ADC_value(0));	
			DrvUART_Write(UART_PORT0,textout,strlen(textout)); 
			sprintf(textout,"b%d\r\n",get_ADC_value(1));	
			DrvUART_Write(UART_PORT0,textout,strlen(textout)); 
			sprintf(textout,"c%d\r\n",get_ADC_value(2));	
			DrvUART_Write(UART_PORT0,textout,strlen(textout)); 
			sprintf(textout,"d%d\r\n\r\n",get_ADC_value(4));	
			DrvUART_Write(UART_PORT0,textout,strlen(textout)); 
			DrvSYS_Delay(200000);*/
			
			DrvGPIO_SetBit(E_GPC, 1);
			DrvUART_Write(UART_PORT0,"AT+CIPSEND=3\r\n",14);
			DrvSYS_Delay(200000);
			sprintf(textout,"s0%d",command);	
			DrvUART_Write(UART_PORT0,textout,3); 
			DrvSYS_Delay(200000);
		}
		else
		{
			DrvGPIO_ClrBit(E_GPC, 1);
		}
	}
}