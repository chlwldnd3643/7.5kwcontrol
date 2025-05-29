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
const int PIN_SOL_DRAIN = 22;

const int PIN_COD_HEATER = 2;
const int PIN_CBV_OUT = 3;
const int PIN_CBV_IN = 4;
const int PIN_PUMP_SEA = 5;
const int PIN_PUMP_MAIN = 6;

// === Variables ===
bool prev_sig = false;
float tmp = 0.0;
float height_oxywater = 0.0;
const float C = 10.0; // 실험적으로 조정할 기준값

// === 핀에 맞게 제어 상태 반영 함수 ===
void apply_coolant_state() {
    digitalWrite(PIN_COD_HEATER, ctrl.cod_heater);
    digitalWrite(PIN_CBV_OUT,   ctrl.cbv_out);
    digitalWrite(PIN_CBV_IN,    ctrl.cbv_in);
    digitalWrite(PIN_PUMP_SEA,  ctrl.pump_sea);
    digitalWrite(PIN_PUMP_MAIN, ctrl.pump_main);
}

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
    // 실제 핀 설정 필요
}

void set_oxygen_off() {
    // 실제 핀 설정 필요
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
    apply_coolant_state();
}

void loop2() {
    ctrl.cod_heater = 0;
    ctrl.cbv_out = 0;
    ctrl.cbv_in = 1;
    ctrl.pump_sea = 0;
    ctrl.pump_main = 1;
    apply_coolant_state();
}

void loop3() {
    ctrl.cod_heater = 1;
    ctrl.cbv_out = 0;
    ctrl.cbv_in = 0;
    ctrl.pump_sea = 0;
    ctrl.pump_main = 1;
    apply_coolant_state();
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
    ctrl.pump_sea = 0;
    ctrl.pump_main = 0;
    apply_coolant_state();
}

// === Main Loop ===
void loop() {
    // 센서 입력 예시
    tmp = analogRead(A0);
    height_oxywater = analogRead(A1);

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

    delay(1000); // 샘플링 간격
}

void setup() {
    // 핀 초기화
    pinMode(PIN_SOL_HYDROGEN_IN, OUTPUT);
    pinMode(PIN_SOL_HYDROGEN_OUT, OUTPUT);
    pinMode(PIN_SOL_DRAIN, OUTPUT);

    pinMode(PIN_COD_HEATER, OUTPUT);
    pinMode(PIN_CBV_OUT, OUTPUT);
    pinMode(PIN_CBV_IN, OUTPUT);
    pinMode(PIN_PUMP_SEA, OUTPUT);
    pinMode(PIN_PUMP_MAIN, OUTPUT);

    // 필요 시 산소 파트 핀도 설정

    initialize_mode();
}
