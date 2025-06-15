#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>

const char* ssid = "<SSID>";
const char* password = "<PASSWORD>";

// Outgoing webhook URL (to Home Assistant)
const char* outgoingWebhookUrl = "https://example.duckdns.org:<EXTERNAL PORT>/api/webhook/<WEBHOOK ID>";

/*

Example Outgoing webhook URL structure
https://example.ui.nabu.casa/api/webhook/<WEBHOOK ID>"
https://example.duckdns.org:<EXTERNAL PORT>/api/webhook/<WEBHOOK ID>
http://<Internal Home Assistant IP>:8123/api/webhook/<WEBHOOK ID>

*/ 

// Incoming webhook path (from Home Assistant / Node-RED)
const char* incomingWebhookPath = "/api/webhook/<WEBHOOK ID>";

#define BUTTON_PIN 14
#define LED_PIN    2

WebServer server(80);
int lastButtonState = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected!");
  Serial.print("ESP32 IP address: ");
  Serial.println(WiFi.localIP());

  // Incoming webhook handler
  server.on(incomingWebhookPath, HTTP_POST, handleIncomingWebhook);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  int currentState = digitalRead(BUTTON_PIN);

  if (lastButtonState == LOW && currentState == HIGH) {
    Serial.println("Button released — sending webhook...");
    sendOutgoingWebhook();
    delay(200);
  }

  lastButtonState = currentState;
  delay(20);
}

void sendOutgoingWebhook() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(outgoingWebhookUrl);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"message\": \"Hello from ESP32 board\"}";
    int responseCode = http.POST(payload);

    Serial.print("Outgoing webhook response: ");
    Serial.println(responseCode);

    http.end();
  } else {
    Serial.println("WiFi not connected — can't send webhook");
  }
}

void handleIncomingWebhook() {
  Serial.println("Incoming webhook received — LED ON");

  // Read the POST body
  if (server.hasArg("plain")) {
    String payload = server.arg("plain");
    Serial.print("Payload received: ");
    Serial.println(payload);
  } else {
    Serial.println("No payload received.");
  }

  digitalWrite(LED_PIN, HIGH);
  delay(2000);
  digitalWrite(LED_PIN, LOW);

  server.send(200, "text/plain", "OK");
}

