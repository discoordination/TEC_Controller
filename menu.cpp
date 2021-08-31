#include "menu.hpp"

#include <cmath> // Needed for floor and ceil in align.
#include <algorithm> // Needed to operate on vectors.
#include <iostream>


// BasicMenuItem

void BasicMenuItem::removeTrailingSpace() {
	// Remove any trailing space.
	auto index = content.find_last_not_of(' ');
	if (index != std::string::npos) {
		content.erase(index + 1, content.length() - index);
	}
}

void BasicMenuItem::removeLeadingSpace() {
	// Remove any leading space.
	auto index = content.find_first_not_of(' ');
	if (index != std::string::npos) {
		content.erase(0, index);
	}
}

void BasicMenuItem::alignLeft(uint screenWidth) {
	removeLeadingSpace();
	if (content.length() < screenWidth) {
		content.append(screenWidth - content.length(), ' ');
	}
}


void BasicMenuItem::alignCenter(uint screenWidth) {
	removeLeadingSpace();
	removeTrailingSpace();
	if (content.length() < screenWidth) {
		float makeUp = screenWidth - content.length();
		content.insert(0, floor(makeUp * 0.5), ' ');
		content.append(ceil(makeUp * 0.5), ' ');
	}
}


void BasicMenuItem::alignRight(uint screenWidth) {
	removeLeadingSpace();
	removeTrailingSpace();
	if (content.length() < screenWidth) {
		content.insert(0, screenWidth - content.length(), ' '); 
	}
}




// Menu


Menu::Menu(std::vector<std::shared_ptr<BasicMenuItem>> items, uint width, uint height, Alignment alignment, int startIndex, std::function<void()> longPressFunc) :
				items(items),
				width(width),
				height(height),
				alignment(alignment),
				enterButtonLongPressFunc(longPressFunc),
				titleHeight(std::count_if(items.begin(), items.end(), [](auto& item)->bool{ return dynamic_cast<MenuTitle*>(item.get()) != nullptr; })),
				index((startIndex < 0) ? titleHeight : startIndex),  
				screenTopItOffs(titleHeight),
				screenBottomItOffs(screenTopItOffs + height - screenTopItOffs),
				ignoreRotary(false),
				ignoreButton(false),
				closing(false) 	{

	for (auto&& item : this->items) { align(*item, alignment); }
	drawFuncsInitialised();
}

Menu::Menu(uint width, uint height, Alignment alignment, int startIndex = -1) : width(width), height(height), alignment(alignment), index(startIndex), screenTopItOffs(0), screenBottomItOffs(height - 1), ignoreRotary(false), ignoreButton(false), closing(false) {}

//Menu::Menu() : width(0), height(0) {}
// Menu& Menu::Menu(const Menu& other) {
	
// } 


void Menu::operator()() {

	closing = false;
	ignoreRotary = false;
	ignoreButton = false;

	std::for_each(items.begin(), items.end(), [](std::shared_ptr<BasicMenuItem> item){ item->markDirty(); });

	display();
	
	// ie back button hit or something.
	while (!closing) {
		tight_loop_contents();
	}
}


void Menu::align(BasicMenuItem& item, Menu::Alignment how) {

// Alignment left make up to length.
	if (how == Menu::Alignment::Left) {
		item.alignLeft(width);
	}
	if (how == Menu::Alignment::Center) {
		item.alignCenter(width);
	}
	else if (how == Menu::Alignment::Right) {
		item.alignRight(width);
	}
}


void Menu::addItem(std::shared_ptr<BasicMenuItem> item) {
	switch (alignment) {
	case Alignment::Left:
		item->alignLeft(width);
		break;
	case Alignment::Center:
		item->alignCenter(width);
		break;
	case Alignment::Right:
		item->alignRight(width);
	}
	items.push_back(item);
}


void Menu::addItems(std::vector<std::shared_ptr<BasicMenuItem>> items) {
	for (auto &item : items) { 
		switch (alignment) {
		case Alignment::Left:
			item->alignLeft(width);
			break;
		case Alignment::Center:
			item->alignCenter(width);
			break;
		case Alignment::Right:
			item->alignRight(width);
		}
		this->items.push_back(item);
	}
}


void Menu::display() {

// Draw any title lines at the top.
	for (auto titleIt = items.begin(); titleIt != items.begin() + titleHeight; ++titleIt) {
		if ((*titleIt).get()->isDirty()) {
			drawLineFunction((*titleIt)->getContent(), titleIt - items.begin(), false);
			(*titleIt)->markClean();
		}
	}

	auto screenTopIt = std::next(std::begin(items), screenTopItOffs);
	auto screenBottomIt = std::next(std::begin(items), screenBottomItOffs);

	for (auto it = screenTopIt; it != screenBottomIt; ++it) {

		auto pos = it - screenTopIt + titleHeight;		// get scrn pos.

		// Check that there is an item.
		if (pos >= items.size()) {
			drawLineFunction(std::string(width, ' '), pos, false);
			continue;
		}
		// Draw if dirty.
		auto& item = *(*it).get();
		if (item.isDirty()) {
			drawLineFunction(item.getContent(), pos, pos == index);
			item.markClean();
		}
	}
}


int Menu::downButton() {

	if (ignoreRotary) return 0;

	// index is above bottom. // and bottom isn't midway through the screen.
	if (index < height - 1 && index < items.size() - 1) {
		items[screenTopItOffs + index - 1]->markDirty();
		items[screenTopItOffs + index++]->markDirty();
		//index++;
	// if index is at bottom of screen and there are more items.
	} else if (/*index == height - 1 && */screenBottomItOffs < items.size()) {
		screenTopItOffs++;
		screenBottomItOffs++;
		markAllDirty();
	}
	display();
	return 1;
}


int Menu::upButton() {

	if (ignoreRotary) return 0;
	
// Scroll the index up if possibe.
	if (index > titleHeight) {
		items[screenTopItOffs + --index]->markDirty();
		items[screenTopItOffs + index - 1]->markDirty();
		//index--;
	} else if (screenTopItOffs > titleHeight) {
		screenTopItOffs--;
		screenBottomItOffs--;
		markAllDirty();
	}
	display();
	return 0;
}


int Menu::enterButtonDown() {

	if (ignoreButton) return 0;
	ignoreRotary  = true;
	drawLineFunction(items[index]->getContent(), index, false);
	drawRectangleFunction(1, height * index, (width * 8) - 1, height * (index + 1) - 1, 255, false); 
	dumpBufferFunction();
	return 1;
}


int Menu::enterButtonUp() {

	if (ignoreButton) return 0;
	ignoreRotary = false;
	drawLineFunction(items[index]->getContent(), index, true);
	for (uint i = 0; i < 7; ++i) {
		drawLineFunction(items[index]->getContent(), index, i % 2 == 0);
		busy_wait_ms(75);
	}
	dynamic_cast<MenuButton*>(items[index].get())->operator()();
	return 1;
}


int Menu::enterButtonPressedLong() {
	enterButtonLongPressFunc();
	return 1;
}


void Menu::drawFuncsInitialised() {
	assert(drawLineFunction && "You need to initialize the drawing functions.");
}

