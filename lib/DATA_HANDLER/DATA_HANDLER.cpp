#include "DATA_HANDLER.h"

////////////////////////// HELPER FUNCTIONS //////////////////////////

int DATA_HANDLER::get_this_morning()
{
  // Get current time
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }
  time(&now);

  timeinfo.tm_hour = 0;
  timeinfo.tm_min = 0;
  timeinfo.tm_sec = 0;

  auto unix_time = mktime(&timeinfo);

  return unix_time;
}

int DATA_HANDLER::get_tonight()
{
  // Get current time
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }
  time(&now);

  timeinfo.tm_hour = 23;
  timeinfo.tm_min = 59;
  timeinfo.tm_sec = 0;

  auto unix_time = mktime(&timeinfo);

  return unix_time;
}

std::string DATA_HANDLER::get_welcome_message()
{
  // Get current time
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }
  
  auto unix = time(&now);
  std::string status = "";

  if(timeinfo.tm_hour < 12)
  {
    status = "Good morning," + patient_name;
  }
  else if(timeinfo.tm_hour < 18)
  {
    status = "Good afternoon," + patient_name;
  }
  else if(timeinfo.tm_hour <= 23 && timeinfo.tm_min <= 59)
  {
    status = "Good evening," + patient_name;
  }

  // Return status
  return status;
}

int DATA_HANDLER::calculate_current_time()
{
  // Get current time
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }
  return time(&now);
}

int DATA_HANDLER::calculate_threshold_time()
{
  // Get current time
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }
  time(&now);
  
  Serial.print("\nCurrent timestamp:");
  Serial.print(now);

  timeinfo.tm_min += 10;
  auto threshold_time = mktime(&timeinfo);

  Serial.print("\nThreshold timestamp:");
  Serial.print(threshold_time);

  return threshold_time;
}

void DATA_HANDLER::delete_map_element(const int key)
{
  // Remove all elements with specified key value
  alerts.erase(alerts.begin(), alerts.find(key));
}

DATA_HANDLER::DATA_HANDLER()
{
    credentials = "";
    session_key = "";
    patient_name = "";
}

DATA_HANDLER::~DATA_HANDLER()
{
    delete this;
}

std::string DATA_HANDLER::initialise()
{
    // TEST: Initialise the GHome
    GHome.initialise("Kitchen speaker");

    std::string w_message = "Cybercare";

    // Assign credentials
    credentials = "username=alice&password=inW0nderland";

    // Perform POST request to login
    auto login_response = http_post_login(ENDPOINT_LOGIN,credentials);

    if(login_response == 200)
    {
        logged_in = true;
        w_message = get_welcome_message();
    }

    return w_message;
}

int DATA_HANDLER::http_post_list_alerts(std::string url, std::string req_body)
{
  int http_response_code = 0;

  const char* headers[] = {"Set-Cookie"};

  // Create an HTTP client
  HTTPClient client;

  // Begin the transmission
  client.begin(url.c_str());

  // Add appropriate headers
  client.addHeader("Content-Type","application/x-www-form-urlencoded");
  client.addHeader("Cookie", session_key.c_str());

  // Collect headers from the request
  client.collectHeaders(headers,sizeof(headers)/sizeof(headers[0]));

  // Perform the POST request
  http_response_code = client.POST(req_body.c_str());

  if(http_response_code == 200)
  {
    // Get all alerts from the schedule
    auto list_payload = client.getString();

    // Parse the alerts with minimal information from the payload
    parse_alerts(list_payload.c_str());
  }
  else
  {
    Serial.println("\nError code:");
    Serial.print(http_response_code);
  }

  // Perform housekeeping
  client.end();

  return http_response_code;
}

int DATA_HANDLER::http_post_get_specific_alert(std::string url, std::string req_body)
{
  // Create a response code
  int http_response_code = 0;

  const char* headers[] = {"Set-Cookie"};

  // Create an HTTP client
  HTTPClient client;

  // Begin the transmission
  client.begin(url.c_str());

  // Add appropriate headers
  client.addHeader("Content-Type","application/x-www-form-urlencoded");
  client.addHeader("Cookie", session_key.c_str());

  // Collect headers from the request
  client.collectHeaders(headers,sizeof(headers)/sizeof(headers[0]));

  // Perform the POST request
  http_response_code = client.POST(req_body.c_str());

  if(http_response_code == 200)
  {
    // Get the response payload
    auto payload = client.getString();
    
    // Parse with detailed information
    parse_specific_alert(payload.c_str());
  }

  // Return the response code
  return http_response_code;
}

int DATA_HANDLER::http_post_login(std::string url, std::string req_body)
{
  int http_response_code = 0;

  const char* headers[] = {"Set-Cookie"};

  // Check if WIFI is connected
  if(WiFi.status() == WL_CONNECTED)
  {
    // Create an HTTP client
    HTTPClient client;

    // Begin the transmission
    client.begin(url.c_str());

    // Add appropriate headers
    client.addHeader("Content-Type","application/x-www-form-urlencoded");

    // Collect headers from the request
    client.collectHeaders(headers,sizeof(headers)/sizeof(headers[0]));

    // Perform the POST request
    http_response_code = client.POST(req_body.c_str());

    // Perform response check
    if(http_response_code == 200)
    {
      Serial.println("HTTP response code:");
      Serial.print(http_response_code);
      Serial.print(" OK");
      
      // Get cookie
      auto cookie = client.header(client.headerName(0).c_str());
      
      // Initialise stream for parsing
      std::istringstream stream(cookie.c_str());

      // Parse the response header
      std::getline(stream,session_key,';');

      // Print the session label and value
      Serial.println(session_key.c_str());

      // Get payload
      auto payload = client.getString();

      // Parse the credentials
      parse_credentials(payload.c_str());

    }
    else
    {
      Serial.println("Error code:");
      Serial.print(http_response_code);
    }

    // Peform housekeeping
    client.end();
  }
  else
  {
    Serial.println("Lost wireless connection");
  }

  return http_response_code;
}

int DATA_HANDLER::http_post_validate(std::string url, std::string req_body)
{
  // Create a response code
  int http_response_code = 0;

  const char* headers[] = {"Set-Cookie"};

  // Create an HTTP client
  HTTPClient client;

  // Begin the transmission
  client.begin(url.c_str());

  // Add appropriate headers
  client.addHeader("Content-Type","application/x-www-form-urlencoded");
  client.addHeader("Cookie", session_key.c_str());

  // Collect headers from the request
  client.collectHeaders(headers,sizeof(headers)/sizeof(headers[0]));

  // Perform the POST request
  http_response_code = client.POST(req_body.c_str());

  if(http_response_code == 200)
  {
    Serial.println("\nConnection is validated");
  }
  else
  {
    Serial.println("\nError code:");
    Serial.print(http_response_code);
  }

  // Perform housekeeping
  client.end();

  return http_response_code;
}

int DATA_HANDLER::http_post_new_alert(std::string url, std::string req_body)
{
  // Create a response code
  int http_response_code = 0;

  const char* headers[] = {"Set-Cookie"};

  // Create an HTTP client
  HTTPClient client;

  // Begin the transmission
  client.begin(url.c_str());

  // Add appropriate headers
  client.addHeader("Content-Type","application/x-www-form-urlencoded");
  client.addHeader("Cookie", session_key.c_str());

  // Collect headers from the request
  client.collectHeaders(headers,sizeof(headers)/sizeof(headers[0]));

  // Perform the POST request
  http_response_code = client.POST(req_body.c_str());

  if(http_response_code == 200)
  {
    // Successfull request
    Serial.println("\nSuccessfully inserted a new alarm to the schedule");
  }
  else
  {
    Serial.println("\nError code:");
    Serial.print(http_response_code);
  }

  // Perform housekeeping
  client.end();

  return http_response_code;
}

int DATA_HANDLER::http_post_delete_alert(std::string url, std::string req_body)
{
  // Create a response code
  int http_response_code = 0;

  const char* headers[] = {"Set-Cookie"};

  // Create an HTTP client
  HTTPClient client;

  // Begin the transmission
  client.begin(url.c_str());

  // Add appropriate headers
  client.addHeader("Content-Type","application/x-www-form-urlencoded");
  client.addHeader("Cookie", session_key.c_str());

  // Collect headers from the request
  client.collectHeaders(headers,sizeof(headers)/sizeof(headers[0]));

  // Perform the POST request
  http_response_code = client.POST(req_body.c_str());

  if(http_response_code == 200)
  {
    // Successfull request
    Serial.println("\nSuccessfully deleted a new alarm to the schedule");

    // Print the response
    Serial.print(client.getString());
  }
  else
  {
    Serial.println("\nError code:");
    Serial.print(http_response_code);
  }

  // Perform housekeeping
  client.end();

  // Return the response code
  return http_response_code;
}

int DATA_HANDLER::http_post_dismiss_item(std::string url, std::string req_body)
{
  // Create a response code
  int http_response_code = 0;

  const char* headers[] = {"Set-Cookie"};

  // Create an HTTP client
  HTTPClient client;

  // Begin the transmission
  client.begin(url.c_str());

  // Add appropriate headers
  client.addHeader("Content-Type","application/x-www-form-urlencoded");
  client.addHeader("Cookie", session_key.c_str());

  // Collect headers from the request
  client.collectHeaders(headers,sizeof(headers)/sizeof(headers[0]));

  // Perform the POST request
  http_response_code = client.POST(req_body.c_str());

  if(http_response_code == 200)
  {
    // Get the response payload
    auto payload = client.getString();
    
    // Parse with detailed information
    parse_specific_alert(payload.c_str());
  }

  // Return the response code
  return http_response_code;
}

int DATA_HANDLER::http_post_data_batch(std::string url, std::string req_body)
{
  // Create response code
  int http_response_code = 0;

  const char* headers[] = {"Set-Cookie"};

  // Create an HTTP client
  HTTPClient client;

  // Begin the transmission
  client.begin(url.c_str());

  // Add appropriate headers: JSON and session cookie
  client.addHeader("Content-Type","application/json");
  client.addHeader("Cookie", session_key.c_str());

  // Collect headers from the request
  client.collectHeaders(headers,sizeof(headers)/sizeof(headers[0]));

  // Send the JSON via HTTP POST request
  http_response_code = client.POST(req_body.c_str());

  // Return the response code
  return http_response_code;
}

int DATA_HANDLER::http_post_send_message(std::string url, std::string req_body)
{
  // Create response code
  int http_response_code = 0;

  const char* headers[] = {"Set-Cookie"};

  // Create an HTTP client
  HTTPClient client;

  // Begin the transmission
  client.begin(url.c_str());

  // Add appropriate headers: JSON and session cookie
  client.addHeader("Content-Type","application/x-www-form-urlencoded");
  client.addHeader("Cookie", session_key.c_str());

  // Collect headers from the request
  client.collectHeaders(headers,sizeof(headers)/sizeof(headers[0]));

  // Send the JSON via HTTP POST request
  http_response_code = client.POST(req_body.c_str());

  if(http_response_code == 200)
  {
    // Get payload
    auto payload = client.getString();

    // Parse the message to find last message id
    get_message_id(payload.c_str());
  }

  // Return the response code
  return http_response_code;
}

int DATA_HANDLER::http_post_get_messages(std::string url, std::string req_body)
{
  // Create response code
  int http_response_code = 0;

  const char* headers[] = {"Set-Cookie"};

  // Create an HTTP client
  HTTPClient client;

  // Begin the transmission
  client.begin(url.c_str());

  // Add appropriate headers: JSON and session cookie
  client.addHeader("Content-Type","application/x-www-form-urlencoded");
  client.addHeader("Cookie", session_key.c_str());

  // Collect headers from the request
  client.collectHeaders(headers,sizeof(headers)/sizeof(headers[0]));

  // Send the JSON via HTTP POST request
  http_response_code = client.POST(req_body.c_str());

  if(http_response_code == 200)
  {
    // Get payload
    auto payload = client.getString();

    // Parse the message
    parse_message(payload.c_str());
  }

  // Return the response code
  return http_response_code;
}

// ******************************* PARSING FUNCTIONS *******************************

void DATA_HANDLER::parse_credentials(std::string payload)
{
  Serial.print(payload.c_str());

  // Create parameters for full payload
  const size_t capacity = (JSON_OBJECT_SIZE(3)+ 60);

  // Create a JSON document for full payload
  StaticJsonDocument<capacity> doc;

  // Attempt to deserialise JSON
  DeserializationError json_error = deserializeJson(doc, payload);

  // Check for error
  if(json_error)
  {
    Serial.print("\nFailed de-serializing.\n");
    Serial.print(json_error.c_str());
  }

  // Get the credentials from JSON
  auto name = doc["first_name"].as<std::string>();
  patient_name = name;

  // Get welcome message
  auto welcome_message = get_welcome_message();

  // Let google home say welcome message
  GHome.dispatch(welcome_message);
}

void DATA_HANDLER::parse_alerts(std::string payload)
{
  //Serial.println(payload.c_str());

  // Create parameters for full payload
  const size_t capacity = (JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(6) + 60) * 30;

  // Create a JSON document for full payload
  StaticJsonDocument<capacity> doc;

  // Attempt to deserialise JSON
  DeserializationError json_error = deserializeJson(doc, payload);

  // Check for error
  if(json_error)
  {
    Serial.print("\nFailed de-serializing.\n");
    Serial.print(json_error.c_str());
  }

  // Calculate the relevant times for upcoming alert measurements
  auto current_time = calculate_current_time();
  auto threshold_time = calculate_threshold_time();

  Serial.print("\nDocument size:");
  Serial.print(doc.size());

  // Deserialize the array of alerts
  auto array_element = doc["alerts"];

  // Print the size of the array of alerts
  Serial.print("\nSize of alert array:");
  Serial.print(array_element.size());

  for(int i = 0; i < array_element.size(); i++)
  { 
    auto timestamp = doc["alerts"][i]["timestamp"].as<int>();
    auto alert_id = doc["alerts"][i]["alert_id"].as<int>();
    auto dismissed = doc["alerts"][i]["alert_dismissed"].as<bool>();

    // Check for duplicates
    if(std::find(upcoming_alerts_timestamps.begin(), upcoming_alerts_timestamps.end(), timestamp) == upcoming_alerts_timestamps.end())
    {
      // Check if alert occurs within threshold time
      if(timestamp <= threshold_time && timestamp > current_time)
      {
        upcoming_alerts_timestamps.push_back(timestamp);
        upcoming_alert_present = true;
      }
    }

    // Check if the alert is dismissed
    if(dismissed == false)
    {
      // Attempt to find with specified key value
      auto iterator = alerts.find(timestamp);

      // Does not exists in the map
      if(iterator == alerts.end())
      {
        // Get the timestamp of the iterator
        auto itr_timestamp = iterator->first;

        // Timestamp is further than current time
        if(timestamp >= current_time)
        {
          // Make a new vector for alert_IDs, add to vector
          std::vector<int> alert_IDs = {alert_id};

          Serial.print("\nalertID:");
          Serial.print(alert_id);

          // Add pair to map
          alerts.emplace(timestamp,alert_IDs);
        }
      }
      else if(iterator != alerts.end())
      {
        // Flag to insert to vector
        bool exists = false;

        // Get size of current vector
        auto vec_size = iterator->second.size();

        // Traverse through array
        for (int i = 0; i < vec_size; i++)
        {
          auto element = iterator->second[i];

          if(alert_id == element)
          {
            exists = true;            
          }
        }

        Serial.print("\n");
        
        if(exists == false)
        {
          if(timestamp >= current_time)
          {
            Serial.print("\nInserting alertID:");
            Serial.print(alert_id);

            // Timestamp exists, push the alert id to the vector using iterator
            iterator->second.push_back(alert_id);
          }
        }
      }
    }
    else
    {
        // Alerts is already dismissed
        delete_map_element(timestamp);
    }
  }

  // Print the map
  print_map();
}

void DATA_HANDLER::parse_specific_alert(std::string payload)
{
  //Serial.println(payload.c_str());

  // Create parameters for full payload
  const size_t capacity = (JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(6) + 60) * ALERT_THRESHOLD;

  // Create a JSON document for full payload
  StaticJsonDocument<capacity> doc;

  // Attempt to deserialise JSON
  DeserializationError json_error = deserializeJson(doc, payload);

  // Check for error
  if(json_error)
  {
    Serial.print("\nFailed de-serializing.\n");
    Serial.print(json_error.c_str());
  }

  Serial.print("\nDocument size:");
  Serial.print(doc.size());

  // Deserialize the array of alerts
  auto array_element = doc["alerts"];

  // Create a new alert structure
  alert new_alert;

  // Assign values to alert structure
  new_alert.userID = doc["userID"].as<int>();
  new_alert.timestamp = doc["timestamp"].as<int>();
  new_alert.duration = doc["duration"].as<int>();
  new_alert.med_id = doc["med_id"].as<int>();
  new_alert.alert_dismissed = doc["alert_dismissed"].as<bool>();
  new_alert.description = doc["description"].as<std::string>();
  new_alert.slot_number = doc["slot_number"].as<int>();

  // Print the new alert
  print_alert(new_alert);
}

std::string DATA_HANDLER::create_json_data(int device_id, unsigned long timestamp, bool critical_flag, double bpm, std::string activity)
{
  // Create a JSON object
  StaticJsonDocument<128> doc;

  // Create and assign key fields
  doc["deviceID"] = device_id;

  StaticJsonDocument<128> doc_2;

  doc_2["timestamp"] = timestamp;
  doc_2["critical"] = critical_flag;
  doc_2["heartbeat"] = bpm;
  doc_2["activity"] = activity;

  Serial.print("\nDOC 2:\n");
  serializeJsonPretty(doc_2,Serial);

  std::string temp;

  // Create JSON array for storing sensor data
  JsonArray data = doc.createNestedArray("data");
  data.add(doc_2);

  Serial.print("\nDOC:\n");
  serializeJsonPretty(doc,Serial);

  serializeJson(doc, temp);
  
  return temp;
}

void DATA_HANDLER::get_message_id(std::string payload)
{
  Serial.print(payload.c_str());

  // Create parameters for full payload
  const size_t capacity = (JSON_OBJECT_SIZE(1) + 60) * ALERT_THRESHOLD;

  // Create a JSON document for full payload
  StaticJsonDocument<capacity> doc;

  // Attempt to deserialise JSON
  DeserializationError json_error = deserializeJson(doc, payload);

  // Check for error
  if(json_error)
  {
    Serial.print("\nFailed de-serializing.\n");
    Serial.print(json_error.c_str());
  }

  // Parse the JSON
  auto message_id = doc["message_id"];

  // Set last message id
  last_message_id = message_id;
}

void DATA_HANDLER::parse_message(std::string payload)
{
  // Create parameters for full payload
  const size_t capacity = (JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(15) + 60) * 20;

  // Create a JSON document for full payload
  StaticJsonDocument<capacity> doc;

  // Attempt to deserialise JSON
  DeserializationError json_error = deserializeJson(doc, payload);

  // Check for error
  if(json_error)
  {
    Serial.print("\nFailed de-serializing.\n");
    Serial.print(json_error.c_str());
  }

  // Parse the JSON
  auto chat_id = doc["chat_id"].as<int>();
  auto message_array = doc["messages"];
  auto array_size = message_array.size();
  auto JSON_message = message_array[array_size-1];
  auto message_id = JSON_message["message_id"].as<int>();

  if(message_id > last_message_id)
  {
    Serial.print("\n*********************** PARSE GET MESSAGES **************************");
    Serial.print("\nMessage id:");
    Serial.print(message_id);

    auto data = JSON_message["message_data"].as<std::string>();

    // Let google home inform the user
    //GHome.dispatch(data);

    // Housekeeping
    last_message_id = message_id;
  }
}

// ******************************* PRINT FUNCTIONS *******************************

void DATA_HANDLER::print_alert(alert current_alert)
{
  Serial.print("\n********************************");
  Serial.print("\nALERT:\n");

  Serial.print("\nUserID:");
  Serial.print(current_alert.userID);

  Serial.print("\ntimestamp:");
  Serial.print(current_alert.timestamp);

  Serial.print("\nduration:");
  Serial.print(current_alert.duration);

  Serial.print("\nmed_id:");
  Serial.print(current_alert.med_id);

  Serial.print("\nalert_dismissed:");
  Serial.print(current_alert.alert_dismissed);

  Serial.print("\ndescription:");
  Serial.print(current_alert.description.c_str());

  GHome.dispatch("You, have an alert.");
  GHome.dispatch(current_alert.description);

  if(current_alert.med_id != NULL)
  {
    std::string message = patient_name + ",please take your medication from slot " + std::to_string(current_alert.slot_number);
    GHome.dispatch(message);

    Serial.print("\nPlease take medication from the specified slot");
    Serial.print("\nslot_number:");
    Serial.print(current_alert.slot_number);
  }

  Serial.print("\n");
}

void DATA_HANDLER::print_map()
{
  // Create a counter variable
  auto counter = 0;

  // Traverse through the map
  for(auto itr = alerts.begin(); itr != alerts.end(); itr++)
  {
    // Print the counter variable
    Serial.print("\nALERT:");
    Serial.print(++counter);

    // Print the alert_id
    Serial.print("\ntimestamp:");
    Serial.print(itr->first);

    for(int i = 0; i < itr->second.size(); i++)
    {
        Serial.print("\nIDs:");
        Serial.print(itr->second[i]);
    }
    Serial.print("\n");
  }
}
