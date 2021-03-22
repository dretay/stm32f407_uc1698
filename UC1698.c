#include "UC1698.h"
//#include "pic_data.h"

//constants
u8 Contrast_level = 0xbf;

//macros

#define	GPIO_SET(x)		{HAL_GPIO_WritePin(LCD_##x##_GPIO_Port, LCD_##x##_Pin,GPIO_PIN_SET);}
#define	GPIO_RESET(x)		{HAL_GPIO_WritePin(LCD_##x##_GPIO_Port, LCD_##x##_Pin,GPIO_PIN_RESET);}

#define BLINK_GPIOx(_N)       ((GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE-GPIOA_BASE)*(_N)))
#define BLINK_GPIOx_BSRRL(_N) (*((volatile uint16_t*)&(BLINK_GPIOx(_N)->BSRR)))
#define BLINK_GPIOx_BSRRH(_N) (*((volatile uint16_t*)(&BLINK_GPIOx_BSRRL(_N)+1)))

#define	LCD_WRITE_DATA(data)	GPIOD->ODR = 0x00FF;GPIOD->ODR = (data) ;		//Write data

static void write_command(u8 cmd);
static void write_data(u8 cmd);
static void display_address();
static void display_black();
static void display_white();
static void text_dot(u8 data1, u8 data2);
static void Data_processing(u8 temp);



static void init(void)
{	
	//reset();	

	/*power control*/					
	write_command(0xe9);    			//Bias Ratio:1/10 bias
	write_command(0x2b);    			//power control set as internal power
	write_command(0x24);    			//set temperate compensation as 0%

	
	u8 Contrast_level = 0xbf;
	write_command(0x81);
	write_command(Contrast_level);	

	/*display control*/
	write_command(0xa4);    			//all pixel off 
	write_command(0xa6);    			//inverse display off

	/*lcd control*/
	write_command(0xc4);    			//Set LCD Maping Control (MY=1, MX=0)
	write_command(0xa1);    			//line rate 15.2klps
	write_command(0xd1);    			//rgb-rgb
	write_command(0xd5);    			//4k color mode
	write_command(0x84);    			//12:partial display control disable


	/*n-line inversion*/
	write_command(0xc8);
	write_command(0x10);    			//enable NIV, 11 lines

	/*com scan fuction*/
	write_command(0xda);    			//enable FRC,PWM,LRM sequence

	/*window*/
	write_command(0xf4);    			//wpc0:column
	write_command(0x25);    			//start from 112
	write_command(0xf6);    			//wpc1
	write_command(0x5A);    			//end:272

	write_command(0xf5);    			//wpp0:row
	write_command(0x00);    			//start from 0
	write_command(0xf7);    			//wpp1
	write_command(0x9F);    			//end 160

	write_command(0xf8);     		//inside mode 

	write_command(0x89);    			//RAM control

	
	/*scroll line*/
	write_command(0x40);    			//low bit of scroll line
	write_command(0x50);    			//high bit of scroll line

	write_command(0x90);    			//14:FLT,FLB set
	write_command(0x00);

	/*partial display*/
	write_command(0x84);    			//12,set partial display control:off
	write_command(0xf1);    			//com end
	write_command(0x9f);    			//160
	write_command(0xf2);    			//display start 
	write_command(0);    			//0
	write_command(0xf3);    			//display end
	write_command(159);    			//160

	display_address();
	display_white();

	write_command(0xad);    			//display on,select on/off mode.Green Enhance mode disable	 	
}
void Data_processing(u8 temp)  //turns 1byte B/W data to 4k-color data(RRRR-GGGG-BBBB)   
{
	unsigned char temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
	unsigned char h11, h12, h13, h14, h15, h16, h17, h18, d1, d2, d3, d4;

	temp1 = temp & 0x80;
	temp2 = (temp & 0x40) >> 3;
	temp3 = (temp & 0x20) << 2;
	temp4 = (temp & 0x10) >> 1;
	temp5 = (temp & 0x08) << 4;
	temp6 = (temp & 0x04) << 1;
	temp7 = (temp & 0x02) << 6;
	temp8 = (temp & 0x01) << 3;
	h11 = temp1 | temp1 >> 1 | temp1 >> 2 | temp1 >> 3;
	h12 = temp2 | temp2 >> 1 | temp2 >> 2 | temp2 >> 3;
	h13 = temp3 | temp3 >> 1 | temp3 >> 2 | temp3 >> 3;
	h14 = temp4 | temp4 >> 1 | temp4 >> 2 | temp4 >> 3;
	h15 = temp5 | temp5 >> 1 | temp5 >> 2 | temp5 >> 3;
	h16 = temp6 | temp6 >> 1 | temp6 >> 2 | temp6 >> 3;
	h17 = temp7 | temp7 >> 1 | temp7 >> 2 | temp7 >> 3;
	h18 = temp8 | temp8 >> 1 | temp8 >> 2 | temp8 >> 3;
	d1 = h11 | h12;
	d2 = h13 | h14;
	d3 = h15 | h16;
	d4 = h17 | h18;

	write_data(d1);
	write_data(d2);
	write_data(d3);
	write_data(d4);
}
static void disppic(unsigned char *pic)
{
	uint k = 0;
	uint i, j;
	u8 temp;
	for (i = 0; i < 160; i++) // 160*160 B/W picture for example
		{
			for (j = 0; j < 20; j++) // 160 dot/ 8 bite=20 byte
				{
					temp = pic[k++]; 
					Data_processing(temp);
				}
			write_data(0x00); //There are 162-160=2 segment need to write data*/
		}

}
static void display_black()
{
	uint i, j;
	for (i = 0; i < 160; i++)
	{	   
		for (j = 0; j < 81; j++)
		{
			write_data(0xff);
		}
	}
}
void text_dot(u8 data1, u8 data2)
{
	uint i, j;
	for (i = 0; i < 80; i++)
	{
		for (j = 0; j < 81; j++)
		{ 
			write_data(data1); 
		}
		for (j = 0; j < 81; j++)
		{
			write_data(data2); 
		}
	}
}
static void display_white()
{
	uint i, j;
	for (i = 0; i < 160; i++)
	{	  
		for (j = 0; j < 81; j++)
		{ 
			write_data(0x00); 
		}
	}
}
static void display_address()
{
	write_command(0x60);  			//row address LSB
	write_command(0x70);  			//row address MSB
	write_command(0x12);  			//column address MSB
 	write_command(0x05);  			//column address LSB        
}
//static void write_command(u8 cmd)
//{
//	GPIO_RESET(CE); //cs0
//	GPIO_RESET(CD); //cd
//	GPIO_SET(RD); //wr1
//	GPIO_RESET(WR); //wr0
//
//	LCD_WRITE_DATA(cmd);
//
//	GPIO_SET(WR); //wr0
//	GPIO_SET(CE); //cs0
//}
//static void write_data(u8 data)
//{
//	GPIO_RESET(CE);  //cs0
//	GPIO_SET(CD);  //cd
//	GPIO_SET(RD);  //wr1
//	GPIO_RESET(WR);  //wr0
//
//	LCD_WRITE_DATA(data);
//
//	GPIO_SET(WR);  //wr0
//	GPIO_SET(CE);  //cs0
//}
static void Write_number(u8 x, u8 y, u8 *n, u8 k)
{
	u8 i, j, xi; 
	x = 37 + x;
	xi = 0x00 | (x & 0x0f);    	//lsb
	x = 0x10 | ((x & 0xf0) >> 4);  	//msb  

for(i = 0 ; i < 14 ; i++)
	{
		write_command(xi);
		write_command(x);
		write_command(0x60 | (y & 0x0f));
		write_command(0x70 | ((y & 0xf0) >> 4));

		{
			j = (*(n + 14*k + i));
			Data_processing(j);
		}
		write_data(0x00);
		y++;
	}
}
unsigned char picture2[]= {
0xFF,0xFF,0xFF,0xFF,0x80,0x80,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x9F,
0xFF,0xFF,0xF8,0x00,0x90,0x00,0x00,0x08,0x00,0x90,0x00,0x00,0x08,0x00,0x90,0x00,
0x00,0x08,0x00,0x90,0x00,0x00,0x08,0x00,0x90,0x20,0x20,0x08,0x00,0x90,0x60,0xE0,
0x08,0x00,0x90,0xA0,0x20,0x08,0x00,0x91,0x20,0x20,0x08,0x00,0x91,0x20,0x20,0x08,
0x00,0x92,0x20,0x20,0x08,0x00,0x92,0x20,0x20,0x08,0x00,0x94,0x20,0x20,0x08,0x00,
0x97,0xF8,0x20,0x08,0x00,0x90,0x20,0x20,0x08,0x00,0x90,0x20,0x20,0x08,0x00,0x90,
0x20,0x20,0x08,0x00,0x90,0xF8,0xF8,0x08,0x00,0x90,0x00,0x00,0x08,0x00,0x90,0x00,
0x00,0x08,0x00,0x90,0x00,0x00,0x08,0x00,0x90,0x00,0x00,0x08,0x00,0x90,0x00,0x00, 
0x08,0x00,0x90,0x00,0x00,0x08,0x00,0x9F,0xFF,0xFF,0xF8,0x00,0x80,0x00,0x00,0x00,
0x00,0x80,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x80
}; 
void window_display(void)
{
	int k=0;
	int i,j;
	int m=0;
	unsigned char temp,temp1,temp2,temp3,temp4,temp5,temp6,temp7,temp8;
	unsigned char h11,h12,h13,h14,h15,h16,h17,h18,d1,d2,d3,d4;
	write_command(0x70);  //set row msb address
	write_command(0x60);  //set row lsb address
	write_command(0x10);  //set column msb address
	write_command(0x00);  //set column lsb address
	write_command(0xf4);  //set column start address
	write_command(0x00);  //column start address=00
	write_command(0xf6);  //set column end address
	write_command(0x0a);  //column end address=11*3RGB=33 segment
	write_command(0xf5);  //set row start address
	write_command(0x00);  // row start address=00
	write_command(0xf7);  //set row end address
	write_command(0x1f);  // row end address=32
	write_command(0xf8);  // inside mode
	for(i=0;i<32;i++) // 33*32 B/W picture for example
	{
 		for(j=0;j<4;j++)
 		{
//	 		Data_processing(picture2[k++]);   // turns 1byte B/W data to 4k-color data(RRRR-GGGG-BBBB)
//			temp1=temp&0x80;
//			temp2=(temp&0x40)>>3;
//			temp3=(temp&0x20)<<2; 
//			temp4=(temp&0x10)>>1;
//			temp5=(temp&0x08)<<4;
//			temp6=(temp&0x04)<<1;
//			temp7=(temp&0x02)<<6;
//			temp8=(temp&0x01)<<3;
//			h11=temp1|temp1>>1|temp1>>2|temp1>>3;
//			h12=temp2|temp2>>1|temp2>>2|temp2>>3;
//			h13=temp3|temp3>>1|temp3>>2|temp3>>3;
//			h14=temp4|temp4>>1|temp4>>2|temp4>>3;
//			h15=temp5|temp5>>1|temp5>>2|temp5>>3;
//			h16=temp6|temp6>>1|temp6>>2|temp6>>3;
//			h17=temp7|temp7>>1|temp7>>2|temp7>>3;
//			h18=temp8|temp8>>1|temp8>>2|temp8>>3;
//			d1=h11|h12;
//			d2=h13|h14;
//			d3=h15|h16;
//			d4=h17|h18;
	 		write_data(0xFF);
	 		write_data(0xFF);
	 		write_data(0xFF);
	 		write_data(0xFF);
		}
//		write_data(picture2[k++]);
		m=m+1; //must be set row address increase
		write_command(0x0070 | ((m & 0xf0) >> 4));  //set row msb address
		write_command(0x0060 | (m & 0x0f));  //set row lsb address
	}
} 


static void show_something()
{

//	display_address();
//	disppic(pic_data1);
	//	display_address();
	//	Write_number(20, 50, num, 2);
	window_display();

}


const struct uc1698 UC1698 = { 
	.init = init,		
};