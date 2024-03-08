/*********
  Smart Socket
  Author: Jakub Skwierczyński
*********/
// Libraries
#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>
#include "LittleFS.h"
#include <DHT.h>
#include <Filters.h>
#include <Preferences.h>
#include "RTClib.h"

// Access Point (AP) data
const char *ssidAP = "Smart Socket";
const char *passwordAP = "automation";

// Variable indicating WiFi connection
bool conn = false;

// User login and password
const char *http_username = "admin";
const char *http_password = "admin";
String usr = ""; // auxiliary variable for username
String psw = ""; // auxiliary variable for password

// Parameters used in HTTP server communication
// Parameters used in connecting to external networks
const char *par_ssid = "ssid";       // WiFi network name (SSID)
const char *par_pw = "pw";           // WiFi network password
const char *par_ip = "ip";           // IP address
const char *par_gateway = "gateway"; // Gateway address

// Parameters used in main HTML page communication
const char *par_relay = "relay_state";           // relay status
const char *par_nightlight = "nightlight_state"; // night light mode status
const char *par_temp = "temp_state";             // temperature mode status
const char *par_timesec = "input_timesec";       // socket activation time in night light mode (s)
const char *par_kWh_cost = "input_kWh_cost";     // cost of 1kWh
const char *par_t1 = "t1";                       // socket activation condition
const char *par_input_t1 = "input_t1";           // temperature value for socket activation condition
const char *par_t2 = "t2";                       // socket deactivation condition
const char *par_input_t2 = "input_t2";           // temperature value for socket deactivation condition
const char *par_sched = "sched_state";           // schedule status
const char *par_input_ton = "input_ton";         // activation time in schedule mode
const char *par_input_toff = "input_toff";       // deactivation time in schedule mode
const char *par_safety = "safety_state";         // safety status
const char *par_amp = "input_amp";               // maximum allowable current intensity
const char *par_s_amp = "input_s_amp";           // maximum duration of exceeded maximum allowable current intensity
const char *par_deg = "input_deg";               // maximum allowable temperature
const char *par_s_deg = "input_s_deg";           // maximum duration of exceeded maximum allowable temperature
const char *par_login = "user_login";            // username
const char *par_password = "user_pw";            // user password

// Variables to store HTML form values
String ssid;    // WiFi network name (SSID)
String pw;      // WiFi network password
String ip;      // IP address
String gateway; // Gateway address

// File paths for persistently storing input values in LittleFS file system
const char *ssidPath = "/ssid.txt";
const char *pwPath = "/pw.txt";
const char *ipPath = "/ip.txt";
const char *gatewayPath = "/gateway.txt";

// Initialize variable to store local IP address
IPAddress localIP;

// Initialize variable to store gateway address
IPAddress localGateway;

// Set subnet mask to 255.255.0.0
IPAddress subnet(255, 255, 0, 0);

// Time variables
unsigned long previousMillis = 0; // previous time
const long interval = 10000;      // WiFi connection waiting time (10s)

// Define relay control outputs
const int o_relay1 = 13; // relay from L/N
const int o_relay2 = 12; // relay from L/N

// Define motion sensor inputs
const int i_PIR = 19; // motion sensor PIN

// Variables used in motion sensor handling
int timeSec = 5;                // relay activation time after motion detection (in seconds)
unsigned long now = millis();   // current time (in milliseconds) since ESP startup
unsigned long last_trigger = 0; // time since last motion detection
boolean start_timer = false;    // start time counting
boolean motion = false;         // motion detection flag

// Many variables have inverted logic because the relay is triggered for the LOW state
// State variables
int PIR_state = HIGH;         // state based on motion sensor
int total_state = HIGH;       // total state corresponding to relay operation
int relayState = HIGH;        // current relay state
bool nightlightState = false; // night light mode state
bool tempState = false;       // temperature mode state
int tState = HIGH;            // state based on temperature sensor
int t1State = HIGH;           // auxiliary variable for socket activation condition
int t2State = HIGH;           // auxiliary variable for socket deactivation condition
bool schedState = false;      // schedule mode state
int sState = HIGH;            // state based on schedule mode
bool safetyState = true;      // safety state
bool eStopCurr = false;       // emergency stop related to overcurrent protection
bool eStopTemp = false;       // emergency stop related to thermal protection

// ACS712
#define ACS_Pin 34
float CurrentValue;             // ADC reading value
float CurrentAmps;              // estimated AC current value
RunningStatistics inputCurrent; // RunningStatistics class object creation

// ZMPT101B
#define ZMPT101B_Pin 33
float VoltageValue;             // ADC reading value
int VoltageVolts;               // estimated AC voltage value
RunningStatistics inputVoltage; // RunningStatistics class object creation

// Electrical power variable
float W = 0;                         // active power
float kWh = 0;                       // consumed energy
float kWh_cost = 0;                  // cost of 1 kWh
float total_cost = 0;                // total cost
unsigned long lastmillis = millis(); // start time counting

// Temperature variables
String t1 = ""; // stores selected logical operations for socket activation state
String t2 = ""; // stores selected logical operations for socket deactivation state
// 'lt' - "less than"; 'eq' - "equal"; 'gt' - "greater than"
float input_t1 = 0; // value of entered temperature for socket activation condition
float input_t2 = 0; // value of entered temperature for socket deactivation condition

// Schedule variables
String input_ton = "";  // activation time in schedule mode (HH:MM)
String input_toff = ""; // deactivation time in schedule mode (HH:MM)
int input_hon = 0;      // activation hour in schedule mode (HH)
int input_mon = 0;      // activation minute in schedule mode (MM)
int input_hoff = 0;     // deactivation hour in schedule mode (HH)
int input_moff = 0;     // deactivation minute in schedule mode (MM)
String var_day[7];      // array storing information about selected days of the week

// RTC (Real Time Clock) variables
int RTC_year = 0;                                                          // year read from RTC module
int RTC_month = 0;                                                         // month read from RTC module
int RTC_day = 0;                                                           // day of month read from RTC module
int RTC_h = 0;                                                             // hour read from RTC module
int RTC_m = 0;                                                             // minute read from RTC module
int RTC_s = 0;                                                             // second read from RTC module
String RTC_d = "";                                                         // day of week read from RTC module
char Week_days[7][12] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"}; // array storing abbreviated day names

// Security variables
float input_amp = 1;  // maximum allowable current intensity
int input_s_amp = 1;  // maximum duration of exceeded maximum allowable current intensity
float input_deg = 40; // maximum allowable temperature
int input_s_deg = 5;  // maximum duration of exceeded maximum allowable temperature
String alarmMsg = ""; // message about triggered security protection
String msg = "";      // message about current security status

// Temperature and humidity sensor DHT11 on pin 4
DHT dht(4, DHT11); // creating an object of the DHT class
float temp = 0; // variable holding the current temperature value

// Interrupts
// Overcurrent protection
bool overcurrent = false;     // overcurrent flag
bool timerCurrent = false;    // timer activation flag (exceeded set time)
hw_timer_t *timerCurr = NULL; // timer object definition

// Time interrupt function called when maximum duration of exceeded maximum allowable current intensity is exceeded
void IRAM_ATTR onTimerCurr()
{
    if (overcurrent)
    {
        timerCurrent = true;
    }
}

// Thermal protection
bool overtemp = false;         // overtemperature flag
bool timerTemperature = false; // timer activation flag (exceeded set time)
hw_timer_t *timerTemp = NULL;  // timer object definition

// Time interrupt function called when maximum duration of exceeded maximum allowable temperature is exceeded
void IRAM_ATTR onTimerTemp()
{
    if (overtemp)
    {
        timerTemperature = true;
    }
}

// Interrupt function handled during motion detection
void IRAM_ATTR motion_detection()
{ // 'IRAM_ATTR' attribute indicates that the function is stored in IRAM
    if (nightlightState)
    { // nightlightState active mode condition (
        start_timer = true;
        last_trigger = millis();
        PIR_state = LOW;
    }
}

// Creating an object to handle the Real - Time Clock(RTC)
RTC_DS1307 DS1307_RTC;

// Creating an object of the Preferences class, used for data storage in flash memory
Preferences pref;

// Creating an AsyncWebServer object on port 80
AsyncWebServer server(80);

// Function to read a file from the LittleFS file system
String readFile(fs::FS &fs, const char *path)
{
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path); // Attempting to open the file for reading
    if (!file || file.isDirectory())
    {
        Serial.println("- error opening file for reading"); // message in case of file open failure
        return String();
    }

    String fileContent;
    while (file.available())
    {
        fileContent = file.readStringUntil('\n'); // reading the first line
        break;
    }
    return fileContent; // returns the read file content
}

// Void function to write a file to the LittleFS file system
void writeFile(fs::FS &fs, const char *path, const char *message)
{
    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.println("- error opening file for writing");
        return;
    }
    if (file.print(message))
    {
        Serial.println("- file write successful");
    }
    else
    {
        Serial.println("- write failed");
    }
}

// Function determining the ESP connection status
bool initWiFi()
{
    if (ssid == "" || ip == "")
    { // condition for empty 'ssid' and 'ip' variables (first ESP connection)
        Serial.println("Undefined SSID or IP address.");
        return false;
    }

    WiFi.mode(WIFI_STA);                      // set WiFi module to client mode
    localIP.fromString(ip.c_str());           // convert 'ip' variable to IPAddress object
    localGateway.fromString(gateway.c_str()); // convert 'gateway' variable to IPAddress object.

    if (!WiFi.config(localIP, localGateway, subnet))
    {
        Serial.println("STA configuration error");
        return false; // return false in case of error.
    }
    WiFi.begin(ssid.c_str(), pw.c_str()); // start attempting to connect to WiFi network.
    Serial.println("Connecting to WiFi...");

    unsigned long currentMillis = millis(); // current time since ESP startup
    previousMillis = currentMillis;         // assign time value

    while (WiFi.status() != WL_CONNECTED)
    {                             // loop until connection established
        currentMillis = millis(); // current time
        if (currentMillis - previousMillis >= interval)
        { // exceed 10s connection time
            Serial.println("Connection error.");
            return false; // return false in case of error.
        }
    }

    Serial.println(WiFi.localIP()); // display local IP address after connection.
    return true;                    // return true for successful connection.
}

// Function to read temperature
String readDHTTemperature()
{
    // Default unit is Celsius
    float t = dht.readTemperature();
    // Check for any measurement errors
    if (isnan(t))
    {
        // Serial.println("DHT sensor reading error!");
        return "--";
    }
    else
    {
        // Serial.println(t);
        return String(t);
    }
}

// Function to read humidity
String readDHTHumidity()
{
    float h = dht.readHumidity();
    // Check for any measurement errors
    if (isnan(h))
    {
        // Serial.println("DHT sensor reading error!");
        return "--";
    }
    else
    {
        // Serial.println(h);
        return String(h);
    }
}

// Function generating a string representing the relay status,
// the function checks output 1 because both outputs work in parallel
String relay_outputState()
{
    if (digitalRead(o_relay1))
    {
        return "";
    }
    else
    {
        return "checked";
    }
}

// Function generating a string representing the night light mode status
String nightlight_outputState()
{
    if (nightlightState)
    {
        return "checked";
    }
    else
    {
        return "";
    }
}

// Function generating a string representing the temperature mode status
String temp_outputState()
{
    if (tempState)
    {
        return "checked";
    }
    else
    {
        return "";
    }
}

// Function generating a string representing the schedule mode status
String sched_outputState()
{
    if (schedState)
    {
        return "checked";
    }
    else
    {
        return "";
    }
}

// Function generating a string representing the safety status
String safety_outputState()
{
    if (safetyState)
    {
        return "checked";
    }
    else
    {
        return "";
    }
}

// Placeholder substitution in HTML
String rep_placeholder(const String &id)
{
 if (id == "RELAY_BUTTON")
    {
        String relay_outputStateValue = relay_outputState();
        return "<h2>Socket status: <span id=\"relay_outputState\"></span></h2><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggle_relay(this)\" id=\"relay_output\" " + relay_outputStateValue + "><span class=\"slider\"></span></label>";
    }
    else if (id == "NIGHTLIGHT_BUTTON")
    {
        String nightlight_outputStateValue = nightlight_outputState();
        return "<h2>Night light mode: <span id=\"nightlight_outputState\"></span></h2><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggle_nightlight(this)\" id=\"nightlight_output\" " + nightlight_outputStateValue + "><span class=\"slider\"></span></label>";
    }
    else if (id == "TEMP_BUTTON")
    {
        String temp_outputStateValue = temp_outputState();
        return "<h2>Temperature mode: <span id=\"temp_outputState\"></span></h2><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggle_temp(this)\"; id=\"temp_output\" " + temp_outputStateValue + "><span class=\"slider\"></span></label>";
    }
    else if (id == "SCHED_BUTTON")
    {
        String sched_outputStateValue = sched_outputState();
        return "<h2>Schedule: <span id=\"sched_outputState\"></span></h2><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggle_sched(this)\"; id=\"sched_output\" " + sched_outputStateValue + "><span class=\"slider\"></span></label>";
    }
    else if (id == "SAFETY_BUTTON")
    {
        String safety_outputStateValue = safety_outputState();
        return "<h2>Safety: <span id=\"safety_outputState\"></span></h2><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggle_safety(this)\"; id=\"safety_output\" " + safety_outputStateValue + "><span class=\"slider\"></span></label>";
    }
    else if (id == "TEMPERATURE")
    {
        return readDHTTemperature();
    }
    else if (id == "HUMIDITY")
    {
        return readDHTHumidity();
    }
    else if (id == "CURRENT")
    {
        return String(CurrentAmps);
    }
    else if (id == "VOLTAGE")
    {
        return String(VoltageVolts);
    }
    else if (id == "POWER")
    {
        return String(W);
    }
    else if (id == "KWH")
    {
        return String(kWh, 4);
    }
    else if (id == "INPUT_KWH_COST")
    {
        return String(kWh_cost, 2);
    }
    else if (id == "TOTAL_COST")
    {
        return String(total_cost, 4);
    }
    else if (id == "INPUT_TIMESEC")
    {
        return String(timeSec);
    }
    else if (id == "INPUT_T1")
    {
        return String(input_t1);
    }
    else if (id == "INPUT_T2")
    {
        return String(input_t2);
    }
    else if (id == "T1_" + t1) // saving user selection using the "selected" attribute in the dropdown list for socket activation
    {
        return "selected";
    }
    else if (id == "T2_" + t2) // saving user selection using the "selected" attribute in the dropdown list for socket deactivation
    {
        return "selected";
    }
    else if (id == "INPUT_TON")
    {
        return String(input_ton);
    }
    else if (id == "INPUT_TOFF")
    {
        return String(input_toff);
    }
    else if (id == "MON")
    {
        return var_day[0];
    }
    else if (id == "TUE")
    {
        return var_day[1];
    }
    else if (id == "WED")
    {
        return var_day[2];
    }
    else if (id == "THU")
    {
        return var_day[3];
    }
    else if (id == "FRI")
    {
        return var_day[4];
    }
    else if (id == "SAT")
    {
        return var_day[5];
    }
    else if (id == "SUN")
    {
        return var_day[6];
    }
    else if (id == "INPUT_AMP")
    {
        return String(input_amp);
    }
    else if (id == "INPUT_S_AMP")
    {
        return String(input_s_amp);
    }
    else if (id == "INPUT_DEG")
    {
        return String(input_deg);
    }
    else if (id == "INPUT_S_DEG")
    {
        return String(input_s_deg);
    }
    else if (id == "ALARM")
    {
        return msg;
    }
    return String();
}

// Function for splitting a string after the occurrence of a given separator
String divStr(String input, char sep, int index)
{
    // Variable initialization
    int separatorCount = 0; // Counter for occurrences of the separator
    int startIndex = 0;     // Index of the beginning of the found fragment
    int endIndex = -1;      // Index of the end of the found fragment

    // Loop through the entire 'input' string
    for (int i = 0; i < input.length(); i++)
    {
        if (input.charAt(i) == sep)
        {
            separatorCount++; // Increase the separator count
            if (separatorCount == index)
            {
                startIndex = i + 1; // Set the index of the beginning of the found fragment
            }
            else if (separatorCount == index + 1)
            {
                endIndex = i; // Set the index of the end of the found fragment and break the loop
                break;
            }
        }
    }

    // If the end of the fragment is not found and there are exactly 'index' separators
    if (endIndex == -1 && separatorCount == index)
    {
        endIndex = input.length(); // Set the end of the fragment to the end of the 'input' string
    }

    // Return the found fragment or an empty string if not found
    return (separatorCount >= index) ? input.substring(startIndex, endIndex) : "";
}

void setup()
{
    Serial.begin(115200);
    // Check LittleFS
    if (!LittleFS.begin(true))
    {
        Serial.println("Error uploading LittleFS");
        return;
    }

    // Load saved variables from the LittleFS file system
    ssid = readFile(LittleFS, ssidPath);       // WiFi network name (SSID)
    pw = readFile(LittleFS, pwPath);           // WiFi network password
    ip = readFile(LittleFS, ipPath);           // IP address
    gateway = readFile(LittleFS, gatewayPath); // Gateway address
    // Diagnostics
    Serial.println(ssid);
    Serial.println(pw);
    Serial.println(ip);
    Serial.println(gateway);

    // Motion sensor
    pinMode(i_PIR, INPUT_PULLUP);                                            // Set the pin with pull-up (default HIGH state)
    attachInterrupt(digitalPinToInterrupt(i_PIR), motion_detection, RISING); // Interrupt configuration

    // ACS712, current sensor
    pinMode(ACS_Pin, INPUT);
    inputCurrent.setWindowSecs(1);

    // ZMPT101B, single-phase voltage transformer
    pinMode(ZMPT101B_Pin, INPUT);
    inputVoltage.setWindowSecs(1);

    // Condition for successful WiFi connection
    if (initWiFi())
    {
        conn = true;
        // Main program
        // Initialize the use of non-volatile memory
        pref.begin("Data", false); // Create the "Data" area in read/write mode

        // Read configuration data from non-volatile memory
        // Electrical parameters
        kWh = pref.getFloat("kWh", 0);           // Read the saved value for the "kWh" area, return "0" if the area is not found
        kWh_cost = pref.getFloat("kWh_cost", 0); // Read the saved value for the "kWh_cost" area, return "0" if the area is not found

        // Nightlight mode
        timeSec = pref.getInt("timeSec", 5); // Read the saved value for the "timeSec" area, return "5" if the area is not found

        // Temperature mode
        t1 = pref.getString("t1", "gt");          // Read the saved value for the "t1" area, return "gt" if the area is not found
        input_t1 = pref.getFloat("input_t1", 20); // Read the saved value for the "input_t1" area, return "20" if the area is not found
        t2 = pref.getString("t2", "lt");          // Read the saved value for the "t2" area, return "lt" if the area is not found
        input_t2 = pref.getFloat("input_t2", 19); // Read the saved value for the "input_t2" area, return "19" if the area is not found

        // Schedule mode
        input_ton = pref.getString("input_ton", "");     // Read the turn-on time as a string in the format HH:MM; return an empty string if the area is not found
        input_hon = divStr(input_ton, ':', 0).toInt();   // Extract hours from the read turn-on time
        input_mon = divStr(input_ton, ':', 1).toInt();   // Extract minutes from the read turn-on time
        input_toff = pref.getString("input_toff", "");   // Read the turn-off time as a string in the format HH:MM; return an empty string if the area is not found
        input_hoff = divStr(input_toff, ':', 0).toInt(); // Extract hours from the read turn-off time
        input_moff = divStr(input_toff, ':', 1).toInt(); // Extract minutes from the read turn-off time
        var_day[0] = pref.getString("mon", "");          // Read the setting for Monday; return an empty string if the area is not found
        var_day[1] = pref.getString("tue", "");          // Read the setting for Tuesday; return an empty string if the area is not found
        var_day[2] = pref.getString("wed", "");          // Read the setting for Wednesday; return an empty string if the area is not found
        var_day[3] = pref.getString("thu", "");          // Read the setting for Thursday; return an empty string if the area is not found
        var_day[4] = pref.getString("fri", "");          // Read the setting for Friday; return an empty string if the area is not found
        var_day[5] = pref.getString("sat", "");          // Read the setting for Saturday; return an empty string if the area is not found
        var_day[6] = pref.getString("sun", "");          // Read the setting for Sunday; return an empty string if the area is not found

        // Protections
        input_amp = pref.getFloat("input_amp", 9);    // Read the overload protection current value; return "9" if the area is not found
        input_s_amp = pref.getInt("input_s_amp", 5);  // Read the overload protection duration value; return "5" if the area is not found
        input_deg = pref.getFloat("input_deg", 45);   // Read the thermal protection temperature value; return "45" if the area is not found
        input_s_deg = pref.getInt("input_s_deg", 5);  // Read the thermal protection duration value; return "5" if the area is not found
        eStopCurr = pref.getBool("eStopCurr", false); // Read the state of overload protection; return "false" if the area is not found
        eStopTemp = pref.getBool("eStopTemp", false); // Read the state of thermal protection; return "false" if the area is not found
        alarmMsg = pref.getString("alarmMsg", "");    // Read the alarm message; return an empty string if the area is not found

        // User
        usr = pref.getString("http_username", "admin"); // Read the username setting; return "admin" if the area is not found
        http_username = usr.c_str();
        psw = pref.getString("http_password", "admin"); // Read the user password setting; return "admin" if the area is not found
        http_password = psw.c_str();
        Serial.println(http_username);
        Serial.println(http_password);

        // RTC initialization
        if (!DS1307_RTC.begin())
        {
            Serial.println("RTC Error");
        }
        // DS1307_RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));

        // Checking if RTC is running
        if (!DS1307_RTC.isrunning())
        {
            Serial.println("RTC is not running, setting it now...");
            // If the RTC clock is not set, set it to the current date and time of compilation
            DS1307_RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }

        // Starting measurements using the DHT sensor
        dht.begin();

        // Setting the state of relay control outputs to low
        pinMode(o_relay1, OUTPUT);
        pinMode(o_relay2, OUTPUT);
        digitalWrite(o_relay1, HIGH); // LOW for high state
        digitalWrite(o_relay2, HIGH); // LOW for high state
        // Disabling modes after socket power-up
        nightlightState = false; // Nightlight mode is off when the socket is powered up
        schedState = false;      // Schedule mode is off when the socket is powered up
        tempState = false;       // Temperature mode is off when the socket is powered up
        safetyState = true;      // Protections are disabled when the socket is powered up

        // Interrupt initialization
        // Overcurrent protection
        timerCurr = timerBegin(0, 80, true);                     // timer 0 initialization for overcurrent protection
        timerAttachInterrupt(timerCurr, &onTimerCurr, true);     // link the timer with the 'onTimerCurr' function
        timerAlarmWrite(timerCurr, input_s_amp * 1000000, true); // set the counter value at which the interrupt will be generated (s)
        timerAlarmEnable(timerCurr);                             // enable timer interrupt
        // Initialize flags to 'false'
        overcurrent = false;
        timerCurrent = false;

        // Thermal protection
        timerTemp = timerBegin(1, 80, true);                     // timer 1 initialization for thermal protection
        timerAttachInterrupt(timerTemp, &onTimerTemp, true);     // link the timer with the 'onTimerTemp' function
        timerAlarmWrite(timerTemp, input_s_deg * 1000000, true); // set the counter value at which the interrupt will be generated (s)
        timerAlarmEnable(timerTemp);                             // enable timer interrupt
        // Initialize flags to 'false'
        overtemp = false;
        timerTemperature = false;

        // Handling HTTP requests
        // Handling HTTP request on the main server address
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      if (!request->authenticate(http_username, http_password)) // user authentication condition
                          return request->requestAuthentication();              // resend authentication request in case of incorrect authentication
                      request->send(LittleFS, "/main.html", String(), false, rep_placeholder);
                      // in case of successful authentication, send the content of the main.html file from the LittleFS file system along with additional parameters like HTML placeholder replacement
                  });
        // Handling HTTP request on the "/electrical.html" path, i.e., handling electrical parameters
        server.on("/electrical.html", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      if (!request->authenticate(http_username, http_password)) // user authentication condition
                          return request->requestAuthentication();              // resend authentication request in case of incorrect authentication
                      request->send(LittleFS, "/electrical.html", String(), false, rep_placeholder);
                      // in case of successful authentication, send the content of the electrical.html file from the LittleFS file system along with additional parameters like HTML placeholder replacement
                  });
        // Handling HTTP request on the "/sched.html" path, i.e., handling schedule
        server.on("/sched.html", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      if (!request->authenticate(http_username, http_password)) // user authentication condition
                          return request->requestAuthentication();              // resend authentication request in case of incorrect authentication
                      request->send(LittleFS, "/sched.html", String(), false, rep_placeholder);
                      // in case of successful authentication, send the content of the sched.html file from the LittleFS file system along with additional parameters like HTML placeholder replacement
                  });
        // Handling HTTP request on the "/nightlight.html" path, i.e., handling nightlight mode
        server.on("/nightlight.html", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      if (!request->authenticate(http_username, http_password)) // user authentication condition
                          return request->requestAuthentication();              // resend authentication request in case of incorrect authentication
                      request->send(LittleFS, "/nightlight.html", String(), false, rep_placeholder);
                      // in case of successful authentication, send the content of the nightlight.html file from the LittleFS file system along with additional parameters like HTML placeholder replacement
                  });
        // Handling HTTP request on the "/temperature.html" path, i.e., handling temperature mode
        server.on("/temperature.html", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      if (!request->authenticate(http_username, http_password)) // user authentication condition
                          return request->requestAuthentication();              // resend authentication request in case of incorrect authentication
                      request->send(LittleFS, "/temperature.html", String(), false, rep_placeholder);
                      // in case of successful authentication, send the content of the temperature.html file from the LittleFS file system along with additional parameters like HTML placeholder replacement
                  });
        // Handling HTTP request on the "/safety.html" path, i.e., handling protections
        server.on("/safety.html", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      if (!request->authenticate(http_username, http_password)) // user authentication condition
                          return request->requestAuthentication();              // resend authentication request in case of incorrect authentication
                      request->send(LittleFS, "/safety.html", String(), false, rep_placeholder);
                      // in case of successful authentication, send the content of the safety.html file from the LittleFS file system along with additional parameters like HTML placeholder replacement
                  });
        // Handling HTTP request on the "/user.html" path, i.e., handling user parameters
        server.on("/user.html", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      if (!request->authenticate(http_username, http_password)) // user authentication condition
                          return request->requestAuthentication();              // resend authentication request in case of incorrect authentication
                      request->send(LittleFS, "/user.html", String(), false, rep_placeholder);
                      // in case of successful authentication, send the content of the user.html file from the LittleFS file system along with additional parameters like HTML placeholder replacement
                  });
        // Handling HTTP request on the "/logout" path, i.e., in case of logout
        server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      request->send(401); // send lack of authentication
                  });

        // Handling HTTP request on the "/logout_html.html" path, i.e., handling HTML file of logout page
        server.on("/logout_html.html", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      request->send(LittleFS, "/logout_html.html", "text/html"); // send the content of the logout_html.html file from the LittleFS file system
                  });
        // Handling HTTP request on the "/get_user.html" path, i.e., handling user login and password changes
        server.on("/get_user", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
              String u = ""; // auxiliary variable for user login
              String pw = ""; // auxiliary variable for user password
              
              if (request->hasParam(par_login))  // check if the parameter related to the username exists
              {
                  u = request->getParam(par_login)->value();  // get the parameter value and assign it to the auxiliary variable
                  //http_username = u.c_str();  // obtain a pointer
                  pref.putString("http_username", u); // save the value of variable u in non-volatile memory
              }

              if (request->hasParam(par_password))  // check if the parameter related to the user's password exists
              {   
                  pw = request->getParam(par_password)->value();  // get the parameter value and assign it to the auxiliary variable
                  //http_password = pw.c_str(); // obtain a pointer
                  pref.putString("http_password", pw);  // save the value of variable pw in non-volatile memory
              }
              // Diagnostics
              Serial.println(http_username);
              Serial.println(http_password);
              request->send(200, "text/plain", "OK"); // send response to the client
              ESP.restart(); }); // restart ESP, necessary to refresh authentication parameters
        // Handling CSS file style.css
        server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send(LittleFS, "/style.css", "text/css"); });
        // Handling JavaScript file myscript.js
        server.on("/myscript.js", HTTP_GET, [](AsyncWebServerRequest *request)
                  { request->send(LittleFS, "/myscript.js", "text/js"); });
        // Handling HTTP request on the "/update" path, i.e., updating the state of used variables
        server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      if (!request->authenticate(http_username, http_password))
                          return request->requestAuthentication();

                      String inputMessage; // variable to store the message from the request

                      if (request->hasParam(par_relay))
                      {                                                         // check if the parameter related to the relay exists
                          inputMessage = request->getParam(par_relay)->value(); // get the parameter value and assign it to inputMessage
                          digitalWrite(o_relay1, inputMessage.toInt());         // change the state of the relays based on the value of the par_relay parameter
                          digitalWrite(o_relay2, inputMessage.toInt());
                          relayState = !relayState; // change the state
                      }
                      else if (request->hasParam(par_nightlight))
                      {                                                              // check if the parameter related to the nightlight mode exists
                          inputMessage = request->getParam(par_nightlight)->value(); // get the parameter value and assign it to inputMessage
                          nightlightState = !nightlightState;                        // change the state
                      }
                      else if (request->hasParam(par_temp))
                      {                                                        // check if the parameter related to the temperature mode exists
                          inputMessage = request->getParam(par_temp)->value(); // get the parameter value and assign it to inputMessage
                          tempState = !tempState;                              // change the state
                      }
                      else if (request->hasParam(par_sched))
                      {                                                         // check if the parameter related to the schedule mode exists
                          inputMessage = request->getParam(par_sched)->value(); // get the parameter value and assign it to inputMessage
                          schedState = !schedState;                             // change the state
                      }
                      else if (request->hasParam(par_safety))
                      {                                                          // check if the parameter related to the safety exists
                          inputMessage = request->getParam(par_safety)->value(); // get the parameter value and assign it to inputMessage
                          safetyState = !safetyState;                            // change the state
                      }
                      else
                      {                                  // no relevant parameters in the request
                          inputMessage = "No parameter"; // message helpful in analyzing potential errors
                      }
                      Serial.println(inputMessage);           // display the received message in the console
                      request->send(200, "text/plain", "OK"); // send response to the client
                  });
        // Handling HTTP request on the "/get" path, i.e., user-entered parameters
        server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
String inputMessage;  // variable to store the message from the request

if (request->hasParam(par_timesec)) { // checking for the occurrence of a parameter related to the time of relay activation in night light mode
  inputMessage = request->getParam(par_timesec)->value(); // getting the parameter value and assigning it to inputMessage
  timeSec =  inputMessage.toInt();  // assigning the converted value (string to int) to the timeSec variable
  pref.putInt("timeSec", timeSec);  // saving the value of the timeSec variable in non-volatile memory
}
else if (request->hasParam(par_kWh_cost)) { // checking for the occurrence of a parameter related to the price of 1 kWh
  inputMessage = request->getParam(par_kWh_cost)->value();  // getting the parameter value and assigning it to inputMessage
  kWh_cost = inputMessage.toFloat();  // assigning the converted value (string to float) to the kWh_cost variable
  pref.putFloat("kWh_cost", kWh_cost);  // saving the value of the kWh_cost variable in non-volatile memory
}
else if (request->hasParam(par_input_ton)) { // checking for the occurrence of a parameter related to the time of socket activation in schedule mode
  inputMessage = request->getParam(par_input_ton)->value();  // getting the parameter value and assigning it to inputMessage
  input_ton = inputMessage; // assigning the value of the retrieved parameter to the input_ton variable
  pref.putString("input_ton", input_ton); // saving the value of the input_ton variable in non-volatile memory
  input_hon = divStr(input_ton, ':', 0).toInt();  // reading the hour from the "input_ton" value and converting it to an integer (int) with assignment to input_hon
  input_mon = divStr(input_ton, ':', 1).toInt();  // reading the minute from the "input_ton" value and converting it to an integer (int) with assignment to input_mon
  //Serial.println(RTC_d);
  //Serial.println("Entered time: "+String(input_hon)+":"+String(input_mon));
  //Serial.println("RTC read: "+String(RTC_h)+":"+String(RTC_m));
}
else if (request->hasParam(par_input_toff)) { // checking for the occurrence of a parameter related to the time of socket deactivation in schedule mode
  inputMessage = request->getParam(par_input_toff)->value();  // getting the parameter value and assigning it to inputMessage
  input_toff = inputMessage;  // assigning the value of the retrieved parameter to the input_toff variable
  pref.putString("input_toff", input_toff); // saving the value of the input_toff variable in non-volatile memory
  input_hoff = divStr(input_toff, ':', 0).toInt();  // reading the hour from the "input_ton" value and converting it to an integer (int) with assignment to input_hoff
  input_moff = divStr(input_toff, ':', 1).toInt();  // reading the minute from the "input_ton" value and converting it to an integer (int) with assignment to input_moff
}
else {  // no relevant parameters in the request
  inputMessage = "No message sent"; // message helpful in potential error analysis
}
//Serial.println(inputMessage); // displaying the received message in the console
request->send(200, "text/text", inputMessage); }); // sending the response back to the client

        // Handling HTTP request on the "/get_temp" path, i.e., temperature parameters entered by the user
        server.on("/get_temp", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      int par_nr = request->params(); // assigning the number of parameters received in the request (4) to the par_nr variable
                      String par_temp[par_nr];        // creating an array containing par_nr elements

                      // 'for' loop for processing all request parameters
                      for (int i = 0; i < par_nr; i++)
                      {

                          AsyncWebParameter *p = request->getParam(i); // getting the i-th parameter
                          Serial.print("Parameter: ");
                          Serial.println(p->name()); // parameter name
                          Serial.print("Value: ");   // parameter value
                          Serial.println(p->value());
                          Serial.println("------");
                          par_temp[i] = p->value(); // assigning the value of the i-th parameter to the i-th element of the array
                      }
                      // Saving individual values of i-th parameters in non-volatile memory
                      t1 = par_temp[0];
                      pref.putString("t1", t1);
                      input_t1 = par_temp[1].toFloat();
                      pref.putFloat("input_t1", input_t1);
                      t2 = par_temp[2];
                      pref.putString("t2", t2);
                      input_t2 = par_temp[3].toFloat();
                      pref.putFloat("input_t2", input_t2);

                      request->send(200, "text/plain", "OK"); // sending response to the client
                  });

        // Handling HTTP request on the "/safety" path, i.e., safety parameters entered by the user
        server.on("/safety", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      int par_nr = request->params(); // assigning the number of parameters received in the request (4) to the par_nr variable
                      String par_temp[par_nr];        // creating an array containing par_nr elements

                      // 'for' loop for processing all request parameters
                      for (int i = 0; i < par_nr; i++)
                      {

                          AsyncWebParameter *p = request->getParam(i); // getting the i-th parameter
                          // Serial.print("Parameter: ");
                          // Serial.println(p->name()); // parameter name
                          // Serial.print("Value: "); // parameter value
                          // Serial.println(p->value());
                          // Serial.println("------");
                          par_temp[i] = p->value(); // assigning the value of the i-th parameter to the i-th element of the array
                      }
                      // Saving individual values of i-th parameters in non-volatile memory
                      input_amp = par_temp[0].toFloat();
                      pref.putFloat("input_amp", input_amp);
                      input_s_amp = par_temp[1].toInt();
                      pref.putInt("input_s_amp", input_s_amp);
                      input_deg = par_temp[2].toFloat();
                      pref.putFloat("input_deg", input_deg);
                      input_s_deg = par_temp[3].toInt();
                      pref.putInt("input_s_deg", input_s_deg);

                      // Changing the value of the counter at which an interrupt will be generated
                      timerAlarmWrite(timerCurr, input_s_amp * 1000000, true); // overload protection
                      timerAlarmWrite(timerTemp, input_s_deg * 1000000, true); // thermal protection

                      request->send(200, "text/plain", "OK"); // sending response to the client
                  });
        // Handling HTTP request on the "/get_day" path, i.e., cyclic repetition of schedule on specific days of the week
        server.on("/get_day", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      // If a particular parameter
                      //exists and has a value of "on", then change the var_d[x] variable to "checked", otherwise to an empty string 
                      if (request->hasParam("mon") && ((request->getParam("mon")->value()) == "on"))
                      {
                          var_day[0] = "checked";
                      }
                      else
                      {
                          var_day[0] = "";
                      }

                      if (request->hasParam("tue") && ((request->getParam("tue")->value()) == "on"))
                      {
                          var_day[1] = "checked";
                      }
                      else
                      {
                          var_day[1] = "";
                      }

                      if (request->hasParam("wed") && ((request->getParam("wed")->value()) == "on"))
                      {
                          var_day[2] = "checked";
                      }
                      else
                      {
                          var_day[2] = "";
                      }

                      if (request->hasParam("thu") && ((request->getParam("thu")->value()) == "on"))
                      {
                          var_day[3] = "checked";
                      }
                      else
                      {
                          var_day[3] = "";
                      }

                      if (request->hasParam("fri") && ((request->getParam("fri")->value()) == "on"))
                      {
                          var_day[4] = "checked";
                      }
                      else
                      {
                          var_day[4] = "";
                      }

                      if (request->hasParam("sat") && ((request->getParam("sat")->value()) == "on"))
                      {
                          var_day[5] = "checked";
                      }
                      else
                      {
                          var_day[5] = "";
                      }

                      if (request->hasParam("sun") && ((request->getParam("sun")->value()) == "on"))
                      {
                          var_day[6] = "checked";
                      }
                      else
                      {
                          var_day[6] = "";
                      }

                      // Saving schedule repetition preferences to non-volatile memory
                      pref.putString("mon", var_day[0]);
                      pref.putString("tue", var_day[1]);
                      pref.putString("wed", var_day[2]);
                      pref.putString("thu", var_day[3]);
                      pref.putString("fri", var_day[4]);
                      pref.putString("sat", var_day[5]);
                      pref.putString("sun", var_day[6]);

                      request->send(200, "text/plain", "OK"); // sending response to the client
                  });

        // Handling request regarding relay state
        server.on("/relay_state", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      request->send(200, "text/plain", String(digitalRead(o_relay1)).c_str()); // returning current state of the relay (o_relay1) as text
                      // Serial.println("Relay state: "+String(relayState)+" PIR state: "+String(PIR_state)+" Temp state: "+String(tState));
                  });

        // Handling request regarding nightlight mode state
        server.on("/nightlight_state", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      request->send(200, "text/plain", String(nightlightState)); // returning current state of the nightlight mode (nightlightState) as text
                  });

        // Handling request regarding temperature mode state
        server.on("/temp_state", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      request->send(200, "text/plain", String(tempState)); // returning current state of the temperature mode (tempState) as text
                  });

        // Handling request regarding schedule mode state
        server.on("/sched_state", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      request->send(200, "text/plain", String(schedState)); // returning current state of the schedule mode (schedState) as text
                  });

        // Handling request regarding safety state
        server.on("/safety_state", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      request->send(200, "text/plain", String(safetyState)); // returning current state of the safety mode (safetyState) as text
                  });

        // Handling request for temperature reading
        server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      temp = readDHTTemperature().toFloat();                            // assigning temperature value to temp variable
                      request->send_P(200, "text/plain", readDHTTemperature().c_str()); // using function to read temperature from DHT sensor and returning result as text
                  });

        // Handling request for humidity reading
        server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      request->send_P(200, "text/plain", readDHTHumidity().c_str()); // using function to read humidity from DHT sensor and returning result as text
                  });

        // Handling request for current reading
        server.on("/current", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      request->send(200, "text/plain", String(CurrentAmps)); // sending response with the read current intensity as text
                  });

        // Handling request for voltage reading
        server.on("/voltage", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      request->send(200, "text/plain", String(VoltageVolts)); // sending response with the read voltage as text
                  });

        // Handling request for power reading
        server.on("/power", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      request->send(200, "text/plain", String(W)); // sending response with the calculated active power as text
                  });

        // Handling request for electricity consumption (kWh) reading
        server.on("/kWh", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      pref.putFloat("kWh", kWh);                        // saving kWh variable value in non-volatile memory
                      request->send(200, "text/plain", String(kWh, 4)); // sending response with the calculated electricity consumption as text
                  });

        // Handling request for alarm message
        server.on("/alarm", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      request->send(200, "text/plain", msg); // sending response with the alarm message
                  });

        // Handling request for resetting electricity consumption counter
        server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      kWh = 0;                   // resetting electricity consumption (kWh) value
                      pref.putFloat("kWh", kWh); // saving reset kWh variable in non-volatile memory
                      request->send(200);        // sending response with code 200 (OK)
                  });

        // Handling request for resetting safety
        server.on("/safety_reset", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      eStopCurr = false;                    // disabling overload current alarm state
                      eStopTemp = false;                    // disabling thermal alarm state
                      pref.putBool("eStopCurr", eStopCurr); // saving disabled overload current alarm state in non-volatile memory
                      pref.putBool("eStopTemp", eStopTemp); // saving disabled thermal alarm state in non-volatile memory
                      pref.putString("alarmMsg", "");       // clearing alarm message

                      request->send(200); // sending response with code 200 (OK)
                  });

        // Handling request for reading total electricity cost
        server.on("/total_cost", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
                      request->send(200, "text/plain", String(total_cost, 2)); // sending response with the calculated total electricity cost as text
                  });

        // Handling request related to favicon.ico icon
        server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) { // returning favicon.ico file (website icon) in image/ico format
            request->send(LittleFS, "/favicon.ico", "image/ico");
        });

        // Configuring HTTP server to handle requests regarding static resources
        server.serveStatic("/", LittleFS, "/");

        // Starting HTTP server
        server.begin();
    }
        // Error obtaining Wifi connection
        else
        {
            conn = false;
            // Creating Access Point
            Serial.println("Creating AP (Access Point)");
            WiFi.softAP(ssidAP, passwordAP);

            // Getting IP address of the Access Point, default is 192.168.4.1
            IPAddress IP = WiFi.softAPIP();
            Serial.print("IP Address: ");
            Serial.println(IP);

            // Handling HTTP request on the main server address
            server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                      { request->send(LittleFS, "/wifi.html", "text/html"); });

            // Configuring HTTP server to handle requests regarding static resources
            server.serveStatic("/", LittleFS, "/");

            // Handling POST request
            server.on("/", HTTP_POST, [](AsyncWebServerRequest *request)
                      {
        int params = request->params();
        for(int i=0;i<params;i++){
          AsyncWebParameter* p = request->getParam(i);
          if(p->isPost()){
            // Processing SSID from POST request
            if (p->name() == par_ssid) {
              ssid = p->value().c_str();
              Serial.print("SSID set to: ");
              Serial.println(ssid);
              // Saving SSID value to file
              writeFile(LittleFS, ssidPath, ssid.c_str());
            }
            // Processing password from POST request
            if (p->name() == par_pw) {
              pw = p->value().c_str();
              Serial.print("Password set to: ");
              Serial.println(pw);
              // Saving password value to file
              writeFile(LittleFS, pwPath, pw.c_str());
            }
            // Processing IP address from POST request
            if (p->name() == par_ip) {
              ip = p->value().c_str();
              Serial.print("IP Address set to: ");
              Serial.println(ip);
              // Saving IP address value to file
              writeFile(LittleFS, ipPath, ip.c_str());
            }
            // Processing network gateway address from POST request
            if (p->name() == par_gateway) {
              gateway = p->value().c_str();
              Serial.print("Gateway set to: ");
              Serial.println(gateway);
              // Saving network gateway address value to file
              writeFile(LittleFS, gatewayPath, gateway.c_str());
            }
          }
        }
        request->send(200, "text/html; charset=UTF-8", "<html><head><style>body { font-family: Arial, sans-serif; font-size: 20px; background-color: #faf8f8; }</style></head><body><p>Configuration completed.<br><br>ESP will restart, then connect to the router and go to IP address: <a href='http://"+ip+"'>" + ip + "</a><br>If there is no connection, please fill out the form again under IP address: <a href='http://192.168.4.1'>192.168.4.1</a></p></body></html>");
        delay(3000);
        
        // Restarting ESP
        ESP.restart(); });
            // Starting HTTP server
            server.begin();
        }
    }

        void loop()
        {
            if (conn) // If ESP is connected to WiFi
            {
                now = millis(); // current time in milliseconds
                if ((digitalRead(i_PIR) == HIGH) && (motion == false))
                {                  // motion detected condition with previous no motion
                    motion = true; // motion detected
                }

                // Serial.println(timeSec);
                // Change of PIR_state variable after defined number of seconds (timeSec) from motion detection
                if (start_timer && (now - last_trigger > (timeSec * 1000)))
                {
                    PIR_state = HIGH;    // set motion sensor state to no motion
                    start_timer = false; // turn off the timer
                    motion = false;      // no motion
                }

                // Get current time from DS1307 RTC module
                DateTime RTC_now = DS1307_RTC.now();

                // Create TimeSpan object with value of 8 seconds
                TimeSpan offset(0, 0, 0, 8);

                // Offset time by 8 seconds (subtract offset)
                RTC_now = RTC_now - offset;

                // Get year, month, day, hour, minute, and second information from current RTC time
                RTC_year = RTC_now.year();
                RTC_month = RTC_now.month();
                RTC_day = RTC_now.day();
                RTC_h = RTC_now.hour();
                RTC_m = RTC_now.minute();
                RTC_s = RTC_now.second();

                // Get abbreviated name of current day of the week (e.g., "mon" for Monday) and assign it to RTC_d variable
                RTC_d = Week_days[RTC_now.dayOfTheWeek()];

                // Temperature mode
                if (tempState) // if the mode is enabled
                {
                    // Check condition for socket ON state
                    if (((t1 == "lt") && (temp < input_t1)) || ((t1 == "eq") && (temp == input_t1)) || ((t1 == "gt") && (temp > input_t1)))
                    {
                        t1State = LOW; // LOW corresponds to HIGH state (inverted logic)
                    }
                    else
                    {
                        t1State = HIGH; // HIGH corresponds to LOW state (inverted logic)
                    }

                    // Check condition for socket OFF state
                    if (((t2 == "lt") && (temp < input_t2)) || ((t2 == "eq") && (temp == input_t2)) || ((t2 == "gt") && (temp > input_t2)))
                    {
                        t2State = HIGH; // HIGH corresponds to LOW state (inverted logic)
                    }
                    else
                    {
                        t2State = LOW; // LOW corresponds to HIGH state (inverted logic)
                    }

                    tState = t1State || t2State; // logical OR, OFF state of socket takes precedence over ON state if both conditions are met
                }
                else
                {
                    tState = HIGH; // if the mode is disabled, tState is set to HIGH, indicating the relay is off
                }

                // Schedule mode
                if (schedState) // if the mode is enabled
                {
                    // Check if the current day of the week (RTC_d) matches the selected day in the schedule (var_day)
                    if (((RTC_d == "mon") && (var_day[0] == "checked")) || ((RTC_d == "tue") && (var_day[1] == "checked")) || ((RTC_d == "wed") && (var_day[2] == "checked")) || ((RTC_d == "thu") && (var_day[3] == "checked")) || ((RTC_d == "fri") && (var_day[4] == "checked")) || ((RTC_d == "sat") && (var_day[5] == "checked")) || ((RTC_d == "sun") && (var_day[6] == "checked")))
                    {
                        // If the day matches and it's the turn-on hour (input_hon) and turn-on minute (input_mon),
                        // but not the turn-off hour (input_hoff) and turn-off minute (input_moff), set state (sState) to LOW (on)
                        if ((input_hon == RTC_h) && (input_mon == RTC_m) && !((input_hoff == RTC_h) && (input_moff == RTC_m)))
                        {
                            sState = LOW;
                        }
                        // If it's the turn-off hour (input_hoff) and turn-off minute (input_moff),
                        // set state (sState) to HIGH (off)
                        else if ((input_hoff == RTC_h) && (input_moff == RTC_m))
                        {
                            sState = HIGH;
                        }
                    }
                    // If the current day doesn't match the schedule, set state (sState) to HIGH (off)
                    else
                    {
                        sState = HIGH;
                    }
                }
                // If the device is not in schedule mode, set state (sState) to HIGH (off)
                else
                {
                    sState = HIGH;
                }

                // Overload protection
                // If current current (CurrentAmps) is greater than the set maximum value (input_amp) and emergency stop mode is not activated (eStopCurr)
                if ((CurrentAmps > input_amp) && !eStopCurr)
                {
                    overcurrent = true; // set overload flag (overcurrent) to true
                }
                // If the current does not exceed the set value or emergency stop mode is activated (eStopCurr)
                else
                {
                    overcurrent = false;      // reset overload flag (overcurrent)
                    timerWrite(timerCurr, 0); // reset timer
                }

                // Thermal protection
                // If current temperature (temp) is greater than the set maximum value (input_deg) and emergency stop mode is not activated (eStopTemp)
                if ((temp > input_deg) && !eStopTemp)
                {
                    overtemp = true; // set thermal overload flag (overtemp) to true
                }
                // If the temperature does not exceed the set value or emergency stop mode is activated (eStopTemp)
                else
                {
                    overtemp = false;         // reset thermal overload flag (overtemp)
                    timerWrite(timerTemp, 0); // reset timer
                }

                // Safeties
                if (safetyState) // if safety mode is enabled
                {
                    // Overload protection handling
                    if (timerCurrent)
                    {
                        // Prepare date and time in the correct format
                        String date_year = String(RTC_year);
                        String date_month = "";
                        String date_day = "";
                        String date_hour = "";
                        String date_minute = "";
                        String date_second = "";

                        if (RTC_month < 10)
                        {
                            date_month = "0" + String(RTC_month);
                        }
                        else
                        {
                            date_month = String(RTC_month);
                        }

                        if (RTC_day < 10)
                        {
                            date_day = "0" + String(RTC_day);
                        }
                        else
                        {
                            date_day = String(RTC_day);
                        }

                        if (RTC_h < 10)
                        {
                            date_hour = "0" + String(RTC_h);
                        }
                        else
                        {
                            date_hour = String(RTC_h);
                        }

                        if (RTC_m < 10)
                        {
                            date_minute = "0" + String(RTC_m);
                        }
                        else
                        {
                            date_minute = String(RTC_m);
                        }

                        if (RTC_s < 10)
                        {
                            date_second = "0" + String(RTC_s);
                        }
                        else
                        {
                            date_second = String(RTC_s);
                        }

                        String CurrMsg = "";                   // definition of overload protection activation message
                        String lastCurr = String(CurrentAmps); // last measured current value before protection activation
                        // Serial.println("Current exceeded!"); // diagnostics
                        // Overload protection activation message
                        CurrMsg = date_day + '.' + date_month + '.' + date_year + " at " + String(date_hour) + ":" + String(date_minute) + ":" + String(date_second) + " overload protection activated.<br>Last measured current value before protection activation was " + lastCurr + "A.";
                        Serial.println(CurrMsg);

                        // If emergency stop mode is not activated
                        if (!(eStopCurr || eStopTemp))
                        {
                            alarmMsg = CurrMsg;                   // assign message to safety status message
                            pref.putString("alarmMsg", alarmMsg); // save message in non-volatile memory
                        }

                        eStopCurr = true;                     // activate current overload emergency stop
                        pref.putBool("eStopCurr", eStopCurr); // save state in non-volatile memory
                        timerCurrent = false;                 // reset timer flag
                    }

                    // Thermal protection handling
                    if (timerTemperature)
                    {
                        // Prepare date and time in the correct format
                        String date_year = String(RTC_year);
                        String date_month = "";
                        String date_day = "";
                        String date_hour = "";
                        String date_minute = "";
                        String date_second = "";

                        if (RTC_month < 10)
                        {
                            date_month = "0" + String(RTC_month);
                        }
                        else
                        {
                            date_month = String(RTC_month);
                        }

                        if (RTC_day < 10)
                        {
                            date_day = "0" + String(RTC_day);
                        }
                        else
                        {
                            date_day = String(RTC_day);
                        }

                        if (RTC_h < 10)
                        {
                            date_hour = "0" + String(RTC_h);
                        }
                        else
                        {
                            date_hour = String(RTC_h);
                        }

                        if (RTC_m < 10)
                        {
                            date_minute = "0" + String(RTC_m);
                        }
                        else
                        {
                            date_minute = String(RTC_m);
                        }

                        if (RTC_s < 10)
                        {
                            date_second = "0" + String(RTC_s);
                        }
                        else
                        {
                            date_second = String(RTC_s);
                        }

                        String TempMsg = "";            // definition of thermal protection activation message
                        String lastTemp = String(temp); // last measured temperature value before protection activation
                        // Serial.println("Temperature exceeded!");  // diagnostics
                        // Thermal protection activation message
                        TempMsg = date_day + '.' + date_month + '.' + date_year + " at " + String(date_hour) + ":" + String(date_minute) + ":" + String(date_second) + " thermal protection activated.<br>Last measured temperature value before protection activation was " + lastTemp + "°C.";
                        Serial.println(TempMsg);

                        // If emergency stop mode is not activated
                        if (!(eStopCurr || eStopTemp))
                        {
                            alarmMsg = TempMsg;                  // assign message to safety status message
                            pref.putString("alarmMsg", TempMsg); // save message in non-volatile memory
                        }

                        eStopTemp = true;                     // activate thermal overload emergency stop
                        pref.putBool("eStopTemp", eStopTemp); // save state in non-volatile memory
                        timerTemperature = false;             // reset timer flag
                    }
                }
                else
                {
                    // If safety mode is disabled, reset both timers
                    timerWrite(timerTemp, 0);
                    timerWrite(timerCurr, 0);
                }

                // Total socket status
                // If emergency stop mode is disabled
                if (!(eStopCurr || eStopTemp))
                {
                    total_state = (relayState && PIR_state && tState && sState); // occurrence of 0 (true for any status) causes socket activation
                    msg = "<strong style=\"color: green;\">No alarms</strong>";  // safety status message
                }
                // If emergency stop mode is enabled
                else
                {
                    // Disable all modes and states
                    // individual mode changes are blocked when emergency stop is activated
                    schedState = false;      // Schedule mode off
                    nightlightState = false; // Night light mode off
                    tempState = false;       // Temperature mode off
                    relayState = HIGH;       // Relay off
                    safetyState = true;      // Safety mode on
                    total_state = HIGH;      // Total socket status off

                    // Alarm message
                    msg = "<strong style=\"color: red;\">Alarm!<br><br>" + alarmMsg + "<br><br>Click below to reset safety</strong><br><button class=\"button_reset\" id=\"safety_reset\" onclick=\"safety_reset()\">Reset</button>";
                }

                // Set control outputs state for relays
                digitalWrite(o_relay1, total_state);
                digitalWrite(o_relay2, total_state);

                // Electrical parameters
                CurrentAmps = readCurrent();        // current
                VoltageVolts = readVoltage();       // voltage
                W = 1 * VoltageVolts * CurrentAmps; // active power

                kWh = energy_kWh();          // consumed energy
                total_cost = kWh_cost * kWh; // cost of consumed energy
            }
        }

        // Function to read current from ACS712 sensor
        float readCurrent()
        {

            CurrentValue = analogRead(ACS_Pin); // read analog value from ACS_Pin pin
            inputCurrent.input(CurrentValue);   // input the read value to RunningStatistics class object (inputCurrent)

            // Linearization of standard deviation (acting as RMS) of ADC reading from ACS712 sensor
            if ((inputCurrent.sigma() <= 15) || (total_state) || (VoltageVolts == 0)) // reset value if less than 15 or relay total_state off
            {
                CurrentAmps = 0;
            }
            else if ((inputCurrent.sigma() >= 15) && (inputCurrent.sigma() <= 125))
            {
                CurrentAmps = inputCurrent.sigma() * 0.0084 - 0.0338; // coefficients derived from trendline function in Excel
            }
            else
            {
                CurrentAmps = inputCurrent.sigma() * 0.0081 - 0.016; // coefficients derived from trendline function in Excel
            }

            return CurrentAmps; // return current value
        }

        // Function to read voltage from single-phase voltage transformer ZMPT101Bx
        float readVoltage()
        {

            VoltageValue = analogRead(ZMPT101B_Pin); // read analog value from ZMPT101B_Pin pin
            inputVoltage.input(VoltageValue); // input the read value to RunningStatistics class object (inputVoltage)

            VoltageVolts = inputVoltage.sigma() * 0.41 - 8.5691; // coefficients derived from trendline function in Excel
            if ((VoltageVolts <= 20) || (total_state))           // zero out "noise" and in case relay total_state off
            {
                VoltageVolts = 0;
            }

            return VoltageVolts; // return voltage value
        }

        // Function to calculate consumed energy in kWh
        float energy_kWh()
        {
            kWh = kWh + 0.93 * VoltageVolts * CurrentAmps * (millis() - lastmillis) / 3600000000.0; // assuming power factor value of 1
            lastmillis = millis();                                                               // update time needed for energy consumption calculation
            return kWh;                                                                          // return consumed energy value in kWh
        }
