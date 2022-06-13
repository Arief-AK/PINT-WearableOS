#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>
#include <WiFi.h>
#include <time.h>
#include <ctime>
#include <map>

#include <GOOGLE_HOME.h>

#define ALERT_THRESHOLD 5
#define UPCOMING_ALERT_THRESHOLD 4

// API ENDPOINTS
#define ENDPOINT_LOGIN "http://test.cyberca.re/api/login"
#define ENDPOINT_VALIDATE "http://test.cyberca.re/api/validate"
#define ENDPOINT_SCHEDULE "http://test.cyberca.re/api/schedule/list"
#define ENDPOINT_NEW_ALERT "http://test.cyberca.re/api/schedule/newItem"
#define ENDPOINT_SENSOR_UPDATE "http://test.cyberca.re/api/sensors/update"
#define ENDPOINT_SPECIFIC_ALERT_ID "http://test.cyberca.re/api/schedule/alert"
#define ENDPOINT_DISMISS_ITEM "http://test.cyberca.re/api/schedule/dismissItem"
#define ENDPOINT_DELETE_ALERT "http://test.cyberca.re/api/schedule/deleteItem"
#define ENDPOINT_SEND_MESSAGE "http://test.cyberca.re/api/chat/sendMessage"
#define ENDPOINT_GET_MESSAGES "http://test.cyberca.re/api/chat/getMessages"

struct alert
{
  int userID;
  int med_id = 0;
  int slot_number;
  unsigned long timestamp = 0;
  unsigned long duration = 0;
  bool alert_dismissed = false;
  std::string description = "";
};

class DATA_HANDLER
{
  public:

      DATA_HANDLER();
      ~DATA_HANDLER();

      std::string initialise();

      int http_post_list_alerts(std::string url, std::string req_body);
      int http_post_get_specific_alert(std::string url, std::string req_body);
      int http_post_get_messages(std::string url, std::string req_body);
      int http_post_send_message(std::string url, std::string req_body);

      // HELPER FUNCTIONS
      int get_this_morning();
      int get_tonight();
      int calculate_current_time();
      int calculate_threshold_time();

      // FLAGS
      bool logged_in = false;
      bool upcoming_alert_present = false;

      // VECTOR TO STORE UPCOMING ALERTS WITHIN THRESHOLD
      std::vector<int> upcoming_alerts_timestamps;

      // MAP TO STORE ALERTS
      std::map<int,std::vector<int>> alerts;
      
  private:

      // HTTP POST
      int http_post_login(std::string url, std::string req_body);
      int http_post_validate(std::string url, std::string req_body);
      int http_post_new_alert(std::string url, std::string req_body);
      int http_post_delete_alert(std::string url, std::string req_body);
      int http_post_dismiss_item(std::string url, std::string req_body);
      int http_post_data_batch(std::string url, std::string req_body);

      // PARSING FUNCTIONS
      void parse_credentials(std::string payload);
      void parse_alerts(std::string payload);
      void parse_specific_alert(std::string payload);
      void get_message_id(std::string payload);
      void parse_message(std::string payload);
      void delete_map_element(const int key);

      // SENDING FUNCTIONS
      std::string create_json_data(int device_id, unsigned long timestamp, bool critical_flag, double bpm, std::string activity);

      // HELPER FUNCTIONS
      std::string get_welcome_message();

      // PRINT FUNCTIONS
      void print_alert(alert current_alert);
      void print_map();

      // LAST MESSAGE ID
      int last_message_id = -1;

      // SESSION KEYS AND BODY
      std::string session_key;
      std::string credentials;
      std::string patient_name;

      // INSTANCE OF GOOGLE HOME
      GOOGLE_HOME GHome;
};