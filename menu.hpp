#ifndef _MENU_HPP__
#define _MENU_HPP__

#include "pico/stdlib.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>


class BasicMenuItem {

private:
	std::string content; // Mutable so as it can be aligned by the menu.
	bool dirty;

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

public:
	MenuButton(const std::string& content, const std::function<void()>& onClick = {}) : 
		BasicMenuItem(content),
		onClick(onClick)
	{}
	bool selectable() const override { return true; }
	bool scrollable() const override { return true; }
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
	std::vector<std::shared_ptr<BasicMenuItem>> items;
	const uint width;
	const uint height;
	Alignment alignment;
	int index;
	uint screenTop;
	std::function<void(std::string, int yPos, bool inverted)> drawLineFunction;

	void align(std::string& tString, Alignment how);
	void markAllDirty() { 
		for (int i = screenTop; i < screenTop + height; ++i) { items[i]->markDirty(); }
	}

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
			Alignment alignment = Alignment::Left,
			int startIndex = 0  );

	// Menu(std::vector<std::unique_ptr<BasicMenuItem>> items, uint width, uint height, std::function<void(std::string, int yPos, bool inverted)> drawLineFunc, Alignment alignment = Alignment::Left, int startIndex = 0) : 
	// 			Menu("", items, width, height + 1, drawLineFunc, alignment, startIndex) {
	// }


	void display();
	int downButton();
	int upButton();
	int enterButton() { return 1; }
};

#endif // _MENU_HPP__
