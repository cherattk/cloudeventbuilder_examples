/**
* 
*/
//////////////////////////////////
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
//////////////////////////////////

//////////////////////////////////
#include <cloudevent_builder.h>
//////////////////////////////////

//////////////////////////////////
// dht11 sensor library is from https://github.com/adidax/dht11
#include <dht11.h>
/////////////////////////////////
//*
#define SERVER_ENDPOINT "..."  // ex : "http://192.168.1.11:3000/data_receiver"
#ifndef WIFI_SSID
#define WIFI_SSID "..."  // wifi name
#define WIFI_PASS "..."  // wifi pass
#endif
//*/
/////////////////////////////////////////////
WiFiClient client;
HTTPClient http;
CloudEventBuilder ceb;
/////////////////////////////////////////////


/////////////////////////////////////////////
// connect DHT Data pin to PIN D4
// in NodeMCU-esp8266 pin D4 is connected to the built in BLUE LED
const int DHT_SENSOR_PIN = D4;
dht11 DHT11;
////////////////////////////////////////////

void setup() {

  Serial.begin(9600);

  /////////////////////////////////
  // wifi Connection
  ////////////////////////////////
  //*
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Try to connect to WIFI");
  Serial.println(" - Esp8266 MAC : " + WiFi.macAddress());
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Try to connect to WIFI - ");
    Serial.println("MCU MAC Address : " + WiFi.macAddress());
    Serial.println();
    delay(3000);
  }
  Serial.print("Connected To Wifi! MCU's IP address: ");
  Serial.println(WiFi.localIP());
  /**/
}

void loop() {

  ///////////////////////////////////////////////////////
  // Sensor Logic
  ///////////////////////////////////////////////////////
  // Delay between measurements.
  delay(2000);
  int chk = DHT11.read(DHT_SENSOR_PIN);
  float temperature_value = (float)DHT11.temperature;
  float humidity_value = (float)DHT11.humidity;
  Serial.print("Temperature  (C): ");
  Serial.println(temperature_value, 2);
  Serial.print("Humidity (%): ");
  Serial.println(humidity_value, 2);
  // dht11 chek value
  Serial.print("dht11 checksum value : ");
  Serial.println(chk);

  //*
  if ((WiFi.status() == WL_CONNECTED)) {

    ////////////////////////////////////////////////////////
    // Prepare Cloudevent object
    //////////////////////////////////////////////////////
    // required attributes
    ceb.setSpecVersion("v1");
    ceb.setId("12345");
    ceb.setSource("/my.greenhouse.1");
    ceb.setType("my.greenhouse.1.sensor.dht");

    // Optional attributes
    ceb.setSubject("temperature_humidity");
    ceb.setDataContenType("application/json");

    // sensor data container
    string data[] = { "temperature:", "humidity:" };
    // temperature data
    char str__temperature[] = "000.00";
    dtostrf(temperature_value, -5, 2, str__temperature);
    data[0] += str__temperature;
    // humidity data
    char str__humidity[] = "000.00";
    dtostrf(humidity_value, -5, 2, str__humidity);
    data[1] += str__humidity;

    // set temperature/humidity datat to cloudevent object
    ceb.setData(data, 2);

    // Get payload as json string
    string ce_body = ceb.HttpPayload();

    // convert std::string) to (char *) so that can be used by http.POST()
    char *ce_json = new char[ce_body.length() + 1];
    strcpy(ce_json, ce_body.c_str());
    //
    Serial.printf("%s\n", ce_json);

    ///////////////////////////////////////////////////
    // HTTP : Connect and Send CloudEvent Object
    ///////////////////////////////////////////////////
    // configure target server and url
    http.begin(client, SERVER_ENDPOINT);

    // cloudevent required Header
    http.addHeader("Content-Type", "application/cloudevents+json; charset=utf-8");

    // start connection and send HTTP header and body
    int httpCode = http.POST(ce_json);
    delete[] ce_json;

    // response
    Serial.printf("[HTTP] POST response code : %d\n", httpCode);
    if (httpCode != 200) {
      Serial.printf("[HTTP] POST failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    Serial.println("");
    http.end();
  }
  /**/
}
