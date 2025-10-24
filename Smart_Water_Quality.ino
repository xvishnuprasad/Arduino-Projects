#include <ESP8266WiFi.h>
#include "DHT.h"

#define DHTPIN D4
#define DHTTYPE DHT11
#define TURBIDITY_PIN A0
#define TDS_PIN A0  // assign separate analog if needed

DHT dht(DHTPIN, DHTTYPE);

// Wi-Fi credentials
const char* ssid = "user user name";
const char* password = "password";

WiFiServer server(80);

float temperature, humidity, turbidity, tds;

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  while (!client.available()) delay(1);
  client.readStringUntil('\r');
  client.flush();

  // Read sensors
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  int turbidityRaw = analogRead(TURBIDITY_PIN);
  turbidity = map(turbidityRaw, 0, 1023, 0, 142); // Standard max NTU
  int tdsRaw = analogRead(TDS_PIN);
  tds = map(tdsRaw, 0, 1023, 0, 474);             // Standard max ppm

  // HTML dashboard
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>💧 Water Quality Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
      @import url('https://fonts.googleapis.com/css2?family=Poppins:wght@400;600&display=swap');
      body {
        font-family: 'Poppins', sans-serif;
        margin:0;
        padding:0;
        background: linear-gradient(135deg,#1e3c72,#2a5298);
        color:white;
        display:flex;
        flex-direction:column;
        align-items:center;
      }
      h1 {margin-top:20px;font-size:2rem;font-weight:600;text-align:center;}
      .container {
        display:grid;
        grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
        gap: 25px;
        width: 95%;
        max-width: 1200px;
        margin: 40px auto;
      }
      .card {
        background: rgba(255,255,255,0.1);
        border-radius:25px;
        padding:20px;
        backdrop-filter: blur(10px);
        box-shadow:0 8px 25px rgba(0,0,0,0.3);
        text-align:center;
        position: relative;
      }
      canvas {width:100%;height:220px;}
      .value-text {
        position:absolute;top:50%;left:50%;
        transform: translate(-50%, -50%);
        font-size:1.4rem;font-weight:600;
      }
      .label {margin-top:10px;font-size:1.1rem;font-weight:500;}
      .unit {font-size:0.9rem;opacity:0.8;}
      .quality-label {
        position:absolute;top:70%;left:50%;
        transform:translate(-50%,-50%);
        font-size:1rem;font-weight:500;
      }
    </style>
  </head>
  <body>
    <h1>💧 Smart Water Quality Dashboard</h1>
    <div class="container">
      <div class="card">
        <div style="position:relative;">
          <canvas id="tempGauge"></canvas>
          <div class="value-text" id="tempVal"></div>
        </div>
        <div class="label">Temperature</div>
        <div class="unit">(°C)</div>
      </div>
      <div class="card">
        <div style="position:relative;">
          <canvas id="humGauge"></canvas>
          <div class="value-text" id="humVal"></div>
        </div>
        <div class="label">Humidity</div>
        <div class="unit">(%)</div>
      </div>
      <div class="card">
        <div style="position:relative;">
          <canvas id="turbGauge"></canvas>
          <div class="value-text" id="turbVal"></div>
          <div class="quality-label" id="turbLabel"></div>
        </div>
        <div class="label">Turbidity</div>
        <div class="unit">(NTU)</div>
      </div>
      <div class="card">
        <div style="position:relative;">
          <canvas id="tdsGauge"></canvas>
          <div class="value-text" id="tdsVal"></div>
          <div class="quality-label" id="tdsLabel"></div>
        </div>
        <div class="label">TDS</div>
        <div class="unit">(ppm)</div>
      </div>
    </div>

    <script>
      const data = {
        temperature: )rawliteral" + String(temperature) + R"rawliteral(,
        humidity: )rawliteral" + String(humidity) + R"rawliteral(,
        turbidity: )rawliteral" + String(turbidity) + R"rawliteral(,
        tds: )rawliteral" + String(tds) + R"rawliteral(
      };

      function createGauge(ctx, value, max){
        return new Chart(ctx,{
          type:'doughnut',
          data:{datasets:[{data:[value,max-value],backgroundColor:['#00e676','#333'],borderWidth:0}]},
          options:{
            rotation:-90,
            circumference:180,
            cutout:'80%',
            plugins:{legend:{display:false},tooltip:{enabled:false}}
          }
        });
      }

      const tempGauge=createGauge(document.getElementById('tempGauge'),data.temperature,50);
      const humGauge=createGauge(document.getElementById('humGauge'),data.humidity,90);
      const turbGauge=createGauge(document.getElementById('turbGauge'),data.turbidity,142);
      const tdsGauge=createGauge(document.getElementById('tdsGauge'),data.tds,474);

      function setGaugeColor(gauge,value,max){
        let color;
        if(value<max*0.4) color='#00e676';
        else if(value<max*0.75) color='#ffd600';
        else color='#ff1744';
        gauge.data.datasets[0].backgroundColor[0]=color;
        gauge.update();
      }

      // Numeric Values
      document.getElementById('tempVal').innerHTML = data.temperature.toFixed(1)+'°C';
      document.getElementById('humVal').innerHTML = data.humidity.toFixed(1)+'%';
      document.getElementById('turbVal').innerHTML = data.turbidity.toFixed(1)+' NTU';
      document.getElementById('tdsVal').innerHTML = data.tds.toFixed(1)+' ppm';

      // Water quality labels
      function getTurbidityLabel(val){
        if(val<50) return "Clean Water 🟢";
        else if(val<100) return "Moderate 🌕";
        else return "Dirty Water 🟤";
      }
      function getTDSLabel(val){
        if(val<200) return "Pure Water 💧";
        else return "Salt Water 🌊";
      }

      document.getElementById('turbLabel').innerHTML=getTurbidityLabel(data.turbidity);
      document.getElementById('tdsLabel').innerHTML=getTDSLabel(data.tds);

      setGaugeColor(tempGauge,data.temperature,50);
      setGaugeColor(humGauge,data.humidity,90);
      setGaugeColor(turbGauge,data.turbidity,142);
      setGaugeColor(tdsGauge,data.tds,474);

      setTimeout(()=>location.reload(),2000); // refresh every 2 sec
    </script>
  </body>
  </html>
  )rawliteral";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  client.print(html);
  client.println();
}
