// === Coolant Control State Definition ===
struct CoolantControlState {
    int cod_heater;
    int cbv_out;
    int cbv_in;
    int pump_sea;
    int pump_main;
};

CoolantControlState ctrl;

// === Pin Definitions ===
const int PIN_SOL_HYDROGEN_IN = 21;
const int PIN_SOL_HYDROGEN_OUT = 20;
const int PIN_SOL_DRAIN = 22; // 예시 핀. 실제 하드웨어에 따라 수정 필요

// === Variables ===
bool prev_sig = false;
float tmp = 0.0; // 온도 센서 값
float height_oxywater = 0.0;
const float C = 10.0; // 실험적으로 정할 값

// === Function Prototypes ===
void set_hydrogen_on();
void set_hydrogen_off();
void set_oxygen_on();
void set_oxygen_off();
void solenoid_drain(bool sig);
void loop1();
void loop2();
void loop3();
void initialize_mode();
void exit_mode();

// === Hydrogen Subsystem Control ===
void set_hydrogen_on() {
    digitalWrite(PIN_SOL_HYDROGEN_IN, HIGH);
    digitalWrite(PIN_SOL_HYDROGEN_OUT, HIGH);
}

void set_hydrogen_off() {
    digitalWrite(PIN_SOL_HYDROGEN_IN, LOW);
    digitalWrite(PIN_SOL_HYDROGEN_OUT, LOW);
}

// === Oxygen Subsystem Control ===
void set_oxygen_on() {
    // 예시: 실제 사용하려는 디지털 핀 또는 제어 변수를 지정해야 함
    // 예: digitalWrite(PIN_AIR_COMPRESSOR, HIGH);
    //     digitalWrite(PIN_SOLENOID_O2, HIGH);
}

void set_oxygen_off() {
    // 예시: 실제 사용하려는 디지털 핀 또는 제어 변수를 지정해야 함
}

// === Drain Solenoid Control ===
void solenoid_drain(bool sig) {
    digitalWrite(PIN_SOL_DRAIN, sig ? HIGH : LOW);
}

// === Coolant Loop Modes ===
void loop1() {
    ctrl.cod_heater = 0;
    ctrl.cbv_out = 1;
    ctrl.cbv_in = 0;
    ctrl.pump_sea = 1;
    ctrl.pump_main = 1;
}

void loop2() {
    ctrl.cod_heater = 0;
    ctrl.cbv_out = 0;
    ctrl.cbv_in = 1;
    ctrl.pump_sea = 0;
    ctrl.pump_main = 1;
}

void loop3() {
    ctrl.cod_heater = 1;
    ctrl.cbv_out = 0;
    ctrl.cbv_in = 0;
    ctrl.pump_sea = 0;
    ctrl.pump_main = 1;
}

// === System Initialization ===
void initialize_mode() {
    set_hydrogen_on();
    set_oxygen_on();
    loop3();
}

// === System Shutdown ===
void exit_mode() {
    set_hydrogen_off();
    set_oxygen_off();
    ctrl.cod_heater = 0;
    ctrl.cbv_out = 0;
    ctrl.cbv_in = 0;
}

// === Main Control Loop ===
void loop() {
    // 센서 읽기 예시 (사용자 환경에 따라 적절히 수정)
    tmp = analogRead(A0); // 온도센서 입력 (예)
    height_oxywater = analogRead(A1); // 수위센서 입력 (예)

    if (tmp < 70) {
        loop3();
    } else if (tmp < 80) {
        loop2();
    } else {
        loop1();
    }

    bool sig = height_oxywater > C;
    if (sig != prev_sig) {
        solenoid_drain(sig);
        prev_sig = sig;
    }

    delay(1000); // 루프 주기 설정
}

void setup() {
    // 핀 모드 설정
    pinMode(PIN_SOL_HYDROGEN_IN, OUTPUT);
    pinMode(PIN_SOL_HYDROGEN_OUT, OUTPUT);
    pinMode(PIN_SOL_DRAIN, OUTPUT);
    // 다른 핀들에 대해서도 pinMode 추가 필요

    initialize_mode();
}
