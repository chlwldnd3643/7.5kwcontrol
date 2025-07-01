#include <SPI.h>
#include "max6675.h"
#include <avr/wdt.h>


#define RELAY_ON  LOW
#define RELAY_OFF HIGH


// === Pin Definitions ===
const int coolant_pump             = 32;  // 펌프
const int coolant_pump_sea         = 31;  // sea쪽 펌프
const int coolant_heater           = 30;  // 히터
const int coolant_cbv_heater_power = 24;  // heater쪽 밸브 파워
const int coolant_cbv_heater       = 25;  // heater쪽 밸브
const int coolant_cbv_sea_power    = 22;  // sea쪽 밸브 파워
const int coolant_cbv_sea          = 23;  // sea쪽 밸브

const int oxygen_compressor        = 26;  // 산소 컴프레서
const int oxygen_drain_valve       = 27;  // 산소 드레인 벨브

const int hydrogen_in_valve        = 28;  // 수소 입력 벨브
const int hydrogen_out_valve       = 29;  // 수소 드레인 벨브

const int stack_current_sensor     = A1;  // stack 전류 센서
const int stack_voltage_sensor     = A2;  // stack 전압 센서


// 온도 핀
const int thermoDO = 50;  // SO (Serial Out)
const int thermoCLK = 52; // SCK (Serial Clock)

const int thermoCS1 = 46;  // CS1 (Chip Select)
const int thermoCS2 = 48;  // CS2 (Chip Select)

// MAX6675 객체 생성
MAX6675 thermocouple1(thermoCLK, thermoCS1, thermoDO);
MAX6675 thermocouple2(thermoCLK, thermoCS2, thermoDO);

// function parameter
bool systemStarted = false;

unsigned long RELAY_OFF_temp_time = 0;
unsigned long ox_drain_wait_time = 0;
unsigned long hy_drain_wait_time = 0;

bool ox_is_draining = false;
bool hy_is_draining = false;

int elseCounter = 0;

float sensor_value0 = 0.0;
float sensor_value1 = 0.0;
float sensor_value2 = 0.0;
float sensor_value3 = 0.0;

int monitor_count = 0;

int count = 1;


// hydrogen leak
float Vout = 0.0;
const float RL = 10000.0;
const float hydrogen_standard_leak = 300.0;  // leak 기준점
float hydrogen_in_concen = 0.0;  // 현재 in leak 값
float hydrogen_out_concen = 0.0;  // 현재 out leak 값


void setup() {
  Serial.begin(9600);
  waitForStart();
 
  pinMode(coolant_pump, OUTPUT);
  pinMode(coolant_pump_sea, OUTPUT);
  pinMode(coolant_heater, OUTPUT);
  pinMode(coolant_cbv_heater_power, OUTPUT);
  pinMode(coolant_cbv_heater, OUTPUT);
  pinMode(coolant_cbv_sea_power, OUTPUT);
  pinMode(coolant_cbv_sea, OUTPUT);

  pinMode(oxygen_compressor, OUTPUT);
  pinMode(oxygen_drain_valve, OUTPUT);
  
  pinMode(hydrogen_in_valve, OUTPUT);
  pinMode(hydrogen_out_valve, OUTPUT);
 
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
  float temp = thermocouple1.readCelsius();
  Serial.print("섭씨: ");
  Serial.println(temp);
 
  // 냉각 모드 전환
  if (temp < 70) {
    coolant_loop3();
    elseCounter = 0;
  } 
  else if (temp < 80) {
    if (temp > 72) {
      coolant_loop2();
    }
    hydrogen_on();
    oxygen_on();
    elseCounter = 0;
  } 
  else {
    if (temp > 82) {
      coolant_loop1();
      elseCounter ++;
    }
  }

  // stack 전압, 전류 / 습도, 온도 측정
  monitor();

  // safe check(coolant check)
  if (elseCounter >= 100 && temp > 90) {
    cool_problem();
  }

  // safe check(hydrogen leak check)
  current_hydrogen();
  if (hydrogen_in_concen > hydrogen_standard_leak) {
    hydrogen_problem();
  }
  if (hydrogen_out_concen > hydrogen_standard_leak) {
    hydrogen_problem();
  }
 
  // ox solenoid drain
  if (millis() - ox_drain_wait_time > 180000) {
    if (!ox_is_draining) {
      ox_solenoid_drain_on();
    }
    if (millis() - ox_drain_wait_time > 190000) {
      ox_solenoid_drain_off();
    }
  }
 
  // hy solenoid drain
  if (millis() - hy_drain_wait_time > 60000) {
    if (!hy_is_draining) {
      hy_solenoid_drain_on();
    }
    if (millis() - hy_drain_wait_time > 63000) {
      hy_solenoid_drain_off();
    }
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
  digitalWrite(coolant_pump, RELAY_OFF);
  digitalWrite(coolant_pump_sea, RELAY_OFF);
  digitalWrite(coolant_heater, RELAY_OFF);
  digitalWrite(coolant_cbv_heater_power, RELAY_OFF);
  digitalWrite(coolant_cbv_sea_power, RELAY_OFF);
  digitalWrite(coolant_cbv_heater, RELAY_OFF);
  digitalWrite(coolant_cbv_sea, RELAY_OFF);
  digitalWrite(oxygen_compressor, RELAY_OFF);
  digitalWrite(oxygen_drain_valve, RELAY_OFF);
  digitalWrite(hydrogen_in_valve, RELAY_OFF);
  digitalWrite(hydrogen_out_valve, RELAY_OFF);
}


// === Stop ===
void stop_control() {
  Serial.println("stopping the system..");
  hydrogen_off();
  oxygen_off();
  delay(500);
  digitalWrite(coolant_pump, RELAY_OFF);
  digitalWrite(coolant_pump_sea, RELAY_OFF);
  digitalWrite(coolant_heater, RELAY_OFF);
  digitalWrite(coolant_cbv_heater_power, RELAY_OFF);
  digitalWrite(coolant_cbv_sea_power, RELAY_OFF);
  digitalWrite(coolant_cbv_heater, RELAY_OFF);
  digitalWrite(coolant_cbv_sea, RELAY_OFF);
  digitalWrite(oxygen_compressor, RELAY_OFF);
  digitalWrite(oxygen_drain_valve, RELAY_OFF);
  digitalWrite(hydrogen_in_valve, RELAY_OFF);
  digitalWrite(hydrogen_out_valve, RELAY_OFF);
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
  digitalWrite(coolant_heater, RELAY_OFF);
  digitalWrite(coolant_cbv_heater_power, RELAY_ON);
  digitalWrite(coolant_cbv_sea_power, RELAY_ON);

  delay(500);

  digitalWrite(coolant_cbv_heater, RELAY_ON);  // on 상태: 일자로 흐름(히터x)
  digitalWrite(coolant_cbv_sea, RELAY_OFF);  // off 상태: 일자로 흐름(열교환기o)
  digitalWrite(coolant_pump, RELAY_ON);
  digitalWrite(coolant_pump_sea, RELAY_ON);

  delay(4000);

  digitalWrite(coolant_cbv_heater_power, RELAY_OFF);
  digitalWrite(coolant_cbv_sea_power, RELAY_OFF);  
}
 
void coolant_loop2() {
  digitalWrite(coolant_heater, RELAY_OFF);
  digitalWrite(coolant_cbv_heater_power, RELAY_ON);
  digitalWrite(coolant_cbv_sea_power, RELAY_ON);

  delay(500);

  digitalWrite(coolant_cbv_heater, RELAY_ON);  // on 상태: 일자로 흐름(히터x)
  digitalWrite(coolant_cbv_sea, RELAY_OFF);  // off 상태: ㄱ자로 흐름(열교환기x)
  digitalWrite(coolant_pump, RELAY_ON);
  digitalWrite(coolant_pump_sea, RELAY_OFF);

  delay(4000);

  digitalWrite(coolant_cbv_heater_power, RELAY_OFF);
  digitalWrite(coolant_cbv_sea_power, RELAY_OFF);  
}
 
void coolant_loop3() {
  digitalWrite(coolant_heater, RELAY_ON);
  digitalWrite(coolant_cbv_heater_power, RELAY_ON);
  digitalWrite(coolant_cbv_sea_power, RELAY_ON);

  delay(500);

  digitalWrite(coolant_cbv_heater, RELAY_OFF);  // off 상태: ㄱ자로 흐름(히터o)
  digitalWrite(coolant_cbv_sea, RELAY_OFF);  // off 상태: ㄱ자로 흐름(열교환기x)
  digitalWrite(coolant_pump, RELAY_ON);
  digitalWrite(coolant_pump_sea, RELAY_OFF);

  delay(4000);

  digitalWrite(coolant_cbv_heater_power, RELAY_OFF);
  digitalWrite(coolant_cbv_sea_power, RELAY_OFF);
}


// === Subsystem Control ===
void hydrogen_on() {
  digitalWrite(hydrogen_in_valve, RELAY_ON);
}
 
void hydrogen_off() {
  digitalWrite(hydrogen_in_valve, RELAY_OFF);
}
 
void oxygen_on() {
  digitalWrite(oxygen_compressor, RELAY_ON);
}
 
void oxygen_off() {
  digitalWrite(oxygen_compressor, RELAY_OFF);
}
 
// === Drain Control ===
void ox_solenoid_drain_on() {
  digitalWrite(oxygen_drain_valve, RELAY_ON);
  ox_is_draining = true;
}
 
void ox_solenoid_drain_off() {
  digitalWrite(oxygen_drain_valve, RELAY_OFF);
  ox_is_draining = false;
  ox_drain_wait_time = millis();
}
 
 
void hy_solenoid_drain_on() {
  digitalWrite(hydrogen_out_valve, RELAY_ON);
  hy_is_draining = true;
}
 
void hy_solenoid_drain_off() {
  digitalWrite(hydrogen_out_valve, RELAY_OFF);
  hy_is_draining = false;
  hy_drain_wait_time = millis();
}


// 모니터링
void monitor() {
  if (monitor_count == 5) {
    float temp1 = thermocouple1.readCelsius();
    Serial.print("1번 섭씨: ");
    Serial.print(temp1);
    Serial.print(" °C\t");

    float temp2 = thermocouple2.readCelsius();
    Serial.print("2번 섭씨: ");
    Serial.print(temp2);
    Serial.println(" °C");


    sensor_value0 = analogRead(A0);
    sensor_value1 = analogRead(A1);
    sensor_value2 = analogRead(A2);
    sensor_value3 = analogRead(A3);

    float humi_v = sensor_value0 * (5.0 / 1023.0);
    float real_humi = humi_v * 20.0;

    float temp_v = sensor_value1 * (5.0 / 1023.0);
    float real_temp = (temp_v * 20.0) - 40;

    float curr_v = sensor_value2 * (5.0 / 1023.0);
    float real_curr = (curr_v - 2.5) * 150.0;

    float volt_v = sensor_value3 * (5.0 / 1023.0);
    float real_volt = volt_v * 22.0;

    // 문자열 버퍼 선언
    char humi_v_str[8], real_humi_str[8], temp_v_str[8], real_temp_str[8];
    char curr_v_str[8], real_curr_str[8], volt_v_str[8], real_volt_str[8];

    // float → 문자열 변환
    dtostrf(humi_v,     5, 2, humi_v_str);
    dtostrf(real_humi,  6, 2, real_humi_str);
    dtostrf(temp_v,     5, 2, temp_v_str);
    dtostrf(real_temp,  6, 2, real_temp_str);
    dtostrf(curr_v,     5, 2, curr_v_str);
    dtostrf(real_curr,  6, 2, real_curr_str);
    dtostrf(volt_v,     5, 2, volt_v_str);
    dtostrf(real_volt,  6, 2, real_volt_str);

    // 출력용 문자열 조합
    char buffer[160];
    sprintf(buffer, "%4ds |  %s  | %s |  %s  |  %s  |  %s  | %s  | %s  | %s",
            count,
            humi_v_str, real_humi_str,
            temp_v_str, real_temp_str,
            curr_v_str, real_curr_str,
            volt_v_str, real_volt_str);

    // 헤더 및 데이터 출력
    Serial.println(" Time | Humi(V) | Humi(%) | Temp(V) | Temp(°C) | Curr(V) | Real(A) | Volt(V) | Real(V)");
    Serial.println("------+---------+---------+---------+----------+---------+---------+---------+---------");
    Serial.println(buffer);
    Serial.println("");
    Serial.println("");
    monitor_count = 0;
    count++;
  }
  else {
    monitor_count ++;
  }
}

 
// Safe function
void cool_problem() {
  Serial.println("Heat exchange does not work. There are some problems in heat changer");
  stop_control();
}

void hydrogen_problem() {
  Serial.println("Hydrogen leak occurs. So, you have to evacuate carefully!");
  stop_control();
}

void control_problem() {
  // 나중에 작성
  // 1. 아두이노 문제
  // 2. 릴레이 모듈 문제
  // 3. 하드웨어 문제
}


void current_hydrogen() {
  hydrogen_in_concen = analogRead(A4);
  hydrogen_out_concen = analogRead(A5);
}



