#ifndef TGL_DISPLAYTIME_H
#define TGL_DISPLAYTIME_H

#include <WiFiUdp.h>
#include <TimeLib.h>
#include <NTPtime.h>

// NTPtime Time(2); //UA in +2 time zone

unsigned int gLocalPort = 8000;
static const char ntpServerName[] = "ntp6.aliyun.com";
const int timeZone = 8;             // 东八区
const int NTP_PACKET_SIZE = 48;     // NTP时间在消息的前48字节中
byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming & outgoing packets

WiFiUDP gMyUdpClient;

//----  向NTP服务器发送请求  ----//
//-----------------------------//
void sendNTPpacket(IPAddress &address)
{
    memset(packetBuffer, 0, NTP_PACKET_SIZE);

    packetBuffer[0] = 0b11100011; // LI, Version, Mode
    packetBuffer[1] = 0;          // Stratum, or type of clock
    packetBuffer[2] = 6;          // Polling Interval
    packetBuffer[3] = 0xEC;       // Peer Clock Precision

    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;

    gMyUdpClient.beginPacket(address, 123); // NTP requests are to port 123
    gMyUdpClient.write(packetBuffer, NTP_PACKET_SIZE);
    gMyUdpClient.endPacket();
}
//-----------------------------//
time_t getNtpTime()
{
    IPAddress ntpServerIP; // NTP server's ip address

    // while (gMyUdpClient.parsePacket() > 0)
    //     ; // discard any previously received packets

    if (gMyUdpClient.parsePacket() > 0)
        return 0;
    WiFi.hostByName(ntpServerName, ntpServerIP);

    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500)
    {
        int size = gMyUdpClient.parsePacket();
        if (size >= NTP_PACKET_SIZE)
        {
            Serial.println("Receive NTP Response");
            gMyUdpClient.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 = (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
        }
    }
    Serial.println("No NTP Response :-(");
    return 0; // 无法获取时间时返回0
}
//-----------------------------//
void MyTime_Init()
{
    gMyUdpClient.begin(gLocalPort);
    Serial.println("等待同步...");
    setSyncProvider(getNtpTime);
    setSyncInterval(300);
}

#endif