#ifndef TGL_BUTTON_H
#define TGL_BUTTON_H

#include "OneButton.h"

#define PIN_INPUT 0

OneButton gMyButton(PIN_INPUT, true);

extern uint8_t gSystemRunMode;
extern float gfDataHumi;
extern float gfDataTemp;
// 0--正常模式；1--省电模式；2--温度计模式；3--血氧仪模式；4--配网模式；

#define MODE_NORMAL 0
#define MODE_SLEEP 1
#define MODE_THERMOMETER 2
#define MODE_OXIMETER 3
#define MODE_SMARTCONFIG 4

void MyButton_Click()
{
    Serial.println("--my button click-- in sleep mode ");  
    if (gSystemRunMode != MODE_SLEEP)
        gSystemRunMode = MODE_SLEEP;
    else
        gSystemRunMode = MODE_NORMAL;
}

void MyButton_DoubleClick()
{
    if (gSystemRunMode != MODE_THERMOMETER)
        gSystemRunMode = MODE_THERMOMETER;
    else
        gSystemRunMode = MODE_NORMAL;

    Serial.print("--my button doubleclick --Run Mode Thermometer:");
    Serial.println(gSystemRunMode);

    gfDataTemp = 0;
    gfDataHumi = 0;
}

void MyButton_LongClick()
{
    if (gSystemRunMode != MODE_SMARTCONFIG)
        gSystemRunMode = MODE_SMARTCONFIG;
    else
        gSystemRunMode = MODE_NORMAL;

    Serial.println("--my button longclick --show smart config ");
    Serial.println(gSystemRunMode);
}

void MyButton_MultiClick()
{
    if (gSystemRunMode != MODE_OXIMETER)
        gSystemRunMode = MODE_OXIMETER;
    else
        gSystemRunMode = MODE_NORMAL;
    Serial.println("--my button Multclick,  Oxtest ");
    Serial.println(gSystemRunMode);
}

void MyButton_EventInit()
{
    Serial.println("--my button init begin...");

    gMyButton.reset(); // 清除一下按钮状态机的状态
    /**
     * set # millisec after safe click is assumed.
     */
    // void setDebounceTicks(const int ticks);
    gMyButton.setDebounceTicks(80); // 设置消抖时长为80毫秒,默认值为：50毫秒
    /**
     * set # millisec after single click is assumed.
     */
    // void setClickTicks(const int ticks);
    gMyButton.setClickTicks(500); // 设置单击时长为500毫秒,默认值为：400毫秒

    /**
     * set # millisec after press is assumed.
     */
    // void setPressTicks(const int ticks);
    gMyButton.setPressTicks(1000); // 设置长按时长为1000毫秒,默认值为：800毫秒

    gMyButton.attachClick(MyButton_Click);              // 初始化单击回调函数
    gMyButton.attachDoubleClick(MyButton_DoubleClick);  // 初始化双击回调函数
    gMyButton.attachLongPressStart(MyButton_LongClick); // 初始化长按开始回调函数
    gMyButton.attachMultiClick(MyButton_MultiClick);    // 初始化按了多次(3次或以上)回调函数
}

void MyButton_Tick()
{
    gMyButton.tick();
}
#endif

#if 0
  // button.attachClick(attachClick);//初始化单击回调函数
  // button.attachDoubleClick(attachDoubleClick);//初始化双击回调函数
  // button.attachLongPressStart(attachLongPressStart);//初始化长按开始回调函数
  // button.attachDuringLongPress(attachDuringLongPress);//初始化长按期间回调函数
  // button.attachLongPressStop(attachLongPressStop);//初始化长按结束回调函数
  // button.attachMultiClick(attachMultiClick);//初始化按了多次(3次或以上)回调函数

#endif