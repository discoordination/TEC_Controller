#ifndef _GPIO_HPP__
#define _GPIO_HPP__


#include "pico/stdlib.h"
#include "hardware/irq.h"

#include <vector>
#include <functional>
#include <iostream>

class InterruptableGPIO {
	
	inline static std::vector<InterruptableGPIO*> interruptableGPIOs;
	virtual void triggered(uint gpio, uint32_t events) = 0;

protected:
	bool enabled;

public:
	InterruptableGPIO(uint8_t pin);
	InterruptableGPIO& operator=(InterruptableGPIO&& other);
	
	uint8_t pin;
	void disable() { enabled = false; }
	static int64_t reenableGPIOCallback(alarm_id_t id, void* userData);
	static void gpioInterruptHandler(uint gpio, uint32_t events);
};

#warning you need a button to make a button work such as you have a rotary encoder.
class PushButton;

class PushButtonGPIO : public InterruptableGPIO {
	PushButton* parent;
public:
	PushButtonGPIO(uint8_t pin, PushButton* parent) : InterruptableGPIO(pin), parent(parent) {}
	void triggered(uint gpio, uint32_t events) override;
};


class PushButton {
public:
	enum class ButtonState { NotPressed, Pressed };

private:
	PushButtonGPIO buttonGPIO;
	ButtonState buttonState;
	std::function<void()> buttonDownFunction;
	std::function<void()> buttonUpFunction;

public:
	PushButton (uint gpio, std::function<void()> buttonDownFunction, std::function<void()> buttonUpFunction) : buttonGPIO(gpio, this), buttonState(ButtonState::NotPressed), buttonDownFunction(buttonDownFunction), buttonUpFunction(buttonUpFunction) {}
	//void buttonPressed(uint gpio, uint32_t event);
	void buttonUp();
	void buttonDown();
};


class RotaryEncoder;

class RotaryEncoderEncoderGPIO : public InterruptableGPIO {
	RotaryEncoder* parent;
public:
	RotaryEncoderEncoderGPIO(uint8_t pin, RotaryEncoder* parent) : InterruptableGPIO(pin), parent(parent) {}
	void triggered(uint gpio, uint32_t events) override;
};



// class RotaryEncoderPushButtonGPIO : public PushButtonGPIO {
// 	PushButton* parent;
// public:
// 	RotaryEncoderPushButtonGPIO(uint8_t pin, PushButton* parent) : 
// 									PushButtonGPIO(pin, parent) {}
// 	void triggered(uint gpio, uint32_t events) override;
// };


#warning you are adding a push button to the Rotary Encoder
class RotaryEncoder {

	RotaryEncoderEncoderGPIO p1;
	RotaryEncoderEncoderGPIO p2; 
	PushButton button;
	uint8_t state;

	std::function<void()> ccFunction;
	std::function<void()> cFunction;

public:
	RotaryEncoder(const uint8_t p1, const uint8_t p2, const uint8_t buttonPin, std::function<void()> ccFunction, std::function<void()> cFunction, std::function<void()> butDownFunc, std::function<void()> butUpFunc);

	RotaryEncoder(const uint8_t p1, const uint8_t p2, std::function<void()> ccFunction, std::function<void()> cFunction);

	void triggered(uint gpio, uint32_t events);
	void buttonDown();
	void buttonUp();
};

#endif // _GPIO_HPP__

