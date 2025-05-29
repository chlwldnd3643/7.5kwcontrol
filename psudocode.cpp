// === Coolant Control State Definition ===
typedef struct {
    int cod_heater
    int cbv_out
    int cbv_in
} CoolantControlState

CoolantControlState ctrl

// === Hydrogen Subsystem Control ===
function set_hydrogen_on():
    solenoid_hydrogen_block = 1   // 1 = allow flow
    solenoid_hydrogen_out = 1
    recirculation_blower = 1

function set_hydrogen_off():
    solenoid_hydrogen_block = 0
    solenoid_hydrogen_out = 0
    recirculation_blower = 0

// === Oxygen Subsystem Control ===
function set_oxygen_on():
    air_compressor = 1
    solenoid_O2 = 1

function set_oxygen_off():
    air_compressor = 0
    solenoid_O2 = 0

// === Coolant Loop Modes ===
function loop1():                 // High temperature
    ctrl.cod_heater = 0
    ctrl.cbv_out = 1
    ctrl.cbv_in = 0

function loop2():                 // Mid-range temperature
    ctrl.cod_heater = 0
    ctrl.cbv_out = 0
    ctrl.cbv_in = 1

function loop3():                 // Low temperature (heater ON)
    ctrl.cod_heater = 1
    ctrl.cbv_out = 0
    ctrl.cbv_in = 0

// === System Initialization ===
function initialize_mode():
    set_hydrogen_on()
    set_oxygen_on()
    loop3()                      // Set default coolant mode

// === System Shutdown ===
function exit_mode():
    set_hydrogen_off()
    set_oxygen_off()
    ctrl.cod_heater = 0         // Make sure heater is OFF
    ctrl.cbv_out = 0
    ctrl.cbv_in = 0             // Default valve state

// === Main Control Loop ===
function main_loop():
    while (true) {
        if (tmp < 70) {
            loop3()
        }
        else if (tmp < 80) {
            loop2()
        }
        else {
            loop1()
        }
    }

// === System Execution ===
initialize_mode()
main_loop()
// Note: exit_mode() can be called based on an external shutdown trigger.
