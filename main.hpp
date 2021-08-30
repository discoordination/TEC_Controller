#ifndef __MAIN_HPP
#define __MAIN_HPP


#include "pico/stdlib.h"
#include "OLED/oneBitDisplay.h"



namespace PIN {
	inline constexpr uint8_t PWM_A 	 			{ 6 };	
	inline constexpr uint8_t PWM_B 	 			{ 7 };
	inline constexpr uint8_t SDA_PIN 			{ 2 };
	inline constexpr uint8_t SCL_PIN 			{ 3 };
	inline constexpr uint8_t ENCODER_PIN1 		{ 18 };
	inline constexpr uint8_t ENCODER_PIN2		{ 17 };
	inline constexpr uint8_t ENCODER_BUTTON_PIN { 16 };
}

namespace IO {
//	inline constexpr uint8_t PULSES_PER_DETENT	{ 1 };
	inline constexpr uint16_t DEBOUNCE_MS		{ 1 };
}

namespace OLED {
	inline constexpr uint8_t SDA_PIN  { PIN::SDA_PIN };
	inline constexpr uint8_t SCL_PIN  { PIN::SCL_PIN };
	inline constexpr int8_t RESET_PIN { -1 };
	inline constexpr int8_t ADDRESS   { -1 };
	inline constexpr bool FLIP_180    { true };
	inline constexpr bool INVERT      { false };
	inline constexpr bool USE_HW_I2C  { true };
	inline constexpr uint8_t _128x64  { 3 };
}

namespace CONSTANT {
	inline constexpr double PWM_CLK_PERIOD         { 8e-9 };
	inline constexpr double PWM_FREQ               { 200'000 };
	inline constexpr double PWM_PERIOD_FOR_WRAP    { 1 / PWM_FREQ };
	inline constexpr uint32_t PWM_WRAP_VAL_NOPHASE { static_cast<uint16_t>((PWM_PERIOD_FOR_WRAP / PWM_CLK_PERIOD) - 1) };
	inline constexpr uint32_t PWM_WRAP_VAL_PHASE   { PWM_WRAP_VAL_NOPHASE / 2 };
	inline constexpr double DEAD_TIME_S            { 1024e-9 };
	inline constexpr uint16_t DEAD_TIME_CYCL	   { static_cast<uint16_t>(DEAD_TIME_S / PWM_CLK_PERIOD) }; 
}

namespace I2C {
	inline constexpr uint32_t I2CFREQ { 400'000 }; 
}

void init();
void initI2C();
void initDisplay(OBDISP& oled);
void initPWM();

#endif // __MAIN_HPP