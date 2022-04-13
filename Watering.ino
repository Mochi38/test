// ปิดการส่งข้อมูลผ่าน mqtt ไว้ก่อน
#include <M5StickC.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Arduino.h>

#define INPUT_PIN 33
#define PUMP_PIN 32

//const char *ssid = "NTC";
//const char *password = "ntcskywave";
const long utcOffsetInSeconds = 0;
const long THAIOffsetInSeconds = 25200;

const char* mqtt_server = "mqt2.nauticomm.com";
const int mqtt_port = 1883;
const char* mqtt_Client = "mjvfete";
const char* mqtt_username = "nauticomm";
const char* mqtt_password = "latlongvms";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
NTPClient timeClientTH(ntpUDP, "pool.ntp.org", THAIOffsetInSeconds);
bool flag = true;
bool status_water = false;
int soilM_raw;
unsigned long lasttime = 15000;

WiFiClient espClient;
PubSubClient client(espClient);
char msg_data_interval[1000];
char msg_result[1000];
long past = 15000;
String currentDate = "";
String Duration = "";

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");

    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("cloudgarden/soilmoisture/1/command");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println("Callback: " + message);
  Serial.println("");

  if (String(topic) == "cloudgarden/soilmoisture/1/command") {
    if (message.substring(16, 21) == "water") {
      Duration = message.substring(49, 50);
      Serial.println("Comfirmed duration: " + Duration);
      Serial.println("Pump is running: " + Duration + "sec.");
      int duration_conv = Duration.toInt() * 1000;
      int durationP = Duration.toInt();

      // NOW!! PUMP IS TURN ON //
      printLED_pumpON();
      digitalWrite(PUMP_PIN, flag);
      delay(duration_conv);
      digitalWrite(PUMP_PIN, !flag);
      Serial.println("Pump is OFF");
      status_water = true;
      printLED_pumpOFF();


      if (status_water == true) {
        String json_result = "{\"commandName\":" + String("\"water\"") + "," + "\"sensorNo\":" + "\"" + String("1") + "\"," + "\"status\":" + status_water + "," + "\"lastupdate\":" + "\"" + String(currentDate) + "\"" + "}";
        json_result.toCharArray(msg_result, (json_result.length() + 1));
        client.publish("cloudgarden/soilmoisture/1/commandResult", msg_result);
        status_water = false;
      }
    }

    //    if (message == "on") {
    //      client.publish("@shadow/data/update", "{\"data\": {\"led\": \"on\"}}");
    //      Serial.println("Water ON");
    //
    //    } else if (message == "off") {
    //      client.publish("@shadow/data/update", "{\"data\":{\"led\": \"off\"}}");
    //      Serial.println("Water OFF");
    //
    //    }
  }
}

void printLED_pumpON() {
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Lcd.setCursor(15, 60);
  M5.Lcd.printf("ON ");
  M5.Lcd.setTextSize(1);
}

void printLED_pumpOFF() {
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);
  M5.Lcd.setCursor(15, 60);
  M5.Lcd.printf("OFF");
  M5.Lcd.setTextSize(1);
}

void setup() {
  M5.begin();
  Serial.begin(115200);
  //  WiFi.begin(ssid, password);
  //  M5.Power.begin();

  pinMode(INPUT_PIN, INPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(25, OUTPUT);
  digitalWrite(25, 0);

  //  while ( WiFi.status() != WL_CONNECTED ) {
  //    delay ( 500 );
  //    Serial.print ( "." );
  //  }

  //  Serial.println("");
  //  Serial.println("WiFi connected");
  //  Serial.println("IP address: ");
  //  Serial.println(WiFi.localIP());
  //  client.setServer(mqtt_server, mqtt_port);

  //  client.setCallback(callback);

  //  timeClient.begin();
  //  timeClientTH.begin();
  M5.Lcd.setRotation(2);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setCursor(20, 5);
  M5.Lcd.printf("PLANT 1");
  printLED_pumpOFF();
}

char info[30];

void loop() {
  //  timeClient.update();
  //  timeClientTH.update();
  //  unsigned long epochTime = timeClient.getEpochTime();
  //  struct tm *ptm = gmtime ((time_t *)&epochTime);
  //  int monthDay = ptm->tm_mday;
  //  int currentMonth = ptm->tm_mon + 1;
  //  int currentYear = ptm->tm_year + 1900;

  M5.Lcd.setCursor(15, 130);
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  //  M5.Lcd.print(timeClientTH.getFormattedTime());

  //  int currentHour = timeClient.getHours();
  //  int currentMinute = timeClient.getMinutes();
  //  int currentSecond = timeClient.getSeconds();
  //  currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay) + " " + timeClient.getFormattedTime();

  if ((millis() - lasttime) >= 1000) {
    soilM_raw = analogRead(INPUT_PIN);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(2, 30);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Lcd.print("S:" + String(soilM_raw));
    M5.Lcd.setTextSize(1);
    lasttime = millis();
  }

  String json_data_interval = "{\"sensorNo\":" + String("\"1\"") + "," + "\"unitName\":" + "\"" + String("SoilMoisture") + "\"," + "\"SoilMoisture\":" + soilM_raw + "," + "\"status\":"  + status_water + "," + "\"sensorTime\":" + "\"" + String(currentDate) + "\"" + "}";
  json_data_interval.toCharArray(msg_data_interval, (json_data_interval.length() + 1));

  //  if (!client.connected()) {
  //    reconnect();
  //  }
  //  client.loop();
  //  long start = millis();
  //  if (start - past >= 15000) {
  //    Serial.println(json_data_interval);
  //    Serial.println(" ");
  //    client.publish("cloudgarden/soilmoisture/1", msg_data_interval);
  //    past = start;
  //  }

  if (M5.BtnA.wasPressed()) {
    digitalWrite(PUMP_PIN, flag);
    flag = !flag;
    if (status_water == false) {
      printLED_pumpON();
      status_water = true;
    } else {
      printLED_pumpOFF();
      status_water = false;
    }
    M5.update();
  }
  M5.update();
}
