#include <WiFi.h>
#include <WebServer.h>

// WiFi Credentials
const char* ssid = "Vishnu's24";
const char* password = "223334444";

// Pins
#define MQ135_AnalogPin 34  // MQ135 sensor connected to GPIO34

WebServer server(80); // Web server runs on port 80

void setup() {
  Serial.begin(115200);
  pinMode(MQ135_AnalogPin, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.println(WiFi.localIP());

  // Define server routes
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient(); // Handle incoming clients
}

// Serve Webpage
void handleRoot() {
  int airQuality = analogRead(MQ135_AnalogPin);

  String smokeStatus;
  bool smokeDetected = false;

  if (airQuality > 750) {
    smokeStatus = "Smoke Detected!";
    smokeDetected = true;
  } else {
    smokeStatus = "No Smoke";
  }

  // HTML Content
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: 'Poppins', sans-serif; background: linear-gradient(135deg, #89f7fe 0%, #66a6ff 100%); margin: 0; display: flex; align-items: center; justify-content: center; height: 100vh;}";
  html += ".card { background: white; padding: 40px; border-radius: 20px; box-shadow: 0 8px 16px rgba(0,0,0,0.3); text-align: center; max-width: 400px;}";
  html += "h1 { font-size: 2.8em; color: #333; margin-bottom: 20px; }";
  html += "p { font-size: 1.5em; margin: 10px 0; }";
  html += ".status { font-weight: bold; font-size: 1.8em; margin-top: 20px; }";
  html += "</style>";
  html += "<link href='https://fonts.googleapis.com/css2?family=Poppins:wght@400;600&display=swap' rel='stylesheet'>";
  html += "</head><body>";

  html += "<div class='card'>";
  html += "<h1>Smoke Detection</h1>";
  html += "<p class='status'>" + smokeStatus + "</p>";
  html += "<p>Sensor Value: " + String(airQuality) + "</p>";
  html += "</div>";

  // Notification and sound if smoke detected
  html += "<audio id='siren' src='https://www.soundjay.com/misc/sounds/bell-alarm-01.mp3'></audio>";
  html += "<script>";
  if (smokeDetected) {
    html += "alert('SMOKE DETECTED! Take Action Immediately!');";
    html += "var siren = document.getElementById('siren');";
    html += "siren.play();";
  }
  html += "setTimeout(function(){ location.reload(); }, 3000);"; // Refresh page every 3s
  html += "</script>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}
