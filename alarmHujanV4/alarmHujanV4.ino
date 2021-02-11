#include "WiFiEsp.h"
#include "ThingSpeak.h"
#include "SoftwareSerial.h"
#include "DHT.h"
#include <Servo.h>
#include "servo.h"

#define ssid "doyy"
#define pass "akuakuaku"
#define SECRET_CH_ID 1288460
#define SECRET_WRITE_APIKEY "CJSVB42KO69ZYWUD"

//int keyIndex = 0;
WiFiEspClient client;

#ifndef HAVE_HWSERIAL1
SoftwareSerial Serial1(7,6); //RX , TX

#define ESP_BAUDRATE 9600
#else
#define ESP_BAUDRATE 19200
#endif

unsigned long myProjectNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
String myStatus = "";


const int pinRainDrop = A0;
#define DHTPIN 8
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const int buzzer = 12;
const int led = 11;

Servo servoku;
int modeCount = 0;

void setup(){
  Serial.begin(115200);
  
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);

  pinMode(pinRainDrop, INPUT);
  dht.begin();
  pinMode(4, INPUT);

  

  setEspBaudRate(ESP_BAUDRATE);
  while(!Serial1){;}

  Serial.print("Mencari board ESP8266 : ");
  WiFi.init(&Serial1);
  if(WiFi.status() == WL_NO_SHIELD){
    Serial.println("Tidak di Temukan !");
    while(true);
    }
  Serial.println("Berhasil di Temukan ! ");

  ThingSpeak.begin(client);


  }

void loop(){
  servoku.attach(13);
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Menyambungkan ke : ");
    Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);
      Serial.print("Wait a minute..");
      delay(3000);
      }
      Serial.println("Terhubung");
    }

    delay(2000);
    // Sensor RainDrop
    int nilaiRainDrop = analogRead(pinRainDrop);
    Serial.println(nilaiRainDrop);

    // Sensor DHT11
    float t = dht.readTemperature(); // suhu
    float h = dht.readHumidity(); // kelembapan

    if(isnan(t) || isnan(h)){
      Serial.println("Periksa konfigurasi pin/kabelnya");
      Serial.println("Sensor Tidak Terbaca");
      return;
      delay(3000);
      }

    Serial.print("Suhu : ");
    Serial.print(t);
    Serial.print(" Celcius \n");
    Serial.print("Kelembapan : ");
    Serial.print(h);

    // set the fields with the values
    ThingSpeak.setField(1, nilaiRainDrop);
    ThingSpeak.setField(2, t);
    ThingSpeak.setField(3, h);

    if((nilaiRainDrop <= 500 || t <= 16) && modeCount < 3){
      servoku.write(180);
      servoku.detach();
      digitalWrite(led, HIGH);
      digitalWrite(buzzer, HIGH);
      delay(1000);
      modeCount = modeCount + 1; 
      }else if(modeCount = 3) {
        digitalWrite(buzzer, LOW);
        servoku.detach();
        if (nilaiRainDrop > 500 || t > 16){
          modeCount = 0;
          servoku.detach();
          }
        }else{
          servoku.write(0);
          servoku.detach();
          digitalWrite(led, LOW);
          digitalWrite(buzzer, LOW);
          }


    // write to the ThingSpeak Channel
    int x = ThingSpeak.writeFields(myProjectNumber, myWriteAPIKey);

    if(x == 200){
      Serial.println("Channel update successful..");
      }
      else{
        Serial.println("Problem updating channel. HTTP error code " );
        }
    delay(5000);
  }

  void setEspBaudRate(unsigned long baudrate){
    long rates[2] = {9600,19200};

    Serial.print("Setting baudrate ESP8266 ke : ");
    Serial.print(baudrate);
    Serial.println("...");

    for(int i=0; i<6; i++){
      Serial1.begin(rates[i]);
      delay(200);
      Serial1.print("AT+UART_DEF=");
      Serial1.print(baudrate);
      Serial1.print(",8,1,0,0\r\n");
      delay(200);
      }
      Serial1.begin(baudrate);
  }
