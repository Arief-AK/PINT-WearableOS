// INCLUDES
#include <WiFi.h>
#include <NTP_TIME.h>
#include <DATA_HANDLER.h>
#include <HR_SENSOR.h>
#include <GYRO_SENSOR.h>
#include <OLED_DISPLAY.h>

// For BPM formatting
#include <sstream>
#include <iomanip>

// DEFINITIONS
#define PATIENT_AGE 20
#define BPM_ABNORMAL_THRESHOLD 3

// TASK HANDLES
TaskHandle_t API_TASK;
TaskHandle_t DATA_TASK;
TaskHandle_t THRESHOLD_TASK;
TaskHandle_t DISPLAY_TASK;

// Needs this dependancy for full functionality
MAX30105 particleSensor;

// System flow objects
NTP_TIME GlobalTime;
DATA_HANDLER DHandler;

// Device Peripherals
HR_SENSOR HeartSensor;
GYRO_SENSOR GyroSensor;
OLED_DISPLAY Display;

// External dependencies
GOOGLE_HOME GHome;

// EXTERNAL BUTTONS
const int button_0 = 13;
const int button_1 = 15;

// Global variables
std::string SSID_NAME = "ETI1V.IC_DECO";
std::string SSID_PASSWORD = "Superboys123";

float BPM = 0.0f;
float prev_BPM = 0.0f;
int BPM_abnormal_count = 0;
bool setup_complete = false;
bool fall_detected = false;

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

std::string format_BPM()
{
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2) << prev_BPM;
  std::string BPM_message = "BPM:" + ss.str();

  return BPM_message;
}

// **************************** SENSOR OPERATING MULTICORE FUNCTIONS **************************** //

void GetSensors()
{
  for(;;)
  {
    //Serial.print("\nI am getting sensor data...");

    GlobalTime.get_time();

    // Show the time on OLED
    Display.print(GlobalTime.current_time_string, 92, 0, true);

    // Get heart-rate
    HeartSensor.calculate_heart_rate();
    BPM = HeartSensor.get_heart_rate();

    if(BPM != prev_BPM)
    {
      prev_BPM = BPM;
    }

    // Check if heart-rate is criticial
    if(!HeartSensor.is_critical())
    {
      // Display the BPM
      Display.print("3", 0, 24, false);
      Display.print(format_BPM(), 8, 24, false);
    }
    else
    {
      // Print BPM and message
      Display.print("3", 0, 24, false);
      Display.print(format_BPM(), 8, 24, false);
      Display.print("Abnormal heart-rate!", 0, 32, false);
    }

    // Get gyro-acceloremeter
    if(GyroSensor.fall_detected())
    {
      Serial.println("\n*************************************");
      Serial.println("Function: GetSensors");
      Serial.print("\nRunning on core:");
      Serial.print(xPortGetCoreID());

      fall_detected = true;

      // Notify patient fell
      Serial.println("FALL DETECTED");

      std::string critical_message = "chatID=1&message_type=Critical&message=PATIENT HAS FALLEN";
      auto send_message_response = DHandler.http_post_send_message(ENDPOINT_SEND_MESSAGE,critical_message.c_str());
      Serial.print("\nSend message response code:");
      Serial.print(send_message_response);
    }

    // Check for button press
  //   if(digitalRead(button_0) == HIGH)
  //   {
  //     // Send critical message
  //     std::string critical_message = "chatID=1&message_type=Critical&message=PATIENT HAS FALLEN";
  //     auto send_message_response = DHandler.http_post_send_message(ENDPOINT_SEND_MESSAGE,critical_message.c_str());
  //     Serial.print("\nSend message response code:");
  //     Serial.print(send_message_response);

  //     // Delay for human purposes
  //     delay(500);

  //   }

  //   Serial.print("\nButton [0]:");
  //   Serial.print(digitalRead(button_0));

  //   if(digitalRead(button_1) == HIGH)
  //   {
  //     // Inform patient of time via google home
  //     GHome.dispatch(GlobalTime.current_time_string);

  //     // Delay for human purposes
  //     delay(500);
  //   }

  //   Serial.print("\nButton [1]:");
  //   Serial.print(digitalRead(button_1));
  }
}

// **************************** DATA HANDLING MULTICORE OPERATIONS **************************** //

void retrieve_operation()
{
  for(;;)
  {
    // Peform POST request to get schedule list

    auto morning_time = DHandler.get_this_morning();
    auto tonight_time = DHandler.get_tonight();

    std::string body = "start=" + std::to_string(morning_time) + "&end=" + std::to_string(tonight_time);
    auto list_response_code = DHandler.http_post_list_alerts(ENDPOINT_SCHEDULE,body.c_str());
    Serial.print("\nList response code:");
    Serial.print(list_response_code);

    // Wait for 2 minutes until next alert list
    vTaskDelay(120000 / portTICK_PERIOD_MS);
  }
}

void check_threshold_alerts()
{
  for(;;)
  {
    Serial.print("\nI am checking for alert thresholds...");

    if(DHandler.upcoming_alert_present)
    {
      Serial.print("\nLooking out now...");
      auto current_time = DHandler.calculate_current_time();
      auto upcoming_vec_size = DHandler.upcoming_alerts_timestamps.size();

      if(upcoming_vec_size > 0)
      {
        Serial.print("\n****************** VECTOR ******************");

        for(auto element : DHandler.upcoming_alerts_timestamps)
        {
          Serial.print("\n");
          Serial.print(element);
        }

        for(auto timestamp : DHandler.upcoming_alerts_timestamps)
        {
          Serial.print("\nUpcoming timestamp:");
          Serial.print(timestamp);

          Serial.print("\nCurrrent timestamp:");
          Serial.print(current_time);

          if(timestamp <= current_time)
          {
            Serial.print("I am here.....");

            // Get iterator of the current timestamp
            auto iterator = DHandler.alerts.find(timestamp);

            if(iterator != DHandler.alerts.end())
            {
              Serial.print("Timestamp exists in the map");

              auto vec_size = iterator->second.size();
              auto read_time = iterator->first;
              auto id = iterator->second;

              for(auto element : id)
              {
                Serial.print("\nIterator timestamp:");
                Serial.print(read_time);

                Serial.print("\nVector size:");
                Serial.print(vec_size);

                // Peform POST request to get schedule list
                std::string body = "alertID=" + std::to_string(element);
                auto specific_alert_response_code = DHandler.http_post_get_specific_alert(ENDPOINT_SPECIFIC_ALERT_ID,body.c_str());
                Serial.print("\nSpecific alert response code:");
                Serial.print(specific_alert_response_code);
                
                if(specific_alert_response_code == 400)
                {
                  auto to_delete = DHandler.alerts.find(timestamp);
                  auto id_vector = to_delete->second;

                  auto flag = false;
                  auto index = 0;

                  while (!flag)
                  {
                    auto current_id = to_delete->second[index];
                    if(current_id == element)
                    {
                      // Remove alert that is not present in database
                      to_delete->second.erase(to_delete->second.begin() + index);
                      flag = true;
                    }
                  }
                }
              }

              // Remove first alert in the vector
              DHandler.upcoming_alerts_timestamps.erase(DHandler.upcoming_alerts_timestamps.begin());
            }
          }
        }

        auto new_size = DHandler.upcoming_alerts_timestamps.size();
        if(new_size == 0)
        {
          DHandler.upcoming_alert_present = false;
        }
      }
    }

    // Check for messages
    std::string body = "chatID=1";
    auto get_messages_response_code = DHandler.http_post_get_messages(ENDPOINT_GET_MESSAGES, body.c_str());
    Serial.print("\nGet messages response code:");
    Serial.print(get_messages_response_code);

    delay(30000);
  }
}

void display_features()
{
  for(;;)
  {
    // Get the time
    GlobalTime.get_time();
    
    // Show the time on OLED
    Display.print(GlobalTime.current_time_string, 92, 0, true);

    // Check if heart-rate is criticial
    if(!HeartSensor.is_critical())
    {
      // Display the BPM
      Display.print("3", 0, 24, false);
      Display.print(format_BPM(), 8, 24, false);
    }
    else
    {
      // Print BPM and message
      Display.print("3", 0, 24, false);
      Display.print(format_BPM(), 8, 24, false);
      Display.print("Abnormal heart-rate!", 0, 32, false);
    }

    if(fall_detected)    
    {
      // Notify on LCD
      Display.print("\nFALL DETECTED", 0, 24, true);

      // Display for 2.5 seconds
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// **************************** DATA OPERATIONS **************************** //

void handle_data_core_0_API(void *parameters)
{
  // Retrieve data from the API
  retrieve_operation();
}

void handle_data_core_0_sensors(void *parameters)
{
  GetSensors();
}

void handle_data_core_1_threshold(void *parameters)
{
  check_threshold_alerts();
}

void handle_data_core_1_printing(void *parameters)
{
  display_features();
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

  // Setup buttons
  // pinMode(button_0, INPUT_PULLUP);
  // pinMode(button_1, INPUT_PULLUP);

  // Setup global system time
  GlobalTime.initialise();

  // Setup HTTP data handler
  auto welcome_message = DHandler.initialise();

  // Setup heart-beat sensor
  HeartSensor.initialise(particleSensor, PATIENT_AGE);

  // Setup gyro-accelerometer
  GyroSensor.initialise();

  // Setup external google home device
  GHome.initialise("Kitchen speaker");

  // Setup OLED LCD display
  Display.display_setup();

  // Let google home notify patient
  GHome.dispatch(welcome_message);
  delay(500);

  // Display on the LCD
  Display.print(welcome_message,0,24,true);
  delay(500);

  // Assign tasks for core [0]
  xTaskCreatePinnedToCore(handle_data_core_0_API,"handle_data_core_0_API",105000,NULL,5,&API_TASK,0);
  xTaskCreatePinnedToCore(handle_data_core_0_sensors,"handle_data_core_0_sensors",10000,NULL,8,&DATA_TASK,0);
  
  // Assign tasks for core [1]
  xTaskCreatePinnedToCore(handle_data_core_1_threshold,"handle_data_core_1_threshold",10000,NULL,5,&THRESHOLD_TASK,1);
  xTaskCreatePinnedToCore(handle_data_core_1_printing,"handle_data_core_1_printing",10000,NULL,5,&DISPLAY_TASK,1);
  delay(500);
}

// **************************** SYSTEM MAIN LOOP **************************** //

void loop()
{}