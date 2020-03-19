#include "reg51.h"
#include "intrins.h"

#define FOSC    15968000L
#define BAUD    2400

typedef unsigned char BYTE;
typedef unsigned int WORD;

/*Declare SFR associated with the ADC */
sfr ADC_CONTR   =   0xC5;           //ADC control register
sfr ADC_DATA    =   0xC6;           //ADC high 8-bit result register
sfr ADC_LOW2    =   0xBE;           //ADC low 2-bit result register
sfr P1M0        =   0x91;           //P1 mode control register0
sfr P1M1        =   0x92;           //P1 mode control register1

/*Define ADC operation const for ADC_CONTR*/
#define ADC_POWER   0x80            //ADC power control bit
#define ADC_FLAG    0x10            //ADC complete flag
#define ADC_START   0x08            //ADC start control bit
#define ADC_SPEEDLL 0x00            //420 clocks
#define ADC_SPEEDL  0x20            //280 clocks
#define ADC_SPEEDH  0x40            //140 clocks
#define ADC_SPEEDHH 0x60            //70 clocks

void InitUart();
void InitADC();
void SendData(BYTE dat);
BYTE GetADCResult(BYTE ch);
void Delay(WORD n);
void ShowResult(BYTE ch);
BYTE receive_date();

sbit w=P3^7;//选择使用自动巡线还是蓝牙体感 高电平自动寻线

//IN1 1.7
//IN2 1.6
//IN3 3.4
//IN4 3.5

sbit IN3=P3^4;
sbit IN4=P3^5;

sbit IN1=P1^7;
sbit IN2=P1^6;

//l,r 控制左右电机启动或关闭，0打开，1关闭
//1_g 控制电机前进还是后退 1,go 0,back

void main()
{
	BYTE i,max,tar,j,recv,cmd;
	BYTE value[3];
	InitUart();                     //Init UART, use to show ADC result
	InitADC();                      //Init ADC sfr
	while (1)
	{
		cmd=w;
		if(w)
		{
			i=0;
			max=0;
			tar=0;
			j=0;
			while(i<3)
			{
				value[i]=GetADCResult(i+2);
				if(value[i]>max)
				{
					max=value[i];
				}
				i=i+1;			
			}
			while(j<3)
			{
				if(value[j]==max)
				{
					tar=j;
				}
				j=j+1;
			}
			if(tar==0)
			{
				IN1=0;IN2=0;IN3=0;IN4=1;
			}
					if(tar==1)
			{
				 IN1=0;IN2=1;IN3=0;IN4=1;
			}
					if(tar==2)
			{
				IN1=0;IN2=1;IN3=0;IN4=0;
			}
		}
		else
		{
			recv=receive_date();	
			switch(recv)
			{
				case 0x30:IN1=0;IN2=0;IN3=0;IN4=0;SendData(0x30);break;
				
				case 0x34:IN1=0;IN2=0;IN3=0;IN4=1;SendData(0x34);break;
				case 0x32:IN1=0;IN2=1;IN3=0;IN4=1;SendData(0x32);break;
				case 0x35:IN1=0;IN2=1;IN3=0;IN4=0;SendData(0x35);break;
				
				case 0x36:IN1=0;IN2=0;IN3=1;IN4=0;SendData(0x36);break;
				case 0x37:IN1=1;IN2=0;IN3=1;IN4=0;SendData(0x37);break;
				case 0x38:IN1=1;IN2=0;IN3=0;IN4=0;SendData(0x38);break;
				
				default:IN1=0;IN2=0;IN3=0;IN4=0;SendData(0xff);
			}
		}		
	}
}

/*----------------------------
Send ADC result to UART
----------------------------*/
void ShowResult(BYTE ch)
{
    SendData(ch);                   //Show Channel NO.
    SendData(GetADCResult(ch));     //Show ADC high 8-bit result

//if you want show 10-bit result, uncomment next line
//    SendData(ADC_LOW2);             //Show ADC low 2-bit result
}

/*----------------------------
Get ADC result
----------------------------*/
BYTE GetADCResult(BYTE ch)
{
    ADC_CONTR = ADC_POWER | ADC_SPEEDH | ch | ADC_START;
    _nop_();                        //Must wait before inquiry
    _nop_();
    _nop_();
    _nop_();
    while (!(ADC_CONTR & ADC_FLAG));//Wait complete flag
    ADC_CONTR &= ~ADC_FLAG;         //Close ADC

    return ADC_DATA;                //Return ADC result
}

/*----------------------------
Initial UART
----------------------------*/
void InitUart()
{
    SCON = 0x5a;                    //8 bit data ,no parity bit
    TMOD = 0x20;                    //T1 as 8-bit auto reload
    TH1 = TL1 = -(FOSC/12/32/BAUD); //Set Uart baudrate
    TR1 = 1;                        //T1 start running
}

/*----------------------------
Initial ADC sfr
----------------------------*/
void InitADC()
{
    P1 = P1M0 = P1M1 = 0x3c;        //Set all P1 as Open-Drain mode  0011 1100
    ADC_DATA = 0;                   //Clear previous result
    ADC_CONTR = ADC_POWER | ADC_SPEEDLL;
    Delay(2);                       //ADC power-on and delay
}

/*----------------------------
Send one byte data to PC
Input: dat (UART data)
Output:-
----------------------------*/
void SendData(BYTE dat)
{
    while (!TI);                    //Wait for the previous data is sent
    TI = 0;                         //Clear TI flag
    SBUF = dat;                     //Send current data
}

/*----------------------------
Software delay function
----------------------------*/
void Delay(WORD n)
{
    WORD x;

    while (n--)
    {
        x = 5000;
        while (x--);
    }
}

/*----------------------------
receive char
----------------------------*/
BYTE receive_date()
{
    while (!RI);                    
    RI = 0;    
    return SBUF;                   
}