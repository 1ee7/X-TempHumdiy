#include <Arduino.h>
// #include <Wire.h>
#include "Tgl_GetWeather.h"
#include "Tgl_Button.h"
#include "Tgl_Display.h"
#include "Tgl_WifiConfig.h"
#include "Tgl_SyncTime.h"
#include "Tgl_Animation.h"
#include "MAX3010x.h"
#include "spo2_algorithm.h"
#include "heartRate.h"
////////// --- define   ---////////////

///////// ---- init global valu ---////////////
uint64_t mTaskGetWeather = 0;
uint64_t mTaskGetTempHumi = 0;
uint64_t mTaskUpdateScreen = 0;

const int mA_BatVol_Pin = A0; //模拟量输入引脚

int mBat_Vol_RawValue = 0;  //电池电压读取值
int mBat_Vol_RealValue = 0; //电池电压读取值映射值

uint8_t gSystemRunMode = 0;
// 0--正常模式；1--省电模式；2--温度计模式；3--血氧仪模式；4--配网模式；

#define TIME_UPDATE_GETWEATHER 60 // 60s获取一次天气数据
#define TIME_UPDATE_TEMPHUMI 30   // 30s 获取一次温湿度
#define TIME_UPDATE_SCREEN 10     // 10s 跟新一次界面

int g_system_wifi_status = -1; // 系统 wifi状态；0--无网络；1--有网络
uint8_t mBitFlagMode = 0;
void System_RunMode_NorMal();
int System_RunMode_Thermometer();
void System_RunMode_OxiMeter();

//---------------------------//
MAX30105 particleSensor;
uint32_t irBuffer[100];  // infrared LED sensor data
uint32_t redBuffer[100]; // red LED sensor data
int32_t bufferLength;    // data length
int32_t spo2;            // SPO2 value
int8_t validSPO2;        // indicator to show if the SPO2 calculation is valid
int32_t heartRate;       // heart rate value
int8_t validHeartRate;   // indicator to show if the heart rate calculation is valid

void setup()
{
  // put your setup code here, to run once:
  /// @brief 串口波特率设置 ///
  Serial.begin(115200);
  Serial.println("-init- serial end.");

  gSystemRunMode = 0;
  mBitFlagMode = 0;

  ///  @brief 电压采集引脚    ///
  pinMode(mA_BatVol_Pin, INPUT);

  /// @brief 温湿度传感器设置  ///
  MySht3x_Init();

  /// @brief  按钮事件设置    ///
  MyButton_EventInit();

  /// @brief  屏幕初始化设置  ///
  MyDisplay_Init();

  /// @brief wifi配网       ///
  MyWifi_init();

  if (WiFi.status() == WL_CONNECTED) //网络连接成功，获取时间、天气信息
  {
    /// @brief 时间服务器初始化 ///
    MyTime_Init();

    MyWeatherInfo_Get();

    g_system_wifi_status = 1;
  }
  else
  {
    g_system_wifi_status = 0;
  }

  MySht3x_Read();

  /// @brief 清屏初始化     ///
  MyDisplay_EndInit();
}

//---------------------------------//
void loop()
{
  // put your main code here, to run repeatedly:

  switch (gSystemRunMode)
  {

  case MODE_SLEEP:
  {
    Serial.println("system run mode is sleep");
  }
  break;

  case MODE_THERMOMETER:
    if ((mBitFlagMode & 0x02) == 0)
    {
      Serial.println("system run mode is thermometer");
      mBitFlagMode = 0x02;
      MyDisplay_Clear();
    }
    if (System_RunMode_Thermometer() == -1)
    {
      gSystemRunMode = MODE_OXIMETER;
    }
    break;

  case MODE_OXIMETER:
    if ((mBitFlagMode & 0x10) == 0)
    {
      Serial.println("system run mode is OXImeter");
      mBitFlagMode = 0x10;
      {
        int mCountTry = 100;
        // Initialize sensor
        if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) // Use default I2C port, 400kHz speed
        {
          Serial.println("MAX30105 was not found. Please check wiring/power. ");
          while (mCountTry-- > 0)
            ;
        }
        Serial.println("Place your index finger on the sensor with steady pressure.");

        byte ledBrightness = 60; // Options: 0=Off to 255=50mA
        byte sampleAverage = 4;  // Options: 1, 2, 4, 8, 16, 32
        byte ledMode = 2;        // Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
        byte sampleRate = 100;   // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
        int pulseWidth = 411;    // Options: 69, 118, 215, 411
        int adcRange = 4096;     // Options: 2048, 4096, 8192, 16384

        particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); // Configure sensor with these settings

        bufferLength = 100; // buffer length of 100 stores 4 seconds of samples running at 25sps

        // read the first 100 samples, and determine the signal range
        for (byte i = 0; i < bufferLength; i++)
        {
          while (particleSensor.available() == false) // do we have new data?
          {
            particleSensor.check(); // Check the sensor for new data
            MyButton_Tick();
          }

          redBuffer[i] = particleSensor.getRed();
          irBuffer[i] = particleSensor.getIR();
          particleSensor.nextSample(); // We're finished with this sample so move to next sample

          Serial.print(F("red="));
          Serial.print(redBuffer[i], DEC);
          Serial.print(F(", ir="));
          Serial.println(irBuffer[i], DEC);
        }

        // calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
        maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
      }
    }
    System_RunMode_OxiMeter();
    break;

  case MODE_SMARTCONFIG:
    if ((mBitFlagMode & 0x04) == 0)
    {
      Serial.println("system run mode is Smart config");
      mBitFlagMode = 0x04;
      MyDisplay_Clear();
      MyWifi_Mode_SmartConfig_Init();
    }
    MyWifi_Mode_SmartConfig();
    break;

  case MODE_NORMAL:
  default:
    if ((mBitFlagMode & 0x01) == 0)
    {
      Serial.println("system run mode is normal");
      mBitFlagMode = 0x01;
      MyDisplay_Clear();
      mTaskGetWeather = 0;
      mTaskGetTempHumi = 0;
      mTaskUpdateScreen = 0;
      digitalClockInit();
    }
    System_RunMode_NorMal();
    break;
  }

  /// @brief 采集电池电压，显示电池电量 ///
  mBat_Vol_RawValue = analogRead(mA_BatVol_Pin);

  mBat_Vol_RealValue = map(mBat_Vol_RawValue, 0, 1024, 0, 330); //可以修改数值映射330   3.3

  Serial.print("battery value:");
  Serial.println(mBat_Vol_RealValue, DEC);
  MyButton_Tick();
  delay(2);
}

//-----------------------------//
#if 0

  time_t time = NOW();
  uint64_t mtime=millis()/1000;
  Serial.print(time);
  Serial.println("-test1- hello world.");
  Serial.print(mtime, DEC);
  Serial.println("-test2- hello world.");
  delay(1000);
////////// SHT3x /////////////
 float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.print(t); Serial.print("\t\t");
  } else { 
    Serial.println("Failed to read temperature");
  }
  
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
  } else { 
    Serial.println("Failed to read humidity");
  }

#endif
//------------------------------//
void System_RunMode_NorMal()
{
  if (g_system_wifi_status == 0)
  {
    // 10s 更新一次温湿度
    if (mTaskGetTempHumi == 0 || millis() / 1000 - mTaskGetTempHumi > 10)
    {
      mTaskGetTempHumi = millis() / 1000;

      MySht3x_Read();

      MyDisplay_ShowTmpHumi();
    }
  }
  else
  {
    /// 时钟显示 ///
    MyDisplay_ShowTime();

    /// 显示动画 ///
    MyShow_ImgAnim();

    // 60s 更新一次天气
    if (mTaskGetWeather == 0 || millis() / 1000 - mTaskGetWeather > TIME_UPDATE_GETWEATHER)
    {
      mTaskGetWeather = millis() / 1000;

      MyWeatherInfo_Get();
    }
    // 30s 更新一次温湿度
    if (mTaskGetTempHumi == 0 || millis() / 1000 - mTaskGetTempHumi > TIME_UPDATE_TEMPHUMI)
    {
      mTaskGetTempHumi = millis() / 1000;

      MySht3x_Read();

      MyDisplay_WeaterData();
    }

    // 10s 更新一次天气信息
    if (mTaskUpdateScreen == 0 || millis() / 1000 - mTaskUpdateScreen > TIME_UPDATE_SCREEN)
    {
      mTaskUpdateScreen = millis() / 1000;

      MyDisplay_ScrollBanner();
    }
  }
}
//------------------------------//
int System_RunMode_Thermometer()
{
  if (mTaskGetTempHumi == 0 || millis() / 1000 - mTaskGetTempHumi > 1)
  {
    mTaskGetTempHumi = millis() / 1000;

    if (MyThermoMeter_Read() == 0)
    {
      MyDisplay_ShowTmpHumi();
      return 0;
    }
    else
      return -1;
  }
  return 0;
}
//------------------------------//
void System_RunMode_OxiMeter()
{
  // Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second

  // dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
  for (byte i = 25; i < 100; i++)
  {
    redBuffer[i - 25] = redBuffer[i];
    irBuffer[i - 25] = irBuffer[i];
  }

  // take 25 sets of samples before calculating the heart rate.
  for (byte i = 75; i < 100; i++)
  {
    while (particleSensor.available() == false) // do we have new data?
      particleSensor.check();                   // Check the sensor for new data

    // digitalWrite(readLED, !digitalRead(readLED)); // Blink onboard LED with every data read

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); // We're finished with this sample so move to next sample

    // send samples and calculation result to terminal program through UART
    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.print(irBuffer[i], DEC);

    Serial.print(F(", HR="));
    Serial.print(heartRate, DEC);

    Serial.print(F(", HRvalid="));
    Serial.print(validHeartRate, DEC);

    Serial.print(F(", SPO2="));
    Serial.print(spo2, DEC);

    Serial.print(F(", SPO2Valid="));
    Serial.println(validSPO2, DEC);
  }

  // After gathering 25 new samples recalculate HR and SP02
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
}