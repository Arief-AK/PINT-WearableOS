#include "NTP_TIME.h"

NTP_TIME::NTP_TIME()
{
    alarm_on = false;
    m_alarm_set = false;

    m_daylight_offset = 7200;
    m_gmt_offset = 2;
    m_ntp_server = "time.google.com";

    // Initiliase time settings from "time.h" library
    configTime(m_gmt_offset,m_daylight_offset,m_ntp_server.c_str());
}

NTP_TIME::~NTP_TIME()
{
    delete this;
}

void NTP_TIME::get_time()
{
    // Set time present flag
    bool time_present = false;

    // Attempt to get time
    while (!time_present)
    {
        if(!getLocalTime(&timeinfo))
        {
            Serial.println("Failed to obtain time");
            delay(1000);
        }
        else
        {
        time_present = true;
        }  
    }
}

void NTP_TIME::calculate_difference()
{
    // Get current time
    get_time();

    // Check if time is alarm time
    if(timeinfo.tm_hour == m_alarm_hour && timeinfo.tm_min == m_alarm_minutes)
    {
        // Set flag
        alarm_on = true;

        // Notify of alarm
        Serial.println("ALARM");
        delay(1000);
    }
}

void NTP_TIME::set_alarm(int hour, int minutes)
{
    m_alarm_hour = hour;
    m_alarm_minutes = minutes;
}