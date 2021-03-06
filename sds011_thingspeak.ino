#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

SoftwareSerial swSer(D4, 12, false, 256);
const char* host = "api.thingspeak.com";
String url = "/update?api_key=put your key";   // Your Own Key here
const int httpPort = 80;

const char* ssid = "kipfa-class1_2.4G";            
const char* password = "classroom1";

String working(int pm25, int pm10) { 
  return(String("field1=")+String(pm25) +"&field2="+String(pm10));
}

void delivering(String payload) { 
  WiFiClient client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpPort)) {
    Serial.print("connection failed: ");
    Serial.println(payload);
    return;
  }
  String getheader = "GET "+ String(url) +"&"+ String(payload) +" HTTP/1.1";
  client.println(getheader);
  client.println("User-Agent: ESP8266 Kyuho Kim");  
  client.println("Host: " + String(host));  
  client.println("Connection: close");  
  client.println();

  Serial.println(getheader);
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }
  Serial.println("Done cycle.");
}

void connect_ap() {
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\n Got WiFi, IP address: ");
  Serial.println(WiFi.localIP());  
}

void setup() {
  Serial.begin(115200);
  swSer.begin(9600);
  connect_ap();

  Serial.println("\nSoftware serial test started");
}

int stat = 1;
int cnt = 0;
char buf[10];
int readytosend = 0;

void loop() {
  while (swSer.available() > 0) {
    char c;
    c = swSer.read();
    //Serial.print("stat="+ String(stat) +", "+ "cnt="+ String(cnt) +" ");
    //Serial.print(c, HEX);
    //Serial.println(" ");

    switch (stat) {
      case 1:
         if (c == 0xAA) stat = 2;
           break;
       case 2:
         if (c == 0xC0) stat =3;
         else stat = 1;
         break;
       case 3:
         buf[cnt++] = c;
         if (cnt == 7) stat = 4;
         break;
       case 4:
         if (c == 0xAB) {
          //check checusum
          stat = 1;
         }
         else {
          Serial.println("Eh? wrong tailer");
         }
         cnt = 0;
         int pm25 = buf[0] + 256*buf[1];
         int pm10 = buf[2] + 256*buf[3];
         Serial.print(String(pm25) +","+ String(pm10)+ "  ");
         
         if (readytosend++ > 15) {
            delivering(working(pm25, pm10));
            readytosend = 0;
         }
       
        break;
    }
  }
}
