#include <Arduino.h>
#include <TM1637Display.h>
#include <stdint.h>
#include <ESP8266WiFi.h>
void conTimeService();
const int CLK = D6; //Set the CLK pin connection to the display
const int DIO = D5; //Set the DIO pin connection to the display
String ipAddress;
void updateClk();
//------- WiFi Settings -------
char ssid[] = "Vodafone-56413954";       // your network SSID (name)
char password[] = "12345678";  // your network key

String host = "worldclockapi.com";
int numCounter = 0;
uint8_t data[] = {0x0, 0x0, 0x0, 0x0};
uint8_t digit0,digit1,digit2,digit3 =0;

TM1637Display display(CLK, DIO); //set up the 4-Digit Display.
//WiFiClientSecure client;
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
 myClock.setHours(23);
 myClock.setMinutes(58);
 Serial.begin(9600);
 display.setBrightness(0x0f); //set the diplay to maximum brightness
 // Set WiFi to station mode and disconnect from an AP if it was Previously
 // connected
 WiFi.mode(WIFI_STA);
 WiFi.disconnect();
 delay(100);
 // Attempt to connect to Wifi network:
 Serial.print("Connecting Wifi: ");
 Serial.println(ssid);
 WiFi.begin(ssid, password);
 int timeout=0;
 while ((WiFi.status() != WL_CONNECTED) and (timeout<100)) {
   Serial.print(".");
   Serial.print(WiFi.status());
   delay(500);
   timeout++;
 }
 if (WiFi.status() == WL_CONNECTED){
   Serial.println("");
   Serial.println("WiFi connected");
   display.setBrightness(0x07);
   Serial.println("IP address: ");
   IPAddress ip = WiFi.localIP();
   Serial.println(ip);
   ipAddress = ip.toString();
   conTimeService();
  }
  else Serial.println("WiFi connection FAILED");
}

void conTimeService ()
{
    Serial.println("client started");
    if (client.connect(host,80)) {
    client.println("GET /api/json/cet/now HTTP/1.1");
    client.println("Host:"+host);
    client.println("");
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
    while(client.available())
    {
    if (client.available()) {
      c = client.readStringUntil('\r');
      Serial.print(c);
      if (c.indexOf('currentDayTime')>0) t=c.substring(c.indexOf('currentDayTime')+25,c.indexOf('currentDayTime')+30);
      }
    }
    myClock.setHours(t.substring(0,2).toInt());
    myClock.setMinutes(t.substring(3,5).toInt());
    }
    else
    {
      Serial.println("client did not connect");
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
  //conTimeService();

  if ((millis()-innerTime)>999)
  {
    updateClk();
    innerTime=millis();
  }
  if ((millis()-checkTime)>360000) {
    checkTime=millis();
    conTimeService();
  }
  /*
  {
    colon=!colon;
    innerTime=millis();
    myClock.tick();
    Serial.println("Time:"+myClock.getHours()+":"+myClock.getMinutes()+":"+myClock.getSeconds());
    data[0]= display.encodeDigit(myClock.getHours()[0]);
    if (colon) data[1]= 0x80|display.encodeDigit(myClock.getHours()[1]);
    else data[1]= display.encodeDigit(myClock.getHours()[1]);
    data[2]= display.encodeDigit(myClock.getMinutes()[0]);
    data[3]= display.encodeDigit(myClock.getMinutes()[1]);
    display.setSegments(data);
  }*/

}
