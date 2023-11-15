#include <WiFi.h>
#include "bbq_comp.h"
#include <Preferences.h>
#include "html.h"
//#include "MAX31865.h"
#include "max6675.h"// install the library i added in this repo
#include <SPI.h>

#include <WiFiClientSecure.h>
#include "UniversalTelegramBot.h"
#include <ArduinoJson.h>

#define MOSI_PIN 23
#define MISO_PIN 19
#define SCK_PIN 18
// #define SS_PIN  5
#define SS_PIN 5

// #define BOTtoken "6475475496:AAGiEH1D5LNidrHZdCYOsiXtEDvnF14tJYY" // your Bot Token (Get from Botfather)
// // Use @myidbot to find out the chat ID of an individual or a group
// // Also note that you need to click "start" on a bot before it can
// // message you
// #define CHAT_ID "6176430424"
Preferences preferences;
MAX6675 pt100(SCK_PIN, SS_PIN, MISO_PIN);

int highValue = -1;
int lowValue = -1;
int readyValue = -1;
int currentValue = -1;
double temp_offset = 0;
bool change_mode = false;
String lstate = "Normal";
bool change_bot = false;

String ssid = "Lento 502";     // Place your home wifi ssid
String password = "staylento"; //// Place your home wifi password
String ap_ssid = "BBQ-Network";
String ap_password = "bbq-network";
String mode = "1";
String BOTtoken = "";
String CHAT_ID = "";
int sOn = false;
int lOn = false;
int liquid_state = 0;
WiFiServer webServer(80);

WiFiClientSecure client1;
UniversalTelegramBot bot(BOTtoken, client1);

String header;

// Current time
unsigned long currentTime = millis();
unsigned long currentread = millis();
unsigned long previousSend = millis();
bool sent_high = false;
bool sent_low = false;
bool sent_ready = false;
bool sent_liq = false;
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds
const long timeoutTime = 500;
String getsub(String s)
{
  String s1 = s;
  int idx = 0;

  for (int i = 0; i < s1.length(); i++)
  {
    if (s1.charAt(i) == '.')
    {
      idx = i;
      Serial.println(idx);
    }
  }
  Serial.println("Last ip:");
  Serial.println(idx);
  s1.remove(idx);
  Serial.println(s1);
  return s1;
}

String endcodehtml(String s)
{
  String de[] = {"%23", "%24", "%25", "%26", "%2B", "%2C", "%3A", "%3F", "%40"};
  String en[] = {"#", "$", "%", "&", "+", ",", ":", "?", "@"};
  String s1 = s;
  for (int i = 0; i < 9; i++)
  {

    if (s1.indexOf(de[i]) != -1)
    {
      s1.replace(de[i], en[i]);
    }
  }

  Serial.println(s1);
  return s1;
}

int testWifi(int timeretries)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // disconnect to scan wifi
  delay(100);
  //WiFi.begin(ssid, password);
  while (c < timeretries)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("WiFi connected.");

      uint8_t gw[4], sbnet[4], gdns[4];
      for (int i = 0; i < 4; i++)
      {
        gw[i] = WiFi.gatewayIP()[i];
        sbnet[i] = WiFi.subnetMask()[i];
        gdns[i] = WiFi.dnsIP(0)[i];
      }

      // Serial.print("Subnet Mask: ");
      // Serial.println(sbnet);
      // Serial.print("Gateway IP: ");
      // Serial.println(gw);
      // Serial.print("DNS 1: ");
      // Serial.println(gdns);
      // Serial.print("DNS 2: ");
      // Serial.println(WiFi.dnsIP(1));

      IPAddress staticIP(gw[0], gw[1], gw[2], 250);
      IPAddress gateway(gw[0], gw[1], gw[2], gw[3]);
      IPAddress subnet(sbnet[0], sbnet[1], sbnet[2], sbnet[3]);
      IPAddress dns(gdns[0], gdns[1], gdns[2], gdns[3]);

      if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false)
      {
        Serial.println("Configuration failed.");
      }

     // WiFi.begin(ssid, password);

      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.print("Connecting...\n\n");
      }
      Serial.println("Static IP:");
      Serial.println(WiFi.localIP());
      return (1);
    }
    if (c >= timeretries)
    {
      break;
    }
    delay(500);
    Serial.println(WiFi.status());
    c++;
  }
  return (0);
} // end testwifi

void setupAP(void)
{
  WiFi.mode(WIFI_STA); // mode STA
  WiFi.disconnect();   // disconnect to scan wifi
  delay(100);

  Serial.println("");
  delay(100);
 // WiFi.softAP(ap_ssid, ap_password); // change to AP mode with AP ssid and APpass
  Serial.println("softAP");
  Serial.println("");
  Serial.println(WiFi.softAPIP());

  // if(setAPcount < 1)
  // {

  // }
}

void setup()
{

  Serial.begin(9600);
  // getsub();
  double temp = pt100.readCelsius();
  Serial.print("Temperature: ");
  Serial.println(temp);
  preferences.begin("bbqx", false);

  ssid = preferences.getString("ssid", ssid);
  password = preferences.getString("password", password);

  ap_ssid = preferences.getString("ap_ssid", ap_ssid);
  ap_password = preferences.getString("ap_password", ap_password);
  mode = preferences.getString("mode", mode);

  highValue = preferences.getInt("highValue", highValue);
  lowValue = preferences.getInt("lowValue", lowValue);
  readyValue = preferences.getInt("readyValue", readyValue);
  temp_offset = preferences.getDouble("temp_offset", temp_offset);
  sOn = preferences.getBool("sOn", sOn);
  lOn = preferences.getBool("lOn", lOn);

  BOTtoken = preferences.getString("BOTtoken", BOTtoken);
  CHAT_ID = preferences.getString("CHAT_ID", CHAT_ID);

  Serial.println("");
  Serial.println(ssid);
  Serial.println(password);
  Serial.println("");
  Serial.println(ap_ssid);
  Serial.println(ap_password);

  if (testWifi(20)) /*--- if the stored SSID and password connected successfully, exit setup ---*/
  {
    Serial.println("Station mode");
    delay(1000);
  }
  else /*--- otherwise, set up an access point to input SSID and password  ---*/
  {

    Serial.println("Connect timed out, opening AP");

    delay(2000);
    setupAP();
    Serial.println("Working on AP mode");
  }

  if (mode == "STA")
  {
    Serial.println("Working on STA mode");
  }
  if (mode == "2" && BOTtoken != "" && CHAT_ID != "")
  {
    bot.updateToken(BOTtoken);
    client1.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
    Serial.println("certificateing for api.telegram.org....");
  }
  webServer.begin();
  init_liquid_level();

  if (mode == "2" && BOTtoken != "" && CHAT_ID != "")
  {
    bot.sendMessage(CHAT_ID, "BBQ started up", "");
  }
}

void loop()
{
  if (change_mode == true)
  {
    if (mode == "2")
    {
      if (testWifi(20))
      {
        Serial.println("Connected to Wifi, Working in STA mode");
      }
      else
      {
        Serial.println("Can not connect to Wifi. Please check the input");
      }
    }
    else
    {
      setupAP();
    }
    change_mode = false;
  }
  if (change_bot == true)
  {
    Serial.println(BOTtoken);
    Serial.print(CHAT_ID);
    bot.updateToken(BOTtoken);
    client1.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
    Serial.println("certificateing for api.telegram.org....");
    if (mode == "2")
    {
      bot.sendMessage(CHAT_ID, "BBQ bot changged", "");
    }
    change_bot = false;
  }
  if (millis() - currentread > 1000)
  {

    currentValue = pt100.readCelsius() + double(temp_offset);
    // currentValue += 1;
    // if (currentValue > 30)
    // {
    //   currentValue = 0;
    // }
    currentread = millis();

    liquid_state = get_liquid_level_value();
    Serial.println("liquid_state:");
    Serial.println(liquid_state);
    Serial.println("Lquid:");
    Serial.println(lOn);
    if (mode == "2")
    {
      if (sOn == false)
      {
        if (currentValue < highValue)
        {
          sent_high = false;
        }
        if (currentValue > lowValue)
        {
          sent_low = false;
        }
        if (currentValue > highValue)
        {
          if (sent_high == false && BOTtoken != "" && CHAT_ID != "")
          {
            if (bot.sendSimpleMessage(CHAT_ID, "The temperature is very high", ""))
            {
              sent_high = true;
              // sent_low = false;
              // sent_ready =false;
            }
            else
            {
              sent_high = false;
            }
          }
        }
        else if (currentValue < lowValue)
        {
          if (sent_low == false && BOTtoken != "" && CHAT_ID != "")
          {
            if (bot.sendSimpleMessage(CHAT_ID, "The temperature is very low", ""))
            {
              // sent_high = false;
              sent_low = true;
              // sent_ready =false;
            }
            else
            {
              sent_low = false;
            }
          }
        }
        if (currentValue >= readyValue && currentValue < readyValue + 5)
        {
          if (sent_ready == false && BOTtoken != "" && CHAT_ID != "")
          {
            if (bot.sendSimpleMessage(CHAT_ID, "The temperature is Ready", ""))
            {
              sent_ready = true;
            }
            else
            {
              sent_ready = false;
            }
          }
        }
      }

      if (lOn == false)
      {
        if (liquid_state == 1)
        {
          Serial.println("Bottle Full");
          lstate = "Bottle Full";
          if (sent_liq == false && BOTtoken != "" && CHAT_ID != "" )
          {
            if (bot.sendSimpleMessage(CHAT_ID, "Bottle Full", ""))
            {
              sent_liq = true;
            }
            else
            {
              sent_liq = false;
            }
          }
          else
          {

            lstate = "Normal";
            Serial.println("Normal");
          }
        }
        else
        {
          sent_liq = false;
          lstate = "Liquid off";
        }

        if (mode == "2")
        {
          // futher function
        }
      }
    }
  }
  WiFiClient client = webServer.available(); // Listen for incoming clients

  if (client)
  { // If a new client connects,
    currentTime = millis();

    previousTime = currentTime;
    Serial.println("New Client."); // print a message out in the serial port
    String currentLine = "";       // make a String to hold incoming data from the client

    while (client.connected() && currentTime - previousTime <= timeoutTime)
    {
      // loop while the client's connected
      currentTime = millis();
      if (client.available())
      {

        String html = mainPage;

        html.replace("&&current", String(currentValue));
        // html.replace("&&lstate", String(lstate));

        // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        // Serial.write(c);        // print it out the serial monitor
        header += c;
        if (c == '\n')
        { // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            if ((header.indexOf("/getSensorValue") != -1) || (header.indexOf("/16/getSensorValue") != -1))
            {
              //  Serial.println("getSensorValue");
              // Sensör değeri alımını burada gerçekleştirin
              // float sensorValue = ; // Sensör verilerini okuyan işlev

              // İstemciye sensör değerini gönder
              client.println("HTTP/1.1 200 OK");
              client.println("Content-Type: text/plain");
              client.println("Connection: close");
              client.println();
              client.print(currentValue);

              // if (sOn) {
              //   client.print(currentValue);  // Sensör değerini yanıt olarak gönder
              // } else {
              //   client.print("Alarm Closed");
              //   client.print(currentValue);  // Sensör değerini yanıt olarak gönder
              // }
              // // client.print(String(currentValue));
              break;
            }

            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            // render introductions  page
            if ((header.indexOf("GET /16/introductions") >= 0) || (header.indexOf("GET /introductions") >= 0))
            {
              String introductionHtml = introductionPage;
              Serial.println("introductions");
              introductionHtml.replace("&&style", style);
              client.println(introductionHtml);
              break;
            }
            // render settings page
            if ((header.indexOf("GET /16/settings") >= 0) || (header.indexOf("GET /settings") >= 0))
            {
              String settingsHtml = settingsPage;
              Serial.println("Settings page method get");
              settingsHtml.replace("&&style", style);
              settingsHtml.replace("&&pwd", password);
              settingsHtml.replace("&&ssid", ssid);

              client.println(settingsHtml);
              break;
            }
            if ((header.indexOf("GET /16/savesettings") >= 0) || (header.indexOf("GET /savesettings") >= 0))
            {
              Serial.println("Settings page method get");

              String newSsid = getQueryParams(header, "ssid=", "&");
              newSsid.replace("+", " ");
              newSsid = endcodehtml(newSsid);
              String newPwd = getQueryParams(header, "pwd=", "&");
              newPwd = endcodehtml(newPwd);
              String newMode = getQueryParams(header, "mode=", " ");
              change_mode = true;
              mode = newMode;
              if (newMode == "2")
              {
                ssid = newSsid;
                password = newPwd;
                preferences.putString("ssid", newSsid);
                preferences.putString("password", newPwd);
                preferences.putString("mode", newMode);
              }
              if (newMode == "1")
              {
                ap_ssid = newSsid;
                ap_password = newPwd;
                preferences.putString("ap_ssid", newSsid);
                preferences.putString("ap_password", newPwd);
                preferences.putString("mode", newMode);
              }

              String settingsHtml = settingsPage;

              settingsHtml.replace("&&style", style);
              settingsHtml.replace("&&ssid", newSsid);
              settingsHtml.replace("&&pwd", newPwd);

              preferences.putString("ssid", newSsid);
              preferences.putString("pass", newPwd);
              preferences.putString("mode", newMode);

              client.println(settingsHtml);
              break;
            }

            // STILL TEMP KAPATILDI
            if ((header.indexOf("GET /16/sOff") >= 0) || (header.indexOf("GET /sOff") >= 0))
            {
              Serial.println("sOff");
              sOn = false;
              preferences.putBool("sOn", sOn);
            }
            // STILL TEMP ACILDI
            else if ((header.indexOf("GET /16/sOn") >= 0) || (header.indexOf("GET /sOn") >= 0))
            {
              Serial.println("sOn");
              sOn = true;
              preferences.putBool("sOn", sOn);
            }
            // LIQUD KAPATILDI
            else if ((header.indexOf("GET /16/lOff") >= 0) || (header.indexOf("GET /lOff") >= 0))
            {
              Serial.println("lOff");
              lOn = false;
              preferences.putBool("lOn", lOn);
            }
            // LIQUD ACILDI
            else if ((header.indexOf("GET /16/lOn") >= 0) || (header.indexOf("GET /lOn") >= 0))
            {
              Serial.println("lOn");
              lOn = true;
              preferences.putBool("lOn", lOn);
            }
            // RESTART
            else if ((header.indexOf("GET /16/restart") >= 0) || (header.indexOf("GET /restart") >= 0))
            {
              Serial.println("restart");
              ESP.restart();
            }
            else if ((header.indexOf("GET /16/wifiSetup") >= 0) || (header.indexOf("GET /wifiSetup") >= 0))
            {
              Serial.println("wifiSetup");
              // WİFİ AYARLAR SAYFASINA GÖNDER
            }
            else if ((header.indexOf("GET /16/savechatbot") >= 0) || (header.indexOf("GET /savechatbot") >= 0))
            {
              Serial.println("Chat bot setting");
              Serial.println("header");
              Serial.println(header);
              Serial.println("");
              change_bot = true;
              BOTtoken = getQueryParams(header, "token=", "&");
              BOTtoken.replace("+", " ");
              BOTtoken = endcodehtml(BOTtoken);
              Serial.println("BOTtoken:");
              Serial.println(BOTtoken);
              CHAT_ID = getQueryParams(header, "chatid=", " ");
              CHAT_ID.replace("+", " ");
              CHAT_ID = endcodehtml(CHAT_ID);
              Serial.println("CHAT ID:");
              Serial.println(CHAT_ID);
              preferences.putString("BOTtoken", BOTtoken);
              preferences.putString("CHAT_ID", CHAT_ID);

              // WİFİ AYARLAR SAYFASINA GÖNDER
            }

            else if ((header.indexOf("GET /16/set") >= 0) || (header.indexOf("GET /set") >= 0))
            {
              Serial.println("header");
              Serial.println(header);
              Serial.println("");

              String high = getQueryParams(header, "high=", "&");
              String low = getQueryParams(header, "low=", "&");
              String ready = getQueryParams(header, "ready=", "&");
              String offset = getQueryParams(header, "offset=", " ");

              Serial.print("High:");
              Serial.println(high);
              Serial.print("Low:");
              Serial.println(low);
              Serial.print("Ready:");
              Serial.println(ready);
              Serial.print("Temp Offset:");
              Serial.println(offset);
              highValue = high.toInt();
              lowValue = low.toInt();
              readyValue = ready.toInt();
              temp_offset = offset.toDouble();

              preferences.putInt("highValue", highValue);
              preferences.putInt("lowValue", lowValue);
              preferences.putInt("readyValue", readyValue);
              preferences.putDouble("temp_offset", temp_offset);
            }

            /// SAYFAYI YAZDIR
            html.replace("&&style", style);
            if (sOn == true)
            {
              html.replace("&&sButton", sOffButton);
              Serial.println("sOff");
            }
            else
            {
              html.replace("&&sButton", sOnButton);
              Serial.println("sOn");
            }

            if (lOn == true)
            {
              html.replace("&&lButton", lOffButton);
            }
            else
            {
              html.replace("&&lButton", lOnButton);
            }

            html.replace("&&high", String(highValue));
            html.replace("&&low", String(lowValue));
            html.replace("&&ready", String(readyValue));
            html.replace("&&offset", String(temp_offset));

            Serial.println("");
            // Serial.println("H, L, R");
            // Serial.println(highValue);
            // Serial.println(lowValue);
            // Serial.println(readyValue);
            // Serial.println("");

            html.replace("&&script", script);
            client.println(html);
            // Serial.println("html");
            //  Serial.println(html);
            // Serial.println("");

            break;
          }
          else
          { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    // Serial.println("Client disconnected.");
    // Serial.println("");
  }
}