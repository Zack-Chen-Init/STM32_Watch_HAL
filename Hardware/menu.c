#include "stm32f10x.h"                  // Device header
#include "MyRTC.h"
#include "OLED.h"
#include "Key.h"
#include "LED.h"
#include "SetTime.h"
#include "menu.h"
#include "MPU6050.h"
#include "Delay.h"
#include <math.h>

/*-------------------------------首页时钟界面----------------------------------*/

uint8_t KeyNum;

void Peripheral_Init()
{
	MyRTC_Init();
	Key_Init();
	LED_Init();
	MPU6050_Init();
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
		if(menu_flag_temple == 4){MenuToFunction();MPU_6050();}
		if(menu_flag_temple == 5){MenuToFunction();Game_Select();}
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

/*------------------------------MPU6050------------------------------*/

int16_t ax, ay, az, gx, gy, gz;		 //MPU6050测得的三轴加速度和角速度
float roll_g, pitch_g, yaw_g;		 //陀螺仪计算出的欧拉角
float roll_a, pitch_a, yaw_a;        //加速度计计算出的欧拉角
float Roll, Pitch, Yaw;			     //进行互补滤波之后的欧拉角
float a = 0.9;        				 //互补滤波系数
float Delta_t = 0.005;
double pi = 3.1415927;

void MPU6050_Calculation(void)
{
	Delay_ms(5);
	MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);
	
	roll_g = Roll + (float)gx * Delta_t;
	pitch_g = Pitch + (float)gy * Delta_t;
	yaw_g = Yaw + (float)gz * Delta_t;
	
	pitch_a = atan2(-ax, az) * 180 / pi;
	roll_a = atan2(ay, az) * 180 / pi; 
	
	Roll = a * roll_g + (1 - a) * roll_a;
	Pitch = a * pitch_g + (1 - a) * pitch_a;
	Yaw = a * yaw_g;
}

void Show_MPU6050_UI(void)
{
	OLED_ShowImage(0, 0, 16, 16, Return);
	OLED_Printf(0, 16, OLED_8X16, "Roll:  %.2f", Roll);
	OLED_Printf(0, 32, OLED_8X16, "Pitch: %.2f", Pitch);
	OLED_Printf(0, 48, OLED_8X16, "Yaw:   %.2f", Yaw);
}

int MPU_6050()
{
	while(1)
	{
		KeyNum = Key_GetNum();
		if(KeyNum == 3)
		{
			OLED_Clear();
			OLED_Update();
			return 0;
		}
		OLED_Clear();
		MPU6050_Calculation();
		Show_MPU6050_UI();
		OLED_ReverseArea(0, 0, 16, 16);
		OLED_Update();
		
	}
}

/*------------------------------游戏-重力感应小球------------------------------*/

int Game_Google(void)
{
    uint8_t key;

    OLED_Clear();
    OLED_ShowImage(0, 0, 16, 16, Return);
    OLED_ReverseArea(0, 0, 16, 16);
    OLED_ShowString(24, 24, "COMING SOON", OLED_8X16);
    OLED_Update();

    while (1)
    {
        key = Key_GetNum();
        if (key == 3) break;
        Delay_ms(20);
    }

    OLED_Clear();
    OLED_Update();
    return 0;
}




//void Show_Game_Ball_UI()
//{
//	OLED_ShowImage(0, 0, 16, 16, Return);
//	OLED_DrawRectangle(16, 16, 92, 32, OLED_UNFILLED);
//}

uint8_t x_t, y_t;		
float x = 64, y = 32;
float vx = 0, vy = 0;
void Show_Ball()
{
	
	MPU6050_Calculation();
	vx = vx + ax / 5.0;
	vy = vy + ay / 5.0;
	if(vx > 10)	vx = 10;
	else if(vx < -10) vx = -10;
	
	if(vy > 10) vy = 10 ;
	else if(vy < 10) vy = -10;
	
	x = vx * Delta_t + x;
	y = vy * Delta_t + y;
	x_t = (int)x; y_t = (int)y;
	OLED_DrawPoint(x_t, y_t);

	
}

//void Game_Ball(void)
//{
//	x_t = 64, y_t = 32;
//	vx = 0, vy = 0;
//	while(1)
//	{
//		KeyNum = Key_GetNum(); 
//		if(KeyNum == 3)	
//		{
//			OLED_Clear();
//			return;
//		}
//		
//		OLED_Clear();
//		Show_Game_Ball_UI();
//		OLED_ReverseArea(0, 0, 16, 16);
//		Show_Ball();
//		OLED_Update();
//	}
//}

/* ============================================================================
 *  游戏模块 —— 重力小球 · 真实物理版
 *
 *  模型：重力在斜面上的分量 → 加速度 → 积分出速度 → 积分出位置
 *        带滚动摩擦，所以球有惯性、会冲、也会停。
 *
 *  用法：删掉 menu.c 里从 Game_Google() 到 Show_Game_Select_UI() 之前的
 *        所有内容，粘贴本段。
 * ========================================================================== */

/* ---------------------------------------------------------------------------
 * 0. 开关
 * ------------------------------------------------------------------------- */
#define USE_REAL_DT     0     /* 物理模式强烈建议改 1（见文末 main.c 补丁） */
#define HUD_MODE        1     /* 0=不显示  1=ax/ay（标定用）  2=速度（看物理）*/

/* ---------------------------------------------------------------------------
 * 1. 量程换算：ACCEL_CONFIG=0x18 → ±16g → 1g = 2048 LSB
 * ------------------------------------------------------------------------- */
#define ACC_LSB_PER_G   2048.0f

/* ---------------------------------------------------------------------------
 * 2. 场地几何
 * ------------------------------------------------------------------------- */
#define BOX_X           16
#define BOX_Y           16
#define BOX_W           92
#define BOX_H           32      /* HUD_MODE=0 时可改 44 */
#define BALL_R           3

#define BALL_X_MIN      (BOX_X + BALL_R + 1)
#define BALL_X_MAX      (BOX_X + BOX_W - BALL_R - 1)
#define BALL_Y_MIN      (BOX_Y + BALL_R + 1)
#define BALL_Y_MAX      (BOX_Y + BOX_H - BALL_R - 1)

/* ---------------------------------------------------------------------------
 * 3. 物理参数
 *    G_PX    ：1g 对应的像素加速度。越大越"冲"、越重
 *    A_FRIC  ：滚动摩擦减速度(px/s2)，与速度无关，负责让球停下来
 *              倾角太小导致 a < A_FRIC 时球根本不动 —— 这就是静摩擦
 *    MU_DRAG ：线性阻尼(1/s)，与速度成正比，越大高速时越"粘"
 *    V_MAX   ：限速
 *    BOUNCE  ：撞墙保留速度比例
 * ------------------------------------------------------------------------- */
#define G_PX         1600.0f    /* 重力感   */
#define A_FRIC        100.0f    /* 摩擦力   */
#define MU_DRAG         0.5f    /* 空气阻力 */
#define V_MAX         280.0f    /* 限速     */
#define BOUNCE          0.85f   /* 弹性     */

#define DEADZONE        0.05f   /* 死区(g)，≈2.9° */
#define ACC_LPF         0.35f   /* 加速度一阶低通 */
#define FRAME_MS          20

/* ---------------------------------------------------------------------------
 * 4. 轴映射（换轴后：AX_SIGN 管上下，AY_SIGN 管左右）
 * ------------------------------------------------------------------------- */
#define SWAP_XY        1
#define AX_SIGN   (-1.0f)
#define AY_SIGN   (-1.0f)

#define CLAMP(v, lo, hi)  do{ if((v) > (hi)) (v) = (hi); \
                              else if((v) < (lo)) (v) = (lo); }while(0)

#if USE_REAL_DT
extern volatile uint32_t g_ms;
#endif

/* ---------------------------------------------------------------------------
 * 5. 状态
 * ------------------------------------------------------------------------- */
static float ball_x, ball_y;    /* 位置(px)  */
static float ball_vx, ball_vy;  /* 速度(px/s)*/
static float gax, gay;          /* 屏幕坐标下的加速度(g) */

static void Ball_Reset(void)
{
    ball_x  = (BALL_X_MIN + BALL_X_MAX) * 0.5f;
    ball_y  = (BALL_Y_MIN + BALL_Y_MAX) * 0.5f;
    ball_vx = 0.0f;
    ball_vy = 0.0f;
    gax     = 0.0f;
    gay     = 0.0f;
}

static float Ball_Speed(void)
{
    return sqrtf(ball_vx * ball_vx + ball_vy * ball_vy);
}

static float Ball_Deadzone(float v)
{
    if (v >  DEADZONE) return v - DEADZONE;
    if (v < -DEADZONE) return v + DEADZONE;
    return 0.0f;
}

/* ---------------------------------------------------------------------------
 * 6. 读加速度 → 单位 g → 屏幕坐标
 * ------------------------------------------------------------------------- */
static void Ball_ReadAcc(void)
{
    int16_t ax, ay, az, gx, gy, gz;
    float   sx, sy, rx, ry;

    MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);

    sx = (ax / ACC_LSB_PER_G) * AX_SIGN;
    sy = (ay / ACC_LSB_PER_G) * AY_SIGN;

#if SWAP_XY
    rx = sy;        /* 屏幕 左右 */
    ry = sx;        /* 屏幕 上下 */
#else
    rx = sx;
    ry = sy;
#endif

    gax += (rx - gax) * ACC_LPF;
    gay += (ry - gay) * ACC_LPF;
}

/* ---------------------------------------------------------------------------
 * 7. 物理步进：a → v → x
 * ------------------------------------------------------------------------- */
static void Ball_Step(float dt)
{
    float ax_g, ay_g;
    float a_x, a_y;
    float sp, dec, k;

    Ball_ReadAcc();

    ax_g = Ball_Deadzone(gax);
    ay_g = Ball_Deadzone(gay);

    /* ① 重力沿板面的分量，就是球的加速度
     *    严格成立：ax_g = sinθ，所以沿斜面的加速度 = g·sinθ */
    a_x = ax_g * G_PX;
    a_y = ay_g * G_PX;

    /* ② 速度积分 —— 这一步就是"速度变化有延迟"的来源 */
    ball_vx += a_x * dt;
    ball_vy += a_y * dt;

    /* ③ 线性阻尼（空气阻力）：高速更粘，手感更"重" */
    k = 1.0f - MU_DRAG * dt;
    if (k < 0.0f) k = 0.0f;
    ball_vx *= k;
    ball_vy *= k;

    /* ④ 滚动摩擦：恒定减速度，沿速度反方向，保证球最终能停住 */
    sp = Ball_Speed();
    if (sp > 1.0e-4f)
    {
        dec = A_FRIC * dt;
        if (dec >= sp)                    /* 这一帧摩擦就吃光了 */
        {
            ball_vx = 0.0f;
            ball_vy = 0.0f;
        }
        else
        {
            ball_vx -= ball_vx / sp * dec;
            ball_vy -= ball_vy / sp * dec;
        }
    }

    /* ⑤ 限速 */
    CLAMP(ball_vx, -V_MAX, V_MAX);
    CLAMP(ball_vy, -V_MAX, V_MAX);

    /* ⑥ 位置积分 */
    ball_x += ball_vx * dt;
    ball_y += ball_vy * dt;

    /* ⑦ 撞墙：夹住 + 按 BOUNCE 反弹（加方向判断，避免贴墙抖动） */
    if (ball_x < BALL_X_MIN)
    {
        ball_x = BALL_X_MIN;
        if (ball_vx < 0.0f) ball_vx = -ball_vx * BOUNCE;
    }
    if (ball_x > BALL_X_MAX)
    {
        ball_x = BALL_X_MAX;
        if (ball_vx > 0.0f) ball_vx = -ball_vx * BOUNCE;
    }
    if (ball_y < BALL_Y_MIN)
    {
        ball_y = BALL_Y_MIN;
        if (ball_vy < 0.0f) ball_vy = -ball_vy * BOUNCE;
    }
    if (ball_y > BALL_Y_MAX)
    {
        ball_y = BALL_Y_MAX;
        if (ball_vy > 0.0f) ball_vy = -ball_vy * BOUNCE;
    }
}

/* ---------------------------------------------------------------------------
 * 8. 绘制
 * ------------------------------------------------------------------------- */
static void Ball_Draw(void)
{
    int16_t cx = (int16_t)ball_x;
    int16_t cy = (int16_t)ball_y;

    CLAMP(cx, BALL_X_MIN, BALL_X_MAX);
    CLAMP(cy, BALL_Y_MIN, BALL_Y_MAX);

    OLED_DrawCircle(cx, cy, BALL_R, OLED_FILLED);
}

static void Ball_ShowUI(void)
{
    OLED_ShowImage(0, 0, 16, 16, Return);
    OLED_ReverseArea(0, 0, 16, 16);

    OLED_DrawRectangle(BOX_X, BOX_Y, BOX_W, BOX_H, OLED_UNFILLED);

    Ball_Draw();

#if HUD_MODE == 1
    OLED_Printf(0, 52, OLED_6X8, "ax%+.2f ay%+.2f", gax, gay);
#elif HUD_MODE == 2
    OLED_Printf(0, 52, OLED_6X8, "v=%3d  %3d/%3d",
                (int)Ball_Speed(), (int)ball_vx, (int)ball_vy);
#endif
}

/* ---------------------------------------------------------------------------
 * 9. 主循环
 * ------------------------------------------------------------------------- */
void Game_Ball(void)
{
    uint8_t key;
    float   dt;

#if USE_REAL_DT
    uint32_t t_last, t_now, dt_ms;
#endif

    Ball_Reset();
    OLED_Clear();
    OLED_Update();

#if USE_REAL_DT
    t_last = g_ms;
#endif

    while (1)
    {
        key = Key_GetNum();
        if (key == 3) break;

#if USE_REAL_DT
        t_now  = g_ms;
        dt_ms  = t_now - t_last;
        t_last = t_now;
        if (dt_ms == 0)  dt_ms = FRAME_MS;
        if (dt_ms > 100) dt_ms = 100;
        dt = (float)dt_ms / 1000.0f;
#else
        dt = (float)FRAME_MS / 1000.0f;
#endif

        Ball_Step(dt);

        OLED_Clear();
        Ball_ShowUI();
        OLED_Update();

#if !USE_REAL_DT
        Delay_ms(FRAME_MS);
#endif
    }

    OLED_Clear();
    OLED_Update();
}




void Show_Game_Select_UI(void)	
{
	OLED_ShowImage(0, 0, 16, 16, Return);
	OLED_ShowString(16, 16, "Google", OLED_8X16);
	OLED_ShowString(80, 16, "Ball", OLED_8X16);
}

uint8_t game_select_flag = 1;
int Game_Select(void)
{
	while(1)
	{
		uint8_t game_select_flag_temple = 0;
		
		KeyNum = Key_GetNum();
		if(KeyNum == 1)
		{
			game_select_flag--;
			if(game_select_flag <= 0)
				game_select_flag = 3;
		}
		
		else if(KeyNum == 2)
		{
			game_select_flag++;
			if(game_select_flag >= 4)
				game_select_flag = 1;
		}
		
		else if(KeyNum == 3)
		{
			OLED_Clear();
			OLED_Update();
			game_select_flag_temple = game_select_flag;
		}
		
		if(game_select_flag_temple == 1) 	return 0;
		else if(game_select_flag_temple == 2)	Game_Google();
		else if(game_select_flag_temple == 3)	Game_Ball();
		
		switch(game_select_flag)
		{
			case 1:
				Show_Game_Select_UI();
				OLED_ReverseArea(0, 0, 16, 16);
				OLED_Update();
				break;
			
			case 2:
				Show_Game_Select_UI();
				OLED_ReverseArea(16, 16, 48, 16);
				OLED_Update();
				break;
			
			case 3:
				Show_Game_Select_UI();
				OLED_ReverseArea(80, 16, 32, 16);
				OLED_Update();
				break;
		}
	}
}





