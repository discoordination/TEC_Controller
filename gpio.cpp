#include "main.hpp"
#include "gpio.hpp"
#include <iostream>
#include <algorithm>
#include <cassert>
#include <array>
#include <bitset>


InterruptableGPIO::InterruptableGPIO(uint8_t pin) : 
			pin(pin),
			enabled(true) {
	interruptableGPIOs.push_back(this);
}


InterruptableGPIO& InterruptableGPIO::operator=(InterruptableGPIO&& other) {
	std::cout << "Called copy assignment operator" << std::endl;
	assert(1 && "This should not be called.");
	return *this;
}


void InterruptableGPIO::gpioInterruptHandler(uint gpio, uint32_t events) {

	for (auto it = interruptableGPIOs.begin(); it != interruptableGPIOs.end(); ++it) {
		auto interruptableGPIO = *it;
		if (interruptableGPIO->enabled && gpio == interruptableGPIO->pin) {
			interruptableGPIO->triggered();
			//add_alarm_in_ms(IO::DEBOUNCE_MS, &InterruptableGPIO::reenableGPIOCallback, &interruptableGPIO->pin, true);
		}
 	}
}


int64_t InterruptableGPIO::reenableGPIOCallback(alarm_id_t id, void* userData) {

	auto pinPtr = static_cast<uint8_t*>(userData);
	auto pin = *pinPtr;
	auto& vec = InterruptableGPIO::interruptableGPIOs;
	auto result = std::find_if(vec.begin(), vec.end(), [pin](auto& gp) {return gp->pin == pin;});
	if (result != std::end(vec)) { (*result)->enabled = true; gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_FALL, true); }
	return 0;
}


void RotaryEncoderButtonGPIO::triggered() { parent->triggered(); }
void RotaryEncoderPushButtonGPIO::triggered() { parent->buttonTriggered(); }


constexpr uint8_t DIR_NONE		{ 0x00 };
constexpr uint8_t DIR_CW		{ 0x10 };
constexpr uint8_t DIR_CCW		{ 0x20 };

// Use the full-step state table (emits a code at 00 only)
constexpr uint8_t R_START		{ 0x0 }; // 0b 0000
constexpr uint8_t R_CW_FINAL 	{ 0x1 }; // 0b 0001
constexpr uint8_t R_CW_BEGIN 	{ 0x2 }; // 0b 0010
constexpr uint8_t R_CW_NEXT 	{ 0x3 }; // 0b 0011
constexpr uint8_t R_CCW_BEGIN 	{ 0x4 }; // 0b 0100
constexpr uint8_t R_CCW_FINAL 	{ 0x5 }; // 0b 0101
constexpr uint8_t R_CCW_NEXT 	{ 0x6 }; // 0b 0110


const std::array<std::array<uint8_t, 4>, 7> ttable {
  // R_START
	std::array<uint8_t, 4>{ !R_START, R_CW_BEGIN, R_CCW_BEGIN, R_START },
  // R_CW_FINAL
    std::array<uint8_t, 4>{ R_CW_NEXT, R_START, R_CW_FINAL, R_START | DIR_CW },
  // R_CW_BEGIN
    std::array<uint8_t, 4>{ R_CW_NEXT, R_CW_BEGIN, R_START,   R_START },
  // R_CW_NEXT
    std::array<uint8_t, 4>{ R_CW_NEXT, R_CW_BEGIN, R_CW_FINAL, R_START },
  // R_CCW_BEGIN
    std::array<uint8_t, 4>{ R_CCW_NEXT, R_START, R_CCW_BEGIN, R_START },
  // R_CCW_FINAL
    std::array<uint8_t, 4>{ R_CCW_NEXT, R_CCW_FINAL, R_START, R_START | DIR_CCW },
  // R_CCW_NEXT
    std::array<uint8_t, 4>{ R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START }
};




RotaryEncoder::RotaryEncoder(uint8_t p1, uint8_t p2, uint8_t buttonPin, std::function<void()> ccFunction, std::function<void()> cFunction) : 
		p1(p1, this),
		p2(p2, this),
		button(buttonPin, this),
		state(R_START),
		ccFunction(ccFunction),
		cFunction(cFunction)
{}


void RotaryEncoder::triggered() {

	uint8_t pinstate = (gpio_get(PIN::ENCODER_PIN1) << 1) | gpio_get(PIN::ENCODER_PIN2);
	state = ttable[state & 0xF][pinstate];
	
	if ((state & 0x30) == DIR_CW) {
		cFunction();
	} else if ((state & 0x30) == DIR_CCW) {
		ccFunction();
	} 
}
