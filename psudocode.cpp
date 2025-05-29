//PSEDOCODE EDITING!!


initial_mode(){
    
//Hydrogen part initialization
solenoid_hydrogen_block=1; //1means allowed, 0 means blocking
solenoid_hydrogen_out=1;
recirculation_blower=1;

//Oxygen Part initialization
Air_Compressor = 1;
Solenoid_O2 = 1;

//Coolant Part Initialization
cod_heater=1;
cbv -> loop3
}

exit_mode(){
//Hydrogen part exit
solenoid_hydrogen_block=0; //1means allowed, 0 means blocking
solenoid_hydrogen_out=0;
recirculation_blower=0;

//Oxygen Part exit
Air_Compressor = 0;
Solenoid_O2 = 0;

//Coolant Part exit
cod_heater=0;
cbv -> loop3
}


//coolant part hello
while(1){
if tmp < 70:

    cod heater =1

    cbv -> loop3

if 70< tmp < 80:

    cod heater = 0

    cbv -> loop2

if tmp>80:

    cod heater =0

    cbv -> loop1
}
