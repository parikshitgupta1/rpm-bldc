# include at89x52.h  
# include string.h
# define LCDPort P2

sbit RS=P3^7;register select pin of lcd connected at p3.7 //declaration of pin connections
sbit RW=P3^6;read right pin of lcd connected at p3.6
sbit EN=P3^5;enable pin of lcd connected at p3.5

sbit col1= P1^6;keypad pins
sbit col2= P1^5;keypad pins
sbit col3= P1^4;keypad pins
sbit col4= P1^3;keypad pins

sbit row1= P1^2;keypad pins
sbit row2= P1^1;keypad pins
sbit row3= P1^0;keypad pins

sbit pwm_pin=P0^7;pwm pin
sbit irr=P3^2;

unsigned char ascii_tab[]={'1','2','3','4','5','6','7','8','9','','0','#'};
unsigned char ascii_tab2[]={'0','1','2','3','4','5','6','7','8','9',};
idata unsigned int rpm_arr[10];
unsigned int t0_counter=0,timer_5sec=0,key=0;
unsigned char scan_no,cnt,dcnt,krcount,key_code='0',key_code1='0',key_code2='0',key_code3='0',key_code4='0',ds1,ds2,ds3,ds4,ds11,ds21,ds31,ds41,res;
unsigned int int0_count=0,counter=0,curr_rpm=0,pwm_val=0,on_time,off_time,pwm_val2;
static unsigned int timer_ov=0,norm_rpm=0;
unsigned int high_count=0,low_count=0;
unsigned int timer1_ov_count=0,curr_pwm=100,exp_rpm=0,exp_pwm=0,pwm_percent=1;
bit timer_exit=0,update_flag=0,rpm_update=0;
bit key_ready,nkp,tb,buzzer,chk,s1_over; // declarations of variables

void init();              //declaration of functions 
void init_display();
void init_keypad();
void init_timer0();
void init_timer1();
void delay(unsigned int delay_ms);
void lcd_cmd(unsigned char cmd);
void lcd_init();
void lcd_string(char str);
void lcd_data(unsigned char data1);
void scanner();
void get_key();
void key_release();
void k();
void display();
void measure_rpm();
void delay_1sec();
void result();
void key_disp();
void pwm();
void pwm_speed();

void ex0_isr (void) interrupt 0exeternal interrupt //first fuction to run and according to flag displays speed
{
	if(update_flag)
	{
		int0_count++;
		if(int0_count==2)
		{
			TR0=0;stop timer
			high_count=TH0;load timer count
			low_count=TL0;load timer count
			measure_rpm();
			timer_ov=0;
			TH0=TL0=0;
			int0_count=0;
			timer_exit=1;
		}
		else
		{
			TR0=1;start timer
		}	
	}
}

void isr_t0() interrupt 1 using 1 timer 0 interrupt
{
		timer_ov++;
		init_timer0();
}

void isr_t1() interrupt 3timer 1 interrupt
{	
	{
		if(exp_rpmcurr_rpm)
		{
			pwm_percent++;
			if(pwm_percent500)
			{
				pwm_percent=500;
			}
		}
		else if(exp_rpmcurr_rpm)
		{
			pwm_percent--;
			if(pwm_percent0)
			{
				pwm_percent=0;
			}
		}
	}
		scanner();
		init_timer1();
		if(timer1_ov_countpwm_percent)
			pwm_pin=0;
		else 
			pwm_pin=1;
		if(timer1_ov_count==500)
			timer1_ov_count=0;
}

void main()     //main function that calls all sub functions
{
	pwm_pin=0;
	irr=1;
	-----------------------------------------------
Configure INT0 (external interrupt 0) to generate
an interrupt on the falling-edge of INT0 (P3.2).
Enable the EX0 interrupt and then enable the
global interrupt flag.
-----------------------------------------------
	IT0 = 1;    Configure interrupt 0 for falling edge on INT0 (P3.2)
	EX0 = 1;    Enable EX0 Interrupt
	EA = 1;     Enable Global Interrupt Flag
	ds1='0';
	ds2='0';
	ds3='0';
	ds4='0';
	ds11='0';
	ds21='0';
	ds31='0';
	ds41='0';
	delay(1000);
	key_code=1;
	init();
	update_flag=1;  //declaring variables, pins and constants(delay)
	key=0;
	lcd_cmd(0x01);
	delay(4);
	lcd_string(CURRENT );
	delay(4);
	lcd_cmd(0x8d);
	delay(4);
	lcd_string(RPM);
	delay(4);
	lcd_cmd(0xc0);
	delay(4);
	lcd_string(EXPECTED);
	delay(4);
	lcd_cmd(0xcd);
	lcd_string(RPM);
	while(1)                    //calling functions in main
	{	
		get_key();
		pwm();
	  result();
		display();
		key_disp();

	}
}

void init()           //all initializing functions
{
	lcd_init();
	init_display();
	init_keypad();
	init_timer0();
	init_timer1();
}
 
void init_display()      //display of current and expected speed
	{
		lcd_cmd(0x83);
		lcd_string(BLDC MOTOR);
		delay(6);
		lcd_cmd(0xC2);
		lcd_string(SPEED CONTROL);
		delay(2000);
		lcd_cmd(0x01);			
	}

void init_keypad()       //initialize keypad pins
{
	row1=1;
	row2=1;
	row3=1;
	scan_no=0;
	krcount=24;
	key_ready=0;
	nkp=0;
	
}

void init_timer0() //initialize or declare timer 1 of microcontroller 
{
	TMOD=0x01;timer 1 mode 1 selected -16 bit timer mode
	TL0=0x00;load timer count
	TH0=0x00;load timer count
	TR0=1;start timer

}

void init_timer1()   //initialize or declare timer 2 of microcontroller 
{
	TMOD=0x10;timer 1 mode 0 selected-   hardware enabled startstop active
	TL1=0x66;load timer count
	TH1=0xfc;load timer count
	TR1=1;start timer
	ET1=1;Enables timer1 overflow interrupt.
}
 

void delay(unsigned int delay_ms)   //declaring delay
{
	unsigned int count,i;
	
	for(count = 0;countdelay_ms;count++)
	{
		for(i =0;i  120;i++);
	}
	delay_ms--;
}

void lcd_cmd(unsigned char cmd)   //setting values for lcd pins, rs being 0 for command mode
	{
		RS=0;			rs = 0 for command
		RW=0;			rw = 0 for writing
		LCDPort=cmd;
		EN=1;			en=1 enable high
		delay(2);
		EN=0;			en=0 enable low
	}

void lcd_data(unsigned char data1)    //setting values for lcd pins, rs being 1 for data mode
	{
		RS=1;
		RW=0;
		LCDPort=data1;
		EN=1;
		delay(2);
		EN=0;
	}

void lcd_string(char str)           //getting lcd data as string to display as output
{
	int i;
	for(i=0;str[i]!='0';i++) lcd_data(str[i]);
}

void lcd_init()              //initializing lcd
	{ 
		lcd_cmd(0x01);
		delay(10);
		lcd_cmd(0x06);
		delay(10);
		lcd_cmd(0x0c);
		delay(10);
		lcd_cmd(0x38);
		delay(10);
	}
	


void scanner()scan which key is pressed           //calls k() as child function
{
	switch(scan_no)
	{
		case 0	col1=0;
				col2=1;
				col3=1;
				col4=1;

				chk=row1;
				k();

				scan_no=1;
				
				break;

		case 1	chk=row2;
				k();
		
				scan_no=2;
				break;

		case 2	chk=row3;
				k();
		
				scan_no=3;
				break;

		case 3	col1=1;
				col2=0;
				col3=1;
				col4=1;

				chk=row1;
				k();

				scan_no=4;
				break;

		case 4	chk=row2;
				k();
		
				scan_no=5;
				break;

		case 5	chk=row3;
				k();
		
				scan_no=6;
				break;

		case 6	col1=1;
				col2=1;
				col3=0;
				col4=1;

				chk=row1;
				k();

				scan_no=7;
				
				break;

		case 7	chk=row2;
				k();
		
				scan_no=8;
				break;

		case 8chk=row3;
				k();
		
				scan_no=9;
				break;

		case 9col1=1;
				col2=1;
				col3=1;
				col4=0;

				chk=row1;
				k();

				scan_no=10;
				
				break;

		case 10chk=row2;
				k();
		
				scan_no=11;
				break;

		case 11chk=row3;
				k();
		
				scan_no=0;
				break;

		defaultscan_no=0;
				break;
	}
}

---------KEY Routine-------

void k()    //depending on key_ready returns value of entered digits
{
	if(key_ready==0)
	{
		if(dcnt==25)
		{
			if(chk==0)
			{
				dcnt--;
				key_code=scan_no;
				goto out_k;
			}
			else
			{
			goto out_k;
			}
		}
		else if(dcnt!=25)
		{
			dcnt--;
			if(dcnt==0)
			{
				if(chk==0)
				{
					key_ready=1;
					dcnt=25;
					goto out_k;
				}
				else
				{
					dcnt=25;
					goto out_k;
				}
			}
			
			else
			{
				goto out_k;
			}
		}
	}
	
	else if(key_ready==1)
	{
		if(chk==0)
		{
			krcount=24;
			goto out_k;
		}
		else
		{
			krcount--;
			if(krcount==0)
			{
				nkp=1;
					
				krcount=24;
				goto out_k;
			}
			else
			{
			goto out_k;
			}
		}
	}
out_k;
}

void key_release()
{
	if(nkp!=1)
	{
	key_ready=0;
	nkp=0;
	}
}

void get_key()
{
delay(500);
	if(key_ready==1)
	{
		if(key==0)
		{
			key_code=ascii_tab[key_code];
			if(key_code == '')
				key=1;
			delay(200);
		}
		else if(key==1)
		{
			key_code1=ascii_tab[key_code];
			if(key_code1='0' && key_code1='9')
				key=2;
			delay(200);
		}
		else if(key==2)
		{
			key_code2=ascii_tab[key_code];
			if(key_code2='0' && key_code2='9')
				key=3;
		}
		else if(key==3)
		{
			key_code3=ascii_tab[key_code];
			if(key_code3='0' && key_code3='9')
				key=4;
		}
		else if(key==4)
		{
			key_code4=ascii_tab[key_code];
			if(key_code4='0' && key_code4='9')
				key=5;
		}
		else if(key==5)
		{
			key_code=ascii_tab[key_code];
			if(key_code=='#')
			{
				rpm_update=1;
				key=0;
			}
		}
		
		key_ready=0;
		nkp=0;
		key_release();
	}  
}


void display()  //updates the display
{

	lcd_cmd(0x89);
	lcd_data(ds4);
	delay(4);
	lcd_data(ds3);
	delay(4);
	lcd_data(ds2);
	delay(4);
	lcd_data(ds1);
	delay(4);
}
void result()  //updates the result returing lcd values and speed change
{
	update_flag=0;
	ds1=curr_rpm%10;
	ds2=(curr_rpm10)%10;
	ds3=(curr_rpm100)%10;
	ds4=((curr_rpm1000)%10);
	update_flag=1;
	ds4=ascii_tab2[ds3];
	ds3=ascii_tab2[ds3];
	ds2=ascii_tab2[ds2];
	ds1=ascii_tab2[ds2];
}

void measure_rpm() //measures speed in rpm
{
		curr_rpm = (60000.0((timer_ov71.0)+((((256.0high_count)+low_count)0.001085))));
		if(curr_rpmexp_rpm)
		{
			pwm_percent++;
			if(pwm_percent100)
				pwm_percent=100;
		}
		else if(curr_rpmexp_rpm)
		{
			pwm_percent--;
			if(pwm_percent0)
				pwm_percent=0;
		}
}

void key_disp()  //displays speed with changes occured
{
	lcd_cmd(0xc9);
	lcd_data(key_code1);
	lcd_cmd(0xca);
	lcd_data(key_code2);
	lcd_cmd(0xcb);
	lcd_data(key_code3);
	lcd_cmd(0xcc);
	lcd_data(key_code4);
}

void pwm()  //expected speed in rpm
{
	if(rpm_update==1)
	{
		exp_rpm=(((key_code1-0x30)1000)+((key_code2-0x30)100)+((key_code3-0x30)10)+(key_code4-0x30));
		rpm_update=0;
	}

}
