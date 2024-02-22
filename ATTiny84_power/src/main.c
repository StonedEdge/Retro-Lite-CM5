#include <Arduino.h>
#include "bq24292i.h"

byte BQ24292i_ADDRESS = 0xD6; //BQ24292i slave address (8-bit) 
byte power_btn = 2; //Power button connected to this pin. Low Active
byte sys_on = 1; //Regulator power. Active High
byte sht_dwn = 8; //Connected to GPIO25. Signal to start Pi Shutdown. Active High
byte low_volt_shutdown = 9; //Connected to GPIO16 on pi. Used for low voltage shut down

byte lowVoltInState = LOW;
byte lastLowVoltInState = LOW;
byte systemState = 0; //Low Power Off
bool shutdownInit = false;

unsigned long powerBtnTimer;
unsigned long shutDown;
long powerOnDelay = 1000;
long powerOffDelay = 3000;
long shutDownDelay = 10000;
bool btnTimerStarted = false;
bool shutDownTimerStarted = false;

unsigned long lastLowVoltDebounce = 0;
unsigned long debounceDelay = 50;

void BQ_INIT() {
    bq24292i_set_iin_max(BQ_IIN_MAX_3000MA);
    bq24292i_set_vin_max(5080);

    bq24292i_set_vsys_min(3000);
    bq24292i_set_charge_config(BQ_CHG_CONFIG_ENABLE);

    bq24292i_set_charge_current(2496);

    bq24292i_set_term_current(256);
    bq24292i_set_precharge_current(256);

    bq24292i_set_recharge_offset(BQ_VRECHG_100MV);
    bq24292i_set_batlow_voltage(BQ_VBATLOW_2800MV);
    bq24292i_set_max_charge_voltage(4208);

    bq24292i_set_charge_timer(BQ_CHG_TIMER_8H);
    bq24292i_set_wdt_config(BQ_WATCHDOG_DISABLE);
    bq24292i_set_charge_termination(true);

    bq24292i_set_max_temp(BQ_MAX_TEMP_80C);
    bq24292i_set_voltage_clamp(0);
    bq24292i_set_comp_resistor(0);

    bq24292i_set_interrupt_mask(BQ_INT_BAT_AND_CHRG_FAULT);
    bq24292i_set_batfet_enabled(false);
}

void lowVoltShutdownDebounce() {
    int input = digitalRead(low_volt_shutdown);
    if (input != lastLowVoltInState)
        lastLowVoltDebounce = millis() + debounceDelay;

    if (millis() > lastLowVoltDebounce)
        lowVoltInState = input;

    lastLowVoltInState = input;
}

void shutdownTimer() {
    if (!shutDownTimerStarted) {
        shutDown = millis() + shutDownDelay;
        shutDownTimerStarted = true;
        digitalWrite(sht_dwn, HIGH);//Tell Pi to Shut down
        shutdownInit = true;
    }
    else if (millis() > shutDown) {
        digitalWrite(sys_on, LOW);
        digitalWrite(sht_dwn, LOW);
        systemState = 0;
        shutDownTimerStarted = false;
        shutdownInit = false;
    }
}

void powerTimerCheck() {
    if (!btnTimerStarted) {
        btnTimerStarted = true;
        powerBtnTimer = millis() +
            (systemState == 0) ? powerOnDelay : powerOffDelay;
    }
    else if (millis() > powerBtnTimer) {
        if (systemState == 0) {
            systemState = 1;
            digitalWrite(sys_on, HIGH);
            btnTimerStarted = false;
        } else {
            systemState = 0;
            digitalWrite(sht_dwn, HIGH);
            btnTimerStarted = false;
            shutdownInit = true;
        }
    }
}

void setup() {
    i2c_master_init();
    BQ_INIT(); 
    pinMode(power_btn, INPUT_PULLUP);
    pinMode(sys_on, OUTPUT);
    pinMode(sht_dwn, OUTPUT);
    pinMode(low_volt_shutdown, INPUT);
}

void loop() {
    byte powerBtnState = !digitalRead(power_btn);
    lowVoltShutdownDebounce();

    if (shutdownInit || lowVoltInState)
        shutdownTimer();
    else if (powerBtnState)
        powerTimerCheck();
    else
        btnTimerStarted = false;
}
