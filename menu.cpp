#include "menu.hpp"


#include <algorithm> // Needed to operate on vectors.
#include <iostream>
#include <cmath>


namespace {
	std::string& removeTrailingSpace(std::string& str) {

		auto index = str.find_last_not_of(' ');
		if (index != std::string::npos) {
			str.erase(index + 1, str.length() - index);
		}
		return str;
	}


	std::string& removeLeadingSpace(std::string& str) {

		auto index = str.find_first_not_of(' ');
		if (index != std::string::npos) {
			str.erase(0, index);
		}
		return str;
	}


	std::string& alignLeft(std::string& str, const uint width) {
		
		removeLeadingSpace(str);
		if (str.length() < width) {
			str.append(width - str.length(), ' ');
		}
		return str;
	}


	std::string& alignCenter(std::string& str, const uint width) {
		
		removeLeadingSpace(str);
		removeTrailingSpace(str);

		if (str.length() < width) {
			float makeUp = width - str.length();
			str.insert(0, floor(makeUp * 0.5), ' ');
			str.append(ceil(makeUp * 0.5), ' ');
		}

		return str;
	}


	std::string& alignRight(std::string& str, const uint width) {

		removeLeadingSpace(str);
		removeTrailingSpace(str);
		
		if (str.length() < width) {
			str.insert(0, width - str.length(), ' '); 
		}

		return str;
	}
}




// BasicMenuItem


void BasicMenuItem::align(const uint screenWidth, const MenuUtils::Alignment alignment) {
	switch (alignment) {
		case MenuUtils::Alignment::Left:
			alignLeft(content, screenWidth);
			break;
		case MenuUtils::Alignment::Center:
			alignCenter(content, screenWidth);
			break;
		case MenuUtils::Alignment::Right:
			alignRight(content, screenWidth);
	}
}


// MenuSetting




// Menu


Menu::Menu(std::vector<std::shared_ptr<BasicMenuItem>> items, uint widthPixels, uint heightPixels, uint fontWidth, uint fontHeight, int fontCmd, MenuUtils::Alignment alignment, int startIndex, std::function<void()> longPressFunc) :
				items(items),
				widthColumns(ceil(static_cast<float>(widthPixels) / static_cast<float>(fontWidth))),
				heightRows(ceil(static_cast<float>(heightPixels) / static_cast<float>(fontHeight))),
				widthPixels(widthPixels),
				heightPixels(heightPixels),
				fontWidth(fontWidth),
				fontHeight(fontHeight),
				fontCmd(fontCmd),
				byteRowsPerCharacter(fontHeight / 8),
				titleHeight(std::count_if(items.begin(), items.end(), [](auto& item)->bool{ return dynamic_cast<MenuTitle*>(item.get()) != nullptr; })),
				alignment(alignment),
				index((startIndex < 0) ? titleHeight : startIndex),  
				screenTopItOffs(titleHeight),
				screenBottomItOffs(screenTopItOffs + heightRows - screenTopItOffs),
				ignoreRotary(false),
				ignoreButton(false),
				closing(false),
				enterButtonLongPressFunc(longPressFunc) {

	for (auto&& item : this->items) { align(*item, alignment); }
	drawFuncsInitialised();
}

Menu::Menu(	uint widthPixels, uint heightPixels, uint fontWidth, uint fontHeight, int fontCmd, MenuUtils::Alignment alignment, int startIndex = -1  ) :
				widthColumns(ceil(static_cast<double>(widthPixels) / static_cast<double>(fontWidth))),
				heightRows(ceil(static_cast<float>(heightPixels) / static_cast<float>(fontHeight))),
				widthPixels(widthPixels),
				heightPixels(heightPixels),
				fontWidth(fontWidth),
				fontHeight(fontHeight),
				fontCmd(fontCmd),
				byteRowsPerCharacter(fontHeight / 8),
				alignment(alignment),
				index(startIndex),
				screenTopItOffs(0),
				screenBottomItOffs(heightRows - 1),
				ignoreRotary(false),
				ignoreButton(false),
				closing(false) {}

//Menu::Menu() : width(0), height(0) {}
// Menu& Menu::Menu(const Menu& other) {
	
// } 


void Menu::operator()() {

	closing = false;
	ignoreRotary = false;
	ignoreButton = false;

	std::for_each(items.begin(), items.end(), [](std::shared_ptr<BasicMenuItem> item){ item->markDirty(); });

	draw();
	
	// ie back button hit or something.
	while (!closing) {
		tight_loop_contents();
	}
}


void Menu::align(BasicMenuItem& item, MenuUtils::Alignment how) {

	item.align(widthColumns, how);
}


void Menu::addItem(const std::shared_ptr<BasicMenuItem>& item) {

	items.push_back(item);
	items.back()->align(widthColumns, alignment);
}


void Menu::addItems(const std::vector<std::shared_ptr<BasicMenuItem>>& items) {

	for (auto &item : items)
		this->addItem(item);
}


void Menu::draw() {

// Draw any title lines at the top.
	for (auto titleIt = items.begin(); titleIt != items.begin() + titleHeight; ++titleIt) {
		if ((*titleIt)->isDirty()) {

			auto row = (titleIt - items.begin()) * byteRowsPerCharacter;
			drawLineFunction((*titleIt)->getContent(), row, false, fontCmd);
			(*titleIt)->markClean();
		}
	}
	// This is for the items.  Between top and bottom it.
	auto screenTopIt = std::next(std::begin(items), screenTopItOffs);
	auto screenBottomIt = std::next(std::begin(items), screenBottomItOffs);

	for (auto it = screenTopIt; it != screenBottomIt; ++it) {
		
		auto itemNumber = it - screenTopIt + titleHeight;
		auto row = itemNumber * byteRowsPerCharacter;		// get scrn pos.

		// Check that there is an item.
		if (itemNumber >= items.size()) {

			auto blankLine = std::string(widthColumns, ' ');
			drawLineFunction(blankLine, row, false, fontCmd);
			continue;
		}
		// Draw if dirty.
		auto& item = **it;

		if (item.isDirty()) {

			drawLineFunction(item.getContent(), row, row == index * byteRowsPerCharacter, fontCmd);
			item.markClean();
		}
	}
}


void Menu::markAllDirty() { 
	for (uint i = titleHeight; i < items.size(); ++i) { items[i]->markDirty(); }
}


int Menu::downButton() {

	if (ignoreRotary) return 0;

	// index is above bottom. // and bottom isn't midway through the screen.
	if (index < static_cast<int>(heightRows - 1) && index < static_cast<int>(items.size() - 1)) {
		items[screenTopItOffs + index - 1]->markDirty();
		items[screenTopItOffs + index++]->markDirty();
		
	// if index is at bottom of screen and there are more items.
	} else if (/*index == height - 1 && */screenBottomItOffs < items.size()) {
		screenTopItOffs++;
		screenBottomItOffs++;
		markAllDirty();
	}
	draw();
	return 1;
}


int Menu::upButton() {

	if (ignoreRotary) return 0;
	
// Scroll the index up if possibe.
	if (index > static_cast<int>(titleHeight)) {
		items[screenTopItOffs + --index]->markDirty();
		items[screenTopItOffs + index - 1]->markDirty();
		
	} else if (screenTopItOffs > titleHeight) {
		screenTopItOffs--;
		screenBottomItOffs--;
		markAllDirty();
	}
	draw();
	return 0;
}


int Menu::enterButtonDown() {

	if (ignoreButton) return 0;
	ignoreRotary  = true;

	// Item you are pointing at = index - titleHeight + screenTopItOffset
	auto itemNumber = index - titleHeight + screenTopItOffs;
	auto itemIt = std::next(std::begin(items), itemNumber);

	// Index is your position on the screen relative to number of drawn rows for font size.
	// The row on the screen.
	auto row = index * byteRowsPerCharacter;	

	drawLineFunction((*itemIt)->getContent(), row, false, fontCmd);

	drawRectangleFunction(1, row * 8, widthPixels - 1, ((row + byteRowsPerCharacter) * 8) - 1, 255, false);

	dumpBufferFunction();

	return 1;
}


int Menu::enterButtonUp() {

	if (ignoreButton) return 0;
	ignoreRotary = false;

	auto itemNumber = index - titleHeight + screenTopItOffs;
	auto itemIt = std::next(std::begin(items), itemNumber);
	auto row = index * byteRowsPerCharacter;	

	drawLineFunction((*itemIt)->getContent(), row, true, fontCmd);
	for (uint i = 0; i < 7; ++i) {
		drawLineFunction((*itemIt)->getContent(), row, i % 2 == 0, fontCmd);
		busy_wait_ms(75);
	}
	dynamic_cast<MenuButton*>(itemIt->get())->operator()();
	return 1;
}


int Menu::enterButtonPressedLong() {

	if (enterButtonLongPressFunc)
		enterButtonLongPressFunc();

	return 1;
}


void Menu::drawFuncsInitialised() {
	assert(drawLineFunction && "You need to initialize the drawing functions.");
}

