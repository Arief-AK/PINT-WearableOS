#include <WiFi.h>
#include <NTP_TIME.h>
#include <HR_SENSOR.h>

std::string SSID_NAME = "ETI1V.IC_DECO";
std::string SSID_PASSWORD = "Superboys123";

NTP_TIME GlobalTime;
//HR_SENSOR HeartSensor; // Uncomment if sensor is connected

float BPM;

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
  if(GlobalTime.set_alarm(config_hour,config_min))
  {
    Serial.println("\nAlarm set.");
    Serial.println("New alarm: " + String(config_hour) + ":" + String(config_min));
  }
  else
  {
    Serial.println("\nUnsuccefful operation.");
  }
}

void setup()
{
  // Setup serial
  Serial.begin(115200);

  // Request network credentials
  //request_network_credentials();

  // Setup WI-FI
  wifi_config();

  // Setup global system time
  GlobalTime.initialise();

  // **************************** DEBUGING PURPOSES **************************** //

  // pins
  pinMode(32,OUTPUT);

  // set timer interval every 1 min
  if(GlobalTime.set_interval(0,1,0)){
    Serial.println("Attempt to set for every 1 min");
  }

  // set timer interval every 3 min
  if(GlobalTime.set_interval(0,3,0)){
    Serial.println("Attempt to set for every 3 min");
  }

}

void loop()
{
  // Retrieve current time, check if alarm or interval occurred
  if(GlobalTime.get_time())
  {
    // Alarm or interval has occurred
    if(GlobalTime.interval_on)
    {
      // Interval has occurred
      GlobalTime.interval_on = false;

      // TODO: PERFORM CONTINOUS GATHERING OF SENSOR DATA

      // Get heart-rate (BPM)
      //BPM = HeartSensor.get_heart_rate();
      digitalWrite(32,HIGH);

      Serial.println("LED TURNED ON");
      delay(1000);
      digitalWrite(32,LOW);
      Serial.println("LED TURNED OFF");
    }
    else
    {
      // Alarm has occurred
      GlobalTime.alarm_on = false;
    }
  }

  // // Check if heart-rate is criticial
  // if(!HeartSensor.is_critical())
  // {
  //   // Heart-rate is normal
  //   Serial.println("BPM: ");
  //   Serial.println(BPM);
  // }
  // else
  // {
  //   // Heart-rate is abnormal
  //   Serial.println("SEND ALARM TO SERVER");
  // }
  
}