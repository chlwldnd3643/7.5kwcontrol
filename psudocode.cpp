// === Pin Definitions ===
const int coolant_pump1        = 22;  // heater쪽 펌프
const int coolant_pump2        = 23;  // sea쪽 펌프
const int coolant_heater       = 24;
const int coolant_cbv1         = 25;  // heater쪽 밸브
const int coolant_cbv2         = 26;  // sea쪽 밸브
const int coolant_temperature  = A0;  // 온도 센서

const int hydrogen_valve1      = 28;
const int hydrogen_drain_valve = 29;

const int oxygen_compressor    = 30;
const int oxygen_drain_valve   = 31;

const float water_height_limit = 100.0;       // 수위 임계값
const unsigned long drain_duration = 5000;    // 드레인 지속 시간 (ms)

unsigned long drain_start_time = 0;
bool is_draining = false;
bool prev_drain_sig = false;

// === Setup ===
void setup() {
  pinMode(coolant_pump1, OUTPUT);
  pinMode(coolant_pump2, OUTPUT);
  pinMode(coolant_heater, OUTPUT);
  pinMode(coolant_cbv1, OUTPUT);
  pinMode(coolant_cbv2, OUTPUT);

  pinMode(hydrogen_valve1, OUTPUT);
  pinMode(hydrogen_drain_valve, OUTPUT);

  pinMode(oxygen_compressor, OUTPUT);
  pinMode(oxygen_drain_valve, OUTPUT);

  initialize_mode();
}

// === Main Loop ===
void loop() {
  // 온도 측정 (센서 특성에 따라 변환 필요)
  float temp = analogRead(coolant_temperature) * (5.0 / 1023.0) * 100.0;

  // 냉각 모드 전환
  if (temp < 70) {
    coolant_loop3();
  } else if (temp < 80) {
    coolant_loop2();
  } else {
    coolant_loop1();
  }

  // 수위 센서 처리 (예: 아날로그 A1)
  float water_height = analogRead(A1);
  bool drain_sig = water_height > water_height_limit;

  if (drain_sig != prev_drain_sig) {
    solenoid_drain(drain_sig);
    prev_drain_sig = drain_sig;
  }

  // 드레인 시간 만료 시 밸브 닫기
  if (is_draining && millis() - drain_start_time >= drain_duration) {
    digitalWrite(oxygen_drain_valve, LOW);
    is_draining = false;
  }

  delay(1000);
}

// === Initialization ===
void initialize_mode() {
  hydrogen_on();
  oxygen_on();
  coolant_loop3(); // 기본 히터 모드
}

// === Coolant Control ===
void coolant_loop1() {
  digitalWrite(coolant_heater, LOW);
  digitalWrite(coolant_cbv1, LOW);
  digitalWrite(coolant_cbv2, HIGH);
  digitalWrite(coolant_pump1, LOW);
  digitalWrite(coolant_pump2, HIGH);
}

void coolant_loop2() {
  digitalWrite(coolant_heater, LOW);
  digitalWrite(coolant_cbv1, LOW);
  digitalWrite(coolant_cbv2, LOW);
  digitalWrite(coolant_pump1, HIGH);
  digitalWrite(coolant_pump2, LOW);
}

void coolant_loop3() {
  digitalWrite(coolant_heater, HIGH);
  digitalWrite(coolant_cbv1, HIGH);
  digitalWrite(coolant_cbv2, LOW);
  digitalWrite(coolant_pump1, HIGH);
  digitalWrite(coolant_pump2, LOW);
}

// === Subsystem Control ===
void hydrogen_on() {
  digitalWrite(hydrogen_valve1, HIGH);
}

void hydrogen_off() {
  digitalWrite(hydrogen_valve1, LOW);
}

void oxygen_on() {
  digitalWrite(oxygen_compressor, HIGH);
}

void oxygen_off() {
  digitalWrite(oxygen_compressor, LOW);
}

// === Drain Control ===
void solenoid_drain(bool sig) {
  if (sig) {
    digitalWrite(oxygen_drain_valve, HIGH);
    drain_start_time = millis();
    is_draining = true;
  }
}
