#include <WiFi.h>
#include <HTTPClient.h>
#include <chrono>

#define ALERT_LIMIT 14

struct alert
{
    int hour;
    int min;

    std::string description;
};

class DATA_HANDLER
{
    public:

        DATA_HANDLER();
        ~DATA_HANDLER();

        void initialise();

        bool HTTP_POST(std::string url, std::string request_body);
        bool HTTP_GET(std::string url);

    private:

        void parse_alerts();
        unsigned long calculate_unix_time();

        std::string m_time_server_name;
        std::string m_base_server_name;

        HTTPClient m_client;

        alert m_alerts[ALERT_LIMIT];
};