#include "reg52.h"
#include "lcd.h"
#include <stdio.h>
typedef unsigned char uchar;
typedef unsigned int uint;
sbit jog_R = P1^0;
sbit jog_L = P1^1;
sbit start = P2^0;
sbit stop = P2^1;
sbit reverse = P2^2;
sbit addr_up= P2^6;
sbit addr_low= P2^7;
sbit RS485En=P3^2;
sbit freq_inc=P3^6;
sbit freq_dec=P3^7;
sbit set= P3^3;
sbit pbutton=P3^5;
sbit inc=P2^4;
sbit dec=P3^4;
char ADR;
sbit cont = P2^3;
uchar frame[] ={0x02,0x0E,0x01,	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  0x00,0x00,0x00,0x00};
uchar rx_data;
bit direction;
uint freq;
int split=1000;
#define frame_size 16
uchar ReplyFrame[frame_size];
uchar data_count = 0;
uchar pcount=0;
char pselect=0;
uchar count=0;
uchar Msg[7];
uchar param_h,param_l,PNU;
int val_select=0;
int max_v;

void address();
void parameters();
void Delay(uint itime);
void address();


uint match(uchar char2, uchar char1)
{
   return (char2 << 8) | char1;
}

uchar BCC(uchar *frame)
{
	uchar BCC_value=0;
	int i;
	for(i=0;i<frame_size-1;i++)
	{
	BCC_value ^= frame[i];
	}
	return BCC_value;
}
uchar BCC_rx(uchar *frame)
{
	uchar BCC_value=0;
	int i;
	for(i=0;i<frame_size-1;i++)
	{
	BCC_value ^= frame[i];
	}
	return BCC_value;
} 
void UART_Init()
{
	TMOD = 0x21;	// Timer 1, 8-bit auto reload mode 
	TH1 = 0xEF;		// Load value for 2400 baud rate
	SCON = 0xD0;	// Mode 3, reception enable, with parity bit 
	PCON=0x80;		//double baud rate
	TR1 = 1;		// Start timer 1 

}  


void Transmit_data(uchar tx_data)
{
	 
	ACC = tx_data;
	TB8 = P;			//Ngarkohet parity bit
	SBUF = tx_data;		// Ngarkohet karakt. ne SBUF register 
	while (TI==0);		// Pret deri sa te dergohet
	TI = 0;			    // Clear TI flag  
}

void String(char *str)
{
	int i;
	for(i=0;i<frame_size-1;i++)	// Dergohet secili karakter deri ne fund te frame 
	{
		Transmit_data(str[i]);	// Thirret funksioni transmit data 
	}
	Transmit_data(BCC(str));	
}

void Serial_ISR() interrupt 4    
{
	
 if(TF0==0){
  if(RI==1){

	ACC = SBUF;     
	rx_data = SBUF;
	RI=0;
	if (RB8 == P)
	{
   		// pariteti eshte ne rregull per 'Even' parity systems; 
   		// pariteti gabim per 'Odd' parity systems
		if(data_count==0){
			if(rx_data==0x02)	//frame i ri
			{
				ReplyFrame[data_count++]=rx_data;
			}
			}
		else{
			  ReplyFrame[data_count++]=rx_data;
			  if(data_count>=frame_size){
			   
			   if(ReplyFrame[data_count-1]==BCC_rx(ReplyFrame))
			   {
			   freq=match(ReplyFrame[data_count-3],ReplyFrame[data_count-2]);
			   if(count==0){
  				 val_select=match(ReplyFrame[data_count-7],ReplyFrame[data_count-6]);
				 }
			   }	
			   if(freq>10000){
			   direction=1;
				freq=20000-freq;
			   }else direction=0;
			   data_count=0;
			   TF0=1;
			  }	
		} 
	}
	else
	{
   		// pariteti eshte ne rregull per 'Odd' parity systems; 
   		// pariteti gabim per 'Even' parity systems
	}
 }
 }
} 

 void Timer0_ISR (void) interrupt 1   
{
	
	RS485En=1;
	TH0=0xE8;
	TL0=0x1F;//2tch
	TR0=1;
	while(TF0==0);
	if(jog_R==0){
	 frame[11]=0x05;frame[12]=0x7E;
	}
	else if(jog_L==0){
	 frame[11]=0x06;frame[12]=0x7E;
	}
	else if(stop==0){
	 frame[11]=0x04;frame[12]=0x7E;
	}
	else if(start==0){
	 frame[11]=0x04;frame[12]=0x7F;
	}
	else if(reverse==0){
	 frame[11]=0x0D;frame[12]=0x7F;
	}
	if(pcount==0)
	{
	    if(freq_inc==0){
	 	split+=100;			//P2009=1; //frequency decimal absolute values
		}
		else if(freq_dec==0){
		split-=100;
		}
	}
	frame[13]=split>> 8;
	frame[14]=split& 0xFF;

	if(pselect!=0){
	if(count==1&&set==0){
	frame[3]=param_h;frame[4]=param_l;
	frame[5]=PNU;
	frame[9]=val_select>> 8;
	frame[10]=val_select& 0xFF;
	}
	else if(count==0){
	frame[3]=param_h;frame[4]=param_l;
	frame[5]=PNU;
	frame[9]=0;
	frame[10]=0;
	}
		else{
	  frame[3]=frame[4]=frame[5]=frame[9]=frame[10]=0;
	}
	}
	else{
	  frame[3]=frame[4]=frame[5]=frame[9]=frame[10]=0;
	}

	frame[2]=ADR;
	String(frame);
	RS485En = 0; 
	TH0=0x00;
	TL0=0x00;
	TR0=1;
	TF0 = 0;    
}


void main()
{
	ADR = 0x01;	  

	UART_Init();
	Lcd4_Init();
	TH0=0xE8;
	TL0=0x1F;//2tch
	EA  = 1;		// Enable global interrupt 
	ES = 1;  		// Enable serial interrupt 	
	ET0 = 1;         // Enable Timer0 interrupts
	TR0=1;
	RS485En = 0; // set for rx
	Lcd4_Clear();
    Lcd4_Set_Cursor(1,8);
	Lcd4_Write_String("F:");
    Lcd4_Set_Cursor(1,0);
	Lcd4_Write_String("S:");
	while(1){

		address();

		sprintf(Msg, "%5.2f", (float)freq/100);
		if(direction==1){
		   Lcd4_Set_Cursor(1,10);
		   Lcd4_Write_String("-");
		}
		else
		{
		   Lcd4_Set_Cursor(1,10);
		   Lcd4_Write_String(" ");
		}
		Lcd4_Set_Cursor(1,11);
		Lcd4_Write_String(Msg);

		sprintf(Msg, "%3d", split/100);
		Lcd4_Set_Cursor(1,2);
		Lcd4_Write_String(Msg);	 
if(pbutton==0){
	 Lcd4_Set_Cursor(2,0);
	 pcount++;
	 switch(pcount){
	   case 1:
    	 Lcd4_Write_String("P");
		 break;
	   default:
	   	 pcount=pselect=val_select=0;
         Lcd4_Write_String("            ");
		 break;
	 }
	 Delay(10);
}
if(pcount!=0){
if(pcount==1){
Lcd4_Set_Cursor(2,0);
	 if(inc==0){
		   pselect++;
		 count=0;
		parameters();
		Delay(13);
		}
	if(dec==0){
	   pselect--;
	   count = 0;
	   parameters();
	   Delay(13);
	}		 
}

if(pselect!=0){
if(cont==0){
 count++;
 Delay(10);
} 
if(count>1){
 count=0;
}
if(count==0){
Lcd4_Set_Cursor(2,6);
Lcd4_Write_String("      ");
}
if(count==1){
  if(freq_inc==0){
   val_select++;
   	  Delay(5);
   }
     else  if(freq_dec==0){
   val_select--;
   	  Delay(5);
   }
if(pselect==1){
if(set==0){
      param_h=0x22;param_l=0xBC;
	  }
	  else {
	        param_h=0x12;param_l=0xBC;
	  }
	  max_v=6;	  
}
else if(pselect==2){
if(set==0){
      param_h=0x23;param_l=0xE8;
	  }
	  else{
	   param_h=0x13;param_l=0xE8;
	   }
	  max_v=6; 
}
else if(pselect==3){
if(set==0){
      param_h=0x20;param_l=0x0B;
	  PNU=0x80;		//nese parametri >1999  bit15
	  }
	  else {
	        param_h=0x10;param_l=0x0B;
	  PNU=0x80;		//nese parametri >1999  bit15
	  }
	  max_v=31;
 }
 if(val_select>max_v){
	    val_select=0;
	 }
if(val_select<0){
	    val_select=max_v;
	 }
	  Lcd4_Set_Cursor(2,6);
      sprintf(Msg, "%d ", val_select);
      Lcd4_Write_String(Msg);
}

}	  
}
}
}


void Delay(uint itime)
{
uint i,j;
for (i=0;i<itime;i++) // 1ms delay
{
for (j=0;j<667;j++);//667 1ms
}
}

void address(){
		if(addr_up==0){
			ADR +=1;
			Delay(8);
      	}
    	else if (addr_low==0){
			ADR-=1;
		   	Delay(8);
	    }
		if(ADR>31){
 			ADR=0;
		}
		if(ADR<0){
 			ADR=31;
		}
		sprintf(Msg, "A%Bu ",ADR);
		Lcd4_Set_Cursor(2,13);
        Lcd4_Write_String(Msg);
}

void parameters(){
 switch(pselect){
		  case 1:
		  	 param_h=0x12;param_l=0xBC;
			 PNU=0x00;
			 Lcd4_Write_String("P0700:   ");
			 break;
		  case 2:
		  	 param_h=0x13;param_l=0xE8;
			 PNU=0x00;
			 Lcd4_Write_String("P1000:   ");
			 break;
	      case 3:
		  	 param_h=0x10;param_l=0x0B;
	 		 PNU=0x80;
			 Lcd4_Write_String("P2011:   ");
			 break;
		  case 4:
		  	 param_h=0x10;param_l=0x18;
	 		 PNU=0x80;
			 Lcd4_Write_String("r2024:   ");
			 break;
		  case 5:
		  	 param_h=0x10;param_l=0x19;
	 		 PNU=0x80;
			 Lcd4_Write_String("r2025:   ");
			 break;
		  default:
		  	 param_h=0x00;param_l=0x00;
	 		 PNU=0x00;
			 val_select=0;
			 Lcd4_Write_String("P        ");
			 break;

}
if(pselect>5){
pselect=0;
}
if(pselect<0){
pselect=6;
}
 
}
