// Saser Sensor code for Robojax.com
//#define DETECT 2 // pin 2 for  sensor
//#define ACTION 8 // pin 8 for action to do someting
//#define LASER 6 // pin 8 for action to do someting

#include <ESP8266WiFi.h>                  //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>                    //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>             //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>                  //https://github.com/tzapu/WiFiManager WiFi Configuration Magic


// Parameters
WiFiManager wm;                           // Wifi Manager
ESP8266WebServer server(80);
const String webPageBase = "<h1>ESP8266 Web Server</h1>";
String webPage = "";
const char smtpServerUrl[] = "mail.smtp2go.com";
WiFiClient espClient;

bool isCalibrationMode = false;
bool isReady = false;
const int pinLaser = 14;
const int pinDetector = 13;

void setup() {
  Serial.begin(9600);                     // Speed
  Serial.println("Laser Test App");
  pinMode(pinDetector, INPUT);            // Define detector input pin// 
  // Laser
  pinMode(pinLaser, OUTPUT);              // Define laser output pin
  digitalWrite(pinLaser, LOW);            // Turn on laser

  // Access-Point
//  if (wm.autoConnect("AGHAI-NAME") ) {
//    Serial.println("connected.");       // WiFi.localIP()
//    delay(5000);
//  }
  wm.autoConnect("AGHAI-NAME");           // Will stop here till connected
//  Serial.println(WiFi.localIP());
  server.begin();
  myRouter();
}

void loop() {
  server.handleClient();
  int detected = digitalRead(pinDetector);// read Laser sensor
  if( isReady && digitalRead(pinLaser) == HIGH) {
    if (digitalRead(pinDetector) == HIGH) {
      Serial.println("Laser detected");
    } else if (isCalibrationMode) {
      Serial.println("No laser Detection");
    } else {
      // No laser Detection - Someone cut the wire
      Serial.println("ALERT!!!!");
      turnDetection(false);
      sendingAlert();
    }
  }
  delay(200);  
}

void myRouter() {
  // Server handling
  server.on("/", [](){
    webPage = webPageBase + "<p>Detection "
               + "<a href=\"detection2on\"><button>ON</button></a>&nbsp;"
               + "<a href=\"detection2off\"><button>OFF</button></a>"
               + "<br/>"
               + "<a href=\"resetDetection\"><button>Reset Detection</button></a>"
               + "<br/>"
               + "<a href=\"calibrationMode\"><button>Calibration Mode</button></a>"
               + "</p>";
    server.send(200, "text/html", webPage);
  });
  server.on("/calibrationMode", [](){
    webPage = webPageBase + "<p>Calibration Mode ON</p>";
    server.send(200, "text/html", webPage);
    isCalibrationMode = true;
    turnDetection(true);
    Serial.println("Calibration is ON");
    delay(1000); 
  });
  server.on("/detection2on", [](){
    webPage = webPageBase + "<p>Detection ON</p>";
    server.send(200, "text/html", webPage);
    isCalibrationMode = false;
    turnDetection(true);
    delay(1000); 
  });
  server.on("/detection2off", [](){
    webPage = webPageBase + "<p>Detection OFF</p>";
    server.send(200, "text/html", webPage);
    isCalibrationMode = false;
    turnDetection(false);
    delay(1000); 
  });
  server.on("/resetDetection", [](){
    webPage = webPageBase + "<p>Reset Detection</p>";
    isCalibrationMode = false;
    turnDetection(true);
    server.send(200, "text/html", webPage);
    delay(5000); 
  });
}
void turnDetection(bool isOn) {
  if (isOn) {
    isReady = true;
    digitalWrite(pinLaser, HIGH);
    Serial.println("Detection is ON");
  } else {
    isReady = false;
    digitalWrite(pinLaser, LOW);
    Serial.println("Detection is OFF");
  }
}
void sendingAlert() {
  byte ret = sendEmail();
}

byte sendEmail() {
  if (espClient.connect(smtpServerUrl, 2525) == 1) {
    Serial.println(F("connected"));
  } 
  else 
  {
    Serial.println(F("connection failed"));
    return 0;
  }
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending EHLO"));
  espClient.println("EHLO www.example.com");
  if (!emailResp()) 
    return 0;
  //
  /*Serial.println(F("Sending TTLS"));
  espClient.println("STARTTLS");
  if (!emailResp()) 
  return 0;*/
  //  
  Serial.println(F("Sending auth login"));
  espClient.println("AUTH LOGIN");
  if (!emailResp()) 
    return 0;
  //  
  Serial.println(F("Sending User"));
  // Change this to your base64, ASCII encoded username
  /*
  For example, the email address test@gmail.com would be encoded as dGVzdEBnbWFpbC5jb20=
  */
  espClient.println("YWdoYWljYXJAZ21haWwuY29t"); //base64, ASCII encoded Username
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending Password"));
  // change to your base64, ASCII encoded password
  /*
  For example, if your password is "testpassword" (excluding the quotes),
  it would be encoded as dGVzdHBhc3N3b3Jk
  */
  espClient.println("c210cFBhc3M=");//base64, ASCII encoded Password
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending From"));
  // change to sender email address
  espClient.println(F("MAIL From: aghaicar@gmail.com"));
  if (!emailResp()) 
    return 0;
  // change to recipient address
  Serial.println(F("Sending To"));
  espClient.println(F("RCPT To: iofirag@gmail.com"));
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending DATA"));
  espClient.println(F("DATA"));
  if (!emailResp()) 
    return 0;
  Serial.println(F("Sending email"));
  // change to recipient address
  espClient.println(F("To:  iofirag@gmail.com"));
  // change to your address
  espClient.println(F("From: aghaicar@gmail.com"));
  espClient.println(F("Subject: ESP8266 ALERT e-mail\r\n"));
  espClient.println(F("This is is a test e-mail sent from ESP8266.\n"));
  espClient.println(F("Second line of the test e-mail."));
  espClient.println(F("Third line of the test e-mail."));
  //
  espClient.println(F("."));
  if (!emailResp()) 
    return 0;
  //
  Serial.println(F("Sending QUIT"));
  espClient.println(F("QUIT"));
  if (!emailResp()) 
    return 0;
  //
  espClient.stop();
  Serial.println(F("disconnected"));
  return 1;
}

byte emailResp() {
  byte responseCode;
  byte readByte;
  int loopCount = 0;

  while (!espClient.available()) {
    delay(1);
    loopCount++;
    // Wait for 20 seconds and if nothing is received, stop.
    if (loopCount > 20000) {
      espClient.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }

  responseCode = espClient.peek();
  while (espClient.available()) {
    readByte = espClient.read();
    Serial.write(readByte);
  }

  if (responseCode >= '4') {
    //  efail();
    return 0;
  }
  return 1;
}
