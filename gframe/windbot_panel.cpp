#include "windbot_panel.h"
#include <IGUIComboBox.h>
#include <IGUIStaticText.h>
#include <IGUICheckBox.h>
#include "config.h"
#include "data_manager.h"
#include "deck_manager.h"
#include "fmt.h"

namespace ygo {

int WindBotPanel::CurrentIndex() {
	int selected = cbBotDeck->getSelected();
	return selected >= 0 ? cbBotDeck->getItemData(selected) : selected;
}

int WindBotPanel::CurrentEngine() {
	int selected = cbBotEngine->getSelected();
	return selected >= 0 ? cbBotEngine->getItemData(selected) : selected;
}

void WindBotPanel::Refresh(int filterMasterRule, int lastIndex) {
	int oldIndex = CurrentIndex();
	int lastBot = oldIndex >= 0 ? oldIndex : lastIndex;
	cbBotDeck->clear();
	cbBotEngine->clear();
	genericEngineIdx = -1;
	int i = 0;
	for (const auto& bot : bots) {
		if(genericEngine == &bot)
			continue;
		if (filterMasterRule == 0 || bot.masterRules.find(filterMasterRule) != bot.masterRules.end()) {
			int newIndex = cbBotDeck->addItem(bot.name.data(), i);
			cbBotEngine->addItem(bot.name.data(), i);
			if(i == lastBot) {
				cbBotDeck->setSelected(newIndex);
				cbBotEngine->setSelected(newIndex);
			}
		}
		++i;
	}
	if(genericEngine) {
		genericEngineIdx = cbBotEngine->addItem(genericEngine->name.data(), i);
	}
	////kdiy//////
	// for(auto& file : Utils::FindFiles(DeckManager::GetDeckFolder(), { EPRO_TEXT("ydk") })) {
	// 	file.erase(file.size() - 4);
	// 	cbBotDeck->addItem(Utils::ToUnicodeIfNeeded(file).data(), i);
	// 	i++;
	// }
	////kdiy//////
	UpdateDescription();
}

void WindBotPanel::UpdateDescription() {
	int index = CurrentIndex();
	if (index < 0) {
		deckProperties->setText(L"");
		return;
	}
	if (index >= (int)(bots.size() - (genericEngine != nullptr)) || index != CurrentEngine()) {
		deckProperties->setText(L"???");
		return;
	}
	auto& bot = bots[index];
	std::wstring params = [&bot] {
		if(bot.difficulty != 0)
			return epro::format(gDataManager->GetSysString(2055), bot.difficulty);
		return std::wstring{ gDataManager->GetSysString(2056) };
	}();
	params.push_back(L'\n');
	if (bot.masterRules.size()) {
		std::wstring mr;
		for (auto rule : bot.masterRules) {
			if (mr.size())
				mr.push_back(L',');
			mr.append(epro::to_wstring(rule));
		}
		params.append(epro::format(gDataManager->GetSysString(2057), mr)).push_back(L'\n');
	}
	deckProperties->setText(params.data());
}

void WindBotPanel::UpdateEngine() {
	int index = CurrentIndex();
	if(index >= (int)bots.size()) {
		if(genericEngineIdx != -1)
			cbBotEngine->setSelected(genericEngineIdx);
		else
			cbBotEngine->setSelected(0);
	} else {
		cbBotEngine->setSelected(cbBotDeck->getSelected());
	}
	UpdateDescription();
}

bool WindBotPanel::LaunchSelected(int port, epro::wstringview pass) {
	int index = CurrentIndex();
	int engine = CurrentEngine();
	if (index < 0 || engine < 0) return false;
	const wchar_t* overridedeck = nullptr;
	std::wstring tmpdeck{};
	/////kdiy//////
	// const auto maxsize = (int)(bots.size() - (genericEngine != nullptr));
	// if(engine != index || index >= maxsize) {
	// 	if(index >= maxsize) {
	// 		tmpdeck = Utils::ToUnicodeIfNeeded(DeckManager::GetDeckPath(Utils::ToPathString(cbBotDeck->getItem(cbBotDeck->getSelected()))));
	// 		overridedeck = tmpdeck.data();
	// 	} else {
	// 		overridedeck = bots[index].deckfile.data();
	// 	}
	// }
	if(bots[index].deck == L"AI_perfectdicky") {
		if(aiDeckSelect2->getSelected() == 0)
			tmpdeck = Utils::ToUnicodeIfNeeded(DeckManager::GetDeckPath(Utils::ToPathString(aiDeckSelect->getItem(aiDeckSelect->getSelected()))));
		else
		    tmpdeck = Utils::ToUnicodeIfNeeded(DeckManager::GetDeckPath(Utils::ToPathString(epro::format(EPRO_TEXT("{}/{}"), aiDeckSelect2->getItem(aiDeckSelect2->getSelected()), aiDeckSelect->getItem(aiDeckSelect->getSelected())))));
		overridedeck = tmpdeck.data();
		bots[index].deckpath = aiDeckSelect->getItem(aiDeckSelect->getSelected());
	}
	/////kdiy//////
	// 1 = scissors, 2 = rock, 3 = paper
	/////kdiy//////
	//auto res = bots[engine].Launch(port, pass, !chkMute->isChecked(), chkThrowRock->isChecked() * 2, overridedeck);
	auto res = bots[engine].Launch(port, pass, !chkMute->isChecked(), chkThrowRock->isChecked() * 2, overridedeck, chkSeed->getSelected());
	/////kdiy//////
#if EDOPRO_LINUX || EDOPRO_MACOS
	if(res > 0)
		windbotsPids.push_back(res);
#endif
	return res;
}

std::wstring WindBotPanel::GetParameters(int port, epro::wstringview pass) {
	int index = CurrentIndex();
	int engine = CurrentEngine();
	if(index < 0 || engine < 0) return {};
	const wchar_t* overridedeck = nullptr;
	std::wstring tmpdeck{};
	const auto maxsize = (int)(bots.size() - (genericEngine != nullptr));
	if(engine != index || index >= maxsize) {
		if(index >= maxsize) {
			tmpdeck = Utils::ToUnicodeIfNeeded(DeckManager::GetDeckPath(Utils::ToPathString(cbBotDeck->getItem(cbBotDeck->getSelected()))));
			overridedeck = tmpdeck.data();
		} else {
			overridedeck = bots[index].deckfile.data();
		}
	}
	// 1 = scissors, 2 = rock, 3 = paper
    ////kdiy/////////
	//return bots[engine].GetLaunchParameters(port, pass, !chkMute->isChecked(), chkThrowRock->isChecked() * 2, overridedeck);
    return bots[engine].GetLaunchParameters(port, pass, !chkMute->isChecked(), chkThrowRock->isChecked() * 2, overridedeck, chkSeed->getSelected());
    ////kdiy/////////
}

}
