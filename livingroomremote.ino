#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// Defines the macros "WIFISSID" and "WIFIPSK"
#include "credentials.hpp"

#include "tvcommands.hpp"

const char* ssid     = WIFISSID;
const char* password = WIFIPSK;

ESP8266WebServer server(80);

const int led = LED_BUILTIN;

const String postForms = "<html>\
  <head>\
    <title>ESP8266 Web Server POST handling</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>POST plain text to /postplain/</h1><br>\
    <form method=\"post\" enctype=\"text/plain\" action=\"/postplain/\">\
      <input type=\"text\" name=\'{\"hello\": \"world\", \"trash\": \"\' value=\'\"}\'><br>\
      <input type=\"submit\" value=\"Submit\">\
    </form>\
    <h1>POST form data to /postform/</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
      <input type=\"text\" name=\"hello\" value=\"world\"><br>\
      <input type=\"submit\" value=\"Submit\">\
    </form>\
  </body>\
</html>";

const String tvHtml = "<html>\
  <head>\
    <title>Living Room Remote</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>TV Remote</h1><br>\
    <form method=\"post\" enctype=\"text/plain\" action=\"/tv/\">\
      <input type=\"submit\" name=\"action\"  value=\"Source\">\
      <input type=\"submit\" name=\"action\"   value=\"Volume Up\">\
      <input type=\"submit\" name=\"action\" value=\"Volume Down\">\
    </form>\
    <h1>POST form data to /postform/</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
      <input type=\"text\" name=\"hello\" value=\"world\"><br>\
      <input type=\"submit\" value=\"Submit\">\
    </form>\
  </body>\
</html>";

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/html", postForms);
  digitalWrite(led, 0);
}

void handlePlain() {
  if (server.method() != HTTP_POST) {
    digitalWrite(led, 1);
    server.send(405, "text/plain", "Method Not Allowed");
    digitalWrite(led, 0);
  } else {
    digitalWrite(led, 1);
    server.send(200, "text/plain", "POST body was:\n" + server.arg("plain"));
    digitalWrite(led, 0);
  }
}

void handleForm() {
  if (server.method() != HTTP_POST) {
    digitalWrite(led, 1);
    server.send(405, "text/plain", "Method Not Allowed");
    digitalWrite(led, 0);
  } else {
    digitalWrite(led, 1);
    String message = "POST form was:\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(200, "text/plain", message);
    digitalWrite(led, 0);
  }
}

void handleTv()
{
    static const String VOLUME_UP_ACTION_STRING="action=Volume Up";
    static const String VOLUME_DOWN_ACTION_STRING="action=Volume Down";
    static const String POWER_ACTION_STRING="action=Power";
    static const String SOURCE_ACTION_STRING="action=Source";

    Serial.print("Source string: \"");
    Serial.print(SOURCE_ACTION_STRING);
    Serial.println("\"");
    Serial.print("VolUp string: \"");
    Serial.print(VOLUME_UP_ACTION_STRING);
    Serial.println("\"");

    if (server.method() == HTTP_POST)
    {
        String passedData = server.arg("plain");
        passedData.trim(); // Remote leading and trailing whitespace. Form data ends with a blank newline, so we're getting rid of that.
        Serial.print("Got data from server: \"");
        Serial.print(passedData);
        Serial.println("\"");
        if (passedData.equals(VOLUME_UP_ACTION_STRING))
        {
            sendCommand(TV_VOLUME_UP);
        }
        else if (passedData.equals(VOLUME_DOWN_ACTION_STRING))
        {
            sendCommand(TV_VOLUME_DOWN);
        }
        else if (passedData.equals(SOURCE_ACTION_STRING))
        {
            Serial.println("Changing TV source");
            //server.send(200, "text/plain", "POST body was:\n" + server.arg("plain"));
            sendCommand(TV_SOURCE);
        }
        else
        {
            Serial.println("ERROR: Got unexpected form data for TV!");
            server.send(400, "text/plain", "Bad request");
            return;
        }
    }
    if (server.method() == HTTP_GET)
    {
        server.send(200, "text/html", tvHtml);
    }
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  WiFi.begin(ssid, password);
  Serial.println("");

  irLed.begin();

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/postplain/", handlePlain);

  server.on("/postform/", handleForm);

  server.on("/tv", handleTv);
  server.on("/tv/", handleTv);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  //Serial.println("Changing source\n");
  //sendCommand(TV_SOURCE);
  //delay(2000);
}

