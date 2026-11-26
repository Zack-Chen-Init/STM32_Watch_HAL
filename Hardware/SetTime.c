#include "stm32f10x.h"                  // Device header
#include "Key.h"
#include "menu.h"
#include "OLED.h"
#include "MyRTC.h"


//注意此处的 return 0 全部相当于函数结束，由于没有定义 return 0 之后的行为，所以相当于直接结束

void Show_SetTime_FirstUI(void)
{
	OLED_ShowImage(0, 0, 16, 16, Return);
	OLED_Printf(0, 16, OLED_8X16, "年:%4d", MyRTC_Time[0]);
	OLED_Printf(0, 32, OLED_8X16, "月:%2d", MyRTC_Time[1]);
	OLED_Printf(0, 48, OLED_8X16, "日:%2d", MyRTC_Time[2]);
}

void Show_SetTime_SecondUI(void)
{
	OLED_Printf(0, 0, OLED_8X16, "时:%2d", MyRTC_Time[3]);
	OLED_Printf(0, 16, OLED_8X16, "分:%2d", MyRTC_Time[4]);
	OLED_Printf(0, 32, OLED_8X16, "秒:%2d", MyRTC_Time[5]);
}



void Change_MyRTC_Time(uint8_t i, uint8_t flag)
{
	if(flag == 1) MyRTC_Time[i]++;
	else MyRTC_Time[i]--;
	MyRTC_SetTime();
}

extern uint8_t KeyNum;

int SetYear(void)			
{	while(1)	
	{
		KeyNum = Key_GetNum();
		if(KeyNum == 1)
		{
			Change_MyRTC_Time(0, 1);
		}
		if(KeyNum == 2)
		{
			Change_MyRTC_Time(0, 0);
		}
		if(KeyNum == 3)
		{
			return 0;
		}
		
		Show_SetTime_FirstUI();
		OLED_ReverseArea(24, 16, 32, 16);
		OLED_Update();
		
	}
}

int SetMonth(void)
{
	while(1)	
	{
		KeyNum = Key_GetNum();
		if(KeyNum == 1)
		{
			Change_MyRTC_Time(1, 1);
			if(MyRTC_Time[1] >= 13) MyRTC_Time[1] = 1;
		}
		if(KeyNum == 2)
		{
			Change_MyRTC_Time(1, 0);
			if(MyRTC_Time[1] <= 0) MyRTC_Time[1] = 12;
		}
		if(KeyNum == 3)
		{
			return 0;
		}
		
		Show_SetTime_FirstUI();
		OLED_ReverseArea(24, 32, 16, 16);
		OLED_Update();
	}
}

int SetDay(void)
{
	while(1)	
	{
		KeyNum = Key_GetNum();
		if(KeyNum == 1)
		{
			Change_MyRTC_Time(2, 1);
			if(MyRTC_Time[2] >= 32) MyRTC_Time[2] = 1;
		}
		if(KeyNum == 2)
		{
			Change_MyRTC_Time(2, 0);
			if(MyRTC_Time[2] <= 0) MyRTC_Time[2] = 31;
		}
		if(KeyNum == 3)
		{
			return 0;
		}
		Show_SetTime_FirstUI();
		OLED_ReverseArea(24, 48, 16, 16);
		OLED_Update();
	}
	
	
}

int SetHour(void)
{
	while(1)	
	{
		KeyNum = Key_GetNum();
		if(KeyNum == 1)
		{
			Change_MyRTC_Time(3, 1);
			if(MyRTC_Time[3] >= 24) MyRTC_Time[3] = 0;
		}
		if(KeyNum == 2)
		{
			Change_MyRTC_Time(3, 0);
			if(MyRTC_Time[3] < 0) MyRTC_Time[3] = 23;
		}
		if(KeyNum == 3)
		{
			return 0;
		}
		
		Show_SetTime_SecondUI();
		OLED_ReverseArea(24, 0, 16, 16);
		OLED_Update();
	}
}

int SetMinute(void)
{
	while(1)	
	{
		KeyNum = Key_GetNum();
		if(KeyNum == 1)
		{
			Change_MyRTC_Time(4, 1);
			if(MyRTC_Time[4] >= 61) MyRTC_Time[4] = 0;
		}
		if(KeyNum == 2)
		{
			Change_MyRTC_Time(4, 0);
			if(MyRTC_Time[4] < 0) MyRTC_Time[4] = 59;
		}
		if(KeyNum == 3)
		{
			return 0;
		}
		
		Show_SetTime_SecondUI();
		OLED_ReverseArea(24, 16, 16, 16);
		OLED_Update();
	}
}

int SetSecond(void)
{
	while(1)	
	{
		KeyNum = Key_GetNum();
		if(KeyNum == 1)
		{
			Change_MyRTC_Time(5, 1);
			if(MyRTC_Time[5] >= 61) MyRTC_Time[5] = 0;
		}
		if(KeyNum == 2)
		{
			Change_MyRTC_Time(5, 0);
			if(MyRTC_Time[5] < 0) MyRTC_Time[5] = 59;
		}
		if(KeyNum == 3)
		{
			return 0;
		}
		
		Show_SetTime_SecondUI();
		OLED_ReverseArea(24, 32, 16, 16);
		OLED_Update();
	}
}



extern uint8_t KeyNum;
int set_time_flag = 1;
int SetTime(void)
{
	while(1)
	{
		uint8_t set_time_flag_temple = 0;
		KeyNum = Key_GetNum();
		
		if(KeyNum == 1)				//上一项
		{
			set_time_flag--;
			if(set_time_flag <=0) set_time_flag = 7;
		}
		else if(KeyNum == 2)		//下一项
		{
			set_time_flag++;
			if(set_time_flag >= 8)	set_time_flag = 1;
			
		}
		else if(KeyNum == 3)		//确认
		{
			OLED_Clear();
			OLED_Update();
			set_time_flag_temple = set_time_flag;
		}
		
		if(set_time_flag_temple == 1)   {return 0;}
		if(set_time_flag_temple == 2)	{SetYear();}   	//年
		if(set_time_flag_temple == 3)	{SetMonth();}  	//月
		if(set_time_flag_temple == 4)	{SetDay();}		//日
		if(set_time_flag_temple == 5)	{SetHour();}		//时
		if(set_time_flag_temple == 6)	{SetMinute();}		//分
		if(set_time_flag_temple == 7)	{SetSecond();}		//秒
		
		
		switch(set_time_flag)    //反向加深背景色 即 年月日 前面的选中框UI
		{
			case 1:
				OLED_Clear();
				Show_SetTime_FirstUI();
				OLED_ReverseArea(0, 0, 16, 16);
				OLED_Update();
				break;
			
			case 2:
				OLED_Clear();
				Show_SetTime_FirstUI();
				OLED_ReverseArea(0, 16, 16, 16);
				OLED_Update();
				break;
			
			case 3:
				OLED_Clear();
				Show_SetTime_FirstUI();
				OLED_ReverseArea(0, 32, 16, 16);
				OLED_Update();
				break;
			
			case 4:
				OLED_Clear();
				Show_SetTime_FirstUI();
				OLED_ReverseArea(0, 48, 16, 16);
				OLED_Update();
				break;
			
			case 5:
				OLED_Clear();
				Show_SetTime_SecondUI();
				OLED_ReverseArea(0, 0, 16, 16);
				OLED_Update();
				break;
			
			case 6:
				OLED_Clear();
				Show_SetTime_SecondUI();
				OLED_ReverseArea(0, 16, 16, 16);
				OLED_Update();
				break;
			
			case 7:
				OLED_Clear();
				Show_SetTime_SecondUI();
				OLED_ReverseArea(0, 32, 16, 16);
				OLED_Update();
				break;
		}
	}
}