#include <Arduino.h>
#include "SoftwareSerial.h"
#include "bq24292i.h"

#define TEST_FIXTURE 0
#define SERIAL_DEBUG 0

byte in_power_btn = PIN_PA2;  // Power button connected to this pin. Active Low
byte out_power_on = PIN_PA1;  // Regulator power. Active High
byte out_power_btn = PIN_PA0; // CM5 Power "button". Active Low

#if SERIAL_DEBUG
#define rxPin PIN_PB1     // Pin used for Serial receive
#define txPin PIN_PB0     // Pin used for Serial transmit
SoftwareSerial mySerial(rxPin, txPin);
#else
byte out_shutdown = PIN_PB0; // Signal to start CM5 Shutdown. Active High
byte in_shutdown = PIN_PB1;  // Signal from CM5 saying it's shutting down. Active Low
#endif

byte led_vbat_low = PIN_PB2;
#if TEST_FIXTURE
byte led_power_on = PIN_PA7;
byte led_shutdown = PIN_PA5;
#endif

bool shutdownState = false;
bool lastShutdownState = false;
bool powerState = false;
bool shutdownInit = false;

unsigned long powerBtnTimer;
unsigned long shutdownTimer;
long powerOnDelay = 1000;
long powerOffDelay = 3000;
long shutdownDelay = 10000;
bool btnTimerStarted = false;
bool shutdownTimerStarted = false;
bool lastPowerBtnState = false;

unsigned long lastShutdownDebounce = 0;
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

bool shutdownDebounce() {
    bool input = !digitalRead(in_shutdown);
    if (input != lastShutdownState)
        lastShutdownDebounce = millis() + debounceDelay;

    if (millis() > lastShutdownDebounce)
        shutdownState = input;

    lastShutdownState = input;

    return powerState && (shutdownState || shutdownInit);
}

void shutdownTimerCheck() {
    if (!shutdownTimerStarted) {
        shutdownTimerStarted = true;
        shutdownTimer = millis() + shutdownDelay;
#if (!SERIAL_DEBUG)
        digitalWrite(out_shutdown, HIGH); // Tell CM5 to shut down
#endif
#if TEST_FIXTURE
        digitalWrite(led_shutdown, HIGH);
#endif
        shutdownInit = true;
    }
    else if (millis() > shutdownTimer) {
        shutdownTimerStarted = false;
        digitalWrite(out_power_on, LOW);
#if (!SERIAL_DEBUG)
        digitalWrite(out_shutdown, LOW);
#endif
#if TEST_FIXTURE
        digitalWrite(led_power_on, LOW);
        digitalWrite(led_shutdown, LOW);
#endif
        powerState = 0;
        shutdownInit = false;
    }
}

void powerTimerCheck() {
    if (!btnTimerStarted) {
        btnTimerStarted = true;
        vBatReadCounter = 0;
        vBatSum = 0;
        powerBtnTimer = millis() +
            (powerState ? powerOffDelay : powerOnDelay);
    }
    else if (millis() > powerBtnTimer) {
        btnTimerStarted = false;
        if (powerState) {
            powerState = false;
#if (!SERIAL_DEBUG)
            digitalWrite(out_shutdown, HIGH);
#endif
#if TEST_FIXTURE
            digitalWrite(led_shutdown, HIGH);
#endif
            shutdownInit = true;
        }
        else if (readAvgVBAT() > 3200) { // Check battery voltage before allowing out_power_on to go high
            powerState = true;
            digitalWrite(out_power_on, HIGH);
#if TEST_FIXTURE
            digitalWrite(led_power_on, HIGH);
#endif
        }
        else {
            // Flash LED if the user presses the button down during low battery and system off state
            for (int i = 0; i < 10; i++) {
                digitalWrite(led_vbat_low, HIGH);
                delay(250);
                digitalWrite(led_vbat_low, LOW);
                delay(250);
            }
        }
    }
    else if (!powerState)
        readAvgVBAT();      // Keep reading voltage while button is pressed
}

void setup() {
    cli();                  // Disable interrupts
    CLKPR = (1 << CLKPCE);  // Prescaler enable
    CLKPR = 0x00;           // Clock division factor
    sei();                  // Enable interrupts

    pinMode(in_power_btn, INPUT_PULLUP);
    pinMode(out_power_on, OUTPUT);
    pinMode(out_power_btn, OUTPUT);
#if (!SERIAL_DEBUG)
    pinMode(out_shutdown, OUTPUT);
    pinMode(in_shutdown, INPUT_PULLUP);
#endif
    pinMode(led_vbat_low, OUTPUT);
#if TEST_FIXTURE
    pinMode(led_power_on, OUTPUT);
    pinMode(led_shutdown, OUTPUT);
#endif

    pinMode(A3, INPUT);
    analogReference(INTERNAL);  // Setup the ADC voltage ref as 1.1V

    digitalWrite(out_power_on, LOW);
    digitalWrite(out_power_btn, HIGH);
#if (!SERIAL_DEBUG)
    digitalWrite(out_shutdown, LOW);
#endif
    digitalWrite(led_vbat_low, LOW);
#if TEST_FIXTURE
    digitalWrite(led_power_on, LOW);
    digitalWrite(led_shutdown, LOW);
#endif

    i2c_master_init();

    BQ_INIT();

#if SERIAL_DEBUG
    mySerial.begin(9600);
    delay(2000);
    mySerial.println("SETUP Complete");
    if (bq24292i_is_present())
        mySerial.println("bq24292i is present")
#endif
}

void loop() {
    bool powerBtnState = !digitalRead(in_power_btn);

    if (shutdownDebounce())
        shutdownTimerCheck();
    else if (powerBtnState)
        powerTimerCheck();
    else if (lastPowerBtnState && btnTimerStarted) {
        // User released the power button before timer elapsed, pass it on.
        digitalWrite(out_power_btn, LOW);
        delay(200);
        digitalWrite(out_power_btn, HIGH);
    }
    else
        btnTimerStarted = false;

    lastPowerBtnState = powerBtnState;
}
