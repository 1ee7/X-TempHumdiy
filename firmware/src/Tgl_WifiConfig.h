#ifndef TGL_WIFICONFIG_H
#define TGL_WIFICONFIG_H

#include <ESP8266WiFi.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "Tgl_Display.h"
#include "qr.h"

extern TFT_eSPI gMyTft;
extern byte gLoadNum;

const char ssid[] = ""; // WIFI名称 修改这2个就可以了
const char pass[] = ""; // WIFI密码

extern int g_system_status; // 系统 运行状态；0--无网络；1--有网络

int mcount = 0;
//-------------------------//
/**************************
 * *       微信配网
 **************************/
void MyWifi_SmartConfig(void) // 微信配网
{
    WiFi.mode(WIFI_STA); // 设置STA模式

    gMyTft.pushImage(0, 0, 240, 240, qr);

    Serial.println("\r\nWait for Smartconfig..."); // 打印log信息

    WiFi.beginSmartConfig(); // 开始SmartConfig，等待手机端发出用户名和密码

    while (1)
    {
        Serial.print(".");

        if (mcount++ > 600)
        {
            Serial.println("SmartConfig failed and timeout.");

            g_system_wifi_status = 0;

            mcount = 0;
            break;
        }

        delay(100);                 // wait for a second
        if (WiFi.smartConfigDone()) // 配网成功，接收到SSID和密码
        {
            Serial.println("SmartConfig Success");
            Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
            Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
            WiFi.setAutoConnect(true); // 设置自动连接
            g_system_wifi_status = 1;
            break;
        }
    }
    gLoadNum = 194;
}

//-------------------------//

void MyWifi_Mode_SmartConfig_Init(void) // 微信配网
{
    WiFi.mode(WIFI_STA); // 设置STA模式

    gMyTft.pushImage(0, 0, 240, 240, qr);

    Serial.println("\r\nWait for Smartconfig..."); // 打印log信息

    WiFi.beginSmartConfig(); // 开始SmartConfig，等待手机端发出用户名和密码
}

void MyWifi_Mode_SmartConfig(void) // 微信配网
{

    Serial.print(".");

    // if (mcount++ > 600)
    // {
    // Serial.println("SmartConfig failed and timeout.");

    // g_system_status = 0;

    //     mcount = 0;
    // }

    delay(100);                 // wait for a second
    if (WiFi.smartConfigDone()) // 配网成功，接收到SSID和密码
    {
        Serial.println("SmartConfig Success");
        Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
        Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
        WiFi.setAutoConnect(true); // 设置自动连接

        delay(5000);
        ESP.restart();
    }
}

//-------------------------//
bool MyWifi_AutoConfig()
{
    WiFi.begin();
    for (int i = 0; i < 2; i++)
    {
        int wstatus = WiFi.status();
        if (wstatus == WL_CONNECTED)
        {
            Serial.println("WIFI SmartConfig Success");
            Serial.printf("SSID:%s", WiFi.SSID().c_str());
            Serial.printf(", PSW:%s\r\n", WiFi.psk().c_str());
            Serial.print("LocalIP:");
            Serial.print(WiFi.localIP());
            Serial.print(" ,GateIP:");
            Serial.println(WiFi.gatewayIP());
            return true;
        }
        else
        {
            Serial.print("WIFI AutoConfig Waiting......");
            Serial.println(wstatus);
            delay(1000);
        }
    }
    Serial.println("WIFI AutoConfig Faild!");
    return false;
}

//-------------------------//
void MyWifi_Loading()
{

    int count = 100;
    // while (WiFi.status() != WL_CONNECTED)

    for (int i = 0; i < 300; i++)
    {
        MyDisplay_Loading(70);
        if (gLoadNum >= 194)
        {
            MyWifi_SmartConfig();
            break;
        }

        if (WiFi.status() == WL_CONNECTED)
            break;
    }
    delay(10);
    while (gLoadNum < 194) // 让动画走完
    {
        MyDisplay_Loading(1);
    }
}

//-----------------------------//
void MyWifi_init()
{
    WiFi.mode(WIFI_STA);
    delay(500);
    WiFi.begin(ssid, pass);

    MyWifi_AutoConfig();

    MyWifi_Loading();
}

#endif