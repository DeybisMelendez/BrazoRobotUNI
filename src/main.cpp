#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>

Servo servo1, servo2, servo3, servo4, servo5;

int pin1 = 13, pin2 = 12, pin3 = 14, pin4 = 27, pin5 = 26;

const char *ssid = "RobotArm";
const char *password = "12345678";

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

int servoAng2 = 90;
int servoAng3 = 90;

// HTML SEGURO Y REDUCIDO (funciona en todos los ESP32)
const char index_html[] PROGMEM = R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<title>Brazo Robótico</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<style>
body{
  background: #0d0d0d;
  margin: 0;
  padding: 0;
  font-family: "Arial", sans-serif;
  display: flex;
  flex-direction: column;
  height: 100vh;
  color: white;
}
h1{
  text-align: center;
  margin: 15px 0 10px;
  font-size: 26px;
}
.speed-box{
  text-align:center;
  padding:10px;
  margin-bottom:8px;
}
.speed-box input{
  width:80%;
}
.container{
  flex: 1;
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 12px;
  padding: 12px;
  box-sizing: border-box;
}

.btn{
  display: flex;
  justify-content: center;
  align-items: center;
  font-size: 22px;
  padding: 25px 10px;
  border-radius: 14px;
  background: rgba(255,255,255,0.05);
  border: 1px solid rgba(255,255,255,0.15);
  color: #fff;
  backdrop-filter: blur(6px);
  transition: 0.15s;
  user-select: none;
  touch-action: manipulation;
}
.btn:active{
  background: rgba(0,180,255,0.35);
  border-color: rgba(0,200,255,0.6);
  transform: scale(0.97);
}
.section-title{
  grid-column: span 2;
  text-align: center;
  margin-top: 10px;
  margin-bottom: -5px;
  font-size: 20px;
  color: #00c8ff;
  font-weight: bold;
}
</style>
</head>

<body>

<h1>BRAZO ROBÓTICO</h1>

<!-- CONTROL DE VELOCIDAD -->
<div class="speed-box">
  <label for="speed">Velocidad: </label><br>
  <input type="range" id="speed" min="1" max="10" value="3">
  <div id="speedValue">Velocidad actual: 3</div>
</div>


<div class="container">
  
  <div class="section-title">BASE</div>
  <div class="btn" id="bL">◀️ Izquierda</div>
  <div class="btn" id="bR">Derecha ▶️</div>

  <div class="section-title">BRAZO</div>
  <div class="btn" id="aU">⬆️ Subir</div>
  <div class="btn" id="aD">⬇️ Bajar</div>

  <div class="section-title">ANTEBRAZO</div>
  <div class="btn" id="fU">⬆️ Subir</div>
  <div class="btn" id="fD">⬇️ Bajar</div>

  <div class="section-title">MUÑECA</div>
  <div class="btn" id="mU">⬆️ Subir</div>
  <div class="btn" id="mD">⬇️ Bajar</div>

  <div class="section-title">PINZA</div>
  <div class="btn" id="pC">🤏 Cerrar</div>
  <div class="btn" id="pO">👐 Abrir</div>

  <div class="section-title">Linea</div>
  <button class="btn" onclick='lineup()' id="lineup">Arriba</button>
  <button class="btn" onclick='linedown()' id="linedown">Abajo</button>

</div>

<script>
var ws = new WebSocket("ws://" + location.hostname + ":81/");
ws.onopen = () => console.log("WS conectado");

// VALORES DE SERVO
let a=[90,90,90,90,90];
let step = 3;     // cambiable por slider
let tm=null;

// ACTUALIZAR VELOCIDAD DESDE SLIDER
document.getElementById("speed").addEventListener("input", function(){
  step = parseInt(this.value);
  document.getElementById("speedValue").innerText = "Velocidad actual: " + step;
});

function mv(s,d){
  if(tm) clearInterval(tm);
  tm = setInterval(()=>{
    a[s-1]+=d*step;
    if(a[s-1]<0) a[s-1]=0;
    if(a[s-1]>180) a[s-1]=180;
    ws.send(s+":"+a[s-1]);
  },80);
}

function stop(){
  if(tm){
    clearInterval(tm);
    tm=null;
  }
}

function bind(id,s,d){
  let b=document.getElementById(id);

  // Mouse
  b.onmousedown=()=>mv(s,d);
  b.onmouseup=stop;
  b.onmouseleave=stop;

  // Touch
  b.ontouchstart=(e)=>{ e.preventDefault(); mv(s,d); };
  b.ontouchend=stop;
}

function lineup() {
  ws.send("6:1");
}
function linedown() {
  ws.send("6:0");
}

// VINCULACIÓN DE BOTONES
bind("bL",1,-1);
bind("bR",1,+1);

bind("aU",2,+1);
bind("aD",2,-1);

bind("fU",3,-1);
bind("fD",3,+1);

bind("mU",4,+1);
bind("mD",4,-1);

bind("pC",5,-1);
bind("pO",5,+1);
</script>

</body>
</html>
)";

void moveServo(int id, int ang)
{
  Serial.printf("Mover servo %d a %d grados\n", id, ang);
  ang = constrain(ang, 0, 180);
  switch (id)
  {
  case 1:
    servo1.write(ang);
    Serial.print("Servo 1: ");
    break;
  case 2:
    servo2.write(ang);
    servoAng2 = ang;
    Serial.print("Servo 2: ");
    break;
  case 3:
    servo3.write(ang);
    servoAng3 = ang;
    Serial.print("Servo 3: ");
    break;
  case 4:
    servo4.write(ang);
    Serial.print("Servo 4: ");
    break;
  case 5:
    servo5.write(ang);
    Serial.print("Servo 5: ");
    break;
  }
  Serial.println(ang);
}

void onWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t len)
{
  if (type == WStype_TEXT)
  {
    String m = (char *)payload;
    int p = m.indexOf(':');
    int s = m.substring(0, p).toInt();
    int a = m.substring(p + 1).toInt();
    if (s == 6)
    {
      if (a == 1)
      {
        while (servoAng2 != 45 && servoAng3 != 45)
        {
          if (servoAng2 > 45)
          {
            servoAng2 -= 1;
          }
          else
          {
            servoAng2 += 1;
          }
          if (servoAng3 > 45)
          {
            servoAng3 -= 1;
          }
          else
          {
            servoAng3 += 1;
          }
          servo2.write(servoAng2);
          servo3.write(servoAng3);
          delay(10);
        }
      }
      else if (a == 0)
      {
        servo2.write(90);
        servo3.write(120);
      }
    }
    else
    {
      moveServo(s, a);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  servo1.attach(pin1);
  servo2.attach(pin2);
  servo3.attach(pin3);
  servo4.attach(pin4);
  servo5.attach(pin5);

  WiFi.softAP(ssid, password);
  Serial.print("AP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", []()
            { server.send(200, "text/html", index_html); });
  server.begin();

  webSocket.begin();
  webSocket.onEvent(onWsEvent);

  Serial.println("Sistema listo");

  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  servo5.write(90);
}

void loop()
{
  server.handleClient();
  webSocket.loop();
}
