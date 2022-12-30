#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Servo.h>

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value =0;


//SSID
const char* ssid = "HuelepedosnetV2";
const char* password = "raspberry";

//Direccion del MQTT
const char* mqtt_server = "192.168.200.129";

//boton y led
const int pushButton = 19;
const int ledPin1=13; //pin GPIO13
int boton = 0;
int bpasado=0;
//Servo
static const int servoPin = 17;
Servo servo1;
int posDegrees = 0;

void setup()
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin1,OUTPUT);
  pinMode(pushButton, INPUT_PULLDOWN);

  //Servo
  servo1.attach(servoPin);

}

void servoUno() {
    servo1.write(posDegrees);
    Serial.println(posDegrees);  
}

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Conectando a... ");
  Serial.println(ssid);

  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print("...");
  }
  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP());
}

//realiza conexion mqtt (recibe datos)
void callback(char* topic, byte* message, unsigned int length)
{
  Serial.print("Mensaje recibido en topic:  ");
  Serial.print(topic);
  Serial.print(", Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) { 
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
   Serial.println();
  
  //primer output (topic: esp32/output1)
  if (String(topic) == "esp32/output1"){
    Serial.print("Cambio de salida: "); 
    
    if(messageTemp == "on")
    {
      Serial.println("on");
      digitalWrite(ledPin1, HIGH);
    }
    else if(messageTemp == "off")
    {
      Serial.println("off"); 
      digitalWrite(ledPin1, LOW);
    }
  }

  //Segundo output (topic: esp32/output2)
    if (String(topic) == "esp32/output2"){
    Serial.print("Cambio de salida: "); 
    if(messageTemp == "abre")
    {
      posDegrees = 180;
      servoUno();
    }
    else if(messageTemp == "cierra")
    {
      posDegrees = 90;    
      servoUno();
    }
  }

}

//Bucle hasta que se reconecte 

void reconnect()
{ 
while (!client.connected())
{
  Serial.print("Intentando conexion MQTT..."); 
  if (client.connect("ESP32Client"))
  {
    Serial.println("conectado"); 
    client.subscribe("esp32/output1"); //esp32/output1'********* 
    client.subscribe("esp32/output2"); //********Topic: 'esp32/output2'*********   }
  }
  else{
  Serial.print("Fallo, rc=");
  Serial.print(client.state());
  Serial.println(" Intentelo de nuevo en 5s"); 
  delay(5000);
  }
}
}


//realiza conexion mqtt (PUBLISHER)

void loop()
{ 
  if (!client.connected()){
    reconnect();
    }
  client.loop();
  long now = millis(); 
  if (now - lastMsg > 1000) //100ms (tiempo de muestreo),
   {
    lastMsg = now;

  bpasado = boton;
  boton = digitalRead(pushButton);

  if(bpasado!=boton){
  //Convertir el valor a char array
  char btnString[8];
  dtostrf(boton,1,2,btnString);
  Serial.print("boton:  ");
  Serial.println(btnString);
  client.publish("esp32/boton",btnString);
  }
  }

   
}
