#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <LuxpowerTCP.h>

Preferences prefs;
WebServer   server(80);
LuxpowerTCP inverter;

String wifi_ssid   = "";
String wifi_pass   = "";
String dongle_sn   = "";
String inverter_sn = "";
String inv_ip      = "";
uint32_t interval  = 3;

LuxRealtimeData rtData;
LuxEnergyData   enData;

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>LuxPower Monitor</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { font-family: system-ui, sans-serif; background: #0b1329; color: #fff; padding: 16px; }
    .wrap { max-width: 540px; margin: 0 auto; }
    h1 { text-align: center; color: #38bdf8; font-size: 20px; margin-bottom: 16px; }
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
    .card { background: #1e293b; padding: 14px; border-radius: 8px; border: 1px solid #334155; }
    .t { font-size: 11px; color: #94a3b8; text-transform: uppercase; }
    .v { font-size: 24px; font-weight: bold; margin: 4px 0; }
    .s { font-size: 12px; color: #cbd5e1; }
    .pv { color: #f59e0b; } .bat { color: #10b981; } .grid-c { color: #60a5fa; } .eps { color: #ec4899; }
  </style>
</head>
<body>
  <div class="wrap">
    <h1>LUXPOWER SNA PRO</h1>
    <div class="grid">
      <div class="card">
        <div class="t">Mặt Trời (PV)</div>
        <div class="v pv"><span id="ppv">0</span> W</div>
        <div class="s">Hôm nay: <span id="epv">0</span> kWh</div>
      </div>
      <div class="card">
        <div class="t">Pin Lưu Trữ</div>
        <div class="v bat"><span id="soc">0</span>%</div>
        <div class="s"><span id="vbat">0</span> V | <span id="pbat">0</span> W</div>
      </div>
      <div class="card">
        <div class="t">Điện Lưới</div>
        <div class="v grid-c"><span id="pgrid">0</span> W</div>
        <div class="s"><span id="vgrid">0</span> V</div>
      </div>
      <div class="card">
        <div class="t">Tải Dùng (EPS)</div>
        <div class="v eps"><span id="peps">0</span> W</div>
        <div class="s">Nhiệt: <span id="temp">0</span> °C</div>
      </div>
    </div>
  </div>
  <script>
    async function load() {
      try {
        let r = await fetch('/data');
        let d = await r.json();
        document.getElementById('ppv').innerText = d.ppv;
        document.getElementById('epv').innerText = d.epv;
        document.getElementById('soc').innerText = d.soc;
        document.getElementById('vbat').innerText = d.vbat;
        document.getElementById('pbat').innerText = d.pchg > 0 ? ("+" + d.pchg) : ("-" + d.pdis);
        document.getElementById('pgrid').innerText = d.pgrid;
        document.getElementById('vgrid').innerText = d.vgrid;
        document.getElementById('peps').innerText = d.peps;
        document.getElementById('temp').innerText = d.temp;
      } catch(e) {}
    }
    setInterval(load, 2000);
    load();
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", HTML_PAGE);
}

void handleData() {
  char json[256];
  snprintf(json, sizeof(json),
    "{\"ppv\":%u,\"epv\":%.1f,\"soc\":%u,\"vbat\":%.1f,\"pchg\":%u,\"pdis\":%u,\"pgrid\":%d,\"vgrid\":%.1f,\"peps\":%u,\"temp\":%u}",
    rtData.pPvTotal, enData.epvToday, rtData.soc, rtData.vBat,
    rtData.pCharge, rtData.pDischarge, rtData.pGrid, rtData.vGrid,
    rtData.pEps, rtData.tempRad
  );
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);

  prefs.begin("lux_cfg", true);
  wifi_ssid   = prefs.getString("ssid", "YOUR_SSID");
  wifi_pass   = prefs.getString("pass", "YOUR_PASS");
  dongle_sn   = prefs.getString("dongle_sn", "BA12345678");
  inverter_sn = prefs.getString("inv_sn", "1234567890");
  inv_ip      = prefs.getString("inv_ip", "");
  interval    = prefs.getUInt("interval", 3);
  prefs.end();

  WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.printf("\nDashboard IP: http://%s\n", WiFi.localIP().toString().c_str());

  inverter.begin(inv_ip.c_str(), dongle_sn.c_str(), inverter_sn.c_str());
  inverter.setInterval(interval);

  if (inv_ip.length() == 0) {
    String found = inverter.discoverIP();
    if (found.length() > 0) Serial.printf("Auto-discovered Inverter: %s\n", found.c_str());
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.begin();
}

void loop() {
  server.handleClient();

  if (inverter.shouldPoll()) {
    inverter.readRealtime(rtData);
    delay(100);
    inverter.readEnergy(enData);
  }
}
