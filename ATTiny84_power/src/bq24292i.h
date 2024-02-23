/**
 * Based on Jeff Longo's code from https://github.com/jefflongo/bq24292i/
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool i2c_master_init(void);
bool i2c_master_write_u8(const uint8_t device, const uint8_t reg, const uint8_t data);
bool i2c_master_read_u8(const uint8_t device, const uint8_t reg, uint8_t* data);

typedef enum {
    BQ_IIN_MAX_100MA = 0b000U,
    BQ_IIN_MAX_150MA = 0b001U,
    BQ_IIN_MAX_500MA = 0b010U,
    BQ_IIN_MAX_900MA = 0b011U,
    BQ_IIN_MAX_1200MA = 0b100U,
    BQ_IIN_MAX_1500MA = 0b101U,
    BQ_IIN_MAX_2000MA = 0b110U,
    BQ_IIN_MAX_3000MA = 0b111U,
} bq24292i_iin_max_t;

typedef uint16_t bq24292i_vin_max_t;

typedef uint16_t bq24292i_vsys_min_t;

typedef enum {
    BQ_CHG_CONFIG_DISABLE = 0b00U,
    BQ_CHG_CONFIG_ENABLE = 0b01U,
    BQ_CHG_CONFIG_OTG = 0b10U,
} bq24292i_chg_config_t;

typedef uint16_t bq24292i_chg_current_t;

typedef uint16_t bq24292i_term_current_t;

typedef uint16_t bq24292i_prechg_current_t;

typedef enum {
    BQ_VRECHG_100MV = 0U,
    BQ_VRECHG_300MV = 1U,
} bq24292i_vrechg_offset_t;

typedef enum {
    BQ_VBATLOW_2800MV = 0U,
    BQ_VBATLOW_3000MV = 1U,
} bq24292i_vbatlow_t;

typedef uint16_t bq24292i_vchg_max_t;

typedef enum {
    BQ_CHG_TIMER_5H = 0b00U,
    BQ_CHG_TIMER_8H = 0b01U,
    BQ_CHG_TIMER_12H = 0b10U,
    BQ_CHG_TIMER_20H = 0b11U,
    BQ_CHG_TIMER_DISABLE,
} bq24292i_chg_timer_t;

typedef enum {
    BQ_WATCHDOG_DISABLE = 0b00U,
    BQ_WATCHDOG_40S = 0b01U,
    BQ_WATCHDOG_80S = 0b10U,
    BQ_WATCHDOG_160S = 0b11U,
} bq24292i_watchdog_conf_t;

typedef enum {
    BQ_MAX_TEMP_60C = 0b00U,
    BQ_MAX_TEMP_80C = 0b01U,
    BQ_MAX_TEMP_100C = 0b10U,
    BQ_MAX_TEMP_120C = 0b11U,
} bq_24292i_max_temp_t;

typedef uint8_t bq24292i_clamp_voltage_t;

typedef uint8_t bq24292i_comp_resistor_t;

typedef enum {
    BQ_INT_NONE = 0b00U,
    BQ_INT_BAT_FAULT = 0b01U,
    BQ_INT_CHRG_FAULT = 0b10U,
    BQ_INT_BAT_AND_CHRG_FAULT = 0b11U,
} bq24292i_interrupt_mask_t;

typedef enum {
    BQ_STATE_NOT_CHARGING = 0b00U,
    BQ_STATE_PRECHARGE = 0b01U,
    BQ_STATE_FAST_CHARGE = 0b10U,
    BQ_STATE_TERMINATED = 0b11U,
} bq24292i_charge_state_t;

typedef enum {
    BQ_SOURCE_UNKNOWN = 0b00U,
    BQ_SOURCE_USB = 0b01U,
    BQ_SOURCE_ADAPTER = 0b10U,
    BQ_SOURCE_OTG = 0b11U,
} bq24292i_source_type_t;

enum {
    BQ_FAULT_THERM = 0x07U,
    BQ_FAULT_BAT = 0x08U,
    BQ_FAULT_CHG = 0x30U,
    BQ_FAULT_BOOST = 0x40U,
    BQ_FAULT_WATCHDOG = 0x80U,
};
typedef uint8_t bq24292i_fault_t;

bool bq24292i_is_present(void);

bool bq24292i_set_iin_max(bq24292i_iin_max_t ma);
bool bq24292i_get_iin_max(bq24292i_iin_max_t* ma);

bool bq24292i_set_vin_max(bq24292i_vin_max_t mv);
bool bq24292i_get_vin_max(bq24292i_vin_max_t* mv);

bool bq24292i_set_vsys_min(bq24292i_vsys_min_t mv);
bool bq24292i_get_vin_max(bq24292i_vsys_min_t* mv);

bool bq24292i_set_charge_config(bq24292i_chg_config_t conf);
bool bq24292i_get_charge_config(bq24292i_chg_config_t* conf);

bool bq24292i_reset_wdt(void);

bool bq24292i_set_charge_current(bq24292i_chg_current_t ma);
bool bq24292i_get_charge_current(bq24292i_chg_current_t* ma);

bool bq24292i_set_term_current(bq24292i_term_current_t ma);
bool bq24292i_get_term_current(bq24292i_term_current_t* ma);

bool bq24292i_set_precharge_current(bq24292i_prechg_current_t ma);
bool bq24292i_get_precharge_current(bq24292i_prechg_current_t* ma);

bool bq24292i_set_recharge_offset(bq24292i_vrechg_offset_t offset);
bool bq24292i_get_recharge_offset(bq24292i_vrechg_offset_t* offset);

bool bq24292i_set_batlow_voltage(bq24292i_vbatlow_t mv);
bool bq24292i_get_batlow_voltage(bq24292i_vbatlow_t* mv);

bool bq24292i_set_max_charge_voltage(bq24292i_vchg_max_t mv);
bool bq24292i_get_max_charge_voltage(bq24292i_vchg_max_t* mv);

bool bq24292i_set_charge_timer(bq24292i_chg_timer_t conf);
bool bq24292i_get_charge_timer(bq24292i_chg_timer_t* conf);

bool bq24292i_set_wdt_config(bq24292i_watchdog_conf_t conf);
bool bq24292i_get_wdt_config(bq24292i_watchdog_conf_t* conf);

bool bq24292i_set_charge_termination(bool enable);
bool bq24292i_get_charge_termination(bool* enable);

bool bq24292i_set_max_temp(bq_24292i_max_temp_t temp);
bool bq24292i_get_max_temp(bq_24292i_max_temp_t* temp);

bool bq24292i_set_voltage_clamp(bq24292i_clamp_voltage_t mv);
bool bq24292i_get_voltage_clamp(bq24292i_clamp_voltage_t* mv);

bool bq24292i_set_comp_resistor(bq24292i_comp_resistor_t mohms);
bool bq24292i_get_comp_resistor(bq24292i_comp_resistor_t* mohms);

bool bq24292i_set_interrupt_mask(bq24292i_interrupt_mask_t mask);
bool bq24292i_get_interrupt_mask(bq24292i_interrupt_mask_t* mask);

bool bq24292i_set_batfet_enabled(bool enable);
bool bq24292i_get_batfet_enabled(bool* enable);

bool bq24292i_is_vsys_boosted(bool* result);

bool bq24292i_is_overtemp(bool* result);

bool bq24292i_is_charger_connected(bool* result);

bool bq24292i_is_in_dpm(bool* result);

bool bq24292i_get_charge_state(bq24292i_charge_state_t* state);

bool bq24292i_get_source_type(bq24292i_source_type_t* source);

bool bq24292i_check_faults(bq24292i_fault_t* faults);
