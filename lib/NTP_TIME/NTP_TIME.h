#include <Arduino.h>
#include <stdlib.h>
#include <string>
#include "time.h"

class NTP_TIME
{
    public:
        
        NTP_TIME();
        ~NTP_TIME();

        void get_time();
        void calculate_difference();
        void set_alarm(int hour, int minutes);

        bool alarm_on;

    private:

        struct tm timeinfo;
        
        std::string m_ntp_server;
        long m_gmt_offset;
        bool m_alarm_set;

        int m_daylight_offset;
        int m_alarm_hour;
        int m_alarm_minutes;
};