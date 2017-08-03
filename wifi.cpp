#include <ESP8266WiFi.h>

#include "wifi.h"

namespace wifi {
  void maintain_wifi_connection(const char* ssid, const char* password) {
    if (WiFi.status() == WL_CONNECTED)
      return;

    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

