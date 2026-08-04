#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>


// Forward declarations
void applyOutputs();
String button(const char* label, const char* channel, bool state);
String ledIndicator(const char* label, const char* channel, bool state);
void handleRoot();
void handleSet();
void handleAllOff();

const char* AP_SSID = "ESP32-LED-Control";
const char* AP_PASSWORD = "12345678";

const int RED_PIN = 27;
const int GREEN_PIN = 25;
const int YELLOW_PIN = 32;

WebServer server(80);
bool redOn = false;
bool greenOn = false;
bool yellowOn = false;

void applyOutputs() {
  digitalWrite(RED_PIN, redOn ? HIGH : LOW);
  digitalWrite(GREEN_PIN, greenOn ? HIGH : LOW);
  digitalWrite(YELLOW_PIN, yellowOn ? HIGH : LOW);
}

String button(const char* label, const char* channel, bool state) {
  String html = "<form action='/set' method='get'><input type='hidden' name='channel' value='";
  html += channel;
  html += "'><input type='hidden' name='state' value='";
  html += state ? "0" : "1";
  html += "'><button class='";
  html += channel;
  html += state ? "" : " off-state";
  html += "' type='submit'>";
  html += label;
  html += ": ";
  html += state ? "ON" : "OFF";
  html += "</button></form>";
  return html;
}

// Sinh 1 vòng tròn LED trực quan, tự sáng đúng màu theo trạng thái thực tế (redOn/greenOn/yellowOn)
String ledIndicator(const char* label, const char* channel, bool state) {
  String html = "<div class='led-wrap'><div class='led ";
  html += channel;
  html += state ? " on" : "";
  html += "'></div><div class='led-label'>";
  html += label;
  html += "</div></div>";
  return html;
}

void handleRoot() {
  String page = R"rawliteral(<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>
<title>ESP32 LED Control</title><style>
body{font-family:Arial;text-align:center;background:#202124;color:#fff;margin:0;padding:24px}
h1{font-size:28px;margin-bottom:4px}
.subtitle{color:#9aa0a6;font-size:14px;margin-bottom:24px}
.card{max-width:420px;margin:auto;background:#303134;border-radius:16px;padding:20px}
.leds{display:flex;justify-content:center;gap:36px;margin:10px 0 28px 0}
.led-wrap{display:flex;flex-direction:column;align-items:center;gap:8px}
.led{width:56px;height:56px;border-radius:50%;background:#1a1a1a;border:3px solid #000;
box-shadow:inset 0 0 8px rgba(0,0,0,0.6);transition:all .15s ease}
.led.on.red{background:#ff3b30;box-shadow:0 0 25px 8px rgba(255,59,48,.65),inset 0 0 10px rgba(255,255,255,.4)}
.led.on.green{background:#34c759;box-shadow:0 0 25px 8px rgba(52,199,89,.65),inset 0 0 10px rgba(255,255,255,.4)}
.led.on.yellow{background:#ffe600;box-shadow:0 0 25px 8px rgba(255,230,0,.65),inset 0 0 10px rgba(255,255,255,.4)}
.led-label{font-size:12px;color:#9aa0a6}
form{margin:14px 0}
button{width:100%;padding:18px;border:0;border-radius:10px;color:white;font-size:20px;font-weight:bold;cursor:pointer;
transition:transform .08s ease,filter .15s ease}
button:active{transform:scale(.97)}
.red{background:#b3261e}.green{background:#188038}.yellow{background:#FFFF00;color:#000}
.off-state{filter:brightness(.6)}
.badge{display:inline-block;margin-top:18px;font-size:12px;color:#9aa0a6;border:1px solid #555;border-radius:20px;padding:4px 12px}
</style></head><body>
<h1>ESP32 LED Control</h1>
<div class='subtitle'>Dieu khien qua WiFi - ESP32</div>
<div class='card'>
<div class='leds'>
)rawliteral";
  page += ledIndicator("RED", "red", redOn);
  page += ledIndicator("GREEN", "green", greenOn);
  page += ledIndicator("YELLOW", "yellow", yellowOn);
  page += R"rawliteral(</div>
)rawliteral";
  page += button("RED", "red", redOn);
  page += button("GREEN", "green", greenOn);
  page += button("YELLOW", "yellow", yellowOn);
  page += R"rawliteral(<form action='/alloff' method='get'><button style='background:#666' type='submit'>ALL OFF</button></form>
<div class='badge'>ESP32 Access Point</div>
</div></body></html>)rawliteral";
  server.send(200, "text/html", page);
}

void handleSet() {
  String channel = server.arg("channel");
  bool state = server.arg("state") == "1";
  if (channel == "red") redOn = state;
  else if (channel == "green") greenOn = state;
  else if (channel == "yellow") yellowOn = state;
  applyOutputs();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleAllOff() {
  redOn = false;
  greenOn = false;
  yellowOn = false;
  applyOutputs();
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  applyOutputs();

  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/set", HTTP_GET, handleSet);
  server.on("/alloff", HTTP_GET, handleAllOff);
  server.begin();

  Serial.println();
  Serial.println("LED access point started");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("Password: ");
  Serial.println(AP_PASSWORD);
  Serial.print("Control page: http://");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
}