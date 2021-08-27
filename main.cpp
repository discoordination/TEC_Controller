#include "main.hpp"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "menu.hpp"
#include "gpio.hpp"

#include <iostream>
#include <vector>
#include <bitset>
#include <any>

OBDISP oled;

Menu m1 { 
			std::vector<std::shared_ptr<BasicMenuItem>> { 
						std::make_shared<MenuTitle>("MENU"),
						std::make_shared<MenuButton>("One"),
						std::make_shared<MenuButton>("Two"),
						std::make_shared<MenuButton>("Three"),
						std::make_shared<MenuButton>("Four"),
						std::make_shared<MenuButton>("Five"),
						std::make_shared<MenuButton>("Six"),
						std::make_shared<MenuButton>("Seven"),
						std::make_shared<MenuButton>("Eight"),
						std::make_shared<MenuButton>("Nine") },
			128 / 8,
			8,
			[](std::string str, int yPos, bool inv) { 
				obdWriteString(&oled, false, 0, yPos, const_cast<char*>(str.c_str()), FONT_8x8, inv, true);
				},
			Menu::Alignment::Center 
		};


struct Setting {};


void initPWM() {
	
	auto sliceNum = pwm_gpio_to_slice_num(PIN::PWM_A);
	gpio_set_function(PIN::PWM_A, GPIO_FUNC_PWM);
	gpio_set_function(PIN::PWM_B, GPIO_FUNC_PWM);
	gpio_set_drive_strength(PIN::PWM_A, GPIO_DRIVE_STRENGTH_8MA);
	gpio_set_drive_strength(PIN::PWM_B, GPIO_DRIVE_STRENGTH_8MA);
	
	pwm_set_wrap(sliceNum, CONSTANT::PWM_WRAP_VAL_PHASE);
	pwm_set_output_polarity(sliceNum, true, true);
	pwm_set_phase_correct(sliceNum, true);

	pwm_set_both_levels(sliceNum, (CONSTANT::PWM_WRAP_VAL_PHASE / 2) + (CONSTANT::DEAD_TIME_CYCL / 2), (CONSTANT::PWM_WRAP_VAL_PHASE / 2) - (CONSTANT::DEAD_TIME_CYCL / 2));
	pwm_set_enabled(sliceNum, true);
}


void initI2C() {
	
	i2c_init(i2c1, I2C::I2CFREQ);
	gpio_pull_up(PIN::SDA_PIN);
	gpio_pull_up(PIN::SCL_PIN);
	gpio_set_function(PIN::SDA_PIN, GPIO_FUNC_I2C);
	gpio_set_function(PIN::SCL_PIN, GPIO_FUNC_I2C);
}


void initDisplay(OBDISP& oled) {

	while ( obdI2CInit(&oled, OLED::_128x64, OLED::ADDRESS, OLED::FLIP_180, OLED::INVERT, OLED::USE_HW_I2C, OLED::SDA_PIN, OLED::SCL_PIN, OLED::RESET_PIN, I2C::I2CFREQ, i2c1) < 0);
	//obdSetBackBuffer(&oled, bbuffer);
}


void initInputs() {

	gpio_set_dir(PIN::ENCODER_PIN1, false);
	gpio_set_input_hysteresis_enabled(PIN::ENCODER_PIN1, true);
	gpio_set_dir(PIN::ENCODER_PIN2, false);
	gpio_set_input_hysteresis_enabled(PIN::ENCODER_PIN2, true);
	gpio_set_dir(PIN::ENCODER_BUTTON_PIN, false);
	gpio_set_input_hysteresis_enabled(PIN::ENCODER_BUTTON_PIN, true);

	
	gpio_set_irq_enabled_with_callback(PIN::ENCODER_PIN1, GPIO_IRQ_EDGE_FALL + GPIO_IRQ_EDGE_RISE, true, &InterruptableGPIO::gpioInterruptHandler);

	gpio_set_irq_enabled_with_callback(PIN::ENCODER_PIN2, GPIO_IRQ_EDGE_FALL + GPIO_IRQ_EDGE_RISE, true, &InterruptableGPIO::gpioInterruptHandler);

	gpio_set_irq_enabled_with_callback(PIN::ENCODER_BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, &InterruptableGPIO::gpioInterruptHandler);
}


void init() {

	stdio_init_all();
	for(int i = 0; i < 50; ++i) std::cout << std::endl;
	initI2C();
	initPWM();
	initInputs();
}



int main(int argc, const char* argv[]) {

	init();
	initDisplay(oled);
	
	RotaryEncoder rotary(PIN::ENCODER_PIN1, PIN::ENCODER_PIN2, PIN::ENCODER_BUTTON_PIN, [](){ m1.upButton(); }, [](){ m1.downButton(); });

	for(uint i{0}; i < 8; ++i) obdWriteString(&oled, 0, 0, i, (char*)"                ", FONT_8x8, false, true);
	
	sleep_ms(750);

	obdWriteString(&oled, 0, 0, 2, (char*)"Up and running.", FONT_8x8, false, true);

	sleep_ms(1500);
	for(uint i{0}; i < 8; ++i) obdWriteString(&oled, 0, 0, i, (char*)"                ",FONT_8x8, false, true); 


	m1.display();

	while (1) {
		//sleep_ms();
		//m1.display();
		tight_loop_contents();
	}

	return 0;
}
