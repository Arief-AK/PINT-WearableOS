// INCLUDES
#include <WiFi.h>
#include <NTP_TIME.h>
#include <HR_SENSOR.h>

// DEFINITIONS
#define PATIENT_AGE 60
#define BPM_ABNORMAL_THRESHOLD 3

// Needs this dependancy for full functionality
MAX30105 particleSensor;

// System flow objects
NTP_TIME GlobalTime;

// Device Peripherals
HR_SENSOR HeartSensor;

// External dependencies

// Global variables
std::string SSID_NAME = "ETI1V.IC_DECO";
std::string SSID_PASSWORD = "Superboys123";

float BPM = 0.0f;
int BPM_abnormal_count = 0;

// **************************** SETUP FUNCTIONS **************************** //

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

// **************************** SYSTEM SETUP FUNCTION **************************** //

void setup()
{
  // Setup serial
  Serial.begin(115200);

  // Request network credentials
  request_network_credentials();

  // Setup WI-FI
  wifi_config();

  // Setup heart-beat sensor
  HeartSensor.initialise(particleSensor, PATIENT_AGE);

  // Setup global system time
  GlobalTime.initialise();

  // **************************** DEBUGING PURPOSES **************************** //

  // pins
  pinMode(32,OUTPUT);

  // set timer interval every 1 min
  if(GlobalTime.set_interval(0,1,0)){
    Serial.println("Attempt to set for every 1 min");
  }
}

// **************************** SYSTEM MAIN LOOP **************************** //

void loop()
{
  // Retrieve heartbeat sensor data
  HeartSensor.calculate_heart_rate();

  // Retrieve current time, check if alarm or interval occurred
  if(GlobalTime.get_time())
  {
    // Alarm or interval has occurred
    if(GlobalTime.interval_on)
    {
      // Interval has occurred
      GlobalTime.interval_on = false;

      // Get heart-rate (BPM)
      BPM = HeartSensor.get_heart_rate();
      digitalWrite(32,HIGH);

      Serial.println("LED TURNED ON");
      delay(1000);
      digitalWrite(32,LOW);
      Serial.println("LED TURNED OFF");

      // Check if heart-rate is criticial
      if(!HeartSensor.is_critical())
      {
        // Heart-rate is normal
        Serial.println("BPM: ");
        Serial.println(BPM);
      }
      else
      {
        // Heart-rate is abnormal
        Serial.println("Abnormal count increase");
        BPM_abnormal_count++;
      }
    }
    else
    {
      // Alarm has occurred
      GlobalTime.alarm_on = false;
    }
  }

  // Check if the heart-rate has been abnormal consecutively
  if(BPM_abnormal_count == BPM_ABNORMAL_THRESHOLD)
  {
    // Heart-beat is critical
    Serial.println("HEART-RATE CRITICAL, SEND ALARM TO SERVER");
    
    // Reset count
    BPM_abnormal_count = 0;
  }
}