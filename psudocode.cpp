// === Coolant Control State Definition ===
typedef struct {
    int cod_heater
    int cbv_out
    int cbv_in
    int pump_sea
    int pump_main
} CoolantControlState

CoolantControlState ctrl

// === Hydrogen Subsystem Control ===
function set_hydrogen_on():
    pin 21 on //solenoid_hydrogen_in = 1   // 1 = allow flow
    pin 20 on // solenoid_hydrogen_out = 1

function set_hydrogen_off():
    pin 21 off // solenoid_hydrogen_in = 0
    pin 20 on // solenoid_hydrogen_out = 0

// === Oxygen Subsystem Control ===
function set_oxygen_on():
    air_compressor = 1
    solenoid_O2 = 1

function set_oxygen_off():
    air_compressor = 0
    solenoid_O2 = 0

function solenoid_drain(sig):
    solenoid_drain = sig;

// === Coolant Loop Modes ===
function loop1():                 // High temperature
    ctrl.cod_heater = 0
    ctrl.cbv_out = 1
    ctrl.cbv_in = 0
    ctrl.pump_sea = 1
    ctrl.pump_main = 1

function loop2():                 // Mid-range temperature
    ctrl.cod_heater = 0
    ctrl.cbv_out = 0
    ctrl.cbv_in = 1
    ctrl.pump_sea = 0
    ctrl.pump_main = 1

function loop3():                 // Low temperature (heater ON)
    ctrl.cod_heater = 1
    ctrl.cbv_out = 0
    ctrl.cbv_in = 0
    ctrl.pump_sea = 0
    ctrl.pump_main = 1
function 
    
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
        //water detection on oxygen output
        sig = height_oxywater > C; // we should select C with experiment!
        if (sig != prev_sig) {
            solenoid_drain(sig);
            prev_sig = sig;
        }
    
    }

// === System Execution ===
prev_sig=false;
initialize_mode()
main_loop()
// Note: exit_mode() can be called based on an external shutdown trigger.
