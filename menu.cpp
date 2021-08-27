#include "menu.hpp"

#include <cmath>


Menu::Menu(std::vector<std::shared_ptr<BasicMenuItem>> items, uint width, uint height, std::function<void(std::string, int yPos, bool inverted)> drawLineFunc, Alignment alignment, int startIndex) :
						title(title),
						items(items),
						width(width),
						height(height),
						alignment(alignment),
						index(startIndex),
						screenTop(0),
						drawLineFunction(drawLineFunc) {
		
		align(this->title, Alignment::Center);
		for (auto&& item : this->items) { align(item->getContent(), alignment); } 
	}




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

	auto offset = (title != "");
	if (offset) drawLineFunction (title, 0, false);
	for (auto itemIt = items.begin() + screenTop; (itemIt != items.begin() + screenTop + height && itemIt != items.end()); ++itemIt) {
		auto& item = *itemIt;
		if (item->isDirty()) {
			drawLineFunction(item->getContent(), (itemIt - items.begin()) + ((title == "") ? 0 : 1), (itemIt - items.begin() == index));
			item->markClean();
		}
	}
}
