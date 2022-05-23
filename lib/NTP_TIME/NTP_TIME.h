#include <Arduino.h>
#include <string>
#include "time.h"

#define PILL_SEGMENTS 14
#define SENSOR_COUNT 3

struct alarm_time
{
    int hour;
    int min;
};

struct interval_time
{
    int hour;
    int min;
    int sec;
};

class NTP_TIME
{
    public:
        
        NTP_TIME();
        ~NTP_TIME();

        void initialise();

        bool get_time();

        bool set_alarm(int hour, int minutes);
        bool set_interval(int hour, int minutes, int seconds);

        bool alarm_on;
        bool interval_on;

        std::string current_time_string;

    private:

        void obtain_time();

        struct tm timeinfo;

        alarm_time m_alarms[PILL_SEGMENTS];

        interval_time m_intervals[SENSOR_COUNT];
        interval_time m_last_interval_time[SENSOR_COUNT];

        std::string m_ntp_server;
        
        long m_gmt_offset;

        bool m_alarm_set;
        bool m_interval_set;

        int m_daylight_offset;
        int m_alarm_index;
        int m_interval_index;
};