#include "DHT.h"
#include <Wire.h>
#include <EasyUltrasonic.h>

#include <ArduinoJson.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <EasyBuzzer.h>

/* Buzzer */
int buzzerPIN = 13;
unsigned int frequency = 1000;
unsigned int beeps = 10;
unsigned int duration = 1000;
/* Buzzer */

// Set the LCD address to 0x3F for a 16 chars and 2 line display
//LiquidCrystal_I2C lcd(0x3F,16,2);
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DHTPIN 2  // pin digital conectado al sensor DHT.
#define DHTTYPE DHT11 // Tipo de sensor DHT.

#define ECHOPIN 3 // pin digital conectado al pin echo del sensor ultrasónico.
#define TRIGPIN 3 //promedio de estudiantes de facultad ingenieria UMG  pin digital conectado al pin trig del sensor ultrasónico. 

/* START KEYPAD CONFIG */
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'D','#','0','*'},
  {'C','9','8','7'},
  {'B','6','5','4'},
  {'A','3','2','1'}
};

byte colPINS[COLS] = {4, 5, 6, 7};
byte rowPINS[ROWS] = {8, 9, 10, 11};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPINS, colPINS, ROWS, COLS);

char CLAVE[7];
char CLAVE_MAESTRA[7] = "123456";
byte INDICE = 0;
/* FINISH KEYPAD CONFIG */


EasyUltrasonic ultrasonic; // Define un objeto ultrasonic.
DHT dht(DHTPIN, DHTTYPE);  // Inicializa el sensor DHT.

bool abierto = false;
unsigned long previousMillis = 0;  // Variable para almacenar el tiempo de la última lectura
const unsigned long interval = 5000;  // Intervalo de tiempo deseado (2 segundos)

int distance = 10;
int Cel = 0;
int Far = 0;

void setup() {
  Serial.begin(9600);
  EasyBuzzer.setPin(buzzerPIN); // Configura el PIN donde esta conectado el Buzzer
  ultrasonic.attach(TRIGPIN, ECHOPIN, 2, 70); // Adjunta el sensor ultrasónico a los pines definidos arriba.
  dht.begin(); // Inicializa el sensor DHT y lo pone a trabajar.
  pinMode(12, OUTPUT);
}

void loop() {
  EasyBuzzer.update();
  // Keypad Test
  char customKey = customKeypad.getKey();

  if (customKey) {
    CLAVE[INDICE] = customKey;
    INDICE++;
    Serial.print(customKey);
  }
  if (INDICE == 6) {
    if(!strcmp(CLAVE, CLAVE_MAESTRA)){
      abierto = true;
      Serial.println(" Correcta");
    } else {
      abierto = false;
      digitalWrite(12, HIGH);
      delay(4000);
      digitalWrite(12, LOW);
      delay(4000);
      Serial.println("Incorrecta");
    }
    INDICE=0;
  }
  // Keypad Test

  unsigned long currentMillis = millis();  // Obtener el tiempo actual
  
  if (currentMillis - previousMillis >= interval) {  // Verificar si ha pasado el tiempo deseado
    previousMillis = currentMillis;  // Actualizar el tiempo de la última lectura

    float C = dht.readTemperature();     // variable para almacenar la temperatura en Celsius
    float F = dht.readTemperature(true); // variable para almacenar la temperatura en Fahrenheit
    float Hum = dht.readHumidity(); // Read humidity

    if (isnan(C) || isnan(F)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    distance = ultrasonic.getDistanceCM();
    Cel = roundf(C * 100) / 100;
    Far = roundf(F * 100) / 100;


    StaticJsonDocument<200> doc; // Se define variable doc para creación del JSON de respuesta.

    doc["Celcius"] = Cel;
    doc["Farenheit"] = Far;
    doc["distance"] = distance;
    doc["puerta"] = abierto; // Agrega el valor de la variable abierto al objeto doc

    serializeJson(doc, Serial); // Transmite por el puerto serial el JSON generado arriba.
    Serial.println();
    abierto = false;
  }
  if ((distance < 6) || (Cel > 30)){
    EasyBuzzer.beep(frequency, beeps);
    EasyBuzzer.stopBeep();
  }
}