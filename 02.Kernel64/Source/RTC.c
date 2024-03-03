#include "RTC.h"

#include "Hardware.h"
#include "Types.h"

#define GET_DATA_AND_CONVERT_BIN(type)                  \
  ({                                                    \
    kSetPortByte(RTC_CMOS_ADDRESS, RTC_ADDRESS_##type); \
    uint8 data = kGetPortByte(RTC_CMOS_DATA);           \
    RTC_BCD_TO_BIN(data);                               \
  })
void kReadRTCTime(uint8* hour, uint8* minute, uint8* second) {
  *hour = GET_DATA_AND_CONVERT_BIN(HOUR);
  *minute = GET_DATA_AND_CONVERT_BIN(MINUTE);
  *second = GET_DATA_AND_CONVERT_BIN(SECOND);
}

void kReadRTCDate(uint16* year, uint8* month, uint8* day_of_month,
                  uint8* day_of_week) {
  uint8 data;

  data = GET_DATA_AND_CONVERT_BIN(YEAR);
  *year = RTC_BCD_TO_BIN(data) + 2000;

  *month = GET_DATA_AND_CONVERT_BIN(MONTH);
  *day_of_month = GET_DATA_AND_CONVERT_BIN(DAYOFMONTH);
  *day_of_week = GET_DATA_AND_CONVERT_BIN(DAYOFWEEK);
}
#undef GET_DATA_AND_CONVERT_BIN

inline const char* kConvertDayOfWeekToString(uint8 day_of_week) {
  static const char* const day_of_week_string[8] = {
      "Error",     "Sunday",   "Monday", "Tuesday",
      "Wednesday", "Thursday", "Friday", "Saturday"};

  if (day_of_week >= 8) {
    return day_of_week_string[0];
  }
  return day_of_week_string[day_of_week];
}