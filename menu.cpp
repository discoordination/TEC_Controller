#include "menu.hpp"

#include <cmath> // Needed for floor and ceil in align.
#include <algorithm> // Needed to operate on vectors.
#include <iostream>


Menu::Menu(std::vector<std::shared_ptr<BasicMenuItem>> items, uint width, uint height, std::function<void(std::string, int yPos, bool inverted)> drawLineFunc, Alignment alignment, int startIndex) :
				items(items),
				width(width),
				height(height),
				alignment(alignment),
				titleHeight(std::count_if(items.begin(), items.end(), [](auto& item)->bool{ return dynamic_cast<MenuTitle*>(item.get()) != nullptr; })),
				index((startIndex < 0) ? titleHeight : startIndex),  
				screenTopItOffs(titleHeight),
				screenBottomItOffs(screenTopItOffs + height - screenTopItOffs),
				drawLineFunction(drawLineFunc),
				ignoreRotary(false),
				ignoreButton(false) 	{

	for (auto&& item : this->items) { align(item->getContent(), alignment); }
}



// I believe align works but need to test right alignment.
void Menu::align(std::string& tString, Menu::Alignment how) {

// First remove any leading space.
	auto index = tString.find_first_not_of(' ');
	if (index != std::string::npos) {
		tString.erase(0, index);
	}

// Alignment left make up to length.
	if (how == Menu::Alignment::Left) {
		if (tString.length() < width) {
			tString.append(width - tString.length(), ' ');
		}
		return;
	}

// Remove any trailing space.
	index = tString.find_last_not_of(' ');
	if (index != std::string::npos) {
		tString.erase(index + 1, tString.length() - index);
	}


	if (how == Menu::Alignment::Center) {
		if (tString.length() < width) {
			float makeUp = width - tString.length();
			tString.insert(0, floor(makeUp * 0.5), ' ');
			tString.append(ceil(makeUp * 0.5), ' ');
		}
	}
	else if (how == Menu::Alignment::Right) {
		if (tString.length() < width) {
			tString.insert(0, width - tString.length(), ' '); 
		}
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
	return 1;
}

int Menu::enterButtonUp() {
	if (ignoreButton) return 0;
	return 1;
}
