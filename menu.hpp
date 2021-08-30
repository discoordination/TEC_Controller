#ifndef _MENU_HPP__
#define _MENU_HPP__

#include "pico/stdlib.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>


#pragma message "TODO: Adapt to use different font sizes"
#pragma message "TODO: Add different types of menu item such as setting adjust"


class BasicMenuItem {

private:
	std::string content; // Mutable so as it can be aligned by the menu.
	volatile bool dirty;

protected:
	BasicMenuItem(const std::string& content) : content(content), dirty(true) {}
	virtual ~BasicMenuItem() {}

public:
	std::string& getContent() { return content; }

	virtual bool selectable() const = 0;
	virtual bool scrollable() const = 0;

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





class Menu {

public:
	enum class Alignment {
		Left, Center, Right
	};

private:
	void display();

	std::vector<std::shared_ptr<BasicMenuItem>> items;
	const uint width;	// width of screen in characters.
	const uint height;	// height of screen in characters
	const uint titleHeight;	// top of scrolling part of the screen below title.
	Alignment alignment;	// Alignment of screen items.
	volatile int index;			// index of selection on screen.
	std::function<void(std::string, int yPos, bool inverted)> drawLineFunction;
	std::function<void(int x1, int y1, int x2, int y2, uint8_t colour, uint8_t filled)> drawRectangleFunction;
	std::function<void()> dumpBufferFunction;

	uint screenTopItOffs; // top of scrolling section of screen offset.
	uint screenBottomItOffs; // bottom of screen offset.

	bool wantsToClose;

	void align(std::string& tString, Alignment how);
	void markAllDirty() { 
		for (int i = titleHeight; i < items.size(); ++i) { items[i]->markDirty(); }
	}

// Problem... doesn't know about rotary encoder.  better not... better disable paying attention.
	bool ignoreRotary;
	bool ignoreButton;

	void drawButtonPressed();
	void drawButtonUnpressed();

public:
// Initialize with vector of menu items shared_ptr.
// width and height.
// the drawline function of your display library wrapped to allow same args.
// desired menu alignment
// desired start pos of menu.
	Menu(	std::vector<std::shared_ptr<BasicMenuItem>> items,
		 	uint width,
			uint height,
			std::function<void(std::string, int yPos, bool inverted)> drawLineFunc,
			std::function<void(int x1, int y1, int x2, int y2, uint8_t colour, uint8_t filled)> drawRecFunc,
			std::function<void()> dumpBufferFunc = {},
			Alignment alignment = Alignment::Left,
			int startIndex = -1 );

	int downButton();
	int upButton();
	int enterButtonDown();
	int enterButtonUp();
	void operator()();
	void endMenu() { wantsToClose = true; }
};





#endif // _MENU_HPP__
