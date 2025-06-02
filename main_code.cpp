#include <SPI.h>

#include "max6675.h"

#include <avr/wdt.h>
 
// 온도 핀

int thermoDO = 50;  // SO (Serial Out)

int thermoCS = 53;  // CS (Chip Select)

int thermoCLK = 52; // SCK (Serial Clock)

// MAX6675 객체 생성

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
 
 
// === Pin Definitions ===

const int coolant_pump1         = 32;  // heater쪽 펌프

const int coolant_pump2         = 31;  // sea쪽 펌프

const int coolant_heater        = 30;  // 히터

const int coolant_cbv1_power    = 24;  // heater쪽 밸브 파워

const int coolant_cbv1          = 25;  // heater쪽 밸브

const int coolant_cbv2_power    = 22;  // sea쪽 밸브 파워

const int coolant_cbv2          = 23;  // sea쪽 밸브
 
 
// function parameter

bool systemStarted = false;

bool heaterOn = false;

bool lastHeaterOn = false;
 
unsigned long high_temp_time = 0;

int elseCounter = 0;
 
 
void setup() {

  Serial.begin(9600);

  waitForStart();

  pinMode(coolant_pump1, OUTPUT);

  pinMode(coolant_pump2, OUTPUT);

  pinMode(coolant_heater, OUTPUT);

  pinMode(coolant_cbv1_power, OUTPUT);

  pinMode(coolant_cbv1, OUTPUT);

  pinMode(coolant_cbv2_power, OUTPUT);

  pinMode(coolant_cbv2, OUTPUT);
 
  initialize_mode();
 
  lastHeaterOn = heaterOn;
 
  Serial.println("If you want to start pump1, please press 'a'!");

  Serial.println("If you want to stop pump1, please press 'b'!");

  Serial.println("If you want to start pump2, please press 'c'!");

  Serial.println("If you want to stop pump2, please press 'd'!");

  Serial.println("If you want to start heater, please press 'H'!");

  Serial.println("If you want to stop heater, please press 'F'!");

  Serial.println("If you want to stop, please press 'T'!");
 
}
 
void loop() {

  // 정지

  if (Serial.available()) {

    char command_t = Serial.read();

    if (command_t == 't' || command_t == 'T') {

      stop_control();

    }

    else if (command_t == 'a' || command_t == 'A') {

      digitalWrite(coolant_pump1, HIGH);

    }

    else if (command_t == 'b' || command_t == 'B') {

      digitalWrite(coolant_pump1, LOW);

    }

    else if (command_t == 'c' || command_t == 'C') {

      digitalWrite(coolant_pump2, HIGH);

    }

    else if (command_t == 'd' || command_t == 'D') {

      digitalWrite(coolant_pump2, LOW);

    }     
 
    else if (command_t == 'h' || command_t == 'H') {

      if (!heaterOn) {

        digitalWrite(coolant_heater, HIGH);

        digitalWrite(coolant_cbv1, LOW);

        digitalWrite(coolant_pump1, HIGH);

        heaterOn = true;

        Serial.println("Heater ON");

      }

    }

    else if (command_t == 'f' || command_t == 'F') {

      if (heaterOn) {

        digitalWrite(coolant_heater, LOW);

        digitalWrite(coolant_cbv1, HIGH);

        digitalWrite(coolant_pump1, HIGH);

        heaterOn = false;

        Serial.println("Heater OFF");

      }

    }

    else {

      Serial.println("Unknown command.");

    }

  }
 
  if (heaterOn != lastHeaterOn) {

    Serial.println(heaterOn ? "ON" : "OFF");

    lastHeaterOn = heaterOn;

  }
 
  // 온도 측정

  float temp = thermocouple.readCelsius();

  Serial.print("섭씨: ");

  Serial.println(temp);
 
 
  delay(500);

}
 
 
void waitForStart() {

  Serial.println("If you want to start, please press 'S'!");

  while (!systemStarted) {

    if (Serial.available()) {

      char command = Serial.read();

      if (command == 'S' || command == 's') {

        systemStarted = true;

        Serial.println("Start!");

      } else {

        Serial.println("you didn't press 'S'..");

      }

    }

    delay(200);

  }

}
 
// === Initialization ===

void initialize_mode() {

  digitalWrite(coolant_cbv1_power, HIGH);

  digitalWrite(coolant_cbv2_power, HIGH);

  delay(500);

  digitalWrite(coolant_cbv1, HIGH);

  digitalWrite(coolant_cbv2, LOW);

}
 
 
// === Stop ===

void stop_control() {

  Serial.println("stopping the system..");

  digitalWrite(coolant_cbv1_power, LOW);

  digitalWrite(coolant_cbv2_power, LOW);

  digitalWrite(coolant_heater, LOW);

  delay(2000);

  systemReset();

}
 
void systemReset() {

  Serial.println("system reset");

  wdt_enable(WDTO_250MS);

  while (true) { }

}
 
 
