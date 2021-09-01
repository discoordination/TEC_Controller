#ifndef _MENU_HPP__
#define _MENU_HPP__

#include "pico/stdlib.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <sstream>
#include <iomanip>


#pragma message "TODO: Add new features."
#pragma message "TODO: Fix crash on setting onclick"
#pragma message "TODO: Add different types of menu item such as setting adjust"
#pragma message "TODO: Great refactoring idea.  Add a static or referenced display struct that provides all the information about the display so that you don't have to pass it in for each menu.  This would mean that the constructors for the menus could be shorter and simpler."


namespace MenuUtils {
	enum class Alignment {
		Left, Center, Right
	};
}





// BasicMenuItem

class BasicMenuItem {

private:
	volatile bool dirty;

protected:
	std::string content; // Mutable so as it can be aligned by the menu.
	BasicMenuItem(const std::string& content) : dirty(true), content(content) {}
	virtual ~BasicMenuItem() {}

public:
	std::string& getContent() { return content; }

	virtual bool selectable() const = 0;
	virtual bool scrollable() const = 0;

	virtual void align(const uint screenWidth, const MenuUtils::Alignment align);

	void markDirty() { dirty = true; }
	void markClean() { dirty = false; }
	bool isDirty() const { return dirty; }
};



class MenuButton : public BasicMenuItem {

	std::function<void()> onClick;
	// you can choose these functions to do something funky but then call the onClick function.
	std::function<void()> buttonDownFunction;
	std::function<void()> buttonUpFunction;

public:
	MenuButton(const std::string& content, const std::function<void()>& onClick = {}) : 
		BasicMenuItem(content),
		onClick(onClick)
	{}
	bool selectable() const override { return true; }
	bool scrollable() const override { return true; }
	void operator()() const {  if (onClick) onClick(); }
};



class MenuTitle : public BasicMenuItem {


public:
	MenuTitle(const std::string& content) :
		BasicMenuItem(content)
	{}
	bool selectable() const override { return false; }
	bool scrollable() const override { return false; }
};




#pragma message "TODO: specialize for double and float so that only they have nDecimal places"
template <typename T>
class MenuSetting : public BasicMenuItem {

	T& settingRef;
	const T min, max;
	const bool showSign;
	const uint8_t nDecimalPlaces;

public:
	MenuSetting(const std::string& name, T& settingRef, const T min, const T max, bool showSign = false, const uint8_t nDecimalPlaces = 0) : 
		BasicMenuItem(name),
		settingRef(settingRef),
		min(min),
		max(max),
		showSign(showSign),
		nDecimalPlaces(nDecimalPlaces)
	{}

	void align(const uint screenWidth, const MenuUtils::Alignment alignment) override;

	bool selectable() const override { return true; }
	bool scrollable() const override { return true; }
	//void operator()() const {  if (onClick) onClick(); }
};


#include <iostream>
template <typename T>
void MenuSetting<T>::align(const uint screenWidth, const MenuUtils::Alignment alignment) {
	// ignore aligment for this one we will just stick content on the right and the value on the left.
	BasicMenuItem::align(screenWidth, MenuUtils::Alignment::Left);  // aligns the title.
	std::stringstream stream;
	if (showSign) stream << std::showpos;
	stream << std::fixed << std::setprecision(nDecimalPlaces) << settingRef << std::endl;
	std::string val;
	stream >> val;
	// std::cout << "Content length: " << content.length() << std::endl;
	// std::cout << "Content: " << content << ".\n";
	// std::cout << "val: " << std::showpos << val << std::endl;
	//content.insert(content.length(), std::string(screenWidth - content.length' '));
	content.insert(content.length() - val.length(), val);
}




// Example of how to initialize menu drawing functions.

// #include <functional>

// const std::function<void(std::string, int, bool)> Menu::drawLineFunction { 
// 	[](std::string str, int yPos, bool inv){ 
//  		obdWriteString(&oled, false, 0, yPos, const_cast<char*>(str.c_str()), FONT_8x8, inv, true); 
// }};

// const std::function<void(int,int,int,int,uint8_t,uint8_t)> Menu::drawRectangleFunction {
// 	[](int x1, int y1, int x2, int y2, uint8_t colour, uint8_t filled) {
//  		obdRectangle(&oled, x1, y1, x2, y2, colour, filled);
// }};

// const std::function<void()> Menu::dumpBufferFunction {
// 		[]() { obdDumpBuffer(&oled, bbuffer);
// }};



class Menu {

public:

// Set these globally.
	const static std::function<void(std::string&, int yPos, bool inverted, int fontCmd)> drawLineFunction;
	const static std::function<void(int x1, int y1, int x2, int y2, uint8_t colour, uint8_t filled)> drawRectangleFunction;
	const static std::function<void()> dumpBufferFunction;

private:
	std::vector<std::shared_ptr<BasicMenuItem>> items;
	const uint widthColumns;	// width of screen in characters.
	const uint heightRows;	// height of screen in characters
	const uint widthPixels;
	const uint heightPixels;
	const uint fontWidth;
	const uint fontHeight;
	const int fontCmd; 				// how to choose the font.
	const uint byteRowsPerCharacter;	// Write functions are all in byte row format.
	uint titleHeight;				// top of scrolling part of the screen below title.
	MenuUtils::Alignment alignment;	// Alignment of screen items.
	volatile int index;			// index of selection on screen.

	uint screenTopItOffs; // top of scrolling section of screen offset.
	uint screenBottomItOffs; // bottom of screen offset.
	
	bool ignoreRotary;
	bool ignoreButton;
	bool closing; // Breaks out of the operator() loop.

	std::function<void()> enterButtonLongPressFunc; // What to do on a long press.  This is not for a particular item but for the whole menu.

	// set the menu to ignore input of certain types.

	void align(BasicMenuItem& item, MenuUtils::Alignment how);
	void draw();					// Redraw the menu. Could be public.
	void drawFuncsInitialised();	// Allow to assert menu initialized properly.
	void markAllDirty();			// Menu only draws dirty items.


public:
// Initialize with vector of menu items shared_ptr.
// width and height.
// the drawline function of your display library wrapped to allow same args.
// the drawRectangle function of your display library wrapped.
// the dumpBuffer function of your display library. optional.
// desired menu alignment
// desired start pos of menu.
	Menu(	std::vector<std::shared_ptr<BasicMenuItem>> items,
		 	uint widthPixels,
			uint heightPixels,
			uint fontWidth,
			uint fontHeight,
			int fontCmd,
			MenuUtils::Alignment alignment = MenuUtils::Alignment::Left,
			int startIndex = -1,
			std::function<void()> longPressFunc = {}
		);

	Menu(uint widthPixels, uint heightPixels, uint fontWidth, uint fontHeight, int fontCmd, MenuUtils::Alignment alignment, int startIndex);
	//Menu();
	//Menu(const Menu& other);

	void addItem(const std::shared_ptr<BasicMenuItem>& item);
	void addItems(const std::vector<std::shared_ptr<BasicMenuItem>>& items);

	int downButton();
	int upButton();
	int enterButtonDown();
	int enterButtonUp();
	int enterButtonPressedLong();

	void operator()();  // Runs the menu in a loop.
	void closeMenu() { closing = true; } // Breaks out of the loop.
};



#endif // _MENU_HPP__
