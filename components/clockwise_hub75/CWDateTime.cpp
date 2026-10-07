#include "CWDateTime.h"
#include "esphome/core/application.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/core/log.h"
#include <string>

using namespace esphome;
using namespace esphome::time;

static const char *const TAG = "CWDateTime";

void CWDateTime::set_rtc(esphome::time::RealTimeClock *rtc) {
  rtc_ = rtc;
  if (rtc_) {
    ESP_LOGI(TAG, "RTC pointer linked: %p (source determined by selector)", rtc_);
  } else {
    ESP_LOGW(TAG, "RTC pointer cleared or invalid!");
  }
}

void CWDateTime::begin() {
  ESP_LOGI(TAG, "CWDateTime initialized. Waiting for RTC to be assigned via YAML.");
}

std::string format_ez_time(const esphome::ESPTime &tm, const std::string &format) {
    if (!tm.is_valid()) return "";

    std::string out = "";
    bool escape_char = false;

    auto is_leap = [](int y) { return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0); };
    auto zeropad = [](int num, int len) {
        std::string s = std::to_string(num);
        while (s.length() < len) s = "0" + s;
        return s;
    };

    const char* monthDays[] = {"31","28","31","30","31","30","31","31","30","31","30","31"};
    const char* dayShort[] = {"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"}; // ESPTime wday 1=Sun
    const char* dayFull[] = {"", "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    const char* monthShort[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const char* monthFull[] = {"", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

    int hour12 = tm.hour % 12;
    if (hour12 == 0) hour12 = 12;

    for (char c : format) {
        if (escape_char) {
            out += c;
            escape_char = false;
        } else {
            switch (c) {
                case '\\': case '~': escape_char = true; break;
                case 'd': out += zeropad(tm.day_of_month, 2); break;
                case 'D': out += dayShort[tm.day_of_week]; break; 
                case 'j': out += std::to_string(tm.day_of_month); break;
                case 'l': out += dayFull[tm.day_of_week]; break;
                case 'N': out += std::to_string(tm.day_of_week == 1 ? 7 : tm.day_of_week - 1); break;
                case 'S': {
                    int d = tm.day_of_month;
                    if (d == 1 || d == 21 || d == 31) out += "st";
                    else if (d == 2 || d == 22) out += "nd";
                    else if (d == 3 || d == 23) out += "rd";
                    else out += "th";
                    break;
                }
                case 'w': out += std::to_string(tm.day_of_week - 1); break;
                case 'F': out += monthFull[tm.month]; break;
                case 'm': out += zeropad(tm.month, 2); break;
                case 'M': out += monthShort[tm.month]; break;
                case 'n': out += std::to_string(tm.month); break;
                case 't': out += (tm.month == 2 && is_leap(tm.year)) ? "29" : monthDays[tm.month - 1]; break;
                case 'Y': out += std::to_string(tm.year); break;
                case 'y': out += zeropad(tm.year % 100, 2); break;
                case 'a': out += (tm.hour < 12) ? "am" : "pm"; break;
                case 'A': out += (tm.hour < 12) ? "AM" : "PM"; break;
                case 'g': out += std::to_string(hour12); break;
                case 'G': out += std::to_string(tm.hour); break;
                case 'h': out += zeropad(hour12, 2); break;
                case 'H': out += zeropad(tm.hour, 2); break;
                case 'i': out += zeropad(tm.minute, 2); break;
                case 's': out += zeropad(tm.second, 2); break;
                case 'z': out += std::to_string(tm.day_of_year - 1); break; 
                default: out += c; break;
            }
        }
    }
    return out;
}

String CWDateTime::getFormattedTime() {
  if (!rtc_) return "00:00:00";

  esphome::ESPTime t = rtc_->now();
  if (!t.is_valid()) return "00:00:00";

  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", t.hour, t.minute, t.second);
  return String(buf);
}

String CWDateTime::getFormattedTime(const char *format) {
  
  return format_ez_time(rtc_->now(), format);
  //return getFormattedTime();
}

int CWDateTime::getHour() {
  if (!rtc_) return 0;
  esphome::ESPTime t = rtc_->now();
  return t.is_valid() ? t.hour : 0;
}

int CWDateTime::getMinute() {
  if (!rtc_) return 0;
  esphome::ESPTime t = rtc_->now();
  return t.is_valid() ? t.minute : 0;
}

int CWDateTime::getSecond() {
  if (!rtc_) return 0;
  esphome::ESPTime t = rtc_->now();
  return t.is_valid() ? t.second : 0;
}

int CWDateTime::getDay() {
  if (!rtc_) return 1;
  esphome::ESPTime t = rtc_->now();
  return t.is_valid() ? t.day_of_month : 1;
}

int CWDateTime::getMonth() {
  if (!rtc_) return 1;
  esphome::ESPTime t = rtc_->now();
  return t.is_valid() ? t.month : 1;
}

int CWDateTime::getWeekday() {
  if (!rtc_) return 1;
  esphome::ESPTime t = rtc_->now();
  if (!t.is_valid()) return 1;

  // Compute weekday (1=Monday..7=Sunday)
  int q = t.day_of_month;
  int m = t.month < 3 ? t.month + 12 : t.month;
  int K = (t.year % 100) - (t.month < 3 ? 1 : 0);
  int J = (t.year / 100);
  int h = (q + 13*(m+1)/5 + K + K/4 + J/4 + 5*J) % 7;
  int d = ((h + 5) % 7) + 1;
  return d;
}

long CWDateTime::getMilliseconds() 
{
  return 0;
}

char *CWDateTime::getHour(const char *format) {
  static char buffer[3] = {'\0'};
  if (!rtc_) { strncpy(buffer, "00", sizeof(buffer)); return buffer; }

  esphome::ESPTime t = rtc_->now();
  if (!t.is_valid()) { strncpy(buffer, "00", sizeof(buffer)); return buffer; }

  int hour = t.hour;
  if (!use24hFormat_) {
    // convert 0..23 -> 12-hour 1..12
    hour = hour % 12;
    if (hour == 0) hour = 12;
  }
  snprintf(buffer, sizeof(buffer), "%02d", hour);
  return buffer;
}

char *CWDateTime::getMinute(const char *format) {
  static char buffer[3] = {'\0'};
  if (!rtc_) { strncpy(buffer, "00", sizeof(buffer)); return buffer; }

  esphome::ESPTime t = rtc_->now();
  if (!t.is_valid()) { strncpy(buffer, "00", sizeof(buffer)); return buffer; }

  int minute = t.minute;
  bool nozero = (format && strstr(format, "nozero") != nullptr);
  if (nozero) {
    snprintf(buffer, sizeof(buffer), "%d", minute);
  } else {
    snprintf(buffer, sizeof(buffer), "%02d", minute);
  }
  return buffer;
}

bool CWDateTime::isAM() {
  return getHour() < 12;
}

bool CWDateTime::is24hFormat() {
  return use24hFormat_;
}














