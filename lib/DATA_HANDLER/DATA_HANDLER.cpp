#include <DAHA_HANDLER.h>

DATA_HANDLER::DATA_HANDLER()
{
    m_base_server_name = "http://test.cyberca.re/api/";
    m_time_server_name = "time.google.com";
}

DATA_HANDLER::~DATA_HANDLER()
{
    delete this;
}

void DATA_HANDLER::initialise()
{
    // Check if network connection is established
    if(WiFi.status() == WL_CONNECTED)
    {
        // Connected, ready to go
        Serial.println("ESP32 ready to perform POST and GET requests");
    }
    else
    {
        // Not connected, notify user
        Serial.println("ESP32 is not connected to the internet");
    }
}

bool DATA_HANDLER::HTTP_POST(std::string url, std::string request_body)
{
    // Begin the connection
    m_client.begin(url.c_str());

    // Add appropriate headers
    m_client.addHeader("Content-Type","application/x-www-form-urlencoded");
    m_client.addHeader("Authorization","Bearer 495831"); // Modify after authorisation

    // Perform the POST request
    int http_response_code = m_client.POST(request_body.c_str());

    // Perform response check
    if(http_response_code > 0)
    {
      Serial.println("HTTP response code:");
      Serial.print(http_response_code);
      
      // Retrieve payload
      auto payload = m_client.getString();

      Serial.println(payload.c_str());
    }
    else
    {
      Serial.println("Error code:");
      Serial.print(http_response_code);
    }

    // Peform housekeeping on HTTP client
    m_client.end();

    // Return transmission result
    return http_response_code == 220? true:false;
}

bool DATA_HANDLER::HTTP_GET(std::string url)
{
    // Begin the connection
    m_client.begin(url.c_str());

    // Perform the GET request
    int http_response_code = m_client.GET();

    // Perform response check
    if(http_response_code > 0)
    {
      Serial.println("HTTP response code:");
      Serial.print(http_response_code);
      
      // Retrieve payload
      auto payload = m_client.getString();

      Serial.println(payload.c_str());
    }
    else
    {
      Serial.println("Error code:");
      Serial.print(http_response_code);
    }

    // Peform housekeeping
    m_client.end();
}

void DATA_HANDLER::parse_alerts()
{
    // TBA
}

unsigned long DATA_HANDLER::calculate_unix_time()
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}