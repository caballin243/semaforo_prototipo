#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

//-----Wifi logica:-----
const char* wifi_name = "LAPTOP_Caballin243";
const char* wifi_pasword = "12345678";
WiFiClient clienteWifi;

//-----MQTT Logica:-----
const char* host = "192.168.137.1";
const int puerto = 1883;
const char* id_semaforo = "Semaforo_prototipo";
PubSubClient clientemqtt(clienteWifi);
char* topic_semaforos = "semaforos/calle/Av Quintana Roo/4";

void reconectar_mqtt(){
  while (!clientemqtt.connected()) {
    Serial.print("conectandose...");
    if (clientemqtt.connect(id_semaforo)) {
      Serial.println("Conectado. ");
    } else {
      Serial.print("ERROR, rc=");
      Serial.print(clientemqtt.state());
      Serial.println(" Intentando de nuevo en 5 seg");
      delay(5000);
    }
  }
  clientemqtt.subscribe(topic_semaforos);
}


void reconnectar_wifi() {
  Serial.print("Reconectando");
  while(WiFi.status() != WL_CONNECTED){
    WiFi.begin(wifi_name,wifi_pasword);
    
    delay(1000);
    Serial.print(".");
      
  }
  Serial.println("");
  Serial.println("Conexion lograda");
}

unsigned long time_emergencia = millis();
bool estado_emergencia = false;
void on_message(char* topic, byte* payload, unsigned int cantidad_de_caracteres_del_mensaje){
  char mensaje = (char)payload[0];
  if (mensaje == '1'){
    estado_emergencia=true;
    time_emergencia = millis();
  }
  else if (mensaje == '2'){
    estado_emergencia=false;
  }
}

//-----semaforo Prototipo:-----
#define verde 17
#define amarillo 16
#define rojo 15

void estado_rojo(){
  digitalWrite(verde,LOW);
  digitalWrite(amarillo,LOW);
  digitalWrite(rojo,HIGH);
}
void estado_amarillo(){
  digitalWrite(verde,LOW);
  digitalWrite(amarillo,HIGH);
  digitalWrite(rojo,LOW);
}
void estado_verde(){
  digitalWrite(verde,HIGH);
  digitalWrite(amarillo,LOW);
  digitalWrite(rojo,LOW);
}

unsigned long time_normal;
void estado_normal(){
  time_normal=millis();
}


//-----setup-----
void setup(){

//-----semaforo:-----
  pinMode(verde,OUTPUT);
  pinMode(amarillo,OUTPUT);
  pinMode(rojo,OUTPUT);

//-----mqtt:-----
  clientemqtt.setServer(host,puerto);
  clientemqtt.setCallback(on_message);
//-----wifi:-----
  WiFi.begin(wifi_name,wifi_pasword);
}


//-----loop:-----
void loop(){

//----------Reconectar wifi------------
  if (WiFi.status() != WL_CONNECTED){
    reconnectar_wifi();
  }
//---------Reconectar broker-----------
  if (!clientemqtt.connected()){
    reconectar_mqtt();
  }
  clientemqtt.loop();


//-----Estado de emergencia-----
  if(estado_emergencia){
    estado_verde();//aqui le arbimos el paso por 35 segundos o hasta que llegue un mensaje de emergencia terminada(un "2")
    if(millis()-time_emergencia>=25000){
      estado_emergencia = false;
      estado_normal();
    }
  }
  /*
  else{
    estado_normal();
  }
*/
//-----flujo normal del semaforo-----
  if(estado_emergencia==false){
    if(millis()-time_normal<=10000){
      estado_verde();
    }
    else if(millis()-time_normal<=15000){
      estado_amarillo();
    }
    else if(millis()-time_normal<=25000){
      estado_rojo();
    }
    else{
      estado_normal();
    }
  }
}