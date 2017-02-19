/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <WiFiClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
const char* ssid = "TP-LINK_POCKET_3020_75A74A";
const char* password = "46885555";

#define SCAN_PERIOD 5*60*1000
long lastScanMillis;

//ADC_MODE(ADC_VCC); //vcc read

ESP8266WebServer server(80);



#define SEALEVELPRESSURE_HPA (1013.25)

#undef BME280_ADDRESS //reset library address
#define BME280_ADDRESS (0x77)
Adafruit_BME280 bme; // I2C

byte error, address;

//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

void setup() {
Serial.begin(115200);
  // start http server and connect to wifi access point
 WiFi.begin(ssid, password);
 // Wait for connection
 delay(5000);
  if (WiFi.status() == WL_CONNECTED) {
 
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  }
  else
  {
   Serial.print("NOT Connected to ");
  Serial.println(ssid);  
  }
  

Wire.begin(0, 2);
Wire.beginTransmission(BME280_ADDRESS);
if (error == 0)
{
Serial.print("I2C device found at address 0x");
Serial.print(BME280_ADDRESS,HEX);
}

if (!bme.begin(BME280_ADDRESS)) 
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

//BME gives wrong reading first time. So simply read data once and warm for 2 seconds

Serial.print("Temperature = ");
    float t = bme.readTemperature();
    Serial.print(t);
    Serial.println(" *C");

    float h = bme.readHumidity();
    Serial.print("Humidity = ");
    Serial.print(h);
    Serial.println(" %");
    
    Serial.print("Pressure = ");
    float p = bme.readPressure() / 100.0F;
    Serial.print(p);
    Serial.println(" hPa");
    delay(5000);
      
//Now handle client

server.on ( "/livefeed", serveFeed );
server.on ( "/", serveLivePage);
server.on ( "/setup", serveSetup);
server.on ( "/connect", serveConnect);
server.begin();
  Serial.println ( "First reading not sent to Thingspeak\nLocal HTTP server started" );
}



void loop() 
{
 // server.handleClient();
 long currentMillis = millis();
 if (currentMillis - lastScanMillis > SCAN_PERIOD)
  {
    readBME();
    Serial.print("\nWaiting for next reading after ... ");
    lastScanMillis = currentMillis;
  }
  
  
//delay(5*60*1000);//thngspeaks needs 15 sec delay at least, we upload every 5 minutes
 server.handleClient(); 
}



void serveConnect() {
 String nwrkstr = "";
 String pwdstr="";
 char* nwrk;
 char* pwd;
 
if (server.args() > 0 )  // Are there any POST/GET Fields ? 
    {
       
       for ( uint8_t i = 0; i < server.args(); i++ ) {  // Iterate through the fields
            if (server.argName(i) == "network") 
            {
              nwrkstr=server.arg(i);
             nwrk = new char[nwrkstr.length() + 1];
strcpy(nwrk, nwrkstr.c_str());
// do stuff

                  Serial.print("User entered network-->"+String(nwrk));
            }
            
     
       }
    }
  if (server.args() > 0 )  // Are there any POST/GET Fields ? 
    {
       
       for ( uint8_t i = 0; i < server.args(); i++ ) {  // Iterate through the fields
            if (server.argName(i) == "netpass") 
            {
                  pwdstr=server.arg(i);
                 pwd = new char[pwdstr.length() + 1];
strcpy(pwd, pwdstr.c_str());
// do stuff

                  Serial.print("User entered password-->"+String(pwd));
            }
            
     
       }
    }
String message = "";
    
               Serial.print("connecting to "+String(nwrk));
               WiFi.begin(nwrk, pwd);
               delay(6000);
               if(WiFi.status() == WL_CONNECTED) 
                 {
                   Serial.print("Connected to ");
  Serial.print(String(nwrk));
   message+="Connected to "+nwrkstr;
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
                 
                 }  
                 else
                 {
                  
                 Serial.print("NOT Connected to ");
  Serial.print(String(nwrk));   
  ESP8266WebServer server(80);
   message+="NOT Connected to "+nwrkstr+ "Check password or Wifi AP";  
      
                 }
 
               delete[] nwrk;
               delete[] pwd;

    message+=
"<html>\
  <head>\
       <title>";
       message+="Result</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  <body>";
  
    message+="<br><a href=http://">+WiFi.localIP()+">New Adress </a>";
     message+="<br><a href=/setup>Setup Wifi</a>";
    
   message+="</body>\
</html>",


  server.send ( 200, "text/html", message );   
}
void serveSetup() {
  
  String message = "";
    message+=
"<html>\
  <head>\
       <title>";
       message+="Setup - Classic Layout Weather</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  <body>";    

String netwks="";
  int numSsid = WiFi.scanNetworks();
  for (int thisNet = 0; thisNet < numSsid; thisNet++)
  {   
    
    netwks+="<INPUT type=\"radio\" name=\"network\" value=\""+WiFi.SSID(thisNet)+"\">"+WiFi.SSID(thisNet)+"==> " +WiFi.RSSI(thisNet)+"dBm<BR>";
        
    
  } 

   message+="<form action=/connect>";
  message+="Total networsks found :"+String(numSsid)+"<br>";
  message+=netwks;
  message+="<INPUT type=\"password\" name=\"netpass\" value=\"\">";
  message+="<INPUT type=\"submit\" value=\"Connect\"> <INPUT type=\"reset\">";
  
  message+="</form> ";
   
    message+="<br><a href=/>Home</a>";
   server.send ( 200, "text/html", message ); 
}
void serveLivePage() {


  
  
  
  float t = bme.readTemperature();
   float h = bme.readHumidity();    
   float p = bme.readPressure() / 100.0F;
  String message = "";
  
  message+=
"<html>\
  <head>\
    <meta http-equiv='refresh' content='10'/>\
    <title>";
    message+=String(t)+"c";
    message+=String(h)+"%";
    message+=String(p)+"hPa";
    message+="Classic Layout Weather</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from 4th Cross, Classic Layout!</h1>\
    This is a community service project by Bheema Upadhyaya, site 140.<br> Weather data is updated every 10 seconds. Enjoy weather!<br>\
    ";


    
    message+="Temperature : "+String(t)+ " *C<br>";
    message+="Humidity : "+String(h)+ " %<br>";
    message+="Pressure : "+String(p)+ " hPa<br>";
 message+="<br><a href=/setup>Setup Wifi</a>";
    
    message+="</body>\
</html>",


  server.send ( 200, "text/html", message );
  
}


void handleNotFound() {

  Serial.print("Serving 404 page");
 
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
  
}


void serveFeed()
{
   float t = bme.readTemperature();    
    float h = bme.readHumidity();    
    float p = bme.readPressure() / 100.0F;
    
String result="";
result+="temp="+String(t);
result+="&humidity="+String(h);
result+="&pressure="+String(p);
result+="&espmiilisec="+String(millis());

  server.send ( 200, "text/plain", result );
}
void readBME()
{
   
Serial.print("Temperature = ");
    float t = bme.readTemperature();
    Serial.print(t);
    Serial.println(" *C");

    float h = bme.readHumidity();
    Serial.print("Humidity = ");
    Serial.print(h);
    Serial.println(" %");
    
    Serial.print("Pressure = ");
    float p = bme.readPressure() / 100.0F;
    Serial.print(p);
    Serial.println(" hPa");

    //Serial.print("Approx. Altitude = ");
   // Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    //Serial.println(" m");

    //float v = ESP.getVcc() / 1000.0;
   // Serial.print(v);
    //Serial.println(" Volts");

    Serial.println("\nConnecting to thingsspeak.");

     //uploadThingsSpeak(t,h,p,v); 
      uploadThingsSpeak(t,h,p);  
    
}


//void uploadThingsSpeak(float d1,float d2,float d3,float d4) {
void uploadThingsSpeak(float d1,float d2,float d3) {
  static const char* host = "api.thingspeak.com";
  static const char* apiKey = "TA0ZBLRKDXA6T47A"; //username : bheemaupadhyaya password :Welcome123

  

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  while(client.available()){
    String line = client.readStringUntil('\n');
    Serial.println(line);
    Serial.println("\nRecd response from thingspeak. Uploading data..");
  }
  
  String url = "";
   url += "http://api.thingspeak.com/update";
  //url += streamId;
  url += "?key=";
  url += apiKey;
  url += "&field1=";
  url += d1;
  url += "&field2=";
  url += d2;
  url += "&field3=";
  url += d3;
//url += "&field4=";
  //url += d4;
  Serial.print("\nSending data via URL shwn below... ");
  Serial.println(url);


 client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
     Serial.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");          

  while(!client.available()) {
    delay(200); 
  }             

  while(client.available()){
    String line = client.readStringUntil('\n');
    Serial.println(line);
    
  }
  Serial.print("Data upload succesful.Next reading after 5 minutes....");             
}






