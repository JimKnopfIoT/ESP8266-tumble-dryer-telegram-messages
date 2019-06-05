/* Arduino script for use with ESP8266 ESP-01 and ADS1115. This scipt let's your tumble-dryer (or whatever) inform you when dryer is finished.
   I use similar script for my washing machine.*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClientSecure.h>
#include <ESP8266TelegramBOT.h>
#include <Wire.h>
#define I2CAddressESPWifi 8
int x=32;
#include <Adafruit_ADS1015.h>

uint64_t sleepTimeS =  11400000000; // wait 3 hours (in µSeconds)
uint64_t sleepTime = 300000000; // wait 5 minutes (in µSeconds)
  
Adafruit_ADS1115 ads(0x48);
float V0 = 0.0;
float V1 = 0.0;

extern "C"
{ 
    #include "user_interface.h" 
}

#define inputPin 3 /* RX Pin on ESP-01 can set to LOW, > 3,3K (4,7k) resistor needed to pull down voltage after 
read HIGH state. !Don't use Serial output while using Pin 3 (RX) as Input with LOW state needed because
using Serialoutput will set RX Pin to HIGH! */

// Telegram Settings
// First create a bot with telegram's botfather and write down the bot settings. 
// Findout your own telegramID (this is the adminID to communicate with the bot).
// If you create a channel, findout the channelID (chatID).

#define botName "<botName>"  // for Bot use
#define botUserName "<botUserName>" // for Bot use
#define botToken "<like 123456789:AABBccDDeeFfgGHHiIjjKklLMmnNooPPqQrR" // for Bot use
//#define adminID "like 12345678" // your ID, use this ID if you want do talk directly to the bot
#define chatID "like -123456789" // channelID, use this if you want to talk to the bot via channel, (leading "-" needed)

// Wifi settings 
static char ssid[] = "<your-SSID";
static char password[] = "your-password";
static char hostname[] = "<your-hostname-for-ESP-01";



// ---------------------------------------------------------------------------------------------------------------

TelegramBOT bot(botToken, botName, botUserName);

#define NB_TRYWIFI    10

void setup() {
  Serial.begin(9600);
  while(!Serial) {} // Wait
  static int default_sda_pin = 2;
  static int default_scl_pin = 0;
  Wire.begin(default_sda_pin,default_scl_pin);
  ads.begin();

// Wire.begin(8);                // join i2c bus with address #8 as slave


 // client.setServer(mqtt_server, 1883); // disabled, enable if needed

  rst_info *rinfo = ESP.getResetInfoPtr();
  Serial.println(String("\nResetInfo.reason = ") + (*rinfo).reason + ": " + ESP.getResetReason()); 

  wifi_station_set_hostname(hostname);
  WiFi.begin(ssid, password);
  Serial.print("\n  Connecting to " + String(ssid) + " ");
  int _try = 0;
  // Wait until it is connected to the network
  while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
        }
  delay(300);
  _try++;
  if ( _try >= NB_TRYWIFI ) {
         Serial.println("Can't connect to wifi accesspoint, going into deepsleep!");
         delay(500);
         system_deep_sleep(sleepTime);
         delay(300);
         }
  Serial.println();       
  Serial.println("wifi connected.");  
  delay(300);
  Serial.println("IP-Address: " + WiFi.localIP().toString());
  Serial.println("Hostname:  " + String(hostname));
  delay(300);
  bot.begin();          // initialize telegram bot
  delay(300);
  bot.sendMessage(chatID, "Dryer online.", ""); // Bot <-> Channel
  Serial.println("Dryer online.");
  Serial.println("- - - - - - - - - - - - - - - - - - -\n");
  delay(300); 
  
  /* //disabled, enable if needed
    String macToStr(const uint8_t* mac)
    {
    String result;
    for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
    result += ':';
    }
    return result;
    } 
     //MQTT Connection
            void reconnect() {
            // Loop until we're reconnected
            while (!client.connected()) 
            {
            delay (300);
            Serial.println("Try MQTT connection...");
            // Generate client name based on MAC address and last 8 bits of microsecond counter
            String clientName;  
            clientName += " Dryer ";
            uint8_t mac[6];
            WiFi.macAddress(mac);
            clientName += macToStr(mac);
            Serial.print("Connecting to  ");
            Serial.print(mqtt_server);
            Serial.print(" as ");
            Serial.print(clientName);
            Serial.println();
            
            // Attempt to connect
            // If you do not want to use a username and password, change next line to
            if (client.connect((char*) clientName.c_str())) {
           // if (client.connect((char*) clientName.c_str()), mqtt_user, mqtt_password) {
            Serial.println("Connected.");
            //bot.sendMessage(chatID, "Connected to MQTT Server"); // Bot <-> Channel
            } else {
                    Serial.print("Connection failed, rc=");
                    Serial.print(client.state());
                    Serial.println(" retry in 3 Minutes");
                    // Wait 3 minutes before retrying
                    delay(500);
                    system_deep_sleep(sleepTime);
                    delay(300);
                    }
           Serial.println("- - - - - - - - - - - - - - - - - - -\n");  
           }
   }
 */
 
 }
 
 void check() {
  
{ // I2C scanner beginn
byte error, address;
 int nDevices;
 Serial.println("Scanning...");
 nDevices = 0;
 for(address = 1; address < 127; address++ )
 {
   // The i2c_scanner uses the return value of
   // the Write.endTransmisstion to see if
   // a device did acknowledge to the address.
   Wire.beginTransmission(address);
   error = Wire.endTransmission();
   if (error == 0)
   {
     Serial.print("I2C device found at address 0x");
     if (address<16)
       Serial.print("0");
     Serial.print(address,HEX);
     Serial.println("  !");
     nDevices++;
   }
   else if (error==4)
   {
     Serial.print("Unknow error at address 0x");
     if (address<16)
       Serial.print("0");
     Serial.println(address,HEX);
   }   
 }
 if (nDevices == 0)
   Serial.println("No I2C devices found\n");
 else
   Serial.println("done\n");
 delay(5000);           // wait 5 seconds for next scan
}  // I2C scanner end

  // ADS1115 settings
  int16_t adc0, adc1; //, adc2, adc3; // for SingleEnded
  int16_t results; // for Differential +/- measuring on 0,1
  
  /* Be sure to update this value based on the IC and the gain settings! */
  float multiplier = 0.1875F; // ADS1115  @ +/- 6.144V gain (16-bit results)

  //results = ads.readADC_Differential_0_1();  // for Differential +/- measuring on 0,1
  //Serial.print("Differential: "); Serial.print(results); Serial.print("("); Serial.print(results * multiplier); Serial.println("mV)"); // for Differential +/- measuring on 0,1

adc0 = ads.readADC_SingleEnded(0);
if (adc0 > 20000)
   {
     Serial.print("Dryer has finished!");
     bot.sendMessage(chatID, "Dryer has finished!", ""); // Bot <-> Channel 
     //delay (300);
     delay(1800000); // Wait 30 Minutes when done.
   } else { 
           Serial.print("Dryer still tumbling.");
          }
          
//V0 = (adc0 * 0.1875)/1000;
//adc1 = ads.readADC_SingleEnded(1);
//V1 = (adc1 * 0.1875)/1000;
//adc2 = ads.readADC_SingleEnded(2);
//adc3 = ads.readADC_SingleEnded(3);
Serial.print("A0: "); Serial.print(adc0); Serial.println(adc0 * multiplier); 
//Serial.print("A1: "); Serial.print(adc1); Serial.println(adc1 * multiplier); 
//Serial.print("AIN2: "); Serial.println(adc2);
//Serial.print("AIN3: "); Serial.println(adc3);
Serial.println(" ");
//bot.sendMessage(chatID, "AIN0: " + String(adc0) + "AIN1: " + String(adc1) + "AIN2: " + String(adc2) + "AIN3: " + String(adc3), "");          
//bot.sendMessage(chatID, "A0-Wert-T: " + String(adc0) + " = " + String(adc0 * multiplier) + " mV " + " ; " + "A1-Value-T: " + String(adc1) + " = " + String(adc1 * multiplier) + " mV", "");          


//bot.sendMessage(chatID, "Differential-Value: " + String(results) + " ; " + String(results * multiplier) + " mV", "");    // for Differential +/- measuring on 0,1      

delay(60000); // Check every 1 Minute // for testing check every 3 Minutes delay(180000);
}
 // }
void loop()
{
 check();
}

/* 16. Mai 2019, works perfekt 
 */
