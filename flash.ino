uint8_t weekday(tmElements_t *t)
/* Calculate day of week in proleptic Gregorian calendar. Sunday == 0. */
{
  uint8_t adjustment, mm, yy;
  adjustment = (14 - t->Month) / 12;
  mm = t->Month + 12 * adjustment - 2;
  yy = tmYearToCalendar(t->Year) - adjustment;
  return ((t->Day) + (13 * mm - 1) / 5 + yy + yy / 4 - yy / 100 + yy / 400 + 1) % 7;
}

void flash()
{
  uint16_t year;
  uint8_t i, month, day, hour, minute, second, week;
  uint16_t cds2;
  boolean syncFlash = false;
  boolean wifiFlash = false;
  String t_str;


  year = tmYearToCalendar(tm.Year);
  month = tm.Month;
  day = tm.Day;
  hour = tm.Hour;
  week = weekday(&tm);
  hour = hour % 12;
  if (hour == 0) hour = 12;
  minute = tm.Minute;
  second = tm.Second;
  syncFlash = ((rtcTime - gpsTime) <= SYNKDISPLAYTIME) || (second % 2);
  wifiFlash = (second % 2 == 0) && wifiset;

  u8g2.firstPage();
  do {
    if (hour / 10 == 1) u8g2.drawXBMP(PosH1, WidthBigNum1, HeightBigNum , chs_num10);
    else u8g2.drawXBMP(PosH1, WidthBigNum1, HeightBigNum , chs_space);
    u8g2.drawXBMP(PosH, WidthBigNum, HeightBigNum , chs_num[hour % 10]);
    if (syncFlash) u8g2.drawXBMP(PosColon, WidthBigColon, HeightBigNum , chs_colon);
    else u8g2.drawXBMP(PosColon, WidthBigColon, HeightBigNum , chs_space);
    u8g2.drawXBMP(PosM10, WidthBigNum, HeightBigNum , chs_num[minute / 10]);
    u8g2.drawXBMP(PosM1, WidthBigNum, HeightBigNum , chs_num[minute % 10]);

    u8g2.drawXBMP(Posy10, WidthSmallNum, HeightSmallNum, chs_num_small[year / 10 % 10]);
    u8g2.drawXBMP(Posy1, WidthSmallNum, HeightSmallNum, chs_num_small[year % 10]);
    u8g2.drawXBMP(Posydot, WidthSmalldot, HeightSmallNum, chs_dot_small);
    u8g2.drawXBMP(Posm10, WidthSmallNum, HeightSmallNum, chs_num_small[month / 10 % 10]);
    u8g2.drawXBMP(Posm1, WidthSmallNum, HeightSmallNum, chs_num_small[month  % 10]);
    u8g2.drawXBMP(Posmdot, WidthSmalldot, HeightSmallNum, chs_dot_small);
    u8g2.drawXBMP(Posd10, WidthSmallNum, HeightSmallNum, chs_num_small[day / 10 % 10]);
    u8g2.drawXBMP(Posd1, WidthSmallNum, HeightSmallNum, chs_num_small[day % 10]);

    u8g2.drawXBMP(Poss10, WidthSmallNum, HeightSmallNum, chs_num_small[second / 10 % 10]);
    u8g2.drawXBMP(Poss1, WidthSmallNum, HeightSmallNum, chs_num_small[second  % 10]);

    if (wifiFlash) u8g2.drawXBMP(PosWeek, WidthWeek, HeightWeek, chs_space);
    else u8g2.drawXBMP(PosWeek, WidthWeek, HeightWeek, chs_week[week]);




    i = scrCnt + SCRLWIDTH;
    if (i > text.length()) i = text.length();
    t_str = text.substring(scrCnt, i);
    u8g2.drawStr(Postxt, &t_str[0]);
  } while ( u8g2.nextPage());

  scrPreCnt++;
  if (scrPreCnt > SCRLSPEED) {
    scrCnt++;
    scrPreCnt = 0;
    cds = analogRead(2);
    cds2 = cds;
    if (cds < 600) cds2 = 600;
    if (cds > 900) cds2 = 900;
    brightness = map(cds2, 600, 900, 255, 51);
  }

  if (brightSave < brightness - BRIGHT_INTERVAL) brightSave += BRIGHT_INTERVAL;
  if (brightSave > brightness + BRIGHT_INTERVAL) brightSave -= BRIGHT_INTERVAL;
  if (brightSave < brightness) brightSave++;
  if (brightSave > brightness) brightSave--;

  analogWrite(6, brightSave);

  //Serial.print(brightSave);
  //Serial.print(" ");
  //Serial.println(brightness);

  if (scrCnt >=  text.length()) scrCnt = 0;
} //End of flash()
