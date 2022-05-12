#include <WiFi.h>

#include <NTP_TIME.h>
#include <HR_SENSOR.h>

std::string SSID_NAME;
std::string SSID_PASSWORD;

NTP_TIME GlobalTime;
HR_SENSOR HeartSensor;

// TODO: FIND CREDENTIALS FROM SERVER OR TECHNICIAN
void request_network_credentials()
{
  // TBA
}

void wifi_config()
{
  // Get network credentials
  request_network_credentials();

  // Notify attempt to connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(SSID_NAME.c_str());

  // Attempt to connet to Wi-Fi network
  WiFi.begin(SSID_NAME.c_str(), SSID_PASSWORD.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Notify successful connection
  Serial.println("");
  Serial.println("WiFi connected.");
}

void global_time_set_alarm()
{
  // Reset the configurations
  int config_hour,config_min = 0;

  // Prompt user for alarm configuration
  Serial.println("Wake Up Alarm Configuration:");
  Serial.println("Hour: ");

  // Wait for user input
  while (Serial.available() == 0);
  config_hour = Serial.readStringUntil('\n').toInt();
  Serial.print(config_hour);

  Serial.println("\nMinutes: ");
  while (Serial.available() == 0);
  config_min = Serial.readStringUntil('\n').toInt();
  Serial.print(config_min);

  // Set alarm with system time handler
  GlobalTime.set_alarm(config_hour,config_min);

  Serial.println("\nAlarm set.");
  Serial.println("New alarm: " + String(config_hour) + ":" + String(config_min));
}

void setup()
{
  // Setup serial
  Serial.begin(115200);

  // Setup WI-FI
  wifi_config();

  // Setup system time
  global_time_config();

}

void loop()
{
  
  // Calculate time
  GlobalTime.calculate_difference();
  
}