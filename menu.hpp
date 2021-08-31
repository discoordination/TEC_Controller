#ifndef _MENU_HPP__
#define _MENU_HPP__

#include "pico/stdlib.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>


#pragma message "TODO: Add new features."
#pragma message "TODO: Adapt to use different font sizes"
#pragma message "TODO: Add different types of menu item such as setting adjust"


class BasicMenuItem {

private:
	std::string content; // Mutable so as it can be aligned by the menu.
	volatile bool dirty;

	void removeLeadingSpace();
	void removeTrailingSpace();

protected:
	BasicMenuItem(const std::string& content) : content(content), dirty(true) {}
	virtual ~BasicMenuItem() {}

public:
	std::string& getContent() { return content; }

	virtual bool selectable() const = 0;
	virtual bool scrollable() const = 0;

	void alignLeft(uint screenWidth);
	void alignCenter(uint screenWidth);
	void alignRight(uint screenWidth);

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
	enum class Alignment {
		Left, Center, Right
	};

// Set these globally.
private:
public:
	const static std::function<void(std::string, int yPos, bool inverted)> drawLineFunction;
	const static std::function<void(int x1, int y1, int x2, int y2, uint8_t colour, uint8_t filled)> drawRectangleFunction;
	const static std::function<void()> dumpBufferFunction;

private:
	void display();
	void drawFuncsInitialised();	// Allow to assert menu initialized properly.
	std::vector<std::shared_ptr<BasicMenuItem>> items;
	const uint width;	// width of screen in characters.
	const uint height;	// height of screen in characters
	uint titleHeight;	// top of scrolling part of the screen below title.
	Alignment alignment;	// Alignment of screen items.
	volatile int index;			// index of selection on screen.

	uint screenTopItOffs; // top of scrolling section of screen offset.
	uint screenBottomItOffs; // bottom of screen offset.

	std::function<void()> enterButtonLongPressFunc;
	bool closing;

	void align(BasicMenuItem& item, Alignment how);
	void markAllDirty() { 
		for (int i = titleHeight; i < items.size(); ++i) { items[i]->markDirty(); }
	}

// set the menu to ignore certain input.
	bool ignoreRotary;
	bool ignoreButton;

public:
// Initialize with vector of menu items shared_ptr.
// width and height.
// the drawline function of your display library wrapped to allow same args.
// the drawRectangle function of your display library wrapped.
// the dumpBuffer function of your display library. optional.
// desired menu alignment
// desired start pos of menu.
	Menu(	std::vector<std::shared_ptr<BasicMenuItem>> items,
		 	uint width,
			uint height,
			Alignment alignment = Alignment::Left,
			int startIndex = -1,
			std::function<void()> longPressFunc = {}
			);

	Menu(uint width, uint height, Alignment alignment, int startIndex);
	//Menu();
	//Menu(const Menu& other);

	void addItem(std::shared_ptr<BasicMenuItem> item);
	void addItems(std::vector<std::shared_ptr<BasicMenuItem>> items);

	int downButton();
	int upButton();
	int enterButtonDown();
	int enterButtonUp();
	int enterButtonPressedLong();

	void operator()();
	void closeMenu() { closing = true; }
};



#endif // _MENU_HPP__
