#include <esp8266-google-home-notifier.h>

class GOOGLE_HOME
{
    public:

        GOOGLE_HOME();
        ~GOOGLE_HOME();

        void initialise(std::string device_name, GoogleHomeNotifier notifier);
        void dispatch();

        bool is_connected();

    private:

        GoogleHomeNotifier m_notifier;

        std::string m_device_name;

        bool m_connected;
};