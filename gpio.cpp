#include "main.hpp"
#include "gpio.hpp"
#include <iostream>
#include <algorithm>
#include <cassert>
#include <array>
#include <bitset>


// InterruptableGPIO

InterruptableGPIO::InterruptableGPIO(uint8_t pin) : 
			pin(pin),
			enabled(true) {
	interruptableGPIOs.push_back(this);
	gpio_set_dir(pin, false);
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
			interruptableGPIO->triggered(gpio, events);
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




// RotaryEncoderEncoderGPIO

RotaryEncoderEncoderGPIO::RotaryEncoderEncoderGPIO(uint8_t pin, RotaryEncoder* parent) : InterruptableGPIO(pin), parent(parent) {

	gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_FALL + GPIO_IRQ_EDGE_RISE, true, &InterruptableGPIO::gpioInterruptHandler);
}

void RotaryEncoderEncoderGPIO::triggered(uint gpio, uint32_t events) { 
	parent->triggered(gpio, events); 
}

#warning This is where we will differentiate between up and down functions.
#warning Need to implement some debounce here.
// void RotaryEncoderPushButtonGPIO::triggered(uint gpio, uint32_t events) {
// 	if (events == GPIO_IRQ_EDGE_FALL) {
// 		gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, false);
// 		gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_RISE, true);
// 		parent->buttonPressed();
// 	} else if (events == GPIO_IRQ_EDGE_RISE) {
// 		gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_RISE, false);
// 		gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, true);
// 		parent->buttonUp();
// 	}
// }




// PushButtonGPIO

PushButtonGPIO::PushButtonGPIO(uint8_t pin, PushButton* parent, uint debounceMS) : InterruptableGPIO(pin), parent(parent), debounceMS(debounceMS), t(), count(0), buttonState(ButtonState::NotPressed) {

	gpio_set_irq_enabled_with_callback(pin, GPIO_IRQ_EDGE_FALL + GPIO_IRQ_EDGE_RISE, true, &InterruptableGPIO::gpioInterruptHandler);
}


bool PushButtonGPIO::debounceTimerCallback(repeating_timer_t* t) {

	PushButtonGPIO* gpio = static_cast<PushButtonGPIO*>(t->user_data);

	if (gpio_get(gpio->pin) == 0 && gpio->buttonState == ButtonState::Pressed) {
		if (++gpio->count == gpio->debounceMS) {
			gpio->count = 0;
			gpio_set_irq_enabled_with_callback(gpio->pin, GPIO_IRQ_EDGE_FALL + GPIO_IRQ_EDGE_RISE, true, &InterruptableGPIO::gpioInterruptHandler);
			gpio->parent->buttonDown();
			return false;
		}
	} else if (gpio_get(gpio->pin) == 1 && gpio->buttonState == ButtonState::NotPressed) {
		if (++gpio->count == gpio->debounceMS) {
			gpio->count = 0;
			gpio_set_irq_enabled_with_callback(gpio->pin, GPIO_IRQ_EDGE_FALL + GPIO_IRQ_EDGE_RISE, true, &InterruptableGPIO::gpioInterruptHandler);
			gpio->parent->buttonUp();
			return false;
		}
	} else if (gpio->buttonState == ButtonState::Pressed) {
		gpio->buttonState == ButtonState::NotPressed;
		gpio_set_irq_enabled_with_callback(gpio->pin, GPIO_IRQ_EDGE_FALL + GPIO_IRQ_EDGE_RISE, true, &InterruptableGPIO::gpioInterruptHandler);
		return false;
	} else {
		gpio->buttonState == ButtonState::Pressed;
		gpio_set_irq_enabled_with_callback(gpio->pin, GPIO_IRQ_EDGE_FALL + GPIO_IRQ_EDGE_RISE, true, &InterruptableGPIO::gpioInterruptHandler);
		return false;
	}
	return true;
} 


// tell it the button state is what it is.  turn off the interrupt and set an alarm 
void PushButtonGPIO::triggered(uint gpio, uint32_t events) {

	gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_FALL + GPIO_IRQ_EDGE_RISE, false);

	if (events == GPIO_IRQ_EDGE_FALL) {
		buttonState = ButtonState::Pressed;
	} else if (events == GPIO_IRQ_EDGE_RISE) {
		buttonState = ButtonState::NotPressed;
	}
	
	add_repeating_timer_ms(1, debounceTimerCallback, this, &t);
}




// PushButton

PushButton::PushButton (uint gpio, std::function<void()> buttonDownFunction, std::function<void()> buttonUpFunction, uint debounceMS = 5) : buttonGPIO(gpio, this, debounceMS), buttonDownFunction(buttonDownFunction), buttonUpFunction(buttonUpFunction) {
}

void PushButton::buttonUp() {
	std::cout << "button up detected.\n";
}

void PushButton::buttonDown() {
	std::cout << "button down detected.\n";
}




// RotaryEncoder

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


RotaryEncoder::RotaryEncoder(const uint8_t p1, const uint8_t p2, const uint8_t buttonPin, std::function<void()> ccFunction, std::function<void()> cFunction, std::function<void()> butDownFunc, std::function<void()> butUpFunc) : 
					p1(p1, this),
					p2(p2, this),
					state(R_START),
					ccFunction(ccFunction),
					cFunction(cFunction),
					button(buttonPin, butDownFunc, butUpFunc)
{}

RotaryEncoder::RotaryEncoder(uint8_t p1, uint8_t p2, std::function<void()> ccFunction, std::function<void()> cFunction) : 
		RotaryEncoder(p1, p2, 255, ccFunction, cFunction, {}, {})
{}


void RotaryEncoder::triggered(uint gpio, uint32_t events) {

	uint8_t pinstate = (gpio_get(PIN::ENCODER_PIN1) << 1) | gpio_get(PIN::ENCODER_PIN2);
	state = ttable[state & 0xF][pinstate];
	
	if ((state & 0x30) == DIR_CW) {
	//	std::cout << "CW\n";
		cFunction();
	} else if ((state & 0x30) == DIR_CCW) {
	//	std::cout << "CCW\n";
		ccFunction();
	}
}

void RotaryEncoder::buttonDown() {
	std::cout << "Button down\n";
}

void RotaryEncoder::buttonUp() {
	std::cout << "Button up\n";
}
