#include <reg52.h>
#include <intrins.h>
//Lidhjet e LCD Modulit
sbit RS = P0^2;                                                                   
sbit EN = P0^3;                            
sbit D4 = P0^4;
sbit D5 = P0^5;
sbit D6 = P0^6;
sbit D7 = P0^7;

void Lcd_Delay(unsigned int itime) {
  unsigned int i,j;
  for (i=0; i<itime; i++)
    for (j=0; j<80; j++);
}

void Lcd4_Port(char a)
{
	if(a & 1)
		D4 = 1;
	else 
		D4 = 0;
	
	if(a & 2)
		D5 = 1;
	else
		D5 = 0;
	
	if(a & 4)
		D6 = 1;
	else
		D6 = 0;
	
	if(a & 8)
		D7 = 1;
	else
		D7 = 0;
}
void Lcd4_Cmd(char a)
{ 
	RS = 0;             // => RS = 0
	Lcd4_Port(a);
	EN  = 1;             // => E = 1
  Lcd_Delay(5);
  EN  = 0;             // => E = 0
}

Lcd4_Clear()
{
	Lcd4_Cmd(0);
	Lcd4_Cmd(1);
}

void Lcd4_Set_Cursor(char a, char b)
{
	char temp,z,y;
	if(a == 1)
	{
	  temp = 0x80 + b;
		z = temp>>4;
		y = temp & 0x0F;
		Lcd4_Cmd(z);
		Lcd4_Cmd(y);
	}
	else if(a == 2)
	{
		temp = 0xC0 + b;
		z = temp>>4;
		y = temp & 0x0F;
		Lcd4_Cmd(z);
		Lcd4_Cmd(y);
	}
}

void Lcd4_Init()
{
	Lcd4_Port(0x00);
	Lcd_Delay(170);
	///////////// Reset process from datasheet /////////
  Lcd4_Cmd(0x03);
	Lcd_Delay(50);
  Lcd4_Cmd(0x03);
	Lcd_Delay(110);
  Lcd4_Cmd(0x03);
  /////////////////////////////////////////////////////
  Lcd4_Cmd(0x02);    
	Lcd4_Cmd(0x02);
  Lcd4_Cmd(0x08); 	
	Lcd4_Cmd(0x00); 
	Lcd4_Cmd(0x0C);     
  Lcd4_Cmd(0x00);    
  Lcd4_Cmd(0x06);   
}

void Lcd4_Write_Char(char a)
{
   char temp,y;
   temp = a&0x0F; 
   y = a&0xF0;	
	 RS = 1;             // => RS = 1
   Lcd4_Port(y>>4);             //Data transfer
	 EN = 1;
	 _nop_();
	 _nop_();
	 EN = 0;
	 Lcd4_Port(temp);
	 EN = 1;
	 _nop_();
	 _nop_();
	 EN = 0;
}

void Lcd4_Write_String(char *a)
{
	int i;
	for(i=0;a[i]!='\0';i++)
	 Lcd4_Write_Char(a[i]);
}
