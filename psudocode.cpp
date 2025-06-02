#include <SPI.h>

#include "max6675.h"

#include <avr/wdt.h>
 
// === Pin Definitions ===

const int coolant_pump1         = 22;  // heater쪽 펌프

const int coolant_pump2         = 23;  // sea쪽 펌프

const int coolant_heater        = 24;  // 히터

const int coolant_cbv1_power    = 25;  // heater쪽 밸브 파워

const int coolant_cbv1          = 26;  // heater쪽 밸브

const int coolant_cbv2_power    = 27;  // sea쪽 밸브 파워

const int coolant_cbv2          = 28;  // sea쪽 밸브

const int coolant_temperature   = A0;  // 온도 센서

const int hydrogen_in_valve     = 29;  // 수소 입력 벨브

const int hydrogen_drain_valve  = 30;  // 수소 드레인 벨브

const int hydrogen_leak         = A1;  // 수소 유출 센서

const int oxygen_compressor     = 31;  // 산소 컴프레서

const int oxygen_drain_valve    = 32;  // 산소 드레인 벨브
 
const int stack_current_sensor  = A2;  // stack 전류 센서

const int stack_voltage_sensor  = A3;  // stack 전압 센서
 
 
// 온도 핀

int thermoDO = 50;  // SO (Serial Out)

int thermoCS = 53;  // CS (Chip Select)

int thermoCLK = 52; // SCK (Serial Clock)

// MAX6675 객체 생성

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
 
 
// function parameter

bool systemStarted = false;
 
unsigned long high_temp_time = 0;

unsigned long ox_drain_wait_time = 0;

unsigned long hy_drain_wait_time = 0;
 
bool ox_is_draining = false;

bool hy_is_draining = false;
 
int elseCounter = 0;

const int hy_leak_boundary = C;
 
 
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

  pinMode(hydrogen_in_valve, OUTPUT);

  pinMode(hydrogen_drain_valve, OUTPUT);

  pinMode(oxygen_compressor, OUTPUT);

  pinMode(oxygen_drain_valve, OUTPUT);

  initialize_mode();
 
  Serial.println("If you want to stop, please press 'T'!");
 
}
 
void loop() {

  // 정지

  if (Serial.available()) {

    char command_t = Serial.read();

    if (command_t == 't' || command_t == 'T') {

      stop_control();

    }

  }
 
  // 온도 측정

  float temp = thermocouple.readCelsius();

  Serial.print("섭씨: ");

  Serial.print(temp);

  // 냉각 모드 전환

  if (temp < 70) {

    coolant_loop3();

    elseCounter = 0;

  } else if (temp < 80) {

    coolant_loop2();

    elseCounter = 0;

  } else {

    coolant_loop1();

    elseCounter ++;

  }
 
  // safe check(coolant check)

  if (elseCounter >= 100 && temp > 90) {

    cool_problem();

  }
 
 


  // ox solenoid drain

  if (millis() > ox_drain_wait_time + 180000) {

    if (!ox_is_draining) {

      ox_solenoid_drain_on();

    }

    if (millis() > ox_drain_wait_time + 190000) {

      ox_solenoid_drain_off();

    }

  }

  // hy solenoid drain

  if (millis() > hy_drain_wait_time + 60000) {

    if (!hy_is_draining) {

      hy_solenoid_drain_on();

    }

    if (millis() > hy_drain_wait_time + 63000) {

      hy_solenoid_drain_off();

    }

  }


  // safe check(hydrogen leak check)

  if (hy_leak > hy_leak boundary) {

    hy_leak_stop();

  }
 
 


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

  delay(1000);

  digitalWrite(coolant_cbv1, HIGH);

  digitalWrite(coolant_cbv2, LOW);

  hydrogen_on();

  oxygen_on();

}

// === Stop ===

void stop_control() {

  Serial.println("stopping the system..");

  hydrogen_off();

  oxygen_off();

  delay(1000);

  digitalWrite(coolant_cbv1_power, LOW);

  digitalWrite(coolant_cbv2_power, LOW);

  delay(2000);

  systemReset();

}
 
void systemReset() {

  Serial.println("system reset");

  wdt_enable(WDTO_250MS);

  while (true) { }

}
 
// === Coolant Control ===

void coolant_loop1() {

  digitalWrite(coolant_heater, LOW);

  digitalWrite(coolant_cbv1, HIGH);

  digitalWrite(coolant_cbv2, HIGH);

  digitalWrite(coolant_pump1, HIGH);

  digitalWrite(coolant_pump2, HIGH);

}

void coolant_loop2() {

  digitalWrite(coolant_heater, LOW);

  digitalWrite(coolant_cbv1, HIGH);

  digitalWrite(coolant_cbv2, LOW);

  digitalWrite(coolant_pump1, HIGH);

  digitalWrite(coolant_pump2, LOW);

}

void coolant_loop3() {

  digitalWrite(coolant_heater, HIGH);

  digitalWrite(coolant_cbv1, LOW);

  digitalWrite(coolant_cbv2, LOW);

  digitalWrite(coolant_pump1, HIGH);

  digitalWrite(coolant_pump2, LOW);

}

// === Subsystem Control ===

void hydrogen_on() {

  digitalWrite(hydrogen_in_valve, HIGH);

}

void hydrogen_off() {

  digitalWrite(hydrogen_in_valve, LOW);

}

void oxygen_on() {

  digitalWrite(oxygen_compressor, HIGH);

}

void oxygen_off() {

  digitalWrite(oxygen_compressor, LOW);

}

// === Drain Control ===

void ox_solenoid_drain_on() {

  digitalWrite(oxygen_drain_valve, HIGH);

  ox_is_draining = true;

}

void ox_solenoid_drain_off() {

  digitalWrite(oxygen_drain_valve, LOW);

  ox_is_draining = false;

  ox_drain_wait_time = millis();

}


void hy_solenoid_drain_on() {

  digitalWrite(hydrogen_drain_valve, HIGH);

  hy_is_draining = true;

}

void hy_solenoid_drain_off() {

  digitalWrite(hydrogen_drain_valve, LOW);

  hy_is_draining = false;

  hy_drain_wait_time = millis();

}


// Safe function

void cool_problem() {

  Serial.println("Heat exchange does not work. There are some problems in heat changer");

  stop_control();

}
 
void hy_leak_stop() {

  Serial.println("Hydrogen leak occurs. So, you have to evacuate carefully!");

  stop_control();

}
 
void hy_solenoid_problem() {

  // 나중에 작성

}
 
void ox_solenoid_problem() {

  // 나중에 작성

}
 
