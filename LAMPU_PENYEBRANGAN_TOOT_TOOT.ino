#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#ifndef APSSID
#define APSSID "TOMBOL_PENYEBERANGAN"
#define APPSW "12345678"
#endif

const char *ssid = APSSID;
const char *password = APPSW;

#define merah   0 //D3
#define kuning  5//D1 
#define hijau   4//D2
#define buzer   14//D5
#define buton   12//D6
#define ledkonek  2 //D4 //2  //ini led indikator penanda bahwa ESP siap digunakan


byte durasimerah = 10; 
byte berhenti = 0;

unsigned long millis1 = 0;
unsigned long millis2 = 0;
int interval = 1000;

String pesan = "contoh pesan";
String merah2 = "";
String standby2 = "";
String statebuton2 = "";
String state = "";

bool statebuton = 0;
bool bunyi = 0;
bool standby = 0;

ESP8266WebServer server(80);


void setup() {
  Serial.begin(9600);

  pinMode(merah, OUTPUT);
  pinMode(kuning, OUTPUT);
  pinMode(hijau, OUTPUT);
  pinMode(buzer, OUTPUT);
  pinMode(ledkonek, OUTPUT);
  pinMode(buton, INPUT_PULLUP);

  digitalWrite(merah, LOW);
  digitalWrite(kuning, LOW);
  digitalWrite(hijau, LOW);
  digitalWrite(buzer, LOW);
  digitalWrite(ledkonek, HIGH);

  EEPROM.begin(512); //khusus untuk ESP, bagian ini wajib disertakan
  durasimerah = EEPROM.read(1);
  standby = EEPROM.read(2);

  Serial.print("Configuring access point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
 
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/pesan",handlePesan);
  server.on("/status",handleStatus);
  server.begin();
  Serial.println("HTTP server started");

  Serial.println("VOID SETUP *****************************************");
  Serial.print("DURASI MERAH = ");
  Serial.println(durasimerah);
  Serial.println("END SETUP ******************************************");
  
  delay(2000);
}
void handleStatus() {
  merah2 = String(durasimerah);
  standby2 = String(standby);
  statebuton2 = String(statebuton);
  state = merah2 + "|" + standby2 + "|" + statebuton2;
  Serial.println(state);
  server.send(200, "text/html", state);
}

void handlePesan() {
 pesan = server.arg("Text");
 String parsing = "PESAN DITERIMA : "+pesan;
 Serial.println(pesan);
 server.send(200, "text/html", parsing);
 
 if(pesan.substring(0,5) == "MERAH"){
  durasimerah = pesan.substring(5,pesan.length()).toInt();
  Serial.println("SETTING MERAH : ");
  Serial.println(durasimerah);
  EEPROM.write(1, durasimerah);
  EEPROM.commit();
  //digitalWrite(buzer, HIGH);
  delay(1000);
  pesan = "";
 }
 if(pesan.substring(0,5) == "STNBY"){
  standby = pesan.substring(5,pesan.length()).toInt();
  Serial.println("SETTING STANDBY : ");
  Serial.println(standby);
  EEPROM.write(2, standby);
  EEPROM.commit();
  delay(1000);
  pesan = "";
 }
 
}
void loop() {
  server.handleClient();
  digitalWrite(ledkonek, LOW);
  
  if(!digitalRead(buton)){
    delay(100);
    if (!digitalRead(buton))
    {
     if(statebuton == 0){ 
      digitalWrite(kuning, HIGH);
      digitalWrite(merah, LOW);
      digitalWrite(hijau, LOW);
      delay(2000);
      statebuton = 1;
      berhenti = durasimerah;
     }
    }
  } 
  
  if(millis() - millis1 >= 1000){ //ini artinya sama seperti delay(1000), hanya saja penggunaannya tidak mempengaruhi pembacaan data pada ESP
    millis1 = millis();
    if(statebuton == 1){
      if(berhenti > 0){
        berhenti--;
        digitalWrite(merah, HIGH);
        digitalWrite(kuning, LOW);
        digitalWrite(hijau, LOW);
        if(berhenti <= durasimerah / 2){
          interval = 400;//interval - 100;
        }
      }
      if(berhenti <= 0){
        berhenti = 0;
        statebuton = 0;
        interval = 1000;
        digitalWrite(buzer, LOW);
      }
    }
    if(statebuton == 0){
      interval = 1000;
      bunyi = !bunyi;
      if(standby == 0){
        digitalWrite(hijau , HIGH);
        digitalWrite(merah, LOW);
        digitalWrite(kuning, LOW);
      }
      if(standby == 1){
        digitalWrite(kuning , bunyi);
        digitalWrite(merah, LOW);
        digitalWrite(hijau, LOW);
      }
    }
  }

  if(millis() - millis2 >= interval){
     millis2 = millis();
     if(statebuton == 1){
      bunyi = !bunyi;
      digitalWrite(buzer, bunyi);
     }
     if(statebuton == 0){
      digitalWrite(buzer, LOW);
     }
  }

}
