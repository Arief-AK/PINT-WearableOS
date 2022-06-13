#include "GOOGLE_HOME.h"

GOOGLE_HOME::GOOGLE_HOME()
{
    m_device_name = "UNDEFINED";
    m_connected = false;
}

GOOGLE_HOME::~GOOGLE_HOME()
{
    delete this;
}

void GOOGLE_HOME::initialise(std::string device_name)
{
    GoogleHomeNotifier notifier;

    // Assign member variables
    m_device_name = device_name;
    m_notifier = notifier;

    // When device is not connected
    while (!m_connected)
    {
        // Attempt to connect with google home
        Serial.println("connecting to Google Home...");

        if(m_notifier.device(m_device_name.c_str(),"en-uk") != true)
        {
            Serial.println(m_notifier.getLastError());
            Serial.println("Retrying...");
        }
        else
        {
            m_connected = true;
        }
    }

    // Notify that the google home device is found
    Serial.print("found Google Home device.");

    // Provide audio response from google home device
    m_notifier.notify("Hello, google home device is connected.");
}

void GOOGLE_HOME::dispatch(std::string message)
{
    // Attempt to dispatch message
    try
    {
        m_notifier.notify(message.c_str());
    }
    catch(const std::exception& e)
    {
        Serial.println(m_notifier.getLastError());
    }
}

bool GOOGLE_HOME::is_connected(){
    return m_connected;
}