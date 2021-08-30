#include "main.hpp"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "hardware/irq.h"
#include "menu.hpp"
#include "gpio.hpp"

#include <iostream>
#include <vector>
#include <bitset>
#include <any>

OBDISP oled;
uint8_t bbuffer[1024];

void runM2();

enum class ShownMenu { menu1, menu2 };
ShownMenu shownMenu = ShownMenu::menu1;


void showMenu2();


Menu m1 { 
			std::vector<std::shared_ptr<BasicMenuItem>> { 
						std::make_shared<MenuTitle>("MENU"),
						std::make_shared<MenuButton>("One", showMenu2),
						std::make_shared<MenuButton>("Two"),
						std::make_shared<MenuButton>("Three"),
						std::make_shared<MenuButton>("Four") },
						//std::make_shared<MenuButton>("Five"),
						//std::make_shared<MenuButton>("Six"),
						//std::make_shared<MenuButton>("Seven"),
						//std::make_shared<MenuButton>("Eight"),
						//std::make_shared<MenuButton>("Nine") },
			128 / 8,
			8,
			[](std::string str, int yPos, bool inv) { 
				obdWriteString(&oled, false, 0, yPos, const_cast<char*>(str.c_str()), FONT_8x8, inv, true);
				},
			[](int x1, int y1, int x2, int y2, uint8_t colour, uint8_t filled) {
				obdRectangle(&oled, x1, y1, x2, y2, colour, filled);
			},
			[](){
				obdDumpBuffer(&oled, bbuffer);
			},
			Menu::Alignment::Center 
		};

void showMenu2() { 
	m1.endMenu(); 
	shownMenu = ShownMenu::menu2;
}

Menu m2 {
		std::vector<std::shared_ptr<BasicMenuItem>> {
			std::make_shared<MenuTitle>("MENU 2"),
			std::make_shared<MenuButton>("Say Hi"),
			std::make_shared<MenuButton>("Say Ho"),
			std::make_shared<MenuButton>("Say No") },
		128 / 8,
		8,
		[](std::string str, int yPos, bool inv) { 
			obdWriteString(&oled, false, 0, yPos, const_cast<char*>(str.c_str()), FONT_8x8, inv, true);
		},
		[](int x1, int y1, int x2, int y2, uint8_t colour, uint8_t filled) {
			obdRectangle(&oled, x1, y1, x2, y2, colour, filled);
		},
		[](){
			obdDumpBuffer(&oled, bbuffer);
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
	obdSetBackBuffer(&oled, bbuffer);
	// sometimes oled isn't flipped so flip it again.
	if (!oled.flip) oled.flip;
}


void initInputs() {
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
	
	for(uint i{0}; i < 8; ++i) obdWriteString(&oled, 0, 0, i, (char*)"                ", FONT_8x8, false, true);
	
	sleep_ms(750);

	obdWriteString(&oled, 0, 0, 2, (char*)"Up and running.", FONT_8x8, false, true);

	sleep_ms(1500);
	for(uint i{0}; i < 8; ++i) obdWriteString(&oled, 0, 0, i, (char*)"                ",FONT_8x8, false, true); 



	while (1) {
		volatile int x = 10;
		switch (shownMenu) {
			case ShownMenu::menu1: {
				RotaryEncoder rotary(PIN::ENCODER_PIN1, PIN::ENCODER_PIN2, PIN::ENCODER_BUTTON_PIN, [](){ m1.upButton(); }, [](){ m1.downButton(); },[](){ m1.enterButtonDown(); } , [](){ m1.enterButtonUp(); }, [](){});
				m1();
				break;
			}
			case ShownMenu::menu2: {
				RotaryEncoder rotary(PIN::ENCODER_PIN1, PIN::ENCODER_PIN2, PIN::ENCODER_BUTTON_PIN, [](){ m2.upButton(); }, [](){ m2.downButton(); },[](){ m2.enterButtonDown(); } , [](){m2.enterButtonUp(); }, [](){ shownMenu = ShownMenu::menu1; m2.endMenu(); });
				m2();
				break;
			}
		}
	}

	return 0;
}
