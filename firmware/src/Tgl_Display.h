#ifndef TGL_DISPLAY_H
#define TGL_DISPLAY_H

#include <TFT_eSPI.h>
#include <SPI.h>
#include <TJpg_Decoder.h>
#include "font/ZdyLwFont_20.h"
#include "weathernum.h"
#include "img/humidity.h"
#include "img/temperature.h"
// #include "img/test/testImage.h"
#include "Tgl_SyncTime.h"
#include "number.h"
// extern

TFT_eSPI gMyTft = TFT_eSPI(); // 引脚请自行配置tft_espi库中的 User_Setup.h文件
uint16_t gBgColor = 0x0000;
TFT_eSprite gTftSpriteClk = TFT_eSprite(&gMyTft);

byte gLoadNum = 6;

//-----------------------------//
bool Fft_Output_Cb(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    if (y >= gMyTft.height())
        return 0;
    gMyTft.pushImage(x, y, w, h, bitmap);
    return 1;
}

//----------------------------------//
void MyDisplay_Init()
{
    gMyTft.begin();          /* TFT init */
    gMyTft.invertDisplay(1); // 反转所有显示颜色：1反转，0正常
    gMyTft.fillScreen(0x0000);
    gMyTft.setTextColor(TFT_BLACK, gBgColor);

    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(Fft_Output_Cb);

    Serial.println("-init- dislplay is end.");
}

extern int g_system_wifi_status;
void MyDisplay_EndInit()
{
    gMyTft.fillScreen(TFT_BLACK); // 清屏

    // TJpgDec.drawJpg(0, 0, background, sizeof(background)); //frog 背景图
    if (g_system_wifi_status == 0)
    {
        TJpgDec.drawJpg(25, 183, temperature, sizeof(temperature)); // 温度图标
        TJpgDec.drawJpg(25, 213, humidity, sizeof(humidity));       // 湿度图标
    }
}
//-------------------------------------//
void MyDisplay_Loading(byte mdelayTime)
{
    gTftSpriteClk.setColorDepth(8);

    gTftSpriteClk.createSprite(200, 100); // 创建窗口 w,h,frame=-1
    gTftSpriteClk.fillSprite(0x0000);     // 填充率

    gTftSpriteClk.drawRoundRect(0, 0, 200, 16, 8, 0xFFFF);      // 空心圆角矩形
    gTftSpriteClk.fillRoundRect(3, 3, gLoadNum, 10, 5, 0xFFFF); // 实心圆角矩形
    gTftSpriteClk.setTextDatum(CC_DATUM);                       // 设置文本数据
    gTftSpriteClk.setTextColor(TFT_GREEN, 0x0000);
    gTftSpriteClk.drawString("Connecting to WiFi......", 100, 40, 2); // x,y,font
    gTftSpriteClk.setTextColor(TFT_WHITE, 0x0000);
    gTftSpriteClk.drawRightString("SDD V1.2", 180, 60, 2);
    gTftSpriteClk.pushSprite(20, 110); // 窗口位置

    gTftSpriteClk.deleteSprite();
    gLoadNum += 1;
    delay(mdelayTime);
}

//-----------------------------------//
void MyDisplay_Start_Loading()
{
    while (gLoadNum < 194) // 让动画走完
    {
        MyDisplay_Loading(1);
    }
}

/*------------------------------------
           天气信息写到屏幕上
-------------------------------------*/

TFT_eSprite Wclk = TFT_eSprite(&gMyTft);
TFT_eSprite clkb = TFT_eSprite(&gMyTft);
extern String gStrJsonCityDZ;
extern String gStrJsonDataSK;
extern String gStrJsonFC;

extern float gfDataHumi;
extern float gfDataTemp;
int currentIndex = 0;
String scrollText[7];

WeatherNum wrat;

void MyDisplay_ScrollBanner()
{
    if (scrollText[currentIndex])
    {
        clkb.setColorDepth(8);
        clkb.loadFont(ZdyLwFont_20);
        clkb.createSprite(150, 30);
        clkb.fillSprite(gBgColor);
        clkb.setTextWrap(false);
        clkb.setTextDatum(CC_DATUM);
        clkb.setTextColor(TFT_WHITE, gBgColor);
        clkb.drawString(scrollText[currentIndex], 74, 16);
        clkb.pushSprite(10, 45);

        clkb.deleteSprite();
        clkb.unloadFont();

        if (currentIndex >= 5)
            currentIndex = 0; // 回第一个
        else
            currentIndex += 1; // 准备切换到下一个
    }
}

void MyDisplay_WeaterData()
{
    // 解析第一段JSON
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, gStrJsonDataSK);
    JsonObject sk = doc.as<JsonObject>();
    /***绘制相关文字***/
    Wclk.setColorDepth(8);
    Wclk.loadFont(ZdyLwFont_20);

    String tempStringe = ""; // empty string
    String humiStringe = ""; // empty string

    // gfDataTemp = gSensorSht31.readTemperature();
    // gfDataHumi = gSensorSht31.readHumidity();

    // MySht3x_Read();

    // Serial.print("temp:%f,humi:%f",temp_float,humi_float);
    // 温度
    tempStringe.concat(gfDataTemp);
    Wclk.createSprite(90, 24);
    Wclk.fillSprite(gBgColor);
    Wclk.setTextDatum(CC_DATUM);

    if (gfDataTemp > 30.0)
        Wclk.setTextColor(TFT_RED, gBgColor);
    else if (gfDataTemp < 16.0)
        Wclk.setTextColor(TFT_CYAN, gBgColor);
    else
        Wclk.setTextColor(TFT_WHITE, gBgColor);

    Wclk.drawString(tempStringe + "℃", 36, 13);
    Wclk.pushSprite(60, 184);
    Wclk.deleteSprite();

    // 湿度
    humiStringe.concat(gfDataHumi);
    Wclk.createSprite(90, 24);
    Wclk.fillSprite(gBgColor);
    Wclk.setTextDatum(CC_DATUM);

    if (gfDataHumi > 80.0)
        Wclk.setTextColor(TFT_RED, gBgColor);
    else if (gfDataHumi < 20.0)
        Wclk.setTextColor(TFT_CYAN, gBgColor);
    else
        Wclk.setTextColor(TFT_WHITE, gBgColor);

    Wclk.drawString(humiStringe + "%", 36, 13);
    Wclk.pushSprite(56, 214);
    Wclk.deleteSprite();

    // 城市名称
    Wclk.createSprite(94, 30);
    Wclk.fillSprite(gBgColor);
    Wclk.setTextDatum(CC_DATUM);
    Wclk.setTextColor(TFT_WHITE, gBgColor);
    Wclk.drawString(sk["cityname"].as<String>(), 44, 16);
    Wclk.pushSprite(15, 15);
    Wclk.deleteSprite();

    // PM2.5空气指数
    uint16_t pm25BgColor = gMyTft.color565(156, 202, 127); // 优
    String aqiTxt = "优";
    int pm25V = sk["aqi"];
    if (pm25V > 200)
    {
        pm25BgColor = gMyTft.color565(136, 11, 32); // 重度
        aqiTxt = "重度";
    }
    else if (pm25V > 150)
    {
        pm25BgColor = gMyTft.color565(186, 55, 121); // 中度
        aqiTxt = "中度";
    }
    else if (pm25V > 100)
    {
        pm25BgColor = gMyTft.color565(242, 159, 57); // 轻
        aqiTxt = "轻度";
    }
    else if (pm25V > 50)
    {
        pm25BgColor = gMyTft.color565(247, 219, 100); // 良
        aqiTxt = "良";
    }
    Wclk.createSprite(56, 24);
    Wclk.fillSprite(gBgColor);
    Wclk.fillRoundRect(0, 0, 50, 24, 4, pm25BgColor);
    Wclk.setTextDatum(CC_DATUM);
    Wclk.setTextColor(0x0000);
    Wclk.drawString(aqiTxt, 25, 13);
    Wclk.pushSprite(104, 18);
    Wclk.deleteSprite();

    scrollText[0] = "实时天气 " + sk["weather"].as<String>();
    scrollText[1] = "空气质量 " + aqiTxt;
    scrollText[2] = "风向 " + sk["WD"].as<String>() + sk["WS"].as<String>();

    // 天气图标
    wrat.printfweather(170, 15, atoi((sk["weathercode"].as<String>()).substring(1, 3).c_str()));

    // 左上角滚动字幕
    // 解析第二段JSON
    deserializeJson(doc, gStrJsonCityDZ);
    JsonObject dz = doc.as<JsonObject>();
    // Serial.println(sk["ws"].as<String>());
    // 横向滚动方式

    scrollText[3] = "今日" + dz["weather"].as<String>();

    deserializeJson(doc, gStrJsonFC);
    JsonObject fc = doc.as<JsonObject>();

    scrollText[4] = "最低温度" + fc["fd"].as<String>() + "℃";
    scrollText[5] = "最高温度" + fc["fc"].as<String>() + "℃";

    Wclk.unloadFont();
}
//////////////////////////////////////////////////////////////////

TFT_eSprite Tclk = TFT_eSprite(&gMyTft);

String wk[7] = {"日", "一", "二", "三", "四", "五", "六"};
unsigned char Hour_sign = 60;
unsigned char Minute_sign = 60;
unsigned char Second_sign = 60;
Number dig;
unsigned long DotTime = 0;
bool ibs;

String week()
{

    String s = "周" + wk[weekday() - 1];
    return s;
}

// 月日
String monthDay()
{
    String s = String(month());
    s = s + "月" + day() + "日";
    return s;
}

void digitalClockInit()
{
    Hour_sign = 0;
    Minute_sign = 0;
}

void digitalClockDisplay()
{
    int timey = 82;
    if (hour() != Hour_sign) // 时钟刷新
    {
        dig.printfW3660(6, timey, hour() / 10);
        dig.printfW3660(46, timey, hour() % 10);
        Hour_sign = hour();
    }
    
    if (minute() != Minute_sign) // 分钟刷新
    {
        dig.printfO3660(105, timey, minute() / 10);
        dig.printfO3660(145, timey, minute() % 10);
        Minute_sign = minute();
    }

    if (second() != Second_sign) // 秒刷新
    {
        dig.printfW1830(186, timey + 30, second() / 10);
        dig.printfW1830(206, timey + 30, second() % 10);
        Second_sign = second();
    }

    if (millis() - DotTime > 250)
    { // 0.5s更新
        DotTime = millis();
        ibs = !ibs;
        if (ibs)
            dig.printfDot1260(89, timey);
        else
            dig.printfDotRev1260(89, timey);
    }

    /***日期****/
    Tclk.setColorDepth(8);
    Tclk.loadFont(ZdyLwFont_20); // ？？？？ tgl mark

    // 星期
    Tclk.createSprite(58, 30);
    Tclk.fillSprite(gBgColor);
    Tclk.setTextDatum(CC_DATUM);
    Tclk.setTextColor(TFT_WHITE, gBgColor);
    Tclk.drawString(week(), 29, 16);
    Tclk.pushSprite(102, 150);
    Tclk.deleteSprite();

    // 月日
    Tclk.createSprite(95, 30);
    Tclk.fillSprite(gBgColor);
    Tclk.setTextDatum(CC_DATUM);
    Tclk.setTextColor(TFT_WHITE, gBgColor);
    Tclk.drawString(monthDay(), 49, 16);
    Tclk.pushSprite(5, 150);
    Tclk.deleteSprite();

    Tclk.unloadFont();
    /***日期****/
}

time_t prevDisplay = 0; // 显示时间
void MyDisplay_ShowTime()
{
    // if (now() != prevDisplay && esp8266_Status != 1)
    now();

    if (now() != prevDisplay)
    {
        prevDisplay = now();
        digitalClockDisplay();
    }
}
//-----------------------------//
void MyDisplay_ShowTmpHumi()
{
//////////////////////////////////
#if 0
 if (hour() != Hour_sign) // 时钟刷新
    {
        dig.printfW3660(6, timey, hour() / 10);
        dig.printfW3660(46, timey, hour() % 10);
        Hour_sign = hour();
    }
    if (minute() != Minute_sign) // 分钟刷新
    {
        dig.printfO3660(105, timey, minute() / 10);
        dig.printfO3660(145, timey, minute() % 10);
        Minute_sign = minute();
    }

    if (second() != Second_sign) // 秒刷新
    {
        dig.printfW1830(186, timey + 30, second() / 10);
        dig.printfW1830(206, timey + 30, second() % 10);
        Second_sign = second();
    }
#endif
    /////////////////////////////////
    // 显示温度 gfDataTemp

    TJpgDec.drawJpg(25, 20, temperature, sizeof(temperature)); // 温度图标

    int miTemp = floor(gfDataTemp);
    int myTData = 54;

    int mItemp1 = floor(gfDataTemp * 100.0);
    int mItemp2 = mItemp1 % 100;

    dig.printfW3660(36, myTData, miTemp / 10);
    dig.printfW3660(76, myTData, miTemp % 10);

    dig.printfW1830(135, myTData, mItemp2 / 10);
    dig.printfW1830(175, myTData, mItemp2 % 10);

    // 显示湿度

    TJpgDec.drawJpg(25, 138, humidity, sizeof(humidity)); // 湿度图标

    int miHumi = floor(gfDataHumi);
    int myHData = 168;

    int mIHumi1 = floor(gfDataHumi * 100.0);
    int mIHumi2 = mIHumi1 % 100;

    dig.printfO3660(36, myHData, miHumi / 10);
    dig.printfO3660(76, myHData, miHumi % 10);

    dig.printfW1830(135, myHData, mIHumi2 / 10);
    dig.printfW1830(175, myHData, mIHumi2 % 10);
}

//-------------------------------//
void MyDisplay_Clear()
{
    gMyTft.fillScreen(TFT_BLACK); // 清屏
}

#endif