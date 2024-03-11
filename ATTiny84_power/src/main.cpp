#include <Arduino.h>
#include "SoftwareSerial.h"
#include "bq24292i.h"

byte power_btn = PIN_PA2;           // Power button connected to this pin. Low Active
byte sys_on = PIN_PA1;              // Regulator power. Active High
byte sht_dwn = PIN_PB0;             // Connected to GPIO25. Signal to start CM5 Shutdown. Active High
byte low_volt_shutdown = PIN_PB1;   // Connected to GPIO16 on CM5. Used for low voltage shut down

//#define rxPin PIN_PB1   // Pin used for Serial receive
//#define txPin PIN_PB0   // Pin used for Serial transmit

//SoftwareSerial mySerial(rxPin, txPin);

byte led1 = PIN_PB2;
byte led2 = PIN_PA7;
byte led3 = PIN_PA5;

bool lowVoltInState = false;
bool lastLowVoltInState = false;
bool systemState = false;
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

uint32_t vBatSum;
int vBatReadCounter;

// Returns average VBAT in microvolts
uint32_t readAvgVBAT() {
    // microvolts factor is: (R1 + R2) * 1.1v * 1000 / (1024 * R2) ~= 4.017578125
    vBatSum += analogRead(A3) * 4;
    vBatReadCounter++;
    return vBatSum / vBatReadCounter;
}

void BQ_INIT() {
    bq24292i_set_iin_max(BQ_IIN_MAX_3000MA);
    bq24292i_set_vin_max(5080);

    bq24292i_set_vsys_min(3500);
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
    bool input = !digitalRead(low_volt_shutdown);
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
        digitalWrite(sht_dwn, HIGH); // Tell CM5 to Shut down
        digitalWrite(led3, HIGH);
        shutdownInit = true;
    }
    else if (millis() > shutDown) {
        digitalWrite(sys_on, LOW);
        digitalWrite(sht_dwn, LOW);
        digitalWrite(led2, LOW);
        digitalWrite(led3, LOW);
        systemState = 0;
        shutDownTimerStarted = false;
        shutdownInit = false;
    }
}

void powerTimerCheck() {
    if (!btnTimerStarted) {
        btnTimerStarted = true;
        vBatReadCounter = 0;
        vBatSum = 0;
        powerBtnTimer = millis() +
            ((systemState == 0) ? powerOnDelay : powerOffDelay);
    }
    else if (millis() > powerBtnTimer) {
        btnTimerStarted = false;
        if (systemState) {
            systemState = false;
            digitalWrite(sht_dwn, HIGH);
            digitalWrite(led3, HIGH);
            shutdownInit = true;
        }
        else if (readAvgVBAT() > 3200) { // Check battery voltage before allowing sys_on to go high
            systemState = true;
            digitalWrite(sys_on, HIGH);
            digitalWrite(led2, HIGH);
        }
        else {
            // Flash LED1 if the user presses the button down during low battery and system off state
            for (int i = 0; i < 10; i++) {
                digitalWrite(led1, HIGH);
                delay(250);
                digitalWrite(led1, LOW);
                delay(250);
            }
        }
    }
    else if (!systemState)
        readAvgVBAT();      // Keep reading voltage while button is pressed
}

void setup() {
    cli();                  // Disable interrupts
    CLKPR = (1 << CLKPCE);  // Prescaler enable
    CLKPR = 0x00;           // Clock division factor
    sei();                  // Enable interrupts

    pinMode(power_btn, INPUT_PULLUP);
    pinMode(sys_on, OUTPUT);
    pinMode(sht_dwn, OUTPUT);
    pinMode(low_volt_shutdown, INPUT_PULLUP);
    pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
    pinMode(led3, OUTPUT);

    pinMode(A3, INPUT);
    analogReference(INTERNAL);  // Setup the ADC voltage ref as 1.1V

    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);

    i2c_master_init();

    // digitalWrite(led1, bq24292i_is_present());

    BQ_INIT();

    //mySerial.begin(9600);
    //delay(2000);
    //mySerial.println("SETUP Complete - SoftwareSerial Example");
}

void loop() {
    bool powerBtnState = !digitalRead(power_btn);
    lowVoltShutdownDebounce();

    if (shutdownInit || lowVoltInState)
        shutdownTimer();
    else if (powerBtnState)
        powerTimerCheck();
    else
        btnTimerStarted = false;
}
