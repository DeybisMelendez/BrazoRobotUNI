#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>
#include <LittleFS.h>

Servo servoBase, servoArm, servoForearm, servoWrist, servoGripper;

// Subir datos html a LittleFS antes de compilar: pio run -t uploadfs
// Base, Brazo, Antebrazo, Muñeca, Pinza
const int PIN_BASE = 13, PIN_ARM = 12, PIN_FOREARM = 14, PIN_WRIST = 27, PIN_GRIPPER = 26;

const char *ssid = "BrazoRoboticoUNI";
const char *password = "123456789";
// IP: 192.168.4.1

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void moveServo(int id, int ang)
{
  ang = constrain(ang, 0, 180);
  Serial.printf("Mover servo %d a %d grados\n", id, ang);
  switch (id)
  {
  case 1:
    servoBase.write(ang);
    break;
  case 2:
    servoArm.write(ang);
    break;
  case 3:
    servoForearm.write(ang);
    break;
  case 4:
    servoWrist.write(ang);
    break;
  case 5:
    servoGripper.write(ang);
    break;
  }
}

void onWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t len)
{
  if (type == WStype_TEXT)
  {
    String m = (char *)payload;
    int p = m.indexOf(':');
    int s = m.substring(0, p).toInt();
    int a = m.substring(p + 1).toInt();
    moveServo(s, a);
  }
}

void setup()
{
  Serial.begin(115200);

  servoBase.attach(PIN_BASE);
  servoArm.attach(PIN_ARM);
  servoForearm.attach(PIN_FOREARM);
  servoWrist.attach(PIN_WRIST);
  servoGripper.attach(PIN_GRIPPER);

  LittleFS.begin(true);

  WiFi.softAP(ssid, password);
  Serial.print("AP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", []()
            {
    File f = LittleFS.open("/control.html", "r");
    server.streamFile(f, "text/html");
    f.close(); });
  server.on("/old", []()
            {
    File f = LittleFS.open("/old.html", "r");
    server.streamFile(f, "text/html");
    f.close(); });
  server.begin();
  server.on("/pose", []()
            {
    File f = LittleFS.open("/pose.html", "r");
    server.streamFile(f, "text/html");
    f.close(); });
  server.begin();

  webSocket.begin();
  webSocket.onEvent(onWsEvent);

  Serial.println("Sistema listo");

  servoBase.write(90);
  servoArm.write(90);
  servoForearm.write(90);
  servoWrist.write(90);
  servoGripper.write(90);
}

void loop()
{
  server.handleClient();
  webSocket.loop();
}
