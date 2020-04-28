/**********************************************************
   Title : LCD12232 DIGITAL CLOCK using RTC, GPS
   Date : 2020.03.25.
   Author : Park Yongjoon
 **********************************************************/
#include <Arduino.h>
#include <U8g2lib.h>
#include <DS1307RTC.h>
#include <TimeLib.h>
#include <TinyGPS++.h>
#include "font.h"

#define USEWIFI
//#define USEGPS

//스크롤에서 한번에 표시할 글자 수 지정
#define SCRLWIDTH 7

//스크롤 스피드 지정. 기본5 숫자가 클수록 느림
#define SCRLSPEED 8

//스크롤 글자 내용 지정
String tname = "Kim Jae-hyeon";

//----------------------------------------
//백라이트 밝기가 변하는 속도 클수록 늦음
#define BRIGHT_INTERVAL 10

#ifdef USEWIFI
#define SYNKDISPLAYTIME 3660  //GPS일경우 3초, WIFI일경우 3660초 
#endif

#ifdef USEGPS
#define SYNKDISPLAYTIME 3  //GPS일경우 3초, WIFI일경우 3660초 
#endif

#define WidthBigNum 14
#define WidthBigNum1 9
#define WidthBigColon 2
#define HeightBigNum 26

#define WidthSmallNum 5
#define WidthSmalldot 1
#define HeightSmallNum 8

#define WidthWeek  11
#define HeightWeek  13

#define PosH1     0,3
#define PosH      11,3
#define PosColon  27,3
#define PosM10    31,3
#define PosM1     47,3
#define Posy10    66,7
#define Posy1     72,7
#define Posydot   78,7
#define Posm10    80,7
#define Posm1     86,7
#define Posmdot   92,7
#define Posd10    94,7
#define Posd1     100,7
#define Poss10    63,21
#define Poss1     69,21

#define PosWeek   110, 2

#define Postxt    78,29



/////////////////여기부터 수정 금지 /////////////////////
String text;
uint8_t scrCnt = 0, scrPreCnt = 0;

U8G2_SED1520_122X32_F u8g2(U8G2_R2, 11, 3, 12, A0, A3, 8, 7, 4, /*rs=*/ 9, /*e1=*/ 10, /*e2=*/ 5, /* reset=*/  2);   // Set R/W to low!
u8g2_uint_t offset, width;
tmElements_t tm, gm;
time_t rtcTime, gpsTime, buttonTime, pGpsTime = 0;
TinyGPSPlus gps;

boolean wifiset = false;
uint16_t cds, brightness = 255, brightSave=255;

void setup()
{
  int i;
  for (i = 1; i < SCRLWIDTH; i++) text.concat(' ');
  text.concat(tname);

  pinMode(6, OUTPUT);
  analogWrite(6, brightness);
  delay(500);
  Serial.begin(9600);

  u8g2.begin();

  RTC.read(tm);
  buttonTime = makeTime(tm);
  if (tm.Year < 48) RTC.set(1524787200); //2018년 이전이면 2018.04.27.00:00으로 셋

  //u8g2.setDisplayRotation(U8G2_R2);

  //LCD TEST

  u8g2.firstPage();
  for (i = 0; i < 5; i++) {
    do {
      u8g2.drawXBMP(i * 24, 0, WidthBigNum, HeightBigNum , chs_num[i]);
    } while ( u8g2.nextPage());
  }
  delay(500);
  //u8g2.firstPage();
  for (i = 0; i < 5; i++) {
    do {
      u8g2.drawXBMP(i * 24, 0, WidthBigNum, HeightBigNum , chs_num[i + 5]);
    } while ( u8g2.nextPage());
  }
  delay(500);
  u8g2.firstPage();
  for (i = 0; i < 7; i++) {
    do {
      u8g2.drawXBMP(i * 17, 0, WidthWeek, HeightWeek , chs_week[i]);
    } while ( u8g2.nextPage());
  }
  for (i = 0; i < 10; i++) {
    do {
      u8g2.drawXBMP(i * 12, 16, WidthSmallNum, HeightSmallNum , chs_num_small[i]);
    } while ( u8g2.nextPage());
  }
  delay(500);

  //u8g2.firstPage();
  for (i = 0; i < u8g2.getDisplayHeight(); i++) {
    do {
      u8g2.drawLine(0, 0, u8g2.getDisplayWidth(), u8g2.getDisplayHeight() - 1 - i);
      u8g2.drawLine(0, i, u8g2.getDisplayWidth() - 1, u8g2.getDisplayHeight() - 1);
    } while ( u8g2.nextPage());
  }
  delay(500);
  u8g2.setFont(u8g2_font_6x13_tr); //작은글자
  u8g2.firstPage();
  do {
    u8g2.drawStr(19, 20, &tname[0]);
  } while ( u8g2.nextPage());
  delay(2000);
}


void loop() {
  rtcTime = RTC.get();
  breakTime(rtcTime, tm);

  while (Serial.available()) {
    char c;
    c = Serial.read();
    Serial.write(c);

    if (gps.encode(c)) {
      if (gps.time.isValid()) {
        gm.Year = CalendarYrToTm(gps.date.year());
        gm.Month = gps.date.month();
        gm.Day = gps.date.day();
        gm.Hour = gps.time.hour();
        gm.Minute = gps.time.minute();
        gm.Second = gps.time.second();
#ifdef USEWIFI
        if (gps.time.hour() == 88) {
          wifiset = true;
        }
        else {
          wifiset = false;
          gpsTime = makeTime(gm) + 9 * 60 * 60;
          rtcTime = gpsTime;
          pGpsTime = gpsTime;
          RTC.set(gpsTime);
        } //End of else
#endif
#ifdef USEGPS
        if ((gm.Hour != 0) || (gm.Minute != 0) || (gm.Second != 0)) { //9:00로 초기화 되는 현상 막기 위함
          gpsTime = makeTime(gm) + 9 * 60 * 60;
          if (gpsTime - pGpsTime > 1) {  //1초에 한번만 rtcSet
            rtcTime = gpsTime;
            pGpsTime = gpsTime;
            RTC.set(gpsTime);
          }
        }
#endif
      } //End of gps.time.valid()
    }
  } //End of While

  flash();
}
