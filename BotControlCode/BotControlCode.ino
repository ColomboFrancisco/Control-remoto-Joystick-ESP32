#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "index.h"


// Pines
const int SERVO_1_PIN = 35;
const int SERVO_2_PIN = 26;

// Posic.
const int POSICION_ABAJO = 0;
const int POSICION_ARRIBA = 45;


const int R_MotorForward = 12;
const int R_MotorBack = 13;

const int L_MotorForward = 14;
const int L_MotorBack = 27;

const char* ssid = "hola";
const char* password = "11111111";

WebServer server(80);                               // Web server on port 80
WebSocketsServer webSocket = WebSocketsServer(81);  // WebSocket server on port 81

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);

int pwmx = 0;   // Global controller input
int pwmy = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(R_MotorForward, OUTPUT);
  pinMode(R_MotorBack, OUTPUT);
  pinMode(L_MotorForward, OUTPUT);
  pinMode(L_MotorBack, OUTPUT);

  pinMode(SERVO_1_PIN, OUTPUT);
  pinMode(SERVO_2_PIN, OUTPUT);

  controlarBrazo(POSICION_ABAJO);

  delay(500);

  // Create a Wi-Fi network with the SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Initialize WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Serve a basic HTML page with JavaScript to create the WebSocket connection
  server.on("/", HTTP_GET, []() {
    Serial.println("Web Server: received a web page request");
    String html = HTML_CONTENT;  // Use the HTML content from the index.h file
    server.send(200, "text/html", html);
  });

  server.begin();
  Serial.print("ESP Web Server's IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Handle client requests
  server.handleClient();

  // Handle WebSocket events
  webSocket.loop();

}

void manual_motor_control(int pwmx, int pwmy) {
  const int minInput = 60;

  //pwmy = -pwmy;
  //pwmx = pwmx/2;

  // Calculate motor speeds
  int motor1 = -(pwmy + pwmx);
  int motor2 = (pwmy - pwmx);

  // Clamp values to PWM range (0–255)
  motor1 = constrain(motor1, -255, 255);
  motor2 = constrain(motor2, -255, 255);
  Serial.println(motor1);
  Serial.println(motor2);

  //motor2 = motor2 * 0.90; //percentage of power
  
  // Motor 1 control  
  if (motor1 > minInput) {
    analogWrite(R_MotorForward, sqrt(motor1));
    analogWrite(R_MotorBack, 0);
    
  } else if (motor1 < -minInput) {
    analogWrite(R_MotorForward, 0);
    analogWrite(R_MotorBack, sqrt(-motor1));
    //Serial.println("Motor 1 backward");
  } else {
    analogWrite(R_MotorForward, 0);
    analogWrite(R_MotorBack, 0);
    //Serial.println("Motor 1 stop");
  }

  // Motor 2 control
  if (motor2 > minInput) {
    analogWrite(L_MotorForward, sqrt(motor2));
    analogWrite(L_MotorBack, 0);
    
  } else if (motor2 < -minInput) {
    analogWrite(L_MotorForward, 0);
    analogWrite(L_MotorBack, sqrt(-motor2));
    //Serial.println("Motor 2 backward");
  } else {
    analogWrite(L_MotorForward, 0);
    analogWrite(L_MotorBack, 0);
    //Serial.println("Motor 2 stop");
  }
}


void controlarBrazo(bool estado)
{
  if (estado)
  {
    analogWrite(SERVO_1_PIN, POSICION_ABAJO);
    analogWrite(SERVO_2_PIN, POSICION_ARRIBA);
    Serial.println("Posición de servos arriba");
  }
  else
  {
    analogWrite(SERVO_1_PIN, POSICION_ARRIBA);
    analogWrite(SERVO_2_PIN, POSICION_ABAJO);
    Serial.println("Posición de servos abajo");
  }
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;

    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      }
      break;

    case WStype_TEXT: {
      String message = String((char*)payload);
      message.trim();
      //Serial.printf("[%u] Received text: %s\n", num, message.c_str());

      if (message.startsWith("x")) {
        pwmx = message.substring(1).toInt();   // take number after 'x'
        //Serial.printf("pwmx updated: %d\n", pwmx);
      } 
      else if (message.startsWith("y")) {
        pwmy = message.substring(1).toInt();   // take number after 'y'
        //Serial.printf("pwmy updated: %d\n", pwmy);
      }
      else if (message.startsWith("s")){
        if (message.substring(1) == "true"){
          controlarBrazo(true);
        }
        else{
          controlarBrazo(false);
        }
      }
      manual_motor_control(pwmx, pwmy);
      break;
    }
  }
}

