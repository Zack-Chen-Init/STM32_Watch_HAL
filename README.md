# STM32 智能手表 · 计时与照明版（1_5_Light）

## 注意，此处的核心驱动仍然是基于江协科技的STD库驱动， HAL库正在开发，在基本功能完善后会启动HAL库的转写工作

基于 **STM32F103C8T6 + 0.96 寸 I2C OLED** 的小手表 UI 框架：开机是时钟首页，可以进「菜单」看带滑动动画的图标列表，也可以进「设置」改日期时间。

这是手表系列教程的 **1_5** 阶段，在 1_3（滑动菜单）、1_4（秒表）的基础上，把菜单里的「手电筒」做成了可用的**照明页**：`OFF / ON / SOS` 三档，点灯由 **PB15** 驱动，`SOS` 档每 50 ms 翻转一次做快速闪烁提醒。菜单 7 项里已实现「退回 / 秒表 / 手电筒」，剩下 4 项（MPU6050、游戏、小电视、水平仪）仍是预留位 —— 图标已经画好，就等你填逻辑。

版本历程：

| 版本 | 加了什么 |
| --- | --- |
| 1_1_UI | 时钟首页、OLED 驱动、按键与基础框架 |
| 1_2_TimeSet | 日期时间设置页 |
| 1_3_Menu_Shift | 5 图标横向滑动菜单 + 动画（**本仓库的 Git 基线**） |
| 1_4_Timer | 秒表（`StopWatch`，复用 TIM2 的 1 ms 节拍） |
| 1_5_Light | 照明页 `OFF / ON / SOS`（**本版**，LED 单灯 PB15） |

---

## 1. 硬件与接线

| 器件 | 型号 / 说明 | 接线 |
| --- | --- | --- |
| 主控 | STM32F103C8T6 最小系统板（Blue Pill） | 72 MHz，64 KB Flash / 20 KB RAM |
| 屏幕 | 0.96" OLED，SSD1306，128×64，4 针 I2C | SCL → **PB8**，SDA → **PB9**（软件 I2C 模拟，从机地址 `0x78`） |
| 按键 1 | 轻触按键（上一项） | **PB1** → GND，内部上拉 |
| 按键 2 | 轻触按键（下一项） | **PA6** → GND，内部上拉 |
| 按键 3 | 轻触按键（确认 / 返回） | **PA4** → GND，内部上拉 |
| LED | 外接 LED + 限流电阻（220 Ω ~ 1 kΩ） | **PB15** → 电阻 → LED 阳极，阴极 → GND；**低电平点亮** |
| RTC 晶振 | 32.768 kHz（LSE） | 板载 32.768k 晶振；想掉电走时需接 VBAT 纽扣电池 |

> 按键按下为低电平，代码里用 `GPIO_Mode_IPU` 内部上拉，所以按键一端接引脚、一端接 GND 即可，不用外接电阻。
>
> 和 1_3 相比 **LED 从 PA1/PA2 改到了 PB15**（单灯）。照明的效果就是 PB15 一直输出低电平，LED 长亮。

---

## 2. 目录结构

```
1_5_Light/
├── Project.uvprojx      Keil MDK 工程文件（双击打开）
├── Project.uvoptx       工程选项（下载器、Flash 算法等）
├── keilkill.bat         一键删除编译中间文件的小脚本
├── User/
│   ├── main.c           入口：OLED 初始化 → 外设初始化 → 定时器 → 主循环
│   ├── stm32f10x_it.c   中断服务函数（空实现，TIM2 中断写在 main.c 里）
│   └── stm32f10x_conf.h 标准外设库的配置文件
├── Hardware/
│   ├── OLED.c/.h        0.96" OLED 驱动（软件 I2C + 显存 + 绘图/字库 API）
│   ├── OLED_Data.c/.h   字模与图标数据（ASCII 字库、汉字字库、48×48 边框、32×32 菜单图标）
│   ├── Key.c/.h         3 个按键：20 ms 消抖、松手触发
│   ├── LED.c/.h         单灯 PB15：LED_ON / LED_OFF / LED_Turn / LED_SOS
│   ├── menu.c/.h        首页时钟、设置页、滑动菜单、秒表页、照明页（核心文件）
│   └── SetTime.c/.h     日期时间设置：年/月/日/时/分/秒逐项加减
├── System/
│   ├── Delay.c/.h       SysTick 微秒/毫秒/秒延时
│   ├── MyRTC.c/.h       RTC 走时（LSE + 备份寄存器标记 + 东八区）
│   └── Timer.c/.h       TIM2 定时中断（1 ms），给按键/秒表/闪灯提供节拍
├── Library/             STM32F10x 标准外设库（StdPeriph Driver）
├── Start/               启动文件 + CMSIS 内核文件
└── RTE / DebugConfig    下载器调试配置（可选，删了 Keil 会自动重建）
```

---

## 3. 编译与下载

1. 安装 **Keil MDK 5**，并装好 `Keil.STM32F1xx_DFP` 器件包（Device Family Pack）。
2. 双击 `Project.uvprojx` 打开工程。
3. 编译：`F7`（或 Project → Build Target）。工程用的是标准外设库，宏 `USE_STDPERIPH_DRIVER` 已在工程里配置好。
4. 下载：`F8`，需要 ST-Link / DAP-Link / J-Link，在 `Options for Target → Debug` 里选对应的下载器，`Utilities → Settings` 里确认 Flash 算法是 `STM32F10x Med-density`。

> 注意：工程默认 **不生成 hex 文件**（`CreateHexFile = 0`）。如果要 hex/bin，在 `Options for Target → Output` 里勾选 `Create HEX File`，产物在 `Objects\` 目录下。

编译产物（`Objects\`、`Listings\`）和 Keil 的用户界面布局（`*.uvguix.*`）都已经写进 `.gitignore`，不会污染仓库。

---

## 4. 上手：按键怎么用

| 界面 | K1（PB1） | K2（PA6） | K3（PA4） |
| --- | --- | --- | --- |
| 时钟首页 | 光标移到「菜单」 | 光标移到「设置」 | 进入选中项 |
| 滑动菜单 | 上一个图标 | 下一个图标 | 进入选中项；停在「退回」上则回首页 |
| 设置页 | 光标上移 | 光标下移 | 进入选中项 |
| 设置时间 | 选中项 → 进二级页（逐级下沉） | 选中项 → 进二级页 | 返回上一级 |
| 单项调数值（年/月/日/时/分/秒） | 数值 +1 | 数值 −1 | 确认并退回 |
| **秒表页** | 光标左移一格 | 光标右移一格 | 停在「退回」上则回菜单 |
| **照明页** | 光标左移一格 | 光标右移一格 | 停在「退回」上则回菜单 |

> 秒表和照明页是**「选到就执行」**：高亮框移到「开始 / 停止 / 清除」或「OFF / ON / SOS」上，动作立刻生效（不是再按一下确认）。退回键（左上角图标）才是 K3 确认退出。

界面层级：

```
时钟首页 ──K3(菜单)──> 滑动菜单 ──K3(退回)──> 时钟首页
    │                      ├──K3(秒表)──> 秒表页：开始 / 停止 / 清除
    │                      ├──K3(手电筒)──> 照明页：OFF / ON / SOS
    │                      └──其余 4 项：预留位（进入后直接返回）
    └────K3(设置)──> 设置页 ──K3(日期时间设置)──> 选 年/月/日/时/分/秒 ──> 调值
```

### 秒表页

- 显示格式 `分:秒:百分秒`（`00:00:00` ~ `59:59:99`），高亮框在底部三个按钮间左右移动。
- **开始**：`start_timing_flag = 1`，计时继续累加。
- **停止**：`start_timing_flag = 0`，冻结当前读数（不清零，可以再按「开始」接着走）。
- **清除**：`min = sec = msec = 0`，同时停止。
- 计时精度 10 ms：TIM2 每 1 ms 进一次中断，`StopWatch_Tick()` 里 `Count >= 10` 才 `msec++`；`msec` 到 100 进 1 秒，`sec` 到 60 进 1 分，`min` 到 60 归零。

### 照明页

- **OFF**：`LED_OFF()`，`led_sos_flag = 0`，PB15 输出高电平，LED 灭。
- **ON**：`LED_ON()`，PB15 拉低，LED 长亮（这就是手电筒模式）。
- **SOS**：`led_sos_flag = 1`，`LED_SOS()` 在 TIM2 中断里每 50 ms 翻转一次 PB15，也就是 10 Hz 快速闪烁；离开该档或切回 OFF / ON 时 `led_sos_flag = 0`，闪灯停止。
- 说明：这里的 `SOS` 是**快速闪烁提醒**，不是标准的摩斯码 `… --- …`；想要真摩斯码见第 6 节「玩点 1」。

---

## 5. 代码是怎么跑起来的

### 5.1 主循环：三层状态机

`main()` 只有一个死循环，靠函数返回值切换界面：

```c
while (1) {
    clkflag1 = First_Page_Clock();   // 首页：返回 1=菜单, 2=设置
    if (clkflag1 == 1) Menu();       // 菜单：返回后回到首页
    else if (clkflag1 == 2) SettingPage();
}
```

每个界面内部自己也有一层 `while(1)`，`K3` 确认后 `return` 把控制权交回上一层 —— 这是最朴素也最好理解的「页面栈」，不用 RTOS。

### 5.2 按键：定时器节拍 + 松手沿

- `Timer_Init()`：TIM2 挂在 72 MHz，分频 720 → 100 kHz，计数值 100 → **每 1 ms 进一次中断**。
- `TIM2_IRQHandler()` 里调用 `Key_Tick()`，每 20 次（20 ms）采样一次引脚状态。
- 判断 `PreState != 0 && CurrentState == 0`（**松手瞬间**）才把键码写进 `Key_Num`，天然防抖，也避免长按连发。
- 业务代码只用 `Key_GetNum()`：有键码就取走并清零，没有返回 0。

所以你在任何界面里写 `if (Key_GetNum() == 3) { ... }` 就够了，不需要关心消抖。键码约定：`1` = 上一项（PB1），`2` = 下一项（PA6），`3` = 确认（PA4）。

### 5.3 滑动菜单动画

核心在 `Hardware/menu.c` 的 `Menu_Animation()`：

- 5 个 32×32 图标横向排开，间距 48 px，中间的被 44×44 的 `Frame` 框住。
- `x_pre` 是「当前选中图标的 x 坐标」，初值 48（屏幕中央）。每次刷新按 `Speed = 4` 像素向左/右平移。
- 平移到 `0`（左移）或 `96`（右移）时说明动画结束：选中项 `pre_selection` ±1，`x_pre` 复位回 48。
- 图标数据是 `Menu_Graph[][128]`，每个 32×32 单色位图正好 128 字节（8 页 × 128 列）。
- `menu_flag` 取值 1~7 表示 7 个菜单项，而动画窗格需要索引 `pre_selection + 2`，最远到 7，所以**数组末尾特意多放了一个空白图标**，防止越界。
- 确认键按下后先清屏，再按 `menu_flag` 分发：`1` = 退回，`2` = `StopWatch()`，`3` = `LED()`，`4~7` = 空分支（预留位）。

### 5.4 RTC 走时

`MyRTC_Init()` 第一次运行时：开 LSE → 配置 RTC 预分频 32767 → 用 `MyRTC_Time[]` 的初值写时间 → 在 `BKP_DR1` 写 `0xA5A5` 作为「已初始化」标记。之后上电只做同步，不会重置时间。

`MyRTC_SetTime()` / `MyRTC_ReadTime()` 在 `time_t` 和 `struct tm` 之间转换，并硬编码了 **+8 小时**（东八区），所以改时间不用管 UTC。

### 5.5 1_4 / 1_5 新增：中断里的两个节拍

`main.c` 的 TIM2 中断现在干三件事：

```c
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        Key_Tick();        // 按键 20 ms 消抖扫描
        StopWatch_Tick();  // 秒表 10 ms 累加
        LED_SOS();         // 闪灯 50 ms 翻转
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
    }
}
```

- **秒表**：`StopWatch_Tick()` 内部 `Count >= 10` 才走一步，`start_timing_flag` 为 1 时累加 `min / sec / msec`。计时不用额外定时器，也不占 CPU 循环。
- **闪灯**：`LED_SOS()` 内部 `Count >= 50` 翻转一次；`led_sos_flag` 由照明页控制，只有选了 SOS 档才真的翻转，其他档位即使中断在跑也不会动灯。
- 两个函数都用了 `static` 局部计数器，互不干扰，也避免了往全局里加变量。

---

## 6. 以后怎么玩

### 玩点 1：把 SOS 改成真正的摩斯码

现在 `LED_SOS()` 是「每 50 ms 翻一次」的匀速闪，想发标准 `… --- …` 可以改成状态机：短闪 3 次 → 停 → 长闪 3 次 → 停 → 短闪 3 次 → 长停，用计数变量按「单位时间 = 100 ms」排时间轴即可。

### 玩点 2：把剩下 4 个占位功能填上

`Menu()` 里现在是这样的空壳：

```c
if (menu_flag_temple == 4) {}   // MPU6050
if (menu_flag_temple == 5) {}   // 游戏
if (menu_flag_temple == 6) {}   // 小电视
if (menu_flag_temple == 7) {}   // 水平仪
```

照抄秒表页/照明页的结构：函数内部 `while(1)` + `Key_GetNum()`，`K3` 时 `return`：

```c
if (menu_flag_temple == 4) { MenuToFunction(); MPU6050_Page(); }
```

各功能的思路：

- **MPU6050 / 水平仪**：用**硬件 I2C1（PB6/PB7）**读陀螺仪/加速度，和 OLED 的软件 I2C（PB8/PB9）互不干扰；水平仪就是读出加速度后 `OLED_DrawCircle()` 画个随角度移动的圆。（同目录还有一份 `1_5_MPU6050` 的工程，可以直接对照）
- **小游戏**：128×64 的屏做贪吃蛇最合适 —— 8×8 网格，`OLED_DrawRectangle()` 画格子；3 个按键分别做「左转/右转/暂停」比上下左右更好操作。
- **小电视**：接 ESP8266/ESP32，串口（USART1：PA9/PA10）发 AT 指令，把天气、B 站粉丝数之类的文字拉回来往屏上打。

### 玩点 3：加一个自己的菜单图标

1. 用 **PCtoLCD2002** 取模，参数选：阴码、逐列式、逆向、C51 格式、输出 32×32 → 得到 128 个字节。
2. 把这段数据追加到 `Hardware/OLED_Data.c` 的 `Menu_Graph[][128]` 数组里，注意**空白图标要永远留在最后**，否则 `pre_selection + 2` 会越界。
3. 想加到 8 个菜单项：把 `Menu()` 里 `menu_flag` 的上界从 7 改成 8（两处：`if (menu_flag <= 0) menu_flag = 7;` 和 `if (menu_flag >= 8) menu_flag = 1;`），再补一个空白图标。

### 玩点 4：调手感

- 滑动速度：`uint8_t Speed = 4;` 改成 2 会更慢更丝滑，改 8 更干脆。
- 帧率上限取决于 `OLED_Update()` 全屏刷新的耗时（软件 I2C，整屏约十几到几十毫秒），想更顺可以只刷新变化区域：`OLED_UpdateArea(x, 0, 48, 64)`。
- 按键灵敏度：`Key_Tick()` 里的 `Count >= 20` 就是 20 ms 消抖窗口，嫌迟钝可以减小。
- 秒表刷新：现在每圈循环都重画整屏，读数会跟着 `msec` 跳；想更稳可以在主循环里只在 `msec` 变化时刷新显示。

### 玩点 5：往「真手表」方向走

- 换硬件 I2C / 换 1.3 寸 ST7789 彩屏（SPI）
- 加 MPU6050 的抬腕亮屏，点亮就是调用一次 `LED_ON()` / 关屏 `LED_OFF()`
- 照明改成多档亮度：PB15 接个 NPN 三极管，用 TIM3 PWM 调占空比
- 加 RTC 闹钟 + 蜂鸣器
- 加电池 + 充电管理，做低功耗（停机模式 + RTC 唤醒）
- 3D 打印个表壳，把 Blue Pill 和屏幕塞进去

---

## 7. 常见问题

**Q：程序卡死在 `MyRTC_Init()` 的 `while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET);`**
A：LSE 晶振没起振（没焊晶振 / 晶振坏了 / 负载电容不对）。把 `MyRTC.c` 里注释掉的那段 **LSI 版本**放出来替换掉 LSE 版本即可跑起来（代价是走时精度差、且不能用纽扣电池续命）。

**Q：断电重启时间归零**
A：时间存在 VBAT 域，没接纽扣电池就会丢；接了电池后 `BKP_DR1` 的 `0xA5A5` 标记还在，时间会继续走。

**Q：中文显示成乱码**
A：`OLED_Data.h` 里选的是 `OLED_CHARSET_GB2312`，所以**含中文显示字符串的源文件必须是 GB2312 编码**（`menu.c`、`SetTime.c`、`OLED_Data.c` 都是）。另外注意本项目里有些只含中文注释的文件是 UTF-8（如 `Key.c`、`Delay.c`、`main.c`），在 Keil 里设成 GB2312 打开会看到注释乱码，属正常现象，不影响编译。Keil 的编码设置在 `Edit → Configuration → Editor → Encoding`。

**Q：屏幕不亮 / 花屏**
A：查三件事 —— PB8/PB9 有没有接反、模块地址是不是 `0x78`（有些模块是 `0x7A`，改 `OLED.c` 里 `OLED_I2C_SendByte(0x78)`）、SDA/SCL 是否需要 4.7k 上拉。

**Q：按键没反应**
A：先确认 TIM2 中断在跑（`Timer_Init()` 有没有调用），再确认按下时引脚确实被拉到 0（用万用表量），最后看消抖窗口是不是太短。

**Q：照明页选了 ON，但 LED 不亮**
A：① 确认 LED 接的是 **PB15**（不是 1_3 的 PA1/PA2）；② 确认 LED 极性 —— 本项目是**低电平点亮**，LED 阳极要经电阻接 3.3 V、阴极接 PB15，接反了就是灭；③ 用万用表量 PB15 在 ON 档是否被拉低。

**Q：秒表数字不走 / 一开始就在跑**
A：秒表只在**高亮框停在「开始」**时 `start_timing_flag = 1`；如果停在「停止」或「清除」上，走时会被立刻冻住/清零。想让它「按一下才跑」，需要改成在 K3 上触发动作，而不是靠高亮。

**Q：编译报 `core_cm3.c` 相关错误**
A：新版 MDK 的 CMSIS 已内置 `core_cm3.h`，老工程的 `core_cm3.c` 会重复定义。把 `Start/core_cm3.c` 从工程里移除（右键 → Remove File）即可，头文件保留。

---

## 8. Git 用法

```bash
# 克隆
git clone git@github.com:Zack-Chen-Init/STM32_Watch_HAL.git

# 改完代码后
git add .
git commit -m "feat: 照明页加多档亮度"
git push
```

首次在自己电脑上用 SSH 推送，需要先把 `~/.ssh/id_ed25519.pub` 的内容加到 GitHub → Settings → SSH and GPG keys。

---

## 9. 致谢与许可

OLED 驱动、字库、图形 API 与整体外设库框架源自 **江协科技**（jiangxiekeji.com）的 STM32 入门教程；本项目在其基础上继续搭界面与功能。学习用途，转载注明出处。
