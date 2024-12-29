#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include <curses.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#define NUM 7
#define PWM 1
typedef struct
{
	signed char vel_x;
	signed char vel_y;
	signed char vel_z;
	
	bool end;
}VELOCITY;
typedef struct
{
	unsigned char Frequency_A;
	unsigned char Frequency_B;
	unsigned char Frequency_C;
	unsigned char Frequency_D;
	unsigned char Frequency_E;
	
	bool end;
}FREQUENCY;
void Vel_Select(int ch,VELOCITY* input);
signed char limit_Data(signed char val, signed char low,signed char high);
void Send_Array(int Serial,int num,const char * input_array);
void Send_Vel_Array(VELOCITY*Vel,char *input_array);
void PWM_Init(void);
void PWM_Select(float duty);
void Fre_Select(int ch,FREQUENCY* input);
void Send_Array_Fre(FREQUENCY*Fre,char*input_array);

float duty=0;
int main(void)
{
	initscr();
	cbreak();
	noecho();
	refresh();
	printf("Welcome use the FISH CONTROL SYSTEM\r\n");
	char Send_Data[NUM]={0xAA,0,0,0,0,0,0xCD};
	int fd;
	int ch;
	int recbyte_num=0;
	int receive_data=0;
	VELOCITY vel={0};
	FREQUENCY fre={0};
	//open the serial
	if(wiringPiSetup()<0)return 1;
	PWM_Init();
	while(!fre.end)
	//w:119 a:97 s:115 x:120 f:102 e:101 r:114
	{
		if((fd=serialOpen("/dev/ttyAMA0",115200))<0)return -1;
		//get the key value
		ch=getch();
		Fre_Select(ch,&fre);
		//Serial print Part
		recbyte_num=serialDataAvail(fd);
		while(recbyte_num--)
		{
			receive_data=serialGetchar(fd);
		}
		Send_Array_Fre(&fre,Send_Data);
		Send_Array(fd,NUM,Send_Data);
		
		serialClose(fd);
		//Vel output part
		printf("         Fre1=%d,Fre2=%d,Fre3=%d,Fre4=%d,Fre5=%d\r\n",fre.Frequency_A,fre.Frequency_B,fre.Frequency_C,fre.Frequency_D,fre.Frequency_E);
		printf("   Designed by CHM   press X to exit input=%d\r\n",ch);
		printf("press c-f-v-g-b-h-n-j-m-k to control frequency\r\n");
		printf("   press X to exit,press z to zero all frequency\r\n");
		if(receive_data==1)printf("              Connect correctly\r\n");
		else printf("              Disconnect!!!\r\n");
		for(int i=0;i<7;i++)
		{
			printf("\r\n");
		}
		fflush(stdout);
	}
	endwin();
	return 0;
}
void Fre_Select(int ch,FREQUENCY* input)
{
	if(ch==99)input->Frequency_A--;
	else if(ch==102)input->Frequency_A++;
	
	else if(ch==118)input->Frequency_B--;
	else if(ch==103)input->Frequency_B++;
	
	else if(ch==98)input->Frequency_C--;
	else if(ch==104)input->Frequency_C++;
	
	else if(ch==110)input->Frequency_D--;
	else if(ch==106)input->Frequency_D++;
	
	else if(ch==109)input->Frequency_E--;
	else if(ch==107)input->Frequency_E++;
	
	if(ch==122)
	{
		input->Frequency_A=0;
		input->Frequency_B=0;
		input->Frequency_C=0;
		input->Frequency_D=0;
		input->Frequency_E=0;
	}
	else if(ch==97)
	{
		duty=duty+0.5;
		if(duty>=1.1)duty=0.0f;
		PWM_Select(duty);
	}
	else if(ch==120){
		input->end=true;
		input->Frequency_A=0;
		input->Frequency_B=0;
		input->Frequency_C=0;
		input->Frequency_D=0;
		input->Frequency_E=0;
	}
	
	input->Frequency_A=limit_Data(input->Frequency_A,0,50);
	input->Frequency_B=limit_Data(input->Frequency_B,0,50);
	input->Frequency_C=limit_Data(input->Frequency_C,0,50);
	input->Frequency_D=limit_Data(input->Frequency_D,0,50);
	input->Frequency_E=limit_Data(input->Frequency_E,0,50);
}
void Vel_Select(int ch,VELOCITY* input)
{
	
	if(ch==97)input->vel_x++;
	else if(ch==100)input->vel_x--;
	else if(ch==119)input->vel_y++;
	else if(ch==115)input->vel_y--;
	else if(ch==114)input->vel_z++;
	else if(ch==101)input->vel_z--;
	else if(ch==102)
	{
		duty=duty+0.5;
		if(duty>=1.1)duty=0.0f;
		PWM_Select(duty);
	}
	else if(ch==120){
		input->end=true;
		input->vel_x=0;
		input->vel_y=0;
		input->vel_z=0;
	}
	
	input->vel_x=limit_Data(input->vel_x,-10,10);
	input->vel_y=limit_Data(input->vel_y,0,10);
	input->vel_z=limit_Data(input->vel_z,-10,10);
}
signed char limit_Data(signed char val, signed char low,signed char high)
{
	if(val>high)return 0;
	else if(val<low)return low;
	else return val;
}
void Send_Array(int Serial,int num,const char * input_array)
{
	int i=0;
	for(i=0;i<num;i++){
		char temp_data=*(input_array+i);
		serialPutchar(Serial,temp_data);
	}
	
}
void Send_Array_Fre(FREQUENCY*Fre,char*input_array)
{
	char *p=input_array;
	*(p+1)=Fre->Frequency_A;
	*(p+2)=Fre->Frequency_B;
	*(p+3)=Fre->Frequency_C;
	*(p+4)=Fre->Frequency_D;
	*(p+5)=Fre->Frequency_E;
}
void Send_Vel_Array(VELOCITY*Vel,char *input_array)
{
	char *p=input_array;
	*(p+1)=Vel->vel_x;
	*(p+2)=Vel->vel_y;
	*(p+3)=Vel->vel_z;
}
void PWM_Init(void)
{
	pinMode(PWM,PWM_OUTPUT);
	pwmSetMode(PWM_MODE_MS);
	pwmSetRange(1024);//pwm ARR=1024
	pwmSetClock(25);//19.2MHz/75/1024=750Hz
	pwmWrite(PWM,0);
}
void PWM_Select(float duty)
{
	pwmWrite(PWM,(int)1024*duty);
}
