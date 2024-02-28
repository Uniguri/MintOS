#ifndef MINTOS_RTC_H_
#define MINTOS_RTC_H_

#include "Types.h"

#define RTC_CMOS_ADDRESS 0x70
#define RTC_CMOS_DATA 0x71

#define RTC_ADDRESS_SECOND 0x00
#define RTC_ADDRESS_MINUTE 0x02
#define RTC_ADDRESS_HOUR 0x04
#define RTC_ADDRESS_DAYOFWEEK 0x06
#define RTC_ADDRESS_DAYOFMONTH 0x07
#define RTC_ADDRESS_MONTH 0x08
#define RTC_ADDRESS_YEAR 0x09

#define RTC_BCD_TO_BIN(x) ((((uint8)(x) >> 4) * 10) + ((x) & 0xF))

void kReadRTCTime(uint8* hour, uint8* minute, uint8* second);
void kReadRTCDate(uint16* year, uint8* month, uint8* day_of_month,
                  uint8* day_of_week);
const char* kConvertDayOfWeekToString(uint8 day_of_week);
#endif