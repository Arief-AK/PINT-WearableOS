// INCLUDES
#include <WiFi.h>
#include <NTP_TIME.h>
#include <DATA_HANDLER.h>
#include <HR_SENSOR.h>
#include <GYRO_SENSOR.h>
#include <OLED_DISPLAY.h>
#include <GOOGLE_HOME.h>

// For BPM formatting
#include <sstream>
#include <iomanip>

// DEFINITIONS
#define PATIENT_AGE 20
#define BPM_ABNORMAL_THRESHOLD 3

// Needs this dependancy for full functionality
MAX30105 particleSensor;
// GoogleHomeNotifier notifier;

// System flow objects
NTP_TIME GlobalTime;

// Device Peripherals
HR_SENSOR HeartSensor;
GYRO_SENSOR GyroSensor;
OLED_DISPLAY Display;

// External dependencies
GOOGLE_HOME GHome;

// Global variables
std::string SSID_NAME = "Arief-WIFI";
std::string SSID_PASSWORD = "helohelo";
std::string welcome_message = "Hello, Gertrude";

float BPM = 0.0f;
int BPM_abnormal_count = 0;
bool setup_complete = false;

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

std::string format_BPM()
{
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2) << BPM;
  std::string BPM_message = "BPM:" + ss.str();

  return BPM_message;
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

  // Setup global system time
  GlobalTime.initialise();

  // Setup heart-beat sensor
  HeartSensor.initialise(particleSensor, PATIENT_AGE);

  // Setup gyro-accelerometer
  GyroSensor.initialise();

  // Setup external google home device
  // GHome.initialise("Google Nest", notifier);

  // Setup OLED LCD display
  Display.display_setup();

  Display.print(welcome_message,0,24,true);
  delay(500);

  // **************************** DEBUGING PURPOSES **************************** //

  // pins
  pinMode(32,OUTPUT);

  // set timer interval every half min
  if(GlobalTime.set_interval(0,1,0)){
    Serial.println("Attempt to set interval for every minute");
  }
}

// **************************** SENSOR OPERATING FUNCTIONS **************************** //

void handle_heart_rate()
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

      // Show the time on OLED
      Display.print(GlobalTime.current_time_string, 92, 0, true);

      // Check if heart-rate is criticial
      if(!HeartSensor.is_critical())
      {
        // Heart-rate is normal
        Serial.println("BPM: ");
        Serial.println(BPM);
        
        // Display the BPM
        Display.print("3", 0, 24, false);
        Display.print(format_BPM(), 8, 24, false);
      }
      else
      {
        // Heart-rate is abnormal
        Serial.println("BPM: ");
        Serial.println(BPM);

        // Heart-rate is abnormal
        Serial.println("Abnormal count increase");

        // Print BPM and message
        Display.print("3", 0, 24, false);
        Display.print(format_BPM(), 8, 24, false);
        Display.print("Abnormal heart-rate!", 0, 32, false);

        // Perform housekeeping
        BPM_abnormal_count++;
      }
    }
    else
    {
      // Alarm has occurred
      GlobalTime.alarm_on = false;
    }
  }
  else
  {
      if(!setup_complete)
      {
          // On first-boot
          Display.print("Startup complete",0,24,true);
          delay(150);

          // Show the time on OLED
          Display.print(GlobalTime.current_time_string, 92, 0, true);
          setup_complete = true;
      }
      else
      {
        // Show the time on OLED
        Display.print(GlobalTime.current_time_string, 92, 0, false);
      }
  }

  // Check if the heart-rate has been abnormal consecutively
  if(BPM_abnormal_count == BPM_ABNORMAL_THRESHOLD)
  {
    // Heart-beat is critical
    Serial.println("HEART-RATE CRITICAL, SEND ALARM TO SERVER");

    // Print message
    Display.print("HEART-RATE CRITICAL!", 0, 40, false);
    
    // Reset count
    BPM_abnormal_count = 0;
  }

}

void handle_fall_detection()
{
  // Retrieve gyro-accelerometer sensor data
  if(GyroSensor.fall_detected())
  {
    // Notify patient fell
    Serial.println("FALL DETECTED");

    // Show the time on OLED
    Display.print(GlobalTime.current_time_string, 92, 0, true);
    
    // Notify on LCD
    Display.print("FALL DETECTED", 0, 24, false);
  }
}

void handle_data()
{
  // TBA
}

// **************************** SYSTEM MAIN LOOP **************************** //

void loop()
{
    // Check patient heart-rate
    handle_heart_rate();

    // Perform fall detection
    handle_fall_detection();

    // Perform all necessary data operations
    handle_data();
}