#include "menu.hpp"

#include <cmath> // Needed for floor and ceil in align.
#include <algorithm> // Needed to operate on vectors.
#include <iostream>


Menu::Menu(std::vector<std::shared_ptr<BasicMenuItem>> items, uint width, uint height, std::function<void(std::string, int yPos, bool inverted)> drawLineFunc, Alignment alignment, int startIndex) :
						items(items),
						width(width),
						height(height),
						alignment(alignment),
						index(startIndex),  // needs to be first none title.
						screenTop(0),
						drawLineFunction(drawLineFunc) {

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

	std::cout << std::count_if(items.begin(), items.end(), [](auto& item)->bool{ return dynamic_cast<MenuTitle*>(item.get()) != nullptr; }) << std::endl;

//	auto offset = (title != "");
//	if (offset) drawLineFunction (title, 0, false);
	for (auto itemIt = items.begin() + screenTop; (itemIt != items.begin() + screenTop + height && itemIt != items.end()); ++itemIt) {
		auto& item = *itemIt;
		if (item->isDirty()) {
			drawLineFunction(item->getContent(), (itemIt - items.begin()), (itemIt - items.begin() == index));
			item->markClean();
		}
	}
}

int Menu::downButton() {
	if (index == height - 1)	// At bottom of screen.
		if (screenTop + index < items.size()) { screenTop++; markAllDirty(); }// Move screen down.
		else return index + screenTop; // do nothing.
	else { items[index]->markDirty(); index++; items[index]->markDirty(); } // move index down.
	return index + screenTop;
}


int Menu::upButton() { 
	if (index <= 0) // if index at top.
		if (screenTop == 0) return 0; // if screen at top do nothing.
		else { screenTop--; markAllDirty(); } // move screen up.
	else { items[index]->markDirty(); index--; items[index]->markDirty(); } // else move index up.
	return index + screenTop;
}
