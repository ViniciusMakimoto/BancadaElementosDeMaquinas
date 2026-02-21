/* ESP32 HTTP IoT Server Example for Wokwi.com

  This version serves a gzipped HTML file for efficiency.
  The HTML is embedded for Wokwi simulation and served from
  LittleFS on a physical device.
*/
// #include <Arduino.h>
// #include <WiFi.h>
// #include <WiFiClient.h>
// #include <WebServer.h>
// #include <uri/UriBraces.h>

// #ifdef WOKWI_EMU
// // For Wokwi simulation, include the auto-generated HTML header.
// #include "embedded_html.h"
// #else
// // For physical devices, use LittleFS.
// #include "FS.h"
// #include <LittleFS.h>
// #endif

// #define WIFI_SSID "Wokwi-GUEST"
// #define WIFI_PASSWORD ""
// // Defining the WiFi channel speeds up the connection:
// #define WIFI_CHANNEL 6

// WebServer server(80);

// const int LED1 = 26;
// const int LED2 = 27;

// bool led1State = false;
// bool led2State = false;

// void sendHtml()
// {
//   server.sendHeader("Content-Encoding", "gzip");

// #ifdef WOKWI_EMU
//   // Send the gzipped byte array from PROGMEM
//   server.send_P(200, "text/html", (const char *)index_html_gz, index_html_gz_len);
// #else
//   // Stream the gzipped file from LittleFS
//   File file = LittleFS.open("/index.html.gz", "r");
//   if (!file)
//   {
//     server.send(500, "text/plain", "File not found. Make sure to upload the gzipped filesystem image.");
//     return;
//   }
//   server.streamFile(file, "text/html");
//   file.close();
// #endif
// }

// void sendChartJs()
// {
//   server.sendHeader("Content-Encoding", "gzip");

// #ifdef WOKWI_EMU
//   // Send the gzipped byte array from PROGMEM
//   server.send_P(200, "application/javascript", (const char *)chart_umd_min_js_gz, chart_umd_min_js_gz_len);
// #else
//   // Stream the gzipped file from LittleFS
//   File file = LittleFS.open("/lib/chart.umd.min.js.gz", "r");
//   if (!file)
//   {
//     server.send(404, "text/plain", "File not found.");
//     return;
//   }
//   server.streamFile(file, "application/javascript");
//   file.close();
// #endif
// }

// void setup(void)
// {
//   Serial.begin(115200);
//   pinMode(LED1, OUTPUT);
//   pinMode(LED2, OUTPUT);

// #ifndef WOKWI_EMU
//   if (!LittleFS.begin(true))
//   {
//     Serial.println("LittleFS Mount Failed!");
//     return;
//   }
// #endif

//   WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
//   Serial.print("Connecting to WiFi ");
//   Serial.print(WIFI_SSID);
//   // Wait for connection
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(100);
//     Serial.print(".");
//   }
//   Serial.println(" Connected!");

//   Serial.print("IP address: ");
//   Serial.println(WiFi.localIP());

//   server.on("/", sendHtml);
//   server.on("/lib/chart.umd.min.js", sendChartJs);

//   server.on(UriBraces("/toggle/{}"), []()
//             {
//     String led = server.pathArg(0);
//     Serial.print("Toggle LED #");
//     Serial.println(led);

//     switch (led.toInt()) {
//       case 1:
//         led1State = !led1State;
//         digitalWrite(LED1, led1State);
//         break;
//       case 2:
//         led2State = !led2State;
//         digitalWrite(LED2, led2State);
//         break;
//     }

//     // Since the page is now dynamic with JS, we just send a confirmation
//     // instead of the whole HTML again. A 204 No Content is efficient.
//     server.send(204, ""); });

//   server.begin();
//   Serial.println("HTTP server started (http://localhost:8180)");
// }

// void loop(void)
// {
//   server.handleClient();
//   delay(2);
// }
