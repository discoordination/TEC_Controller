#ifndef _GPIO_HPP__
#define _GPIO_HPP__


#include "pico/stdlib.h"
#include "hardware/irq.h"

//#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <iostream>


#pragma message "TODO: Give unique id to Interruptable GPIOs so they can be removed on object destruction.  Disabled etc."
#pragma message "Move long press to PushButtonGPIO???"


class InterruptableGPIO {
	
	inline static std::map<uint, InterruptableGPIO*> interruptableGPIOs;
	virtual void triggered(uint gpio, uint32_t events) = 0;

protected:
	bool enabled;
	InterruptableGPIO(uint8_t pin);
	~InterruptableGPIO() { interruptableGPIOs.erase(pin); };

public:
	InterruptableGPIO& operator=(InterruptableGPIO&& other);
	
	uint8_t pin;
	void disable() { enabled = false; }
	static int64_t reenableGPIOCallback(alarm_id_t id, void* userData);
	static void gpioInterruptHandler(uint gpio, uint32_t events);
};



class PushButton;

class PushButtonGPIO : public InterruptableGPIO {

public:
	enum class ButtonState { NotPressed, Pressed };

private:
	PushButton* parent;
	uint debounceMS;
	repeating_timer t;
	uint count;

	static bool debounceTimerCallback(repeating_timer_t *t);

public:
	PushButtonGPIO(uint8_t pin, PushButton* parent, uint debounceMS);
	ButtonState buttonState;
	void triggered(uint gpio, uint32_t events) override;
};




class PushButton {

private:
	PushButtonGPIO buttonGPIO;
	std::function<void()> buttonDownFunction;
	std::function<void()> buttonUpFunction;
	std::function<void()> buttonLongPressFunction;

	uint longPressTime;
	alarm_id_t longPressAlarmID;
	static int64_t longPressCallback(alarm_id_t id, void* userData);
public:
	PushButton (uint gpio, std::function<void()> buttonDownFunction, std::function<void()> buttonUpFunction, std::function<void()> buttonLongPressFunction, uint longPressTime, uint debounceMS);

	void buttonUp();
	void buttonDown();
};


class RotaryEncoder;

class RotaryEncoderEncoderGPIO : public InterruptableGPIO {
	RotaryEncoder* parent;
public:
	RotaryEncoderEncoderGPIO(uint8_t pin, RotaryEncoder* parent);
	void triggered(uint gpio, uint32_t events) override;
};



class RotaryEncoder {

	RotaryEncoderEncoderGPIO p1;
	RotaryEncoderEncoderGPIO p2; 
	PushButton button;
	uint8_t state;

	std::function<void()> ccFunction;
	std::function<void()> cFunction;

public:
	RotaryEncoder(const uint8_t p1, const uint8_t p2, const uint8_t buttonPin, std::function<void()> ccFunction, std::function<void()> cFunction, std::function<void()> butDownFunc, std::function<void()> butUpFunc, std::function<void()> longPressFunc);

	RotaryEncoder(const uint8_t p1, const uint8_t p2, std::function<void()> ccFunction, std::function<void()> cFunction);

	void triggered(uint gpio, uint32_t events);

	void buttonDown();
	void buttonUp();
};

#endif // _GPIO_HPP__

