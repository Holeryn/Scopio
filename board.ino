/*
  WiFi Web Server LED Blink

 A simple web server that lets you blink an LED via the web.
 This sketch will print the IP address of your WiFi Shield (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 9.

 If the IP address of your shield is yourAddress:
 http://yourAddress/H turns the LED on
 http://yourAddress/L turns it off

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the WiFi.begin() call accordingly.

 Circuit:
 * WiFi shield attached
 * LED attached to pin 9

 created 25 Nov 2012
 by Tom Igoe
 */
#include <SPI.h>
#include <WiFi101.h>
#include <MKRMotorCarrier.h>

#include "arduino_secrets.h"

#define INTERRUPT_PIN 6

#define MOTOR_ROATATION 25

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup() {
  Serial.begin(115200);      // initialize serial communication
 // pinMode(9, OUTPUT);      // set the LED pin mode
    while(!Serial);

    if(controller.begin()){
        Serial.print("Mkr1000 motor shield connected");
    }else{
        Serial.print("Mkr1000 motor schield not connected");
    }

    Serial.println("Reboot");
    controller.reboot();
    delay(500);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network rape time btw: 
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWiFiStatus();                        // you're connected now, so print out the status    

  M1.setDuty(0);
  M2.setDuty(0);
}


void loop() {
  WiFiClient client = server.available();   // listen for incoming clients
  int dutyR=0;
  int dutyL=0;

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:+
            client.print("<a href=\"/UP\"><button>U</button></a>");
            client.print("<a href=\"/Forward\"><button>N</button></a><br>");
            client.print("<a href=\"/Left\"><button>O</button></a>");
            client.print("<a href=\"/Backward\"><button>S</button></a>");
            client.print("<a href=\"/Right\"><button>E</button></a><br>");
            client.print("<a href=\"/Down\"><button>D</button></a>");
            client.print("<a href=\"/Stop\"><button>Stop</button></a>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check gets
        if(currentLine.endsWith("GET /UP")){
            for(int i = 0; i <180; i+=5)
            {
                servo1.setAngle(i);
                Serial.print("Servo position:");
                Serial.print(i);   
            }
        }else if(currentLine.endsWith("GET /Down")){
            for(int i = 180; i>0; i-=5){
                servo1.setAngle(i);
                Serial.print("Servo position:");
                Serial.print(i);
            }
        }else if(currentLine.endsWith("GET /Forward")){
            for(int i = 0; i <= MOTOR_ROATATION; i++){
                dutyR += i;
                dutyL += i;
                M1.setDuty(dutyL);
                M2.setDuty(dutyR);
            }
        }else if(currentLine.endsWith("GET /Backward")){
            for(int i = MOTOR_ROATATION; i >= 0; i--){
                dutyL -= i;
                dutyR -= i;
                M1.setDuty(dutyL);
                M2.setDuty(dutyR);
            }
        }else if(currentLine.endsWith("GET /Right")){
            for(int i = 0; i <= (MOTOR_ROATATION + 20); i++){
                dutyR -= i;
                M2.setDuty(dutyR);
            }
        }else if(currentLine.endsWith("GET /Left")){
            for(int i = 0; i <= (MOTOR_ROATATION + 20); i++){
                dutyL -= i;
                M1.setDuty(dutyL);
            }
        }else if(currentLine.endsWith("GET /Stop"))
        {
            dutyL = 0;
            dutyR = 0;
            M1.setDuty(dutyL);
            M2.setDuty(dutyR);
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
    controller.ping();
    delay(50);
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
