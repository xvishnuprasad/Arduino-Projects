#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Replace with your WiFi credentials
const char* ssid = "wifi name";
const char* password = "password";

// Soil moisture analog pin
const int soilMoisturePin = A0;

ESP8266WebServer server(80);

String getSoilStatus(int value) {
  if (value > 900) return "🌵 Dry";
  else if (value > 700) return "🟥 Slightly Dry";
  else if (value > 400) return "✅ Good Soil";
  else return "💧 Well Watered";
}



// Webpage HTML
const char* HTML_page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Soil Moisture Monitor</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: 'Segoe UI', sans-serif;
      background: #e0f7fa;
      text-align: center;
      padding: 50px;
    }
    h1 { font-size: 2.5em; }
    .status {
      font-size: 3em;
      margin-top: 20px;
      padding: 20px;
      border-radius: 15px;
      display: inline-block;
    }
    .value {
      font-size: 1.5em;
      margin-top: 10px;
    }
  </style>
</head>
<body>
  <h1>🌱 Soil Moisture Status</h1>
  <div id="status" class="status">Loading...</div>
  <div class="value" id="value">Moisture Value: --</div>

  <script>
    function fetchData() {
      fetch('/moisture')
        .then(response => response.json())
        .then(data => {
          document.getElementById('status').innerText = data.status;
          document.getElementById('value').innerText = "Moisture Value: " + data.value;
        })
        .catch(err => {
          console.log("Fetch error: ", err);
        });
    }
    setInterval(fetchData, 1000);
    fetchData();
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Connected! IP Address: ");
  Serial.println(WiFi.localIP());

  // Root route serves the web page
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", HTML_page);
  });

  // Moisture route returns JSON
  server.on("/moisture", HTTP_GET, []() {
    int value = analogRead(soilMoisturePin);
    String status = getSoilStatus(value);

    String json = "{\"value\":" + String(value) + ",\"status\":\"" + status + "\"}";
    server.send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
