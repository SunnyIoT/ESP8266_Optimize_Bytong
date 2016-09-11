#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <MicroGear.h>
#include <ESP8266HTTPClient.h>
SoftwareSerial swSer(14, 12);

/* Setting WIFI */
const char* ssid              = "ESPERT-3020";
const char* password          = "espertap";

/* Setting Notification */
String smartphone_key = "5681028219797504";
String mode1_message = "SUNNY%20:%20ferment";
String mode2_message = "SUNNY%20:%20watering";
String mode3_message = "SUNNY%20:%20light";
String mode4_message = "SUNNY%20:%20delis";

/* Setting NETPIE */
#define APPID                 "WebApp"
#define KEY                   "kuZyWfJrig1mb7E"
#define SECRET                "ZNMyde0RNtPR04SUz2kLS0woM"
char ALIAS[15]                = "14365118"; // Change your serial number device
char html_alias[15]           = "html_";
String netpie_incoming_text   = "\0";
int secure_incoming_text      = 0;

EspClass Esp;

WiFiClient client;
int timer = 0, state_connect  = 0, state_change_alias = 0;
MicroGear microgear(client);

/* Variable */
#define R_strip 15
#define G_strip 5
#define B_strip 13
int count_noti1 = 0, count_noti2 = 0, count_noti3 = 0, count_noti4 = 0;
int R_brightness = 0, G_brightness = 0, B_brightness = 0;
int fadeAmount = 1;
unsigned long previousMillis = 0;

int start_new                 = 0;  // Mode begin
int mod1_t_on                 = 4;  // Turn on water
int mod1_t_off                = 4;  // Turn off water
int mod2_t_f                  = 20; // Watering
int mod2_di                   = 10; // Height
int mod3_t_LED                = 4;  // Light
int t_ref                     = 26; // Temperature

int cmdistance, h, t, LED, mod, year1, month1, day1, hour1, minute1;
String json1;
int x = 0;
int st_b = 0;
int state_datalogger          = 0;
int day_datalogger            = 0;
int month_datalogger          = 0;
int year_datalogger           = 0;
int hour_datalogger           = 0;
int minute_datalogger         = 0;
/* End Variable */

/* Datalogger send to database */
int state_send_db             = 0;
const char* host              = "io.e28ad.com";
const int httpPort            = 80;

/* Setting JSON */
StaticJsonBuffer<200> jsonIncomingBuffer;
JsonObject& json_incoming     = jsonIncomingBuffer.createObject();

void receiver_data_website();

/* Function NETPIE */
void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
  Serial.print("Incoming message --> ");
  msg[msglen] = '\0';
  netpie_incoming_text = (char*)msg;
  Serial.println(netpie_incoming_text);
  JsonObject& json_incoming = jsonIncomingBuffer.parseObject(netpie_incoming_text);

  if (!json_incoming.success()) {
    //    Serial.println("parseObject() failed");
  } else {

    /* Compare value */
    if (json_incoming["set_temp"] != NULL) {
      t_ref = json_incoming["set_temp"];
      secure_incoming_text = 1; // state secure data
    }

    if (json_incoming["set_height"] != NULL) {
      mod2_di = json_incoming["set_height"];
      secure_incoming_text = 1; // state secure data
    }

    if (json_incoming["set_light"] != NULL) {
      mod3_t_LED = json_incoming["set_light"];
      secure_incoming_text = 1; // state secure data
    }

    if (json_incoming["set_on_water"] != NULL) {
      mod1_t_on = json_incoming["set_on_water"];
      secure_incoming_text = 1; // state secure data
    }

    if (json_incoming["set_off_water"] != NULL) {
      mod1_t_on = json_incoming["set_off_water"];
      secure_incoming_text = 1; // state secure data
    }
    if (json_incoming["set_water"] != NULL) {
      mod2_t_f = json_incoming["set_water"];
      secure_incoming_text = 1; // state secure data
    }

    if (secure_incoming_text == 1) {
      Serial.println(netpie_incoming_text);
      microgear.chat(html_alias, "receiver success");
      receiver_data_website();
      secure_incoming_text = 0;
    }

    jsonIncomingBuffer = StaticJsonBuffer<200>(); // Clear jsonBuffer
    //Serial.println(Esp.getFreeHeap());

  }

}
void onFoundgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Found new member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}
void onLostgear(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.print("Lost member --> ");
  for (int i = 0; i < msglen; i++)
    Serial.print((char)msg[i]);
  Serial.println();
}
void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
  Serial.println("Connected to NETPIE...");
  microgear.setAlias(ALIAS);
}

void process_realtime() {

  if (st_b == 1 && digitalRead(2) == 0) {
    start_new = 1;
    receiver_data_website();
    delay(100);
  }
  if (st_b == 0 && digitalRead(2) == 1) {
    start_new = 0;
    receiver_data_website();
    delay(100);
  }
  st_b = digitalRead(2);
  if (swSer.available()) {
    json1 = swSer.readString();
    x = 1;
  }

  if (x == 1) {

    // Check data not zero

    StaticJsonBuffer<500> jsonBuffer2;
    JsonObject& root = jsonBuffer2.parseObject(json1);
    cmdistance = root["cmdistance"];
    h = root["Humidity"];
    t = root["Temperature"];
    LED = root["LED"];
    mod = root["mod"];
    year1 = root["year"];
    month1 = root["month"];
    day1 = root["day"];
    hour1 = root["hour"];
    minute1 = root["minute"];

    if (t != 0 && h != 0) {
      Serial.println();
      Serial.print(cmdistance);
      Serial.print(" ");
      Serial.print(h);
      Serial.print(" ");
      Serial.print(t);
      Serial.print(" ");
      Serial.print(LED);
      Serial.print(" ");
      Serial.print(mod);
      Serial.print(" ");
      Serial.print(year1);
      Serial.print(" ");
      Serial.print(month1);
      Serial.print(" ");
      Serial.print(day1);
      Serial.print(" ");
      Serial.print(hour1);
      Serial.print(" ");
      Serial.print(minute1);
      Serial.println("");
      x = 0;

      /* Data Logger */
      day_datalogger      = day1;
      month_datalogger    = month1;
      year_datalogger     = year1;
      hour_datalogger     = hour1;
      minute_datalogger   = minute1;
      Serial.print(day_datalogger);
      Serial.print("/");
      Serial.print(month_datalogger);
      Serial.print("/");
      Serial.print(year_datalogger);
      Serial.print(" ");
      Serial.print(hour_datalogger);
      Serial.print(":");
      Serial.println(minute_datalogger);
      /* End Data Logger */
    }
  }

  // mode light rgb
  // source by http://access-excel.tips/wp-content/uploads/2015/02/rgb-color.jpg

  if (mod == 0)  {
    // fade sky
    fade_rgb(0, 0, 0);

    R_brightness = R_brightness + fadeAmount;
    G_brightness = G_brightness + fadeAmount;
    B_brightness = B_brightness + fadeAmount;

    if (R_brightness <= 0 || R_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    if (G_brightness <= 0 || G_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    if (B_brightness <= 0 || B_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
  }
  if (mod == 1)  {
    // fade sky
    fade_rgb(0, G_brightness, B_brightness);

    R_brightness = R_brightness + fadeAmount;
    G_brightness = G_brightness + fadeAmount;
    B_brightness = B_brightness + fadeAmount;

    if (R_brightness <= 0 || R_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    if (G_brightness <= 0 || G_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    if (B_brightness <= 0 || B_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }

    // send notification
    count_noti1++;
    if (count_noti1 == 1) {
      String msg = "http://www.espert.io/MySmartphone/send?key=" + smartphone_key + "&message=" + mode1_message;
      doHttpGet(msg);
      delay(2000);
    }
    count_noti1 = 2;
  }

  if (mod == 2) {
    count_noti1 = 0;
    // fade blue
    fade_rgb(0, 0, B_brightness);

    R_brightness = R_brightness + fadeAmount;
    G_brightness = G_brightness + fadeAmount;
    B_brightness = B_brightness + fadeAmount;

    if (R_brightness <= 0 || R_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    if (G_brightness <= 0 || G_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    if (B_brightness <= 0 || B_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }

    // send notification
    count_noti2++;
    if (count_noti2 == 1) {
      String msg = "http://www.espert.io/MySmartphone/send?key=" + smartphone_key + "&message=" + mode2_message;
      doHttpGet(msg);
      delay(2000);
    }
    count_noti2 = 2;
  }

  if (mod == 3) {
    count_noti2 = 0;
    // fade yellow
    fade_rgb(R_brightness, G_brightness, 0);

    R_brightness = R_brightness + fadeAmount;
    G_brightness = G_brightness + fadeAmount;
    B_brightness = B_brightness + fadeAmount;

    if (R_brightness <= 0 || R_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    if (G_brightness <= 0 || G_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    if (B_brightness <= 0 || B_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }

    // send notification
    count_noti3++;
    if (count_noti3 == 1) {
      String msg = "http://www.espert.io/MySmartphone/send?key=" + smartphone_key + "&message=" + mode3_message;
      doHttpGet(msg);
      delay(2000);
    }
    count_noti3 = 2;

  }

  if (mod == 4) {
    count_noti3 = 0;
    // fade green
    fade_rgb(0, G_brightness, 0);

    R_brightness = R_brightness + fadeAmount;
    G_brightness = G_brightness + fadeAmount;
    B_brightness = B_brightness + fadeAmount;

    if (R_brightness <= 0 || R_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    if (G_brightness <= 0 || G_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    if (B_brightness <= 0 || B_brightness >= 255) {
      fadeAmount = -fadeAmount;
    }

    // send notification
    count_noti4++;
    if (count_noti4 == 1) {
      String msg = "http://www.espert.io/MySmartphone/send?key=" + smartphone_key + "&message=" + mode4_message;
      doHttpGet(msg);
      delay(2000);
    }
    count_noti4 = 2;

  }
  count_noti4 = 0;


  timer += 1;
}

void receiver_data_website() {
  StaticJsonBuffer<200> jsonBuffer3;
  JsonObject& root = jsonBuffer3.createObject();
  JsonArray& data = root.createNestedArray("data");
  data.add(start_new);
  data.add(mod1_t_on);
  data.add(mod1_t_off);
  data.add(mod2_t_f);
  data.add(mod2_di);
  data.add(mod3_t_LED);
  data.add(t_ref);
  root.printTo(Serial);
  root.printTo(swSer);
  swSer.println();
  Serial.println();
  start_new = 0;
}

void send_datalogger() {
  String url = "/laravel/public/device_lost_connect/";
  url += ALIAS;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  Serial.println();
  Serial.println("closing connection");
}

/* Function FADE RGB led strip */
void fade_rgb(int r, int g, int b)  {
  r = map(r, 0, 255, 0, 1023);
  g = map(g, 0, 255, 0, 1023);
  b = map(b, 0, 255, 0, 1023);

  analogWrite(R_strip, r);
  analogWrite(G_strip, g);
  analogWrite(B_strip, b);
}

/* Function HTTP GET Notification */
void doHttpGet(String msg) {
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");

  http.begin(msg); // GET HTTP
  Serial.println(msg);

  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    Serial.print("[CONTENT]\n");

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

void setup() {
  pinMode(2, INPUT);
  pinMode(5, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(15, OUTPUT);
  microgear.on(MESSAGE, onMsghandler);
  microgear.on(PRESENT, onFoundgear);
  microgear.on(ABSENT, onLostgear);
  microgear.on(CONNECTED, onConnected);

  Serial.begin(9600);
  swSer.begin(9600);
  swSer.setTimeout(100);
  Serial.println("Starting...");
  fade_rgb(0, 0, 0);
  if (WiFi.begin(ssid, password)) {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address : ");
  Serial.println(WiFi.localIP());

  microgear.init(KEY, SECRET, ALIAS);
  microgear.connect(APPID);

  if (state_change_alias == 0) {
    strcat(html_alias, ALIAS);
    state_change_alias = 1;
  }
}

void loop() {
  if (Esp.getFreeHeap() >= 32000) {

    if (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      WiFi.begin(ssid, password);
      Serial.print("Connecting to ");
      Serial.print(ssid);
      Serial.println();
      int RETRY_CONNECTION = 0;
      while (WiFi.status() != WL_CONNECTED) {
        if (RETRY_CONNECTION == 30) {
          Serial.print(".");
          RETRY_CONNECTION = 0;
        }
        delay(50);
        RETRY_CONNECTION++;
      }
      Serial.println();
      Serial.println("WiFi connected");
    } else {
      if (state_send_db == 2) {
        send_datalogger();
        state_send_db = 0;
      } else {

        if (microgear.connected()) {

          if (state_connect == 0) {
            Serial.println("Microgear Connected");
            state_connect = 1;
          }

          microgear.loop();

          if (timer >= 1000) {
            microgear.chat(html_alias, json1);
            Serial.print("heap : ");
            Serial.println(Esp.getFreeHeap());
            json1 = "\0";
            timer = 0;
          } else {
            process_realtime();
            // add rgb light
          }
        } else {

          Serial.println("connection lost, reconnect...");

          if (timer >= 5000) {

            // Send data lost connect to database
            if (!client.connect(host, httpPort)) {
              Serial.println("connection failed");
              state_send_db = 2;
              //          return;
            } else {
              send_datalogger();
              state_send_db = 0;
            }

            microgear.connect(APPID);
            timer = 0;
            state_connect = 0;
          } else {
            timer += 100;
          }

        }

        delay(1);
      }

    } // End check wifi connect

  } else {
    ESP.restart();
  }

}
