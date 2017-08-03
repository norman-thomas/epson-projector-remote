#include <ESP8266WiFi.h>

#include <MQTTClient.h>

#include <SoftwareSerial.h>

#include "wifi.h"
#include "mqtt.h"
#include "types.h"

#include "config.h"

#define MQTT_PREFIX_LOCATION ""
#define MQTT_PREFIX_ROOM "wohnzimmer"
#define MQTT_PREFIX MQTT_PREFIX_LOCATION MQTT_PREFIX_ROOM "/devices/projector"

#define MQTT_PROJECTOR_STATUS MQTT_PREFIX "/status"
#define MQTT_PROJECTOR_REFRESH MQTT_PREFIX "/refresh"
#define MQTT_PROJECTOR_POWER MQTT_PREFIX "/power"

#define DEFAULT_REFRESH_RATE 30
//#define BUILTIN_LED D7

unsigned long lastMeasurementTime = 0;
unsigned int refresh_rate = DEFAULT_REFRESH_RATE; // seconds

WiFiClient wifiClient;
MQTTClient mqttClient;

SoftwareSerial swSer(5, 4);

const std::vector<const char*> mqtt_subscriptions = {
  MQTT_PROJECTOR_REFRESH,
  MQTT_PROJECTOR_POWER
};

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(115200);
  Serial.println();
  swSer.begin(9600);//, SERIAL_8N1);

  String myName = String(ESP.getChipId());
  Serial.print("ESP Chip ID: ");
  Serial.println(ESP.getChipId());

  mqttClient.begin(MQTT_SERVER, MQTT_SERVERPORT, wifiClient);
  mqttClient.onMessage(process_mqtt_subscriptions);

  wifi::maintain_wifi_connection(WLAN_SSID, WLAN_PASS);
  mqtt::maintain_connection(mqttClient, myName.c_str(), mqtt_subscriptions);

  Serial.println("Switching off internal LED");
  digitalWrite(BUILTIN_LED, HIGH);
}

void process_mqtt_subscriptions(String &topic, String &payload) {
  if (topic.equals(MQTT_PROJECTOR_REFRESH)) {
    Serial.println("Got MQTT topic '...projector/refresh'");
    powerStatus();
    if (!payload.equals("")) {
      // TODO set new refresh rate
    }
  }
  else if (topic.equals(MQTT_PROJECTOR_POWER)) {
    Serial.println("Got MQTT topic '...projector/power'");
    power(payload);
  }
  delay(100);
}

void loop() {
  wifi::maintain_wifi_connection(WLAN_SSID, WLAN_PASS);
  mqtt::maintain_connection(mqttClient, MQTT_PREFIX_ROOM, mqtt_subscriptions);

  mqttClient.loop();

  long now = millis();
  if (now - lastMeasurementTime > refresh_rate * 1000) {
    lastMeasurementTime = now;
    powerStatus();
  }
}

// **************

POWER getPower()
{
  String result = sendCommand("PWR?");
  if (detectError(result, "projector status check failed"))
    return UNKNOWN;

  if (result.equals("PWR=00"))
    return OFF;
  if (result.equals("PWR=01"))
    return ON;
  if (result.equals("PWR=02"))
    return WARMUP;
  if (result.equals("PWR=03"))
    return COOLINGDOWN;
  if (result.equals("PWR=04"))
    return STANDBYNETWORKON;
  if (result.equals("PWR=05"))
    return ABNORMALSTANDBY;

  report("unexpected status: " + result);
  return UNKNOWN;
}

int power(String arg)
{
  if (arg.equals("on") || arg.equals("1"))
    return powerOn();
  if (arg.equals("off") || arg.equals("0"))
    return powerOff();
  if (arg.equals("?") || arg.equals(""))
    return powerStatus();

  report("unexpected arg: '" + arg + "'");
  return -1;
}

int powerOn()
{
  const POWER status = getPower();
  if (status == ON || status == WARMUP)
  {
      mqttClient.publish(MQTT_PROJECTOR_STATUS, "on", false, 1);
      return 0;
  }

  const String result = sendCommand("PWR ON");
  if (detectError(result, "projector turn on failed"))
  {
    return -1;
  }

  report("PWR ON: " + result);

  const bool cond = result.equals("");
  const String status_str = cond ? "on" : "off";
  mqttClient.publish(MQTT_PROJECTOR_STATUS, status_str, false, 1);
  cond ? report("projector turned on") : report("projector turn on failed");
  return cond ? 0 : -1;
}

int powerOff()
{
  const POWER status = getPower();
  if (status == OFF || status == STANDBYNETWORKON)
  {
    mqttClient.publish(MQTT_PROJECTOR_STATUS, "on", false, 1);
    return 0;
  }

  const String result = sendCommand("PWR OFF");
  if (detectError(result, "projector turn off failed"))
  {
    return -1;
  }
  
  report("PWR OFF: " + result);
  
  const bool cond = result.equals("");
  String status_str = cond ? "off" : "on";
  mqttClient.publish(MQTT_PROJECTOR_STATUS, status_str, false, 1);
  cond ? report("projector turned off") : report("projector turn off failed");
  return cond ? 0 : -1;
}

int powerStatus()
{
  const POWER status = getPower();
  if (status == UNKNOWN)
  {
    return -1;
  }
  String status_str = (status == ON || status == WARMUP) ? "on" : "off";
  mqttClient.publish(MQTT_PROJECTOR_STATUS, status_str, false, 1);
  return (status == ON || status == WARMUP) ? 1 : 0;
}

int checkError(String arg)
{
  const String result = sendCommand("ERR?");
  if (result.startsWith("ERR="))
    result.substring(4);
  return result.toInt();
}

int ok(String arg)
{
  return checkError(arg) == 0 ? 0 : -1;
}


bool detectError(String result, String errorMessage)
{
  const bool error = result.equals("ERR");
  if (error)
    report(errorMessage);
  return error;
}

void report(String descr)
{
  Serial.println(descr);
}

// **************

void write(String cmd)
{
  digitalWrite(BUILTIN_LED, HIGH);
  swSer.print(cmd);
  swSer.print("\r");
  delay(100);
  digitalWrite(BUILTIN_LED, LOW);
}

String read()
{
  digitalWrite(BUILTIN_LED, HIGH);
  String result = "";
  while (swSer.available() > 0)
  {
    result.concat((char) swSer.read());
  }
  delay(100);
  digitalWrite(BUILTIN_LED, LOW);
  return result;
}

String sendCommand(String cmd)
{
  write(cmd);
  String result = read();
  Serial.print("read: >>");
  Serial.print(result);
  Serial.println("<<");
  result.trim();
  if (result.endsWith(":"))
    result.remove(result.length() - 1, 1);
  result.trim();
  return result;
}

