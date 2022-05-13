#include "NTP_TIME.h"

NTP_TIME::NTP_TIME()
{
    alarm_on = false;
    interval_on = false;

    m_alarm_set = false;
    m_interval_set = false;

    m_daylight_offset = 7200;
    m_gmt_offset = 2;
    m_ntp_server = "time.google.com";

    m_alarm_index = 0;
    m_interval_index = 0;

    // Initialiase alarms
    for (int i = 0; i < PILL_SEGMENTS; i++)
    {
        alarm_time new_alarm_time{0,0};
        m_alarms[i] = new_alarm_time;
    }

    // Initialise interval alarms
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        interval_time new_interval{0,0,0};
        m_intervals[i] = new_interval;
        m_last_interval_time[i] = new_interval;
    }
}

NTP_TIME::~NTP_TIME()
{
    delete this;
}

void NTP_TIME::initialise()
{
    // Initiliase time settings from "time.h" library
    configTime(m_gmt_offset,m_daylight_offset,m_ntp_server.c_str());
}

void NTP_TIME::obtain_time()
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

bool NTP_TIME::get_time()
{
    // Get current time
    obtain_time();

    // Traverse through intervals
    for (int i = 0; i < SENSOR_COUNT; i++)
    {
        // Current time - set_interval_time = last interval time
        if(timeinfo.tm_hour - m_intervals[i].hour == m_last_interval_time[i].hour
        && timeinfo.tm_min - m_intervals[i].min == m_last_interval_time[i].min
        && timeinfo.tm_sec - m_intervals[i].sec == m_last_interval_time[i].sec)
        {
            // Store current time information for timing calculation reference
            m_last_interval_time[i].hour = timeinfo.tm_hour;
            m_last_interval_time[i].min = timeinfo.tm_min;
            m_last_interval_time[i].sec = timeinfo.tm_sec;

            // Set flag
            interval_on = true;
        }
    }

    //Serial.println("Finished A");   

    // Traverse through alarms
    for (int i = 0; i < PILL_SEGMENTS; i++)
    {
        // Check if time is alarm time
        if(timeinfo.tm_hour == m_alarms[i].hour && timeinfo.tm_min == m_alarms[i].min)
        {
            // Set flag
            alarm_on = true;

            // Reset array index
            m_alarms[i].hour = 0;
            m_alarms[i].min = 0;
        }   
    }

    // Check for interval or alarm
    if(interval_on || alarm_on)
    {
        return true;
    }
    else
    {
        // Something went wrong...
        return false;
    }
}

bool NTP_TIME::set_alarm(int hour, int minutes)
{
    // Check if array is full
    if(m_alarm_index < 13)
    {
        // Create new alarm time
        alarm_time new_alarm{hour,minutes};

        // Store alarm in next free alarm index
        m_alarms[m_alarm_index] = new_alarm;

        // Increment alarm index
        m_alarm_index++;

        return true;
    }
    
    // Array is full
    return false;
}

bool NTP_TIME::set_interval(int hour, int minutes, int seconds)
{
    // Check if array is full
    if(m_interval_index < 2)
    {
        // Get current time
        obtain_time();

        // Create a new interval
        interval_time new_interval{hour,minutes,seconds};

        // Store alarm in next free alarm index
        m_intervals[m_interval_index] = new_interval;

        // Store current time information for timing calculation reference
        m_last_interval_time[m_interval_index].hour = timeinfo.tm_hour;
        m_last_interval_time[m_interval_index].min = timeinfo.tm_min;
        m_last_interval_time[m_interval_index].sec = timeinfo.tm_sec;

        // Increment alarm index
        m_interval_index++;

        // Set flag
        m_interval_set = true;

        // Successful operation
        return true;
    }
    else
    {
        // Array is full
        return false;
    }
}