#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// ====== ISI BAGIAN INI ======
const char* WIFI_SSID = "LABTJP1";
const char* WIFI_PASS = "123454321";
const char* SUPABASE_URL = "https://lhpubvtrylglspnrtmgd.supabase.co/rest/v1/sensor_data";
const char* SUPABASE_KEY = "sb_publishable_X-XOJOEFoVkXc2k7SFlI-w_v7bn_VmD";  
// ============================

#define DHTPIN  4          // DATA sensor -> GPIO4
#define DHTTYPE DHT11      // sensor biru di foto = DHT11. Jika pakai DHT22 ganti ke DHT22
const unsigned long INTERVAL_MS = 10000;  // kirim tiap 10 detik

DHT dht(DHTPIN, DHTTYPE);
unsigned long lastSend = 0;

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.print("Menghubungkan WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
    delay(500); Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nTersambung, IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nGagal terhubung WiFi (pastikan 2.4 GHz)");
  }
}

void kirimData(float t, float h) {
  WiFiClientSecure client;
  client.setInsecure();            // cukup untuk praktikum; produksi: pakai sertifikat root
  HTTPClient http;
  http.begin(client, SUPABASE_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("apikey", SUPABASE_KEY);
  http.addHeader("Prefer", "return=minimal");

  JsonDocument doc;
  doc["temperature"] = t;
  doc["humidity"] = h;
  String body;
  serializeJson(doc, body);

  int code = http.POST(body);
  Serial.printf("POST %s -> HTTP %d\n", body.c_str(), code);
  if (code != 201) Serial.println(http.getString());  // tampilkan pesan error dari server
  http.end();
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  connectWiFi();
}

void loop() {
  connectWiFi();
  if (millis() - lastSend >= INTERVAL_MS) {
    lastSend = millis();
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(t) || isnan(h)) {
      Serial.println("Gagal baca DHT (NaN) - cek kabel/resistor pull-up");
      return;
    }
    Serial.printf("Suhu %.1f C | Lembap %.1f %%\n", t, h);
    if (WiFi.status() == WL_CONNECTED) kirimData(t, h);
  }
}
