#include <Arduino.h>
#include <TM1637Display.h>
#include <stdint.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic


void conTimeService();
const int CLK = D6; //Set the CLK pin connection to the display
const int DIO = D5; //Set the DIO pin connection to the display
String ipAddress;
void updateClk();
bool timeSet=false;


String host = "worldclockapi.com";
int numCounter = 0;
uint8_t data[] = {0x0, 0x0, 0x0, 0x0};
uint8_t digit0,digit1,digit2,digit3 =0;

TM1637Display display(CLK, DIO); //set up the 4-Digit Display.
WiFiClient client;
int innerTime=0;
long checkTime=0;
bool colon=0;

class Clock {
 public:
    Clock();
    String getHours();
    String getMinutes();
    String getSeconds();
    void setHours(int);
    void setMinutes(int);
    void tick();
  private:
      int hours;
      int minutes;
      int seconds;
};

Clock::Clock() {
  hours=0;
  minutes=0;
  seconds=0;
}
String Clock::getHours() {
    if (hours<10) return "0"+String(hours);
    else return String(hours);
}
void Clock::setHours(int aValue){
    if (aValue<24) hours = aValue;
}
String Clock::getMinutes() {
  if (minutes<10) return "0"+String(minutes);
  else return String(minutes);
}
void Clock::setMinutes(int aValue){
    if (aValue<=59) minutes = aValue;
}
String Clock::getSeconds() {
    if (seconds<10) return "0"+String(seconds);
    else return String(seconds);
}
void Clock::tick (){
    seconds++;
    if(seconds>59) {
        seconds=0;
        minutes++;
        if (minutes>59) {
          minutes=0;
          hours++;
          if (hours>23) {
            hours=0;
          }
        }
    }
}
//
Clock myClock=Clock();

void setup()
{
 myClock.setHours(0);
 myClock.setMinutes(0);
 display.setBrightness(0x0f); //set the diplay to maximum brightness
 Serial.begin(9600);
 WiFiManager wifiManager;
 wifiManager.autoConnect();
}

void conTimeService ()
{
    Serial.println("client started");
    if (client.connect(host,80)) {
    client.println("GET /api/json/cet/now HTTP/1.1");
    client.println("Host:"+host);
    //client.print("Content-Length: ");
    //client.println(PostData.length());
    client.println();
    client.println();

    long interval = 2000;
    unsigned long currentMillis = millis(), previousMillis = millis();

    while(!client.available()){

      if( (currentMillis - previousMillis) > interval ){
        Serial.println("Timeout");
        client.stop();
        break;
      }
      currentMillis = millis();
    }
    // if there are incoming bytes available
    // from the server, read them and print them:
    String c="";
    String t="";
    String json="";
    StaticJsonBuffer<400> jsonBuffer;
    while(client.available())
    {
        json = client.readStringUntil('\r\n');
        Serial.println("content="+json);
        if (json.indexOf("{")==0) {
            JsonObject& root = jsonBuffer.parseObject(json);
            if (root.success()) {
                String currentTime=root["currentDateTime"];
                Serial.println("current time:"+currentTime);
                t="";
                t=currentTime.substring(currentTime.indexOf('T')+1,currentTime.indexOf('T')+6);
                Serial.println("current t:"+t);
            }
            else Serial.println("failed to parse!");
        }

        //if (c.indexOf('currentDayTime')>0) t=c.substring(c.indexOf('currentDayTime')+25,c.indexOf('currentDayTime')+30);
    }
    //include additional checks to avoid corrupted reads
    if (t!="") {
        myClock.setHours(t.substring(0,2).toInt());
        myClock.setMinutes(t.substring(3,5).toInt());
        timeSet=true;
      }
    }
    else
    {
      Serial.println("client failed to connect");
    }
}

//to do: settare un timer che ad ogni secondo chiama un interrupt routine
//nella routine chiamare tick() e fare lampeggiare i :

void updateClk() {
    colon=!colon;
    myClock.tick();
    Serial.println("Time:"+myClock.getHours()+":"+myClock.getMinutes()+":"+myClock.getSeconds());
    data[0]= display.encodeDigit(myClock.getHours()[0]);
    if (colon) data[1]= 0x80|display.encodeDigit(myClock.getHours()[1]);
    else data[1]= display.encodeDigit(myClock.getHours()[1]);
    data[2]= display.encodeDigit(myClock.getMinutes()[0]);
    data[3]= display.encodeDigit(myClock.getMinutes()[1]);
    display.setSegments(data);
}

void loop()
{
  if ((millis()-innerTime)>999)
  {
    updateClk();
    innerTime=millis();
    if(!timeSet) conTimeService();
  }

  if ((millis()-checkTime)>3600000) {
    //wake WiFi
    conTimeService();
    //shutdown wifi
    checkTime=millis();
  }
}
