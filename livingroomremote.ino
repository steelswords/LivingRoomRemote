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

const String rootWebsite = "<html>\
  <head>\
    <title>Remote</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Choose Device to Control</h1><br>\
    <h2>TV</h2>\
    <a href=\"/tv\">\
    <img width=300px src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADwAAAAoCAIAAAAt2Q6oAAABhWlDQ1BJQ0MgcHJvZmlsZQAAKJF9kTtIw1AUhv8+pKIVBzuIOESoThZERXTTKhShQqgVWnUwuekLmjQkKS6OgmvBwcdi1cHFWVcHV0EQfIC4ujgpukiJ5yaFFjEeuNyP/57/595zAX+9zFQzOAaommWkEnEhk10VQq/woRtBDGFGYqY+J4pJeNbXPXVT3cV4lnffn9Wj5EwG+ATiWaYbFvEG8dSmpXPeJ46woqQQnxOPGnRB4keuyy6/cS447OeZESOdmieOEAuFNpbbmBUNlXiSOKqoGuX7My4rnLc4q+Uqa96TvzCc01aWuU5rEAksYgkiBMioooQyLMRo10gxkaLzuId/wPGL5JLJVQIjxwIqUCE5fvA/+D1bMz8x7iaF40DHi21/DAOhXaBRs+3vY9tunACBZ+BKa/krdWD6k/RaS4seAb3bwMV1S5P3gMsdoP9JlwzJkQK0/Pk88H5G35QF+m6BrjV3bs1znD4AaZpV8gY4OARGCpS97vHuzva5/dvTnN8PehJyqgs44eQAAAAJcEhZcwAALiMAAC4jAXilP3YAAAAHdElNRQfnAQsEJxt6Xfx+AAAAGXRFWHRDb21tZW50AENyZWF0ZWQgd2l0aCBHSU1QV4EOFwAAAGJJREFUWMPt07ENgEAIBVBwGCvj/nOYq1zmbI3GjgbzfkkBLwQiGiYjYt/WRuJjnEvHTUNDQ0NDQ0NDQ0NDQ0NDQ0NDQ0NDyz055yzumPmolI9w09B/Q9c84vv5vlIyruWmL8mxD0SLLweWAAAAAElFTkSuQmCC'/>\
    </a>\
    <h2>Bluray Player</h2>\
    <p><b>Coming soon!</b></p>\
  </body>\
</html>";

const String tvHtml = "<html>\n\
  <head>\n\
    <title>Living Room Remote</title>\n\
    <style>\n\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\n\
      button { font-size: 32; background-color: burlywood; }\n\
    </style>\n\
  </head>\n\
  <body>\n\
  <script type='text/javascript'>\n\
    function makeRequest(content) {\n\
        console.log('Function running');\n\
        const http = new XMLHttpRequest();\n\
        http.onreadystatechange = function() { \n\
            if (http.readyState == 4 && http.status == 200) {\n\
            console.log(http.responseText);\n\
            }\n\
        }\n\
        http.open('POST', 'tv', true);\n\
        http.send(content);\n\
        console.log('Posted' + content);\n\
    }\n\
    </script>\n\
    <h1>TV Remote</h1><br>\n\
    <hr/>\n\
    <button type='button' action=\"#\" onclick=\"makeRequest('action=Source')\">Source</button>\n\
    <button type='button' action=\"#\" onclick=\"makeRequest('action=Volume Up')\">Volume Up</button>\n\
    <button type='button' action=\"#\" onclick=\"makeRequest('action=Volume Down')\">Volume Down</button>\n\
    <button type='button' action=\"#\" onclick=\"makeRequest('action=Power')\">Power</button>\n\
      </body>\n\
</html>";

void handleRoot() {
  server.send(200, "text/html", rootWebsite);
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
        if (passedData.equals(POWER_ACTION_STRING))
        {
            sendCommand(TV_POWER);
        }
        else if (passedData.equals(VOLUME_UP_ACTION_STRING))
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

