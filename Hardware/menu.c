#include "stm32f10x.h"                  // Device header
#include "MyRTC.h"
#include "OLED.h"
#include "Key.h"
#include "LED.h"
#include "SetTime.h"
#include "menu.h"

/*-------------------------------首页时钟界面----------------------------------*/

uint8_t KeyNum;

void Peripheral_Init()
{
	MyRTC_Init();
	Key_Init();
	LED_Init();
}

void Show_Clock_UI()
{
	MyRTC_ReadTime();
	OLED_Printf(0, 0, OLED_6X8, "%d-%d-%d", MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
	OLED_Printf(16, 16, OLED_12X24, "%02d:-%02d-%02d", MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
	OLED_ShowString(0, 48, "菜单", OLED_8X16);
	OLED_ShowString(96, 48, "设置", OLED_8X16);
}


int clkflag = 1;

int First_Page_Clock()          		//选择界面代码模版可以恒聚实际需求进行修改
{
	while(1)
	{
		KeyNum = Key_GetNum();
		
		if(KeyNum == 1)				//上一项
		{
			clkflag--;
			if(clkflag <=0) clkflag = 2;
		}
		else if(KeyNum == 2)		//下一项
		{
			clkflag++;
			if(clkflag > 2)	clkflag = 1;
			
		}
		else if(KeyNum == 3)		//确认
		{
			OLED_Clear();
			OLED_Update();
			return clkflag;
		}
		
		switch(clkflag)    //反向加深背景色
		{
			case 1:
				Show_Clock_UI();
				OLED_ReverseArea(0, 48, 32, 16);
				OLED_Update();
				break;
			case 2:
				Show_Clock_UI();
				OLED_ReverseArea(96, 48, 32, 16);
				OLED_Update();
				break;
		}
	}
}


/*-------------------------------设置界面---------------------------------*/

void Show_Setting_Page_UI(void)
{
	OLED_ShowImage(0, 0, 16, 16, Return);
	OLED_ShowString(0, 16, "日期时间设置", OLED_8X16);
}

int setflag = 1;
int SettingPage(void)
{
	while(1)
	{
		uint8_t setflag_temple = 0;
		
		KeyNum = Key_GetNum();
		
		if(KeyNum == 1)				//上一项
		{
			setflag--;
			if(setflag <=0) setflag = 2;
		}
		else if(KeyNum == 2)		//下一项
		{
			setflag++;
			if(setflag > 2)	setflag = 1;
			
		}
		else if(KeyNum == 3)		//确认
		{
			OLED_Clear();
			OLED_Update();
			setflag_temple = setflag;
		}
		
		if(setflag_temple == 1){return 0;}
		else if(setflag_temple == 2){SetTime();}
			
		switch(setflag)    //反向加深背景色
		{
			case 1:
				Show_Setting_Page_UI();
				OLED_ReverseArea(0, 0, 16, 16);
				OLED_Update();
				break;
			case 2:
				Show_Setting_Page_UI();
				OLED_ReverseArea(0, 16, 96, 16);
				OLED_Update();
				break;
		}
	}
	
}

/*-----------------滑动切换动画界面-----------------*/

uint8_t pre_selection;
uint8_t target_selection;
uint8_t x_pre = 48;  //此处赋值为 48 = 128 / 2 - 32 / 16；其中 128 是屏幕的宽度，32是图标的宽度，两个做减法就可以得到图标x坐标的起始位置
uint8_t Speed = 4;
uint8_t move_flag;

void Menu_Animation(void)
{
	OLED_Clear();
	OLED_ShowImage(42, 10, 44, 44, Frame);
	
	if(pre_selection < target_selection)
	{
		x_pre -= Speed;
		if(x_pre == 0)
		{
			pre_selection++;
			move_flag = 0;
			x_pre = 48;
			
		}
	}
	
	if(pre_selection > target_selection)
	{
		x_pre += Speed;
		if(x_pre == 96)
		{
			pre_selection--;
			move_flag = 0;
			x_pre = 48;
			
		}
	}
	
	
	//Q:为什么此处前面有if判断后面没有
	//A:防止数组越界，这里的 pre_selecction - 1，和 pre_selecction - 2 有越界风险，所以此处加上一个if判断
	//数组Menu_Graph定义的时候，作者特地定义了一个空白元素，所以下面只有 pre_selection + 2
	
	//注意，我认为此处最后一个Menu_Graph[pre_selection + 2] 是存在风险的，按照上面的解释，那么此处会有越界风险，但是由于在屏幕上面不会显示，所以看起来没有问题
	if(pre_selection >= 1)
		OLED_ShowImage(x_pre - 48, 16, 32, 32, Menu_Graph[pre_selection - 1]);
	
	if(pre_selection >= 2)
		OLED_ShowImage(x_pre - 96, 16, 32, 32, Menu_Graph[pre_selection - 2]);
	
	
	OLED_ShowImage(x_pre, 16, 32, 32, Menu_Graph[pre_selection]);
	OLED_ShowImage(x_pre + 48, 16, 32, 32, Menu_Graph[pre_selection + 1]);
	OLED_ShowImage(x_pre + 96, 16, 32, 32, Menu_Graph[pre_selection + 2]);
	
	OLED_Update();
}

void Set_Selection(uint8_t move_flag, uint8_t Pre_Selection, uint8_t Target_Selection)
{
	if(move_flag == 1)
	{
		pre_selection = Pre_Selection;
		target_selection = Target_Selection;
	}
	Menu_Animation();
}

void MenuToFunction(void)
{
	for(uint8_t i = 1; i <= 6; i++)
	{
		OLED_Clear();
		if(pre_selection >= 1)
			OLED_ShowImage(x_pre - 48, 16 + 8 * i, 32, 32, Menu_Graph[pre_selection - 1]);	
		
		OLED_ShowImage(x_pre, 16 + 8 * i, 32, 32, Menu_Graph[pre_selection]);
		OLED_ShowImage(x_pre + 48, 16 + 8 * i, 32, 32, Menu_Graph[pre_selection + 1]);
		
		OLED_Update();
	}
}

uint8_t menu_flag = 1;

int Menu(void)
{
	move_flag = 1;
	uint8_t Direct_Flag = 2; //控制移动方向变量 1是向前移动， 2是向后移动
	while(1)
	{
		uint8_t menu_flag_temple = 0;
		
		KeyNum = Key_GetNum();
		
		if(KeyNum == 1)				//上一项
		{
			Direct_Flag = 1;
			move_flag = 1;
			menu_flag--;
			if(menu_flag <=0) menu_flag = 7;
		}
		else if(KeyNum == 2)		//下一项
		{
			Direct_Flag = 2;
			move_flag = 1;
			menu_flag++;
			if(menu_flag >= 8)	menu_flag = 1;
			
		}
		else if(KeyNum == 3)		//确认
		{
			OLED_Clear();
			OLED_Update();
			menu_flag_temple = menu_flag;
		}
		
		if(menu_flag_temple == 1){return 0;}
		if(menu_flag_temple == 2){MenuToFunction();StopWatch();}
		if(menu_flag_temple == 3){MenuToFunction();LED();}
		if(menu_flag_temple == 4){}
		if(menu_flag_temple == 5){}
		if(menu_flag_temple == 6){}
		if(menu_flag_temple == 7){}
			
		
		//此处做一个简单的说明 menu_flag 的取值范围是1-7，但是动画窗格范围是0-7
		//该部分仍然保留，用于理解代码	
			
			
		if(menu_flag == 1)
		{
			if(Direct_Flag == 1)  Set_Selection(move_flag, 1, 0);
			else if(Direct_Flag == 2) Set_Selection(move_flag, 0, 0);
		}
		else
		{
			if(Direct_Flag == 1)  Set_Selection(move_flag, menu_flag, menu_flag - 1);
			else if(Direct_Flag == 2) Set_Selection(move_flag, menu_flag - 2, menu_flag - 1);
		}
	
	}
}

/*----------------------秒表---------------------*/

uint8_t min, sec, msec; 

uint8_t start_timing_flag = 0;
void StopWatch_Tick(void)
{
	static uint16_t Count;
	Count++;
	if(Count >= 10)
	{
		Count = 0;
		if(start_timing_flag == 1)
		{
			msec++;
			if(msec >= 100)
			{
				msec = 0;
				sec++;
				if(sec >= 60)
				{
					sec = 0;
					min++;
					if(min >= 60)
						min = 0;
					
				}
			}
		}
	}
}

void Show_StopWatch_UI(void)
{
	OLED_ShowImage(0, 0, 16, 16, Return);
	OLED_Printf(32, 20, OLED_8X16, "%02d:%02d:%02d", min, sec, msec);
	OLED_ShowString(8, 44, "开始", OLED_8X16);
	OLED_ShowString(48, 44, "停止", OLED_8X16);
	OLED_ShowString(88, 44, "清除", OLED_8X16);
}

uint8_t stopwatch_flag = 1;
int StopWatch(void)
{
	while(1)
	{
		uint8_t stopwatch_flag_temple = 0;
		
		KeyNum = Key_GetNum();
		
		if(KeyNum == 1)				//上一项
		{
			stopwatch_flag--;
			if(stopwatch_flag <=0) stopwatch_flag = 4;
		}
		else if(KeyNum == 2)		//下一项
		{
			stopwatch_flag++;
			if(stopwatch_flag >= 5)	stopwatch_flag = 1;
			
		}
		else if(KeyNum == 3)		//确认
		{
			OLED_Clear();
			OLED_Update();
			stopwatch_flag_temple = stopwatch_flag;
		}
		
		if(stopwatch_flag_temple == 1){return 0;}
			
		switch(stopwatch_flag)    //反向加深背景色
		{
			case 1:									//选中退回键
				Show_StopWatch_UI();
				OLED_ReverseArea(0, 0, 16, 16);
				OLED_Update();
				break;
			
			case 2:									//选中开始键
				Show_StopWatch_UI();
				start_timing_flag = 1;
				OLED_ReverseArea(8, 44, 32, 16);
				OLED_Update();
				break;
			
			case 3:									//选中停止键
				Show_StopWatch_UI();
				start_timing_flag = 0;
				OLED_ReverseArea(48, 44, 32, 16);
				OLED_Update();
				break;
			
			case 4:									//选中清除键
				Show_StopWatch_UI();
				start_timing_flag = 0;								
				min = sec = msec = 0;
				OLED_ReverseArea(88, 44, 32, 16);
				OLED_Update();
				break;
		}
	}
	
}



/*------------------------------手电筒------------------------------*/

void Show_LED_UI()
{
	OLED_ShowImage(0, 0, 16, 16, Return);
	OLED_ShowString(0, 20, "OFF", OLED_12X24);
	OLED_ShowString(52, 20, "ON", OLED_12X24);
	OLED_ShowString(92, 20, "SOS", OLED_12X24);
}


uint8_t led_flag = 1;
int LED()
{
	while(1)
	{
		uint8_t led_flag_temple = 0;
		
		KeyNum = Key_GetNum();
		
		if(KeyNum == 1)				//上一项
		{
			led_flag--;
			if(led_flag <=0) led_flag = 4;
		}
		else if(KeyNum == 2)		//下一项
		{
			led_flag++;
			if(led_flag >= 5)	led_flag = 1;
			
		}
		else if(KeyNum == 3)		//确认
		{
			OLED_Clear();
			OLED_Update();
			led_flag_temple = led_flag;
		}
		
		if(led_flag_temple == 1){return 0;}
			
		switch(led_flag)    //反向加深背景色
		{
			case 1:									//选中退回键
				led_sos_flag = 0;
				Show_LED_UI();
				OLED_ReverseArea(0, 0, 16, 16);
				OLED_Update();
				break;
			
			case 2:									//选中OFF键
				led_sos_flag = 0;
				Show_LED_UI();
				LED_OFF();
				OLED_ReverseArea(0, 20, 36, 24);
				OLED_Update();
				break;
			
			case 3:									//选中ON键
				led_sos_flag = 0;
				Show_LED_UI();
				LED_ON();
				OLED_ReverseArea(52, 20, 24, 24);
				OLED_Update();
				break;
			
			case 4:									//选中SOS键
				led_sos_flag = 1;
				Show_LED_UI();
				LED_SOS();
				OLED_ReverseArea(92, 20, 36, 24);
				OLED_Update();
				break;
		}
	}
}