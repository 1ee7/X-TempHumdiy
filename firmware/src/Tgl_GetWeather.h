#ifndef TGL_DISPLAYWEATHER_H
#define TGL_DISPLAYWEATHER_H

#include <Wire.h>
#include "ArduinoJson.h"
#include "Adafruit_SHT31.h"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

#include "Tgl_Display.h"

#define SDA_PIN 4 // 引脚接法在这里
#define SCL_PIN 5

Adafruit_SHT31 gSensorSht31;
Adafruit_SHT31 gSensorThermoMeter = NULL;
HTTPClient httpClient;
WiFiClient wificlient;

String gStrJsonCityDZ, gStrJsonDataSK, gStrJsonFC;
String gCrtCityCode = "101250101"; // 天气城市代码

float gfDataHumi;
float gfDataTemp;

static uint8_t gInitThermoMeter = 0;

//-------------------------------------//

/*  上电检测外置温湿度传感器，如果失败，则加载板载温湿度传感器  */
void MySht3x_Init()
{
    Wire.begin(SDA_PIN, SCL_PIN);
    gInitThermoMeter = 0;

    uint8_t mNoI2c = 0;
    gSensorSht31 = Adafruit_SHT31(&Wire);
    // if (!gSensorSht31.begin(0x44))
    if (!gSensorSht31.begin(0x44))
    {
        Serial.println("couldnot find STH31-0x45");
        int mCount = 10;
        while (mCount > 0)
        {
            Serial.print(mCount);
            Serial.println(" ,retry  init SHT3X 0x45 ...");
            mCount--;
            delay(100); // 100ms
        }
        mNoI2c = 2;
    }
    else
    {
        Serial.println("Sht3x 0x45 init success.");
        mNoI2c = 1;
    }

    if (mNoI2c == 2)
    {
        if (!gSensorSht31.begin(0x45))
        {
            Serial.println(" couldnot find STH31-0x44");
            int mCount = 10;
            while (mCount > 0)
            {
                Serial.print(mCount);
                Serial.println(" ,retry  init SHT3X-0x44...");
                mCount--;
                delay(100); // 100ms
            }
        }
        else
        {
            Serial.println("Sht3x 0x44 init success.");
        }
    }
}

//-------------------------------------//
void MySht3x_Read()
{
    gfDataTemp = gSensorSht31.readTemperature();
    gfDataHumi = gSensorSht31.readHumidity();

    if (!isnan(gfDataTemp))
    { // check if 'is not a number'
        Serial.print("Temp *C = ");
        Serial.print(gfDataTemp);
        Serial.print("\t\t");
    }
    else
    {
        Serial.println("Failed to read temperature");
    }

    if (!isnan(gfDataHumi))
    { // check if 'is not a number'
        Serial.print("Hum. % = ");
        Serial.println(gfDataHumi);
    }
    else
    {
        Serial.println("Failed to read humidity");
    }
}

float mLastDataTemp = 0;

int8_t MyThermoMeter_Read()
{
    // init
    if (gInitThermoMeter == 0)
    {
        gSensorThermoMeter = Adafruit_SHT31(&Wire);
        if (!gSensorThermoMeter.begin(0x44))
        {
            Serial.println("couldnot find ThermoMeter");
            int mCount = 10;
            while (mCount > 0)
            {
                Serial.print(mCount);
                Serial.println(" ,retry  init ThermoMeter ...");
                mCount--;
                delay(100); // 100ms
            }
            if (mCount == 0)
            {
                Serial.println(" ERRO  init ThermoMeter ...");
                return -1;
            }
        }
        else
        {
            gInitThermoMeter = 1;
            Serial.println("ThermoMeter init success.");
        }
    }

    // read

    mLastDataTemp = gSensorThermoMeter.readTemperature();
    gfDataHumi = gSensorThermoMeter.readHumidity();

    if (!isnan(mLastDataTemp))
    { // check if 'is not a number'
        Serial.print("ThermoMeter Temp *C = ");
        Serial.print(mLastDataTemp);
        Serial.print("\t\t");
        if (mLastDataTemp > gfDataTemp)
            gfDataTemp = mLastDataTemp;
    }
    else
    {
        Serial.println("Failed to read ThermoMeter");
    }

    if (!isnan(gfDataHumi))
    { // check if 'is not a number'
        Serial.print("ThermoMeter Hum. % = ");
        Serial.println(gfDataHumi);
    }
    else
    {
        Serial.println("Failed to read ThermoMeter humidity");
    }

    return 0;
}

// 获取城市天气
void getCityWeater()
{
    // String URL = "http://d1.weather.com.cn/dingzhi/" + cityCode + ".html?_="+String(now());//新
    String URL = "http://d1.weather.com.cn/weather_index/" + gCrtCityCode + ".html?_=" + String(now()); // 原来
    // 创建 HTTPClient 对象
    // HTTPClient httpClient;
    httpClient.begin(wificlient, URL);

    // 设置请求头中的User-Agent
    httpClient.setUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1");
    httpClient.addHeader("Referer", "http://www.weather.com.cn/");

    // 启动连接并发送HTTP请求
    int httpCode = httpClient.GET();
    Serial.println("正在获取天气数据");
    Serial.println(URL);

    // 如果服务器响应OK则从服务器获取响应体信息并通过串口输出
    if (httpCode == HTTP_CODE_OK)
    {

        String str = httpClient.getString();
        int indexStart = str.indexOf("weatherinfo\":");
        int indexEnd = str.indexOf("};var alarmDZ");

        gStrJsonCityDZ = str.substring(indexStart + 13, indexEnd);
        //    Serial.println(jsonCityDZ);

        indexStart = str.indexOf("dataSK =");
        indexEnd = str.indexOf(";var dataZS");
        gStrJsonDataSK = str.substring(indexStart + 8, indexEnd);
        //    Serial.println(jsonDataSK);

        indexStart = str.indexOf("\"f\":[");
        indexEnd = str.indexOf(",{\"fa");
        gStrJsonFC = str.substring(indexStart + 5, indexEnd);
        //    Serial.println(jsonFC);

        // weaterData(&jsonCityDZ, &jsonDataSK, &jsonFC);
        Serial.println("获取成功");
    }
    else
    {
        Serial.println("请求城市天气错误：");
        Serial.print(httpCode);
    }

    // 关闭ESP8266与服务器连接
    httpClient.end();
}

//----------------------------//
//        获取当前城市编码     //
//----------------------------//
void getCityCode()
{
    String URL = "http://wgeo.weather.com.cn/ip/?_=" + String(now());
    // 创建 HTTPClient 对象

    // 配置请求地址。此处也可以不使用端口号和PATH而单纯的
    httpClient.begin(wificlient, URL);

    // 设置请求头中的User-Agent
    httpClient.setUserAgent("Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1");
    httpClient.addHeader("Referer", "http://www.weather.com.cn/");

    // 启动连接并发送HTTP请求
    int httpCode = httpClient.GET();
    Serial.print("Send GET request to URL: ");
    Serial.println(URL);

    // 如果服务器响应OK则从服务器获取响应体信息并通过串口输出
    if (httpCode == HTTP_CODE_OK)
    {
        String str = httpClient.getString();

        int aa = str.indexOf("id=");
        if (aa > -1)
        {
            gCrtCityCode = str.substring(aa + 4, aa + 4 + 9);
            Serial.println(gCrtCityCode);
            // getCityWeater();
        }
        else
        {
            Serial.println("获取城市代码失败");
        }
    }
    else
    {
        Serial.println("请求城市代码错误：");
        Serial.println(httpCode);
    }

    // 关闭ESP8266与服务器连接
    httpClient.end();
}

//---------------------//
void MyWeatherInfo_Get()
{
    // 获取城市code
    getCityCode();
    // 获取天气
    getCityWeater();
    // // 获取温湿度
    // MySht3x_Read();
}

#endif