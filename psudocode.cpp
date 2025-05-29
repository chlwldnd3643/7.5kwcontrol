// === Pin Definitions ===
const int coolant_pump1      = 22;  // heater쪽 펌프
const int coolant_pump2      = 23;  // sea쪽 펌프
const int coolant_heater     = 24;
const int coolant_cbv1       = 25;  // heater쪽 밸브
const int coolant_cbv2       = 26;  // sea쪽 밸브
const int coolant_temperature = A0; // 온도 센서 아날로그 핀

const int hydrogen_valve1    = 28;
const int hydrogen_drain_valve    = 29;

const int oxygen_compressor  = 30;
const int oxygen_drain_valve = 31;

const float water_height_limit = 100.0; // 임계값 예시

const unsigned long drain_duration = 5000; // t초 → 5초 (5000ms)
unsigned long drain_start_time = 0;
bool is_draining = false;

bool prev_drain_sig = false;

// === Setup ===
void setup() {
  // 디지털 출력 핀 설정
  pinMode(coolant_pump1, OUTPUT);
  pinMode(coolant_pump2, OUTPUT);
  pinMode(coolant_heater, OUTPUT);
  pinMode(coolant_cbv1, OUTPUT);
  pinMode(coolant_cbv2, OUTPUT);

  pinMode(hydrogen_valve1, OUTPUT);
  pinMode(hydrogen_valve2, OUTPUT);

  pinMode(oxygen_compressor, OUTPUT);
  pinMode(oxygen_valve1, OUTPUT);

  // 초기화
  initialize_mode();
}

// === Main Loop ===
void loop() {
  // 온도 측정
  float temp = analogRead(coolant_temperature) * (5.0 / 1023.0) * 100.0; // 예시 변환 (센서 특성 따라 수정)

  // 냉각 모드 전환
  if (temp < 70) {
    coolant_loop3();
  } else if (temp < 80) {
    coolant_loop2();
  } else {
    coolant_loop1();
  }

  float water_height = analogRead(A1);
  bool drain_sig = water_height > C;  // C는 정의 필요
  if (drain_sig != prev_drain_sig) {
    solenoid_drain(drain_sig);
    prev_drain_sig = drain_sig;
  }

  // 드레인 타이머 종료 체크
  if (is_draining && millis() - drain_start_time >= drain_duration) {
    digitalWrite(oxygen_drain_valve, LOW);
    is_draining = false;
  }

  delay(1000); // 제어 주기
}

// === Initialization ===
void initialize_mode() {
  hydrogen_on();
  oxygen_on();
  coolant_loop3(); // 기본 시작은 heater mode
}

// === Coolant Control Loops ===
void coolant_loop1() {
  // 히터 OFF
  digitalWrite(coolant_heater, LOW);

  // 밸브 설정 (sea 쪽 열기)
  digitalWrite(coolant_cbv1, LOW);
  digitalWrite(coolant_cbv2, HIGH);

  // 펌프 설정
  digitalWrite(coolant_pump1, LOW);
  digitalWrite(coolant_pump2, HIGH);
}

void coolant_loop2() {
  // 히터 OFF
  digitalWrite(coolant_heater, LOW);

  // 밸브 모두 닫음
  digitalWrite(coolant_cbv1, LOW);
  digitalWrite(coolant_cbv2, LOW);

  // 펌프 설정
  digitalWrite(coolant_pump1, HIGH);
  digitalWrite(coolant_pump2, LOW);
}

void coolant_loop3() {
  // 히터 ON
  digitalWrite(coolant_heater, HIGH);

  // heater loop (cbv1 열기)
  digitalWrite(coolant_cbv1, HIGH);
  digitalWrite(coolant_cbv2, LOW);

  // 펌프 설정
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
// On specific condition the hydrogen drain valve should be on
// On specific condition the oxygen valve should be on

// === Drain Control ===
void solenoid_drain(bool sig) {
  pinMode(oxygen_drain_valve, OUTPUT);
  if (sig) {
    digitalWrite(oxygen_drain_valve, HIGH);
    drain_start_time = millis();  // 현재 시간 저장
    is_draining = true;
  }
}
