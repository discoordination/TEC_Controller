#ifndef _GPIO_HPP__
#define _GPIO_HPP__


#include "pico/stdlib.h"
#include "hardware/irq.h"

#include <vector>
#include <functional>
#include <iostream>

class InterruptableGPIO {
	
	inline static std::vector<InterruptableGPIO*> interruptableGPIOs;
	virtual void triggered() = 0;

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


class RotaryEncoder;


class RotaryEncoderButtonGPIO : public InterruptableGPIO {
	RotaryEncoder* parent;
public:
	RotaryEncoderButtonGPIO(uint8_t pin, RotaryEncoder* parent) : 
									InterruptableGPIO(pin), parent(parent) {}
	void triggered();
};



class RotaryEncoderPushButtonGPIO : public InterruptableGPIO {
	RotaryEncoder* parent;
public:
	RotaryEncoderPushButtonGPIO(uint8_t pin, RotaryEncoder* parent) : 
									InterruptableGPIO(pin), parent(parent) {}
	void triggered();
};



class RotaryEncoder {

	RotaryEncoderButtonGPIO p1;
	RotaryEncoderButtonGPIO p2; 
	RotaryEncoderPushButtonGPIO button;
	uint8_t state;


public:
	RotaryEncoder(const uint8_t p1, const uint8_t p2, const uint8_t buttonPin, std::function<void()> ccFunction, std::function<void()> cFunction);

	std::function<void()> ccFunction;
	std::function<void()> cFunction;

	void triggered();
	void buttonTriggered() {
		std::cout << "push button\n";
		gpio_set_irq_enabled(button.pin, GPIO_IRQ_EDGE_FALL,false); 
	}
};

#endif // _GPIO_HPP__
