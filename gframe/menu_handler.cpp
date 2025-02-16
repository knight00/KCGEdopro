#include "config.h"
#include "client_updater.h"
#include "game_config.h"
#include "menu_handler.h"
#include "netserver.h"
#include "duelclient.h"
#include "deck_manager.h"
#include "replay_mode.h"
#include "single_mode.h"
#include "image_manager.h"
#include "game.h"
#include "server_lobby.h"
#include "utils_gui.h"
#include "CGUIFileSelectListBox/CGUIFileSelectListBox.h"
#include "CGUITTFont/CGUITTFont.h"
#include <IrrlichtDevice.h>
#include <IGUIEnvironment.h>
#include <IGUIButton.h>
#include <IGUICheckBox.h>
#include <IGUIComboBox.h>
#include <IGUIContextMenu.h>
#include <IGUIEditBox.h>
#include <IGUIScrollBar.h>
#include <IGUIStaticText.h>
#include <IGUITabControl.h>
#include <IGUITable.h>
#include <IGUIWindow.h>
#include "address.h"
#include "fmt.h"
//////kdiy///////
#include "CGUIImageButton/CGUIImageButton.h"
#include "sound_manager.h"
//////kdiy///////

namespace ygo {

static void UpdateDeck() {
	/////kdiy/////
	gGameConfig->lastdeckfolder = mainGame->cbDeck2Select->getItem(mainGame->cbDeck2Select->getSelected());
	/////kdiy/////
	gGameConfig->lastdeck = mainGame->cbDeckSelect->getItem(mainGame->cbDeckSelect->getSelected());
	const auto& deck = mainGame->deckBuilder.GetCurrentDeck();
	uint8_t deckbuf[0xf000];
	auto* pdeck = deckbuf;
	static constexpr auto max_deck_size = sizeof(deckbuf) / sizeof(uint32_t) - 2;
	const auto totsize = deck.main.size() + deck.extra.size() + deck.side.size();
	if(totsize > max_deck_size)
		return;
	BufferIO::Write<uint32_t>(pdeck, static_cast<uint32_t>(deck.main.size() + deck.extra.size()));
	BufferIO::Write<uint32_t>(pdeck, static_cast<uint32_t>(deck.side.size()));
	for(const auto& pcard : deck.main)
        ///////kdiy/////////
        if(!(pcard->code == 111 || pcard->code == 112 || pcard->code == 211 || pcard->code == 208 || pcard->code == 85 || pcard->code == 86))
        ///////kdiy/////////
		BufferIO::Write<uint32_t>(pdeck, pcard->code);
	for(const auto& pcard : deck.extra)
        ///////kdiy/////////
        if(!(pcard->code == 111 || pcard->code == 112 || pcard->code == 211 || pcard->code == 208 || pcard->code == 85 || pcard->code == 86))
        ///////kdiy/////////
		BufferIO::Write<uint32_t>(pdeck, pcard->code);
	for(const auto& pcard : deck.side)
        ///////kdiy/////////
        if(!(pcard->code == 111 || pcard->code == 112 || pcard->code == 211 || pcard->code == 208 || pcard->code == 85 || pcard->code == 86))
        ///////kdiy/////////
		BufferIO::Write<uint32_t>(pdeck, pcard->code);
	DuelClient::SendBufferToServer(CTOS_UPDATE_DECK, deckbuf, pdeck - deckbuf);
	gdeckManager->sent_deck = mainGame->deckBuilder.GetCurrentDeck();
}
static void LoadReplay() {
	auto& replay = ReplayMode::cur_replay;
	if(std::exchange(open_file, false)) {
		bool res = replay.OpenReplay(open_file_name);
		if(!res || (replay.IsOldReplayMode() && (!mainGame->coreloaded || !replay.CanBePlayedInOldMode())))
			return;
	} else {
		const auto& list = mainGame->lstReplayList;
		const auto selected = list->getSelected();
		if(selected == -1)
			return;
		const auto path = Utils::ToPathString(list->getListItem(selected, true));
		if(!replay.OpenReplay(path) || (replay.IsOldReplayMode() && (!mainGame->coreloaded || !replay.CanBePlayedInOldMode())))
			return;
	}
	if(mainGame->chkYrp->isChecked() && !replay.yrp)
		return;
	replay.Rewind();
	mainGame->ClearCardInfo();
	mainGame->mTopMenu->setVisible(false);
	mainGame->wCardImg->setVisible(true);
	/////kdiy/////
    //mainGame->wInfos->setVisible(true);
	mainGame->btnLeaveGame->setVisible(true);
	mainGame->wBtnShowCard->setVisible(true);
	/////kdiy/////
	mainGame->wReplay->setVisible(true);
	mainGame->wReplayControl->setVisible(true);
	mainGame->btnReplayStart->setVisible(false);
	mainGame->btnReplayPause->setVisible(true);
	mainGame->btnReplayStep->setVisible(false);
	mainGame->btnReplayUndo->setVisible(false);
	mainGame->wPhase->setVisible(true);
	mainGame->dField.Clear();
	mainGame->HideElement(mainGame->wReplay);
	mainGame->device->setEventReceiver(&mainGame->dField);
	int start_turn;
	try { start_turn = std::stoi(mainGame->ebRepStartTurn->getText());  }
	catch(...) { start_turn = 0; }
	if(start_turn == 1)
		start_turn = 0;
	ReplayMode::StartReplay(start_turn, (mainGame->chkYrp->isChecked() || replay.IsOldReplayMode()));
}
static inline void TriggerEvent(irr::gui::IGUIElement* target, irr::gui::EGUI_EVENT_TYPE type) {
	irr::SEvent event;
	event.EventType = irr::EET_GUI_EVENT;
	event.GUIEvent.EventType = type;
	event.GUIEvent.Caller = target;
	ygo::mainGame->device->postEventFromUser(event);
}

static inline void ClickButton(irr::gui::IGUIElement* btn) {
	TriggerEvent(btn, irr::gui::EGET_BUTTON_CLICKED);
}
bool MenuHandler::OnEvent(const irr::SEvent& event) {
	bool stopPropagation = false;
	if(mainGame->dField.OnCommonEvent(event, stopPropagation))
		return stopPropagation;
	switch(event.EventType) {
	case irr::EET_GUI_EVENT: {
		irr::gui::IGUIElement* caller = event.GUIEvent.Caller;
		int id = caller->getID();
		if(mainGame->wRules->isVisible() && (id != BUTTON_RULE_OK && id != CHECKBOX_EXTRA_RULE && id != COMBOBOX_DUEL_RULE && id != EDITBOX_TEAM_COUNT))
			break;
		if(mainGame->wMessage->isVisible() && id != BUTTON_MSG_OK &&
		   prev_operation != ACTION_UPDATE_PROMPT
		   && prev_operation != ACTION_SHOW_CHANGELOG
#if EDOPRO_LINUX && (IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9)
		   && prev_operation != ACTION_TRY_WAYLAND
#endif
		   )
			break;
		if(mainGame->wQuery->isVisible() && id != BUTTON_YES && id != BUTTON_NO) {
			mainGame->wQuery->getParent()->bringToFront(mainGame->wQuery);
			break;
		}
		if(mainGame->wFileSave->isVisible() && id != BUTTON_FILE_SAVE && id != BUTTON_FILE_CANCEL) {
			mainGame->wFileSave->getParent()->bringToFront(mainGame->wFileSave);
			break;
		}
		switch(event.GUIEvent.EventType) {
		case irr::gui::EGET_ELEMENT_HOVERED: {
			// Set cursor to an I-Beam if hovering over an edit box
			if (event.GUIEvent.Caller->getType() == irr::gui::EGUIET_EDIT_BOX && event.GUIEvent.Caller->isEnabled())
			{
				GUIUtils::ChangeCursor(mainGame->device, irr::gui::ECI_IBEAM);
			}
			break;
		}
		case irr::gui::EGET_ELEMENT_LEFT: {
			// Set cursor to normal if left an edit box
			if (event.GUIEvent.Caller->getType() == irr::gui::EGUIET_EDIT_BOX && event.GUIEvent.Caller->isEnabled())
			{
				GUIUtils::ChangeCursor(mainGame->device, irr::gui::ECI_NORMAL);
			}
			break;
		}
		case irr::gui::EGET_BUTTON_CLICKED: {
			switch(id) {
			////////kdiy///////
			case BUTTON_QQ: {
				Utils::SystemOpen(EPRO_TEXT("https://jq.qq.com/?_wv=1027&k=S1vfY66P"), Utils::OPEN_URL);
				break;
			}
			case BUTTON_QQ2: {
				Utils::SystemOpen(EPRO_TEXT("https://qm.qq.com/cgi-bin/qm/qr?k=0BcdVu6E2gUjyc_WmL25uLNYib2mTPoV"), Utils::OPEN_URL);
				break;
			}
			////////kdiy///////
			case BUTTON_MODE_EXIT: {
				mainGame->device->closeDevice();
				break;
			}
			case BUTTON_ONLINE_MULTIPLAYER: {
				mainGame->isHostingOnline = true;
				mainGame->HideElement(mainGame->wMainMenu);
				////kdiy////////
				#ifdef EK
				mainGame->HideElement(mainGame->wQQ);
				#endif
				////kdiy////////
				mainGame->ShowElement(mainGame->wRoomListPlaceholder);
				break;
			}
			case BUTTON_LAN_REFRESH2: {
				ServerLobby::RefreshRooms();
				break;
			}
			case BUTTON_JOIN_HOST2: {
				if(wcslen(mainGame->ebNickNameOnline->getText()) <= 0) {
					mainGame->PopupMessage(gDataManager->GetSysString(1257), gDataManager->GetSysString(1256));
					///////kdiy/////////////
					for(size_t i = 0; i < mainGame->cbDeck2Select->getItemCount(); ++i) {
						if(gGameConfig->lastdeckfolder == mainGame->cbDeck2Select->getItem(i)) {
							mainGame->cbDeck2Select->setSelected(i);
						}
					}
					///////kdiy/////////////
					break;
				}
				if(mainGame->roomListTable->getSelected() >= 0) {
					mainGame->HideElement(mainGame->wRoomListPlaceholder);
					ServerLobby::JoinServer(false);
					///////kdiy/////////////
					for(size_t i = 0; i < mainGame->cbDeck2Select->getItemCount(); ++i) {
						if(gGameConfig->lastdeckfolder == mainGame->cbDeck2Select->getItem(i)) {
							mainGame->cbDeck2Select->setSelected(i);
						}
					}
					///////kdiy/////////////
				}
				break;
			}
			case BUTTON_JOIN_CANCEL2: {
				mainGame->HideElement(mainGame->wRoomListPlaceholder);
				mainGame->ShowElement(mainGame->wMainMenu);
				////kdiy////////
#ifdef EK
				mainGame->ShowElement(mainGame->wQQ);
#endif
				////kdiy////////
				break;
			}
			case BUTTON_ROOMPASSWORD_OK: {
				ServerLobby::JoinServer(false);
				break;
			}
			case BUTTON_ROOMPASSWORD_CANCEL: {
				mainGame->wRoomPassword->setVisible(false);
				mainGame->ShowElement(mainGame->wRoomListPlaceholder);
				break;
			}
			case BUTTON_LAN_MODE: {
				mainGame->isHostingOnline = false;
				mainGame->btnCreateHost->setEnabled(mainGame->coreloaded);
				mainGame->btnJoinHost->setEnabled(true);
				mainGame->btnJoinCancel->setEnabled(true);
				mainGame->HideElement(mainGame->wMainMenu);
				////kdiy////////
#ifdef EK
				mainGame->HideElement(mainGame->wQQ);
#endif
				////kdiy////////
				mainGame->ShowElement(mainGame->wLanWindow);
				break;
			}
			case BUTTON_JOIN_HOST: {
				try {
					const auto parsed = epro::Host::resolve(mainGame->ebJoinHost->getText(), mainGame->ebJoinPort->getText());
					gGameConfig->lasthost = mainGame->ebJoinHost->getText();
					gGameConfig->lastport = mainGame->ebJoinPort->getText();
					mainGame->dInfo.secret.pass = mainGame->ebJoinPass->getText();
					if(DuelClient::StartClient(parsed.address, parsed.port, 0, false)) {
						mainGame->btnCreateHost->setEnabled(false);
						mainGame->btnJoinHost->setEnabled(false);
						mainGame->btnJoinCancel->setEnabled(false);
						///////kdiy/////////////
						for(size_t i = 0; i < mainGame->cbDeck2Select->getItemCount(); ++i) {
							if(gGameConfig->lastdeckfolder == mainGame->cbDeck2Select->getItem(i)) {
								mainGame->cbDeck2Select->setSelected(i);
							}
						}
						///////kdiy/////////////
					}
					break;
				}
				catch(...) {
					mainGame->PopupMessage(gDataManager->GetSysString(1412));
					break;
				}
			}
			case BUTTON_JOIN_CANCEL: {
				mainGame->HideElement(mainGame->wLanWindow);
				mainGame->ShowElement(mainGame->wMainMenu);
				////kdiy////////
#ifdef EK
				mainGame->ShowElement(mainGame->wQQ);
#endif
				////kdiy////////
				break;
			}
			case BUTTON_LAN_REFRESH: {
				DuelClient::BeginRefreshHost();
				break;
			}
			case BUTTON_CREATE_HOST: {
                ///////kdiy/////////////
                if(!mainGame->git_update || mainGame->git_error) {
                    mainGame->stACMessage->setText(gDataManager->GetSysString(8046).data());
                    mainGame->PopupElement(mainGame->wACMessage, 20);
					mainGame->mRepositoriesInfo->setVisible(true);
                    break;
                }
                ///////kdiy/////////////
				if (wcslen(mainGame->ebNickName->getText())) {
					mainGame->isHostingOnline = false;
					mainGame->btnHostConfirm->setEnabled(true);
					mainGame->btnHostCancel->setEnabled(true);
					mainGame->HideElement(mainGame->wLanWindow);
					mainGame->stHostPort->setVisible(true);
					mainGame->ebHostPort->setVisible(true);
					mainGame->stHostNotes->setVisible(false);
					mainGame->ebHostNotes->setVisible(false);
					mainGame->ShowElement(mainGame->wCreateHost);
					///////kdiy/////////////
					for(size_t i = 0; i < mainGame->cbDeck2Select->getItemCount(); ++i) {
						if(gGameConfig->lastdeckfolder == mainGame->cbDeck2Select->getItem(i)) {
							mainGame->cbDeck2Select->setSelected(i);
						}
					}
					///////kdiy/////////////
				}
				break;
			}
			case BUTTON_CREATE_HOST2: {
				mainGame->isHostingOnline = true;
				mainGame->btnHostConfirm->setEnabled(true);
				mainGame->btnHostCancel->setEnabled(true);
				mainGame->HideElement(mainGame->wRoomListPlaceholder);
				mainGame->stHostPort->setVisible(false);
				mainGame->ebHostPort->setVisible(false);
				mainGame->stHostNotes->setVisible(true);
				mainGame->ebHostNotes->setVisible(true);
				mainGame->ShowElement(mainGame->wCreateHost);
				///////kdiy/////////////
				for(size_t i = 0; i < mainGame->cbDeck2Select->getItemCount(); ++i) {
					if(gGameConfig->lastdeckfolder == mainGame->cbDeck2Select->getItem(i)) {
						mainGame->cbDeck2Select->setSelected(i);
					}
				}
				///////kdiy/////////////
				break;
			}
			case BUTTON_RULE_CARDS: {
				mainGame->PopupElement(mainGame->wRules);
				break;
			}
			case BUTTON_RULE_OK: {
				mainGame->HideElement(mainGame->wRules);
				break;
			}
			case BUTTON_HOST_CONFIRM: {
				DuelClient::is_local_host = false;
				/////kdiy/////
				mainGame->mode->isMode = false;
				for(int i = 0; i < 6; ++i) {
					if(gSoundManager->character[i] > CHARACTER_VOICE - 1)
					    gSoundManager->character[i] = 0;
					mainGame->ebCharacter[i]->setSelected(gSoundManager->character[i]);
				    mainGame->icon[i]->setImage(mainGame->imageManager.icon[gSoundManager->character[i]][gSoundManager->subcharacter[gSoundManager->character[i]]]);
                }
				///////kdiy///////
				if(mainGame->isHostingOnline) {
					ServerLobby::JoinServer(true);
				} else {
					uint16_t host_port;
					try {
						host_port = static_cast<uint16_t>(std::stoul(mainGame->ebHostPort->getText()));
					}
					catch(...) {
						break;
					}
					gGameConfig->gamename = mainGame->ebServerName->getText();
					gGameConfig->serverport = mainGame->ebHostPort->getText();
					mainGame->gBot.Refresh(gGameConfig->filterBot * (mainGame->cbDuelRule->getSelected() + 1), gGameConfig->lastBot);
					if(!NetServer::StartServer(host_port))
						break;
					const auto ip = 0x100007F; //127.0.0.1 in network byte order
					if(!DuelClient::StartClient({ &ip, epro::Address::INET }, host_port)) {
						NetServer::StopServer();
						break;
					}
					DuelClient::is_local_host = true;
					mainGame->btnHostConfirm->setEnabled(false);
					mainGame->btnHostCancel->setEnabled(false);
				}
				break;
			}
			case BUTTON_HOST_CANCEL: {
				if(DuelClient::IsConnected())
					break;
				mainGame->dInfo.isInLobby = false;
				mainGame->btnCreateHost->setEnabled(mainGame->coreloaded);
				mainGame->btnJoinHost->setEnabled(true);
				mainGame->btnJoinCancel->setEnabled(true);
				if(mainGame->wRules->isVisible())
					mainGame->HideElement(mainGame->wRules);
				mainGame->HideElement(mainGame->wCreateHost);
				if(mainGame->isHostingOnline) {
					mainGame->ShowElement(mainGame->wRoomListPlaceholder);
				} else {
					mainGame->ShowElement(mainGame->wLanWindow);
				}
				break;
			}
			/////kdiy/////
			case BUTTON_SIMPLE_CREATE_HOST: {
				if (wcslen(mainGame->ebNickName->getText())) {
					mainGame->HideElement(mainGame->wLanWindow);
					mainGame->ShowElement(mainGame->wCreateHost2);
				}
				break;
			}
			case BUTTON_LOCAL_HOST_CONFIRM: {
				auto selected = mainGame->serverChoice2->getSelected();
				if(selected < 0) break;
				try {
                    const auto serverinfo = epro::Host::resolve(ServerLobby::serversVector2[selected].address, ServerLobby::serversVector2[selected].duelport);
					std::wstring pass = L"";
					std::wstring symbol = L"#";
					if(mainGame->chkAI->isChecked()) {
					    pass += L"AI";
						if(wcslen(mainGame->ebJoinPass2->getText()) <= 0)
						    symbol = L"";
					}
					if(mainGame->chkTag->isChecked()) 
					    pass += pass == L"" ? L"T" : L",T";
					else {
						if(wcslen(mainGame->ebJoinPass2->getText()) <= 0)
					        pass += pass == L"" ? L"S" : L",S";
					}
					if(mainGame->chkMatch->isChecked()) 
						pass += pass == L"" ? L"M" : L",M";
					if(mainGame->cbRule2->getSelected() == 1)
						pass += pass == L"" ? L"TO" : L",TO";
					else if(mainGame->cbRule2->getSelected() == 2)
						pass += pass == L"" ? L"OT" : L",OT";
					if (std::stoi(mainGame->ebTimeLimit2->getText()) != 3)
						pass += fmt::format(pass == L"" ? L"TM{}" : L",TM{}",mainGame->ebTimeLimit2->getText());
					if (mainGame->cbDuelRule2->getSelected() != 4)
						pass += fmt::format(pass == L"" ? L"MR{}" : L",MR{}", mainGame->cbDuelRule2->getSelected() + 1);
					if(mainGame->chkNoCheckDeckContent2->isChecked()) 
						pass += pass == L"" ? L"NC" : L",NC";
					if(mainGame->chkNoShuffleDeck2->isChecked()) 
						pass += pass == L"" ? L"NS" : L",NS";
					if(mainGame->chkNoLFlist2->isChecked()) 
						pass += pass == L"" ? L"NF" : L",NF";
					if (std::stoi(mainGame->ebStartLP2->getText()) != 8000)
					    pass += fmt::format(pass == L"" ? L"LP{}" : L",LP{}", mainGame->ebStartLP2->getText());
					if (std::stoi(mainGame->ebStartHand2->getText()) != 5)
						pass += fmt::format(pass == L"" ? L"ST{}" : L",ST{}", mainGame->ebStartHand2->getText());
					if (std::stoi(mainGame->ebDrawCount2->getText()) != 1)
						pass += fmt::format(pass == L"" ? L"DR{}" : L",DR{}", mainGame->ebDrawCount2->getText());
					pass += symbol;
					if(wcslen(mainGame->ebJoinPass2->getText()) > 0)
						pass += std::wstring(mainGame->ebJoinPass2->getText());
					if(mainGame->chkdefaultlocal->isChecked())
					    pass = L"";	
					if(pass.length() > 20) {
						mainGame->PopupMessage(gDataManager->GetSysString(8033));
						break;
					}
					mainGame->dInfo.secret.pass = pass;
					if(DuelClient::StartClient(serverinfo.address, serverinfo.port, 0, false)) {
						mainGame->btnCreateHost->setEnabled(false);
						mainGame->btnJoinHost->setEnabled(false);
						mainGame->btnJoinCancel->setEnabled(false);
						for(size_t i = 0; i < mainGame->cbDeck2Select->getItemCount(); ++i) {
							if(gGameConfig->lastdeckfolder == mainGame->cbDeck2Select->getItem(i)) {
								mainGame->cbDeck2Select->setSelected(i);
							}
						}
					}
					break;
				}
				catch(...) {
					mainGame->PopupMessage(gDataManager->GetSysString(1412));
					break;
				}
			}
			case BUTTON_LOCAL_HOST_CANCEL: {
				if(DuelClient::IsConnected())
					break;
				mainGame->dInfo.isInLobby = false;
				mainGame->btnCreateHost->setEnabled(mainGame->coreloaded);
				if(mainGame->wRules->isVisible())
					mainGame->HideElement(mainGame->wRules);
				mainGame->HideElement(mainGame->wCreateHost2);
				mainGame->ShowElement(mainGame->wLanWindow);
				break;
			}
			case BUTTON_ICON: {
#ifndef VIP
				break;
#endif
				bool filechk = mainGame->chantcheck();
				if(!filechk)
				    break;
                auto elem = static_cast<irr::gui::CGUIImageButton*>(event.GUIEvent.Caller);
				for(int i = 0; i < 6; ++i) {
                    if(elem == mainGame->icon[i])
                        mainGame->choose_player = i;
                    if(elem == mainGame->icon2[i])
                        mainGame->choose_player = i;
                }
				int player = gSoundManager->character[mainGame->choose_player];
				mainGame->btnCharacter->setImage(mainGame->imageManager.bodycharacter[player][gSoundManager->subcharacter[player]][0]);
				mainGame->btnCharacter_replay->setImage(mainGame->imageManager.bodycharacter[player][gSoundManager->subcharacter[player]][0]);
				break;
			}
			case BUTTON_CHARACTER: {
                auto elem = static_cast<irr::gui::CGUIImageButton*>(event.GUIEvent.Caller);
				auto& replay = ReplayMode::cur_replay;
                std::vector<uint8_t> team1, team2;
				int team1count = mainGame->dInfo.team1, team2count = mainGame->dInfo.team2;
                if(elem != mainGame->btnCharacter) {
				    team1count = replay.GetPlayersCount(0);
				    team2count = replay.GetPlayersCount(1);
				}
				for(uint8_t i = 0; i < team1count; i++)
				    team1.push_back(i);
				for(uint8_t i = team1count; i < team1count + team2count; i++)
				    team2.push_back(i);
				gSoundManager->PlayStartupChant(mainGame->choose_player, (mainGame->choose_player > team1count - 1) ? team1 : team2);
				break;
			}
			case BUTTON_CHARACTER_SELECT2: {
#ifndef VIP
				break;
#endif
				if(gSoundManager->character[mainGame->choose_player] == CHARACTER_VOICE - 1) gSoundManager->character[mainGame->choose_player] = 0;
				else gSoundManager->character[mainGame->choose_player] ++;
				int player = gSoundManager->character[mainGame->choose_player];
				mainGame->btnCharacter->setImage(mainGame->imageManager.bodycharacter[player][gSoundManager->subcharacter[player]][0]);
				mainGame->btnCharacter_replay->setImage(mainGame->imageManager.bodycharacter[player][gSoundManager->subcharacter[player]][0]);
				mainGame->icon[mainGame->choose_player]->setImage(mainGame->imageManager.icon[player][gSoundManager->subcharacter[player]]);
                mainGame->icon2[mainGame->choose_player]->setImage(mainGame->imageManager.icon[player][gSoundManager->subcharacter[player]]);
                mainGame->ebCharacter[mainGame->choose_player]->setSelected(gSoundManager->character[mainGame->choose_player]);
                mainGame->ebCharacter_replay[mainGame->choose_player]->setSelected(gSoundManager->character[mainGame->choose_player]);
				if(mainGame->choose_player == 0)
                    mainGame->ebCharacterDeck->setSelected(player);

 				auto elem = static_cast<irr::gui::CGUIImageButton*>(event.GUIEvent.Caller);
				auto& replay = ReplayMode::cur_replay;
                std::vector<uint8_t> team1, team2;
				int team1count = mainGame->dInfo.team1, team2count = mainGame->dInfo.team2;
                if(elem == mainGame->btnCharacterSelect2_replay) {
					team1count = replay.GetPlayersCount(0);
					team2count = replay.GetPlayersCount(1);
				}
				for(uint8_t i = 0; i < team1count; i++)
					team1.push_back(i);
				for(uint8_t i = team1count; i < team1count + team2count; i++)
					team2.push_back(i);
				gSoundManager->PlayStartupChant(mainGame->choose_player, (mainGame->choose_player > team1count - 1) ? team1 : team2);
				
				if(player > 0 && gSoundManager->textcharacter[player-1].size() > 2) {
					mainGame->btnsubCharacterSelect[0]->setVisible(true);
					mainGame->btnsubCharacterSelect_replay[0]->setVisible(true);
					for(int i = 2; i < 6; i++) {
						if(gSoundManager->textcharacter[player-1].size() > i) {
							mainGame->btnsubCharacterSelect[i-1]->setVisible(true);
							mainGame->btnsubCharacterSelect_replay[i-1]->setVisible(true);
						} else {
							mainGame->btnsubCharacterSelect[i-1]->setVisible(false);
							mainGame->btnsubCharacterSelect_replay[i-1]->setVisible(false);
						}
					}
				} else {
					for(int i = 0; i < 5; i++) {
						mainGame->btnsubCharacterSelect[i]->setVisible(false);
						mainGame->btnsubCharacterSelect_replay[i]->setVisible(false);
					}
				}
				break;
			}
			case BUTTON_CHARACTER_SELECT: {
#ifndef VIP
				break;
#endif
				if(gSoundManager->character[mainGame->choose_player] == 0) gSoundManager->character[mainGame->choose_player] = CHARACTER_VOICE - 1;
				else gSoundManager->character[mainGame->choose_player] --;
				int player = gSoundManager->character[mainGame->choose_player];
				mainGame->btnCharacter->setImage(mainGame->imageManager.bodycharacter[player][gSoundManager->subcharacter[player]][0]);
				mainGame->btnCharacter_replay->setImage(mainGame->imageManager.bodycharacter[player][gSoundManager->subcharacter[player]][0]);
				mainGame->icon[mainGame->choose_player]->setImage(mainGame->imageManager.icon[player][gSoundManager->subcharacter[player]]);
                mainGame->icon2[mainGame->choose_player]->setImage(mainGame->imageManager.icon[player][gSoundManager->subcharacter[player]]);
                mainGame->ebCharacter[mainGame->choose_player]->setSelected(gSoundManager->character[mainGame->choose_player]);
                mainGame->ebCharacter_replay[mainGame->choose_player]->setSelected(gSoundManager->character[mainGame->choose_player]);
				if(mainGame->choose_player == 0)
                    mainGame->ebCharacterDeck->setSelected(player);

 				auto elem = static_cast<irr::gui::CGUIImageButton*>(event.GUIEvent.Caller);
				auto& replay = ReplayMode::cur_replay;
                std::vector<uint8_t> team1, team2;
				int team1count = mainGame->dInfo.team1, team2count = mainGame->dInfo.team2;
                if(elem == mainGame->btnCharacterSelect1_replay) {
					team1count = replay.GetPlayersCount(0);
					team2count = replay.GetPlayersCount(1);
				}
				for(uint8_t i = 0; i < team1count; i++)
					team1.push_back(i);
				for(uint8_t i = team1count; i < team1count + team2count; i++)
					team2.push_back(i);
				gSoundManager->PlayStartupChant(mainGame->choose_player, (mainGame->choose_player > team1count - 1) ? team1 : team2);
				
				if(player > 0 && gSoundManager->textcharacter[player-1].size() > 2) {
					mainGame->btnsubCharacterSelect[0]->setVisible(true);
					mainGame->btnsubCharacterSelect_replay[0]->setVisible(true);
					for(int i = 2; i < 6; i++) {
						if(gSoundManager->textcharacter[player-1].size() > i) {
							mainGame->btnsubCharacterSelect[i-1]->setVisible(true);
							mainGame->btnsubCharacterSelect_replay[i-1]->setVisible(true);
						} else {
							mainGame->btnsubCharacterSelect[i-1]->setVisible(false);
							mainGame->btnsubCharacterSelect_replay[i-1]->setVisible(false);
						}
					}
				} else {
					for(int i = 0; i < 5; i++) {
						mainGame->btnsubCharacterSelect[i]->setVisible(false);
						mainGame->btnsubCharacterSelect_replay[i]->setVisible(false);
					}
				}
				break;
			}
			case BUTTON_SUBCHARACTER_SELECT: {
#ifndef VIP
				    break;
#endif
				auto elem = static_cast<irr::gui::CGUIImageButton*>(event.GUIEvent.Caller);
				int player = gSoundManager->character[mainGame->choose_player];
				for(int i = 0; i < 5; ++i) {
                    if(elem == mainGame->btnsubCharacterSelect[i] || elem == mainGame->btnsubCharacterSelect_replay[i]) {
						gSoundManager->subcharacter[player] = i;
						mainGame->btnCharacter->setImage(mainGame->imageManager.bodycharacter[player][i][0]);
						mainGame->btnCharacter_replay->setImage(mainGame->imageManager.bodycharacter[player][i][0]);
						mainGame->icon[mainGame->choose_player]->setImage(mainGame->imageManager.icon[player][i]);
                		mainGame->icon2[mainGame->choose_player]->setImage(mainGame->imageManager.icon[player][i]);
					}
				}
				break;
			}
			case BUTTON_PW: {
#ifdef Update_PW
				auto pw = mainGame->ebPw->getText();
				std::string t(Update_PW);
				std::wstring a(t.begin(), t.end());
				if(!Utils::FileExists(EPRO_TEXT("./updates/configs.json"))) {
					if(a.compare(pw) == 0) {
					    gClientUpdater->StartUpdate(Game::UpdateDownloadBar, mainGame);
						mainGame->PopupElement(mainGame->updateWindow);
						break;
					} else update_trial++;
				} else update_trial = 3;
				if(update_trial > 2) {
					mainGame->btnLanMode->setEnabled(false);
					if(!Utils::FileExists(EPRO_TEXT("./updates/configs.json")) && Utils::FileExists(EPRO_TEXT("./config/configs.json"))) {
						Utils::MakeDirectory(EPRO_TEXT("updates"));
					    Utils::FileCopy(EPRO_TEXT("./config/configs.json"), EPRO_TEXT("./updates/configs.json"));
					}
					mainGame->HideElement(mainGame->pwupdateWindow);
				}
#endif
				break;
			}
			/////kdiy/////
			case BUTTON_HP_DUELIST: {
				mainGame->cbDeckSelect->setEnabled(true);
				/////kdiy/////
				mainGame->cbDeck2Select->setEnabled(true);
				/////kdiy/////
				DuelClient::SendPacketToServer(CTOS_HS_TODUELIST);
				break;
			}
			case BUTTON_HP_OBSERVER: {
				DuelClient::SendPacketToServer(CTOS_HS_TOOBSERVER);
				break;
			}
			case BUTTON_HP_KICK: {
				CTOS_Kick csk;
				csk.pos = 0;
				while (csk.pos < 6 && mainGame->btnHostPrepKick[csk.pos] != caller)
					csk.pos++;
				DuelClient::SendPacketToServer(CTOS_HS_KICK, csk);
				break;
			}
			case BUTTON_HP_READY: {
				const auto selected = mainGame->cbDeckSelect->getSelected();
				//////kdiy/////
				//if(selected == -1)
					//break;
				//if(!mainGame->deckBuilder.SetCurrentDeckFromFile(Utils::ToPathString(mainGame->cbDeckSelect->getItem(selected))))
				auto folder = Utils::ToPathString(mainGame->cbDeck2Select->getItem(mainGame->cbDeck2Select->getSelected()));
				if(mainGame->cbDeck2Select->getSelected() == -1 || selected == -1 || !mainGame->deckBuilder.SetCurrentDeckFromFile(folder + EPRO_TEXT("/") + Utils::ToPathString(mainGame->cbDeckSelect->getItem(selected))))
				//////kdiy/////
					break;
				UpdateDeck();
				DuelClient::SendPacketToServer(CTOS_HS_READY);
				mainGame->cbDeckSelect->setEnabled(false);
				/////kdiy/////
				mainGame->cbDeck2Select->setEnabled(false);
				/////kdiy/////
				if(mainGame->dInfo.team1 + mainGame->dInfo.team2 > 2)
					mainGame->btnHostPrepDuelist->setEnabled(false);
				break;
			}
			case BUTTON_HP_NOTREADY: {
				DuelClient::SendPacketToServer(CTOS_HS_NOTREADY);
				mainGame->cbDeckSelect->setEnabled(true);
				/////kdiy/////
				mainGame->cbDeck2Select->setEnabled(true);
				/////kdiy/////
				if(mainGame->dInfo.team1 + mainGame->dInfo.team2 > 2)
					mainGame->btnHostPrepDuelist->setEnabled(true);
				break;
			}
			case BUTTON_HP_START: {
				DuelClient::SendPacketToServer(CTOS_HS_START);
				break;
			}
			case BUTTON_HP_CANCEL: {
				DuelClient::StopClient();
				mainGame->dInfo.isInLobby = false;
				mainGame->btnCreateHost->setEnabled(mainGame->coreloaded);
				mainGame->btnJoinHost->setEnabled(true);
				mainGame->btnJoinCancel->setEnabled(true);
				mainGame->HideElement(mainGame->wHostPrepare);
				mainGame->HideElement(mainGame->gBot.window);
				if (mainGame->wHostPrepareR->isVisible())
					mainGame->HideElement(mainGame->wHostPrepareR);
				if (mainGame->wHostPrepareL->isVisible())
					mainGame->HideElement(mainGame->wHostPrepareL);
				if(mainGame->isHostingOnline)
					mainGame->ShowElement(mainGame->wRoomListPlaceholder);
				else
					mainGame->ShowElement(mainGame->wLanWindow);
				mainGame->wChat->setVisible(false);
				break;
			}
			case BUTTON_REPLAY_MODE: {
				mainGame->HideElement(mainGame->wMainMenu);
				////kdiy////////
#ifdef EK
				mainGame->HideElement(mainGame->wQQ);
#endif
                mainGame->btnCharacterSelect_replay->setEnabled(false);
				////kdiy////////
				mainGame->stReplayInfo->setText(L"");
				mainGame->btnLoadReplay->setEnabled(false);
				mainGame->btnDeleteReplay->setEnabled(false);
				mainGame->btnRenameReplay->setEnabled(false);
				mainGame->btnExportDeck->setEnabled(false);
				mainGame->btnShareReplay->setEnabled(false);
				mainGame->ebRepStartTurn->setText(L"1");
				mainGame->chkYrp->setChecked(false);
				mainGame->chkYrp->setEnabled(false);
				mainGame->ShowElement(mainGame->wReplay);
				mainGame->RefreshReplay();
				break;
			}
			/////kdiy//////
			case BUTTON_ENTERTAUNMENT_MODE: {
                if(!mainGame->git_update || mainGame->git_error) {
                    mainGame->stACMessage->setText(gDataManager->GetSysString(8046).data());
                    mainGame->PopupElement(mainGame->wACMessage, 20);
					mainGame->mRepositoriesInfo->setVisible(true);
                    break;
                }
 				mainGame->HideElement(mainGame->wMainMenu);
#ifdef EK
				mainGame->HideElement(mainGame->wQQ);
#endif
				mainGame->stEntertainmentPlayInfo->setText(L"");
				mainGame->ShowElement(mainGame->wEntertainmentPlay);
				mainGame->mode->RefreshEntertainmentPlay(mainGame->mode->modeTexts);
				mainGame->mode->RefreshControlState(0);
				mainGame->mode->InitializeMode();
				break;
			}
			case BUTTON_ENTERTAUNMENT_START_GAME: {
				DuelClient::SendPacketToServer(CTOS_HS_START);
				break;
			}
			case BUTTON_ENTERTAUNMENT_EXIT_GAME: {
				DuelClient::StopClient();
				mainGame->dInfo.isInLobby = false;
				mainGame->mode->DestoryMode();
				mainGame->HideElement(mainGame->wEntertainmentPlay);
				mainGame->ShowElement(mainGame->wMainMenu);
#ifdef EK
				mainGame->ShowElement(mainGame->wQQ);
#endif
				mainGame->wChat->setVisible(false);
				break;
			}
            /////kdiy//////
			case BUTTON_SINGLE_MODE: {
                //////kdiy/////
                if(!mainGame->git_update || mainGame->git_error) {
                    mainGame->stACMessage->setText(gDataManager->GetSysString(8046).data());
					mainGame->mRepositoriesInfo->setVisible(true);
                    break;
                }
                ////kdiy////////
				mainGame->HideElement(mainGame->wMainMenu);
				////kdiy////////
#ifdef EK
				mainGame->HideElement(mainGame->wQQ);
#endif
				////kdiy////////
				mainGame->stSinglePlayInfo->setText(L"");
				mainGame->btnLoadSinglePlay->setEnabled(false);
				mainGame->btnDeleteSinglePlay->setEnabled(false);
				mainGame->btnRenameSinglePlay->setEnabled(false);
				mainGame->btnOpenSinglePlay->setEnabled(false);
				mainGame->btnShareSinglePlay->setEnabled(false);
				mainGame->ShowElement(mainGame->wSinglePlay);
				mainGame->RefreshSingleplay();
				break;
			}
			case BUTTON_LOAD_REPLAY: {
                ////kdiy////////
                if(mainGame->wCharacterReplay->isVisible())
                    mainGame->HideElement(mainGame->wCharacterReplay);
                ////kdiy////////
				if(mainGame->lstReplayList->isDirectory(mainGame->lstReplayList->getSelected()))
					mainGame->lstReplayList->enterDirectory(mainGame->lstReplayList->getSelected());
				else
					LoadReplay();
				break;
			}
			case BUTTON_DELETE_REPLAY: {
				int sel = mainGame->lstReplayList->getSelected();
				if(sel == -1)
					break;
				std::lock_guard<epro::mutex> lock(mainGame->gMutex);
				mainGame->stQMessage->setText(epro::format(L"{}\n{}", mainGame->lstReplayList->getListItem(sel), gDataManager->GetSysString(1363)).data());
				mainGame->PopupElement(mainGame->wQuery);
				prev_operation = id;
				prev_sel = sel;
				break;
			}
			case BUTTON_RENAME_REPLAY: {
				int sel = mainGame->lstReplayList->getSelected();
				if(sel == -1)
					break;
				std::lock_guard<epro::mutex> lock(mainGame->gMutex);
				mainGame->PopupSaveWindow(gDataManager->GetSysString(1362), mainGame->lstReplayList->getListItem(sel), gDataManager->GetSysString(1342));
				prev_operation = id;
				prev_sel = sel;
				break;
			}
			case BUTTON_RENAME_SINGLEPLAY: {
				int sel = mainGame->lstSinglePlayList->getSelected();
				if(sel == -1)
					break;
				std::lock_guard<epro::mutex> lock(mainGame->gMutex);
				mainGame->PopupSaveWindow(gDataManager->GetSysString(1362), mainGame->lstSinglePlayList->getListItem(sel), gDataManager->GetSysString(1201));
				prev_operation = id;
				prev_sel = sel;
				break;
			}
			case BUTTON_CANCEL_REPLAY: {
				mainGame->HideElement(mainGame->wReplay);
				mainGame->ShowElement(mainGame->wMainMenu);
				////kdiy////////
                if(mainGame->wCharacterReplay->isVisible())
                    mainGame->HideElement(mainGame->wCharacterReplay);
#ifdef EK
				mainGame->ShowElement(mainGame->wQQ);
#endif
				////kdiy////////
				break;
			}
			case BUTTON_HP_AI_TOGGLE: {
				if (mainGame->gBot.window->isVisible()) {
					mainGame->HideElement(mainGame->gBot.window);
				}
				else {
					mainGame->ShowElement(mainGame->gBot.window);
				}
				break;
			}
			case BUTTON_BOT_ADD: {
				try {
					////////kdiy/////
					if (mainGame->gBot.aiDeckSelect->getSelected() != -1) {
					   gGameConfig->lastAIdeckfolder = mainGame->gBot.aiDeckSelect2->getItem(mainGame->gBot.aiDeckSelect2->getSelected());
					   gGameConfig->lastAIdeck = mainGame->gBot.aiDeckSelect->getItem(mainGame->gBot.aiDeckSelect->getSelected());
					}
					////////kdiy/////
					int port = std::stoi(gGameConfig->serverport);
					if(mainGame->gBot.LaunchSelected(port, mainGame->dInfo.secret.pass))
						break;
				} catch(...) {}
				mainGame->PopupMessage(gDataManager->GetSysString(12122).data());
				break;
			}
			case BUTTON_BOT_COPY_COMMAND: {
				try {
					int port = std::stoi(gGameConfig->serverport);
					const auto params = mainGame->gBot.GetParameters(port, mainGame->dInfo.secret.pass);
					if(params.size()) {
						Utils::OSOperator->copyToClipboard(mainGame->gBot.GetParameters(port, mainGame->dInfo.secret.pass).data());
						mainGame->stACMessage->setText(gDataManager->GetSysString(12121).data());
						mainGame->PopupElement(mainGame->wACMessage, 20);
					}
				} catch(...) {}
				break;
			}
            ////kdiy////////
            case BUTTON_CHARACTER_REPLAY: {
                for(int i = 0; i < 6; i++) {
                    mainGame->icon2[i]->setVisible(false);
                    mainGame->ebName_replay[i]->setVisible(false);
                    mainGame->btnCharacterSelect_replayreset[i]->setVisible(false);
                    mainGame->ebCharacter_replay[i]->setVisible(false);
                }
#ifndef VIP
                Utils::SystemOpen(EPRO_TEXT("https://afdian.com/p/af7099f4b5fa11ef98ba52540025c377/"), Utils::OPEN_URL);
				break;
#endif
				mainGame->PopupElement(mainGame->wCharacterReplay);
				mainGame->env->setFocus(mainGame->wCharacterReplay);
                auto& replay = ReplayMode::cur_replay;
				const auto& names1 = replay.GetPlayerNames();
				auto& names = replay.playersC;
                if(names.empty() || names.size() < 2)
                    names.assign(names1.begin(), names1.end());
				for(int i = 0; i < replay.GetPlayersCount(0); i++) {
                    if(gSoundManager->character[i] > CHARACTER_VOICE - 1)
					    mainGame->imageManager.icon[gSoundManager->character[i]][gSoundManager->subcharacter[gSoundManager->character[i]]] = 0;
                    mainGame->icon2[i]->setVisible(true);
                    mainGame->icon2[i]->setRelativePosition(mainGame->Resize(40, 45 + i * 35, 70, 75 + i * 35));
				    mainGame->icon2[i]->setImage(mainGame->imageManager.icon[gSoundManager->character[i]][gSoundManager->subcharacter[gSoundManager->character[i]]]);
                    mainGame->ebName_replay[i]->setVisible(true);
                    mainGame->ebName_replay[i]->setRelativePosition(mainGame->Resize(80, 50 + i * 35, 160, 70 + i * 35));
                    mainGame->ebName_replay[i]->setText(Utils::ToUnicodeIfNeeded(names[i]).c_str());
                    mainGame->btnCharacterSelect_replayreset[i]->setVisible(true);
                    mainGame->btnCharacterSelect_replayreset[i]->setRelativePosition(mainGame->Resize(165, 50 + i * 35, 225, 70 + i * 35));
                    mainGame->ebCharacter_replay[i]->setVisible(true);
					mainGame->ebCharacter_replay[i]->setRelativePosition(mainGame->Resize(230, 50 + i * 35, 330, 70 + i * 35));
				}
                int height = 65 + 15 + (replay.GetPlayersCount(0) - 1) * 35;
                mainGame->stCharacterReplay->setRelativePosition(mainGame->Resize(80, height, 180, height + 20));
                height = height + 20 + 15;
				for(int i = 0; i < replay.GetPlayersCount(1); i++) {
                    int j = i + replay.GetPlayersCount(0);
                    if(gSoundManager->character[j] > CHARACTER_VOICE - 1)
					    mainGame->imageManager.icon[gSoundManager->character[j]][gSoundManager->subcharacter[gSoundManager->character[j]]] = 0;
                    mainGame->icon2[j]->setVisible(true);
                    mainGame->icon2[j]->setRelativePosition(mainGame->Resize(40, height + i * 35, 70, height + 30 + i * 35));
				    mainGame->icon2[j]->setImage(mainGame->imageManager.icon[gSoundManager->character[j]][gSoundManager->subcharacter[gSoundManager->character[j]]]);
                    mainGame->ebName_replay[j]->setVisible(true);
                    mainGame->ebName_replay[j]->setRelativePosition(mainGame->Resize(80, height + i * 35 + 5, 160, height + 30 + i * 35 - 5));
                    mainGame->ebName_replay[j]->setText(Utils::ToUnicodeIfNeeded(names[j]).c_str());
                    mainGame->btnCharacterSelect_replayreset[j]->setVisible(true);
                    mainGame->btnCharacterSelect_replayreset[j]->setRelativePosition(mainGame->Resize(165, height + i * 35 + 5, 225, height + 30 + i * 35 - 5));
                    mainGame->ebCharacter_replay[j]->setVisible(true);
                    mainGame->ebCharacter_replay[j]->setRelativePosition(mainGame->Resize(230, height + i * 35 + 5, 330, height + 30 + i * 35 - 5));
				}
                mainGame->btnCharacterSelect_replayclose->setRelativePosition(mainGame->Resize(80, height + 20 + 15 + (replay.GetPlayersCount(1) - 1) * 35, 180, height + 20 + 15 + 20 + (replay.GetPlayersCount(1) - 1) * 35));
                break;
			}
            case BUTTON_NAMERESET_REPLAY: {
                auto& replay = ReplayMode::cur_replay;
				const auto& names1 = replay.GetPlayerNames();
				auto& names = replay.playersC;
				if(names.empty() || names.size() < 2) break;
				auto elem = static_cast<irr::gui::IGUIButton*>(event.GUIEvent.Caller);
                for(int i = 0; i < 6; i++) {
                    if(elem == mainGame->btnCharacterSelect_replayreset[i]) {
                        names[i] = names1[i];
                        mainGame->ebName_replay[i]->setText(Utils::ToUnicodeIfNeeded(names[i]).c_str());
                    }
                }
                std::wstring repinfo;
				time_t curtime = replay.pheader.base.timestamp;
				repinfo.append(epro::format(L"{:%Y/%m/%d %H:%M:%S}\n", fmt::localtime(curtime)));
				for(int i = 0; i < replay.GetPlayersCount(0); i++) {
					repinfo.append(names[i] + L"\n");
				}
				repinfo.append(L"===VS===\n");
				for(int i = 0; i < replay.GetPlayersCount(1); i++) {
					repinfo.append(names[i + replay.GetPlayersCount(0)] + L"\n");
				}
				if(replay.GetTurnsCount())
					repinfo.append(epro::format(L"\n{}: {}", gDataManager->GetSysString(2009), replay.GetTurnsCount()));
				mainGame->stReplayInfo->setText(repinfo.data());
				break;
			}
            case BUTTON_CHARACTEROK_REPLAY: {
				mainGame->HideElement(mainGame->wCharacterReplay);
				break;
			}
            ////kdiy////////
			case BUTTON_EXPORT_DECK: {
				auto sanitize = [](epro::path_string text) {
					constexpr wchar_t chars[] = L"<>:\"/\\|?*";
					for(auto& forbid : chars)
						text.erase(std::remove(text.begin(), text.end(), forbid), text.end());
					return text;
				};
				if(!ReplayMode::cur_replay.IsExportable())
					break;
				const auto& players = ReplayMode::cur_replay.GetPlayerNames();
				if(players.empty())
					break;
				const auto& decks = ReplayMode::cur_replay.GetPlayerDecks();
				if(players.size() > decks.size())
					break;
				const auto replay_name = Utils::GetFileName(ReplayMode::cur_replay.GetReplayName());
				for(size_t i = 0; i < decks.size(); i++) {
					DeckManager::SaveDeck(epro::format(EPRO_TEXT("{} player{:02} {}"), replay_name, i, sanitize(Utils::ToPathString(players[i]))), decks[i].main_deck, decks[i].extra_deck, cardlist_type());
				}
				mainGame->stACMessage->setText(gDataManager->GetSysString(1367).data());
				mainGame->PopupElement(mainGame->wACMessage, 20);
				break;
			}
			case BUTTON_SHARE_REPLAY: {
				const auto& list = mainGame->lstReplayList;
				const auto selected = list->getSelected();
				if(selected != -1 && !list->isDirectory(selected)) {
					Utils::SystemOpen(Utils::ToPathString(list->getListItem(selected, true)), Utils::SHARE_FILE);
				}
				break;
			}
			case BUTTON_DELETE_SINGLEPLAY: {
				int sel = mainGame->lstSinglePlayList->getSelected();
				if(sel == -1)
					break;
				std::lock_guard<epro::mutex> lock(mainGame->gMutex);
				mainGame->stQMessage->setText(epro::format(L"{}\n{}", mainGame->lstSinglePlayList->getListItem(sel), gDataManager->GetSysString(1363)).data());
				mainGame->PopupElement(mainGame->wQuery);
				prev_operation = id;
				prev_sel = sel;
				break;
			}
			case BUTTON_SHARE_SINGLEPLAY: {
				const auto& list = mainGame->lstSinglePlayList;
				const auto selected = list->getSelected();
				if(selected != -1 && !list->isDirectory(selected)) {
					Utils::SystemOpen(Utils::ToPathString(list->getListItem(selected, true)), Utils::SHARE_FILE);
				}
				break;
			}
			case BUTTON_OPEN_SINGLEPLAY: {
				const auto& list = mainGame->lstSinglePlayList;
				const auto selected = list->getSelected();
				if(selected != -1 && !list->isDirectory(selected)) {
					Utils::SystemOpen(Utils::ToPathString(list->getListItem(selected, true)), Utils::OPEN_FILE);
				}
				break;
			}
			case BUTTON_LOAD_SINGLEPLAY: {
				const auto& list = mainGame->lstSinglePlayList;
				const auto selected = list->getSelected();
				if(list->isDirectory(selected))
					list->enterDirectory(selected);
				else {
					if(!open_file && (selected == -1))
						break;
					SingleMode::singleSignal.SetNoWait(false);
					SingleMode::DuelOptions opts;
					if(!open_file)
						opts.scriptName = BufferIO::EncodeUTF8(list->getListItem(selected, true));
					SingleMode::StartPlay(std::move(opts));
				}
				break;
			}
			case BUTTON_CANCEL_SINGLEPLAY: {
				if(mainGame->dInfo.isSingleMode)
					break;
				mainGame->HideElement(mainGame->wSinglePlay);
				mainGame->ShowElement(mainGame->wMainMenu);
				////kdiy////////
#ifdef EK
				mainGame->ShowElement(mainGame->wQQ);
#endif
				////kdiy////////
				break;
			}
			case BUTTON_DECK_EDIT: {
				//////kdiy/////
                if(!mainGame->git_update || mainGame->git_error) {
                    mainGame->stACMessage->setText(gDataManager->GetSysString(8046).data());
                    mainGame->PopupElement(mainGame->wACMessage, 20);
					mainGame->mRepositoriesInfo->setVisible(true);
                    break;
                }
				//mainGame->RefreshDeck(mainGame->cbDBDecks);
				// if(open_file && mainGame->deckBuilder.SetCurrentDeckFromFile(open_file_name, true)) {
				// 	auto name = Utils::GetFileName(open_file_name);
				//mainGame->btnLeaveGame->setRelativePosition(mainGame->Resize(205, 137, 295, 187));
				mainGame->RefreshDeck(mainGame->cbDBDecks, true);
				auto folder = Utils::ToPathString(mainGame->cbDBDecks2->getItem(mainGame->cbDBDecks2->getSelected()));
                for(uint32_t i = 0; i < mainGame->cbDBDecks2->getItemCount() - 1; i++) {
                    mainGame->cbDBDecks22->addItem(mainGame->cbDBDecks2->getItem(i));
                }
				if(open_file && mainGame->deckBuilder.SetCurrentDeckFromFile(folder + EPRO_TEXT("/") + open_file_name, true)) {
					auto name = Utils::GetFileName(folder + EPRO_TEXT("/") + open_file_name);
				//////kdiy/////
					mainGame->ebDeckname->setText(Utils::ToUnicodeIfNeeded(name).data());
					//////kdiy/////
					mainGame->cbDBDecks2->setSelected(-1);
                    mainGame->cbDBDecks22->setSelected(-1);
					//////kdiy/////
					mainGame->cbDBDecks->setSelected(-1);
					open_file = false;
				//////kdiy/////
				// } else if(mainGame->cbDBDecks->getSelected() != -1) {
				} else if(mainGame->cbDBDecks->getSelected() >= 0 && mainGame->cbDBDecks2->getSelected() >= 0) {
					// mainGame->deckBuilder.SetCurrentDeckFromFile(Utils::ToPathString(mainGame->cbDBDecks->getItem(mainGame->cbDBDecks->getSelected())), true);
					mainGame->deckBuilder.SetCurrentDeckFromFile(folder + EPRO_TEXT("/") + Utils::ToPathString(mainGame->cbDBDecks->getItem(mainGame->cbDBDecks->getSelected())), true);
                    mainGame->cbDBDecks22->setSelected(mainGame->cbDBDecks2->getSelected());
                //////kdiy/////
					mainGame->ebDeckname->setText(L"");
				}
				mainGame->HideElement(mainGame->wMainMenu);
				////kdiy////////
#ifdef EK
				mainGame->HideElement(mainGame->wQQ);
#endif
				////kdiy////////
				mainGame->deckBuilder.Initialize();
				break;
			}
			case BUTTON_MSG_OK: {
				mainGame->HideElement(mainGame->wMessage);
				break;
			}
			case BUTTON_YES: {
				mainGame->HideElement(mainGame->wQuery);
				switch(prev_operation) {
#if EDOPRO_LINUX && (IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9)
				case ACTION_TRY_WAYLAND: {
					gGameConfig->useWayland = 1;
					mainGame->SaveConfig();
					Utils::Reboot();
					break;
				}
#endif
				case BUTTON_DELETE_REPLAY: {
					if(Replay::DeleteReplay(Utils::ToPathString(mainGame->lstReplayList->getListItem(prev_sel, true)))) {
						mainGame->stReplayInfo->setText(L"");
						mainGame->lstReplayList->refreshList();
					}
					break;
				}
				case BUTTON_DELETE_SINGLEPLAY: {
					if(Utils::FileDelete(Utils::ToPathString(mainGame->lstSinglePlayList->getListItem(prev_sel, true)))) {
						mainGame->stSinglePlayInfo->setText(L"");
						mainGame->lstSinglePlayList->refreshList();
					}
					break;
				}
				case ACTION_UPDATE_PROMPT: {
					///kdiy//////////
					// gClientUpdater->StartUpdate(Game::UpdateDownloadBar, mainGame);
					// mainGame->HideElement(mainGame->wMainMenu);
					// mainGame->PopupElement(mainGame->updateWindow);
					Utils::SystemOpen(EPRO_TEXT("https://jq.qq.com/?_wv=1027&k=S1vfY66P"), Utils::OPEN_URL);
#if defined(Update_PW)
					mainGame->PopupElement(mainGame->pwupdateWindow);
#else
					gClientUpdater->StartUpdate(Game::UpdateDownloadBar, mainGame);
					mainGame->wMainMenu->setVisible(false);
					mainGame->PopupElement(mainGame->updateWindow);
#endif
					///kdiy//////////
					break;
				}
				case ACTION_SHOW_CHANGELOG: {
					Utils::SystemOpen(EPRO_TEXT("https://afdian.com/p/7676acb6c0af11ed898652540025c377/"), Utils::OPEN_URL);
					mainGame->btnNo->setVisible(true);
					mainGame->btnNo->setEnabled(true);
					break;
				}
				case ACTION_ACKNOWLEDGE_HOST: {
					DuelClient::JoinFromDiscord();
					break;
				}
				}
				prev_operation = 0;
				prev_sel = -1;
				break;
			}
			case BUTTON_NO: {
				switch(prev_operation) {
#if EDOPRO_LINUX && (IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9)
				case ACTION_TRY_WAYLAND:
					gGameConfig->useWayland = 0;
					mainGame->SaveConfig();
					break;
#endif
				case ACTION_UPDATE_PROMPT:
				/////kdiy///////
				    gGameConfig->system_engine = false;
					Utils::DeleteDirectory(EPRO_TEXT("./config/languages/"));
                    Utils::DeleteDirectory(EPRO_TEXT("./repositories/kcg/"));
				/////kdiy///////
				case ACTION_SHOW_CHANGELOG:
                    /////kdiy///////
				    gGameConfig->system_engine = true;
					mainGame->btnNo->setVisible(true);
					mainGame->btnNo->setEnabled(true);
                    /////kdiy///////
					mainGame->wQuery->setRelativePosition(mainGame->ResizeWin(490, 200, 840, 340)); // from Game::OnResize
				default:
					break;
				}
				mainGame->HideElement(mainGame->wQuery);
				prev_operation = 0;
				prev_sel = -1;
				break;
			}
			case BUTTON_FILE_SAVE: {
				mainGame->HideElement(mainGame->wFileSave);
				irr::gui::CGUIFileSelectListBox* list = nullptr;
				if(prev_operation == BUTTON_RENAME_REPLAY)
					list = mainGame->lstReplayList;
				else if(prev_operation == BUTTON_RENAME_SINGLEPLAY)
					list = mainGame->lstSinglePlayList;
				if(list) {
					auto oldname = Utils::ToPathString(list->getListItem(prev_sel, true));
					auto oldpath = Utils::GetFilePath(oldname);
					auto extension = Utils::GetFileExtension(oldname, false);
					auto newname = Utils::ToPathString(mainGame->ebFileSaveName->getText());
					if(Utils::GetFileExtension(newname, false) != extension)
						newname.append(1, EPRO_TEXT('.')).append(extension);
					if(Utils::FileMove(oldname, oldpath + newname))
						list->refreshList();
					else
						mainGame->PopupMessage(gDataManager->GetSysString(1365));
				}
				prev_operation = 0;
				prev_sel = -1;
				break;
			}
			case BUTTON_FILE_CANCEL: {
				mainGame->HideElement(mainGame->wFileSave);
				prev_operation = 0;
				prev_sel = -1;
				break;
			}
			case BUTTON_FILTER_RELAY: {
				ServerLobby::FillOnlineRooms();
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_LISTBOX_CHANGED: {
			switch(id) {
			/////kdiy/////
			case LISTBOX_ENTERTAINMENTPLAY_LIST: {
				int sel = mainGame->lstEntertainmentPlayList->getSelected();
				mainGame->mode->modeIndex = sel;
				if(sel != -1) {
					mainGame->mode->RefreshControlState(mainGame->mode->modeIndex + 1);
					mainGame->mode->SetControlState(mainGame->mode->modeIndex);
				}
				break;
			}
		    /////kdiy/////
			case LISTBOX_LAN_HOST: {
				int sel = mainGame->lstHostList->getSelected();
				if(sel == -1)
					break;
				const auto& selection = DuelClient::hosts[sel];
				mainGame->ebJoinHost->setText(epro::to_wstring(selection.address).data());
				mainGame->ebJoinPort->setText(epro::to_wstring(selection.port).data());
				break;
			}
			case LISTBOX_REPLAY_LIST: {
				int sel = mainGame->lstReplayList->getSelected();
				mainGame->stReplayInfo->setText(L"");
                /////kdiy/////
                mainGame->wCharacterReplay->setVisible(false);
                mainGame->btnCharacterSelect_replay->setEnabled(false);
                /////kdiy/////
				mainGame->btnLoadReplay->setEnabled(false);
				mainGame->btnDeleteReplay->setEnabled(false);
				mainGame->btnRenameReplay->setEnabled(false);
				mainGame->btnExportDeck->setEnabled(false);
				mainGame->btnShareReplay->setEnabled(false);
				mainGame->btnLoadReplay->setText(gDataManager->GetSysString(1348).data());
				if(sel == -1)
					break;
				if(mainGame->lstReplayList->isDirectory(sel)) {
					mainGame->btnLoadReplay->setText(gDataManager->GetSysString(1359).data());
					mainGame->btnLoadReplay->setEnabled(true);
					break;
				}
				auto& replay = ReplayMode::cur_replay;
				const auto path = Utils::ToPathString(mainGame->lstReplayList->getListItem(sel, true));
				replay.OpenReplay(path);

				bool can_be_played = replay.CanBePlayedInStreamedMode() || (replay.CanBePlayedInOldMode() && mainGame->coreloaded);
				mainGame->btnLoadReplay->setEnabled(can_be_played);
                /////kdiy/////
                mainGame->btnCharacterSelect_replay->setEnabled(can_be_played);
                /////kdiy/////

				mainGame->btnDeleteReplay->setEnabled(true);
				mainGame->btnRenameReplay->setEnabled(true);
				mainGame->btnExportDeck->setEnabled(replay.IsExportable());
				mainGame->btnShareReplay->setEnabled(true);
				std::wstring repinfo;
				time_t curtime = replay.pheader.base.timestamp;
				repinfo.append(epro::format(L"{:%Y/%m/%d %H:%M:%S}\n", fmt::localtime(curtime)));
                ///kdiy////
				//const auto& names = replay.GetPlayerNames();
				const auto& names1 = replay.GetPlayerNames();
                auto& names = replay.playersC;
                if(names.empty() || names.size() < 2)
                    names.assign(names1.begin(), names1.end());
                ///kdiy////
				for(int i = 0; i < replay.GetPlayersCount(0); i++) {
					repinfo.append(names[i] + L"\n");
				}
				repinfo.append(L"===VS===\n");
				for(int i = 0; i < replay.GetPlayersCount(1); i++) {
					repinfo.append(names[i + replay.GetPlayersCount(0)] + L"\n");
				}
				if(replay.GetTurnsCount())
					repinfo.append(epro::format(L"\n{}: {}", gDataManager->GetSysString(2009), replay.GetTurnsCount()));
				mainGame->ebRepStartTurn->setText(L"1");
				mainGame->stReplayInfo->setText(repinfo.data());
				mainGame->chkYrp->setChecked(false);
				mainGame->chkYrp->setEnabled(replay.HasPlayableYrp() && mainGame->coreloaded);
				break;
			}
			case LISTBOX_SINGLEPLAY_LIST: {
				mainGame->btnLoadSinglePlay->setEnabled(false);
				mainGame->btnDeleteSinglePlay->setEnabled(false);
				mainGame->btnRenameSinglePlay->setEnabled(false);
				mainGame->btnOpenSinglePlay->setEnabled(false);
				mainGame->btnShareSinglePlay->setEnabled(false);
				int sel = mainGame->lstSinglePlayList->getSelected();
				mainGame->stSinglePlayInfo->setText(L"");
				if(mainGame->lstSinglePlayList->isDirectory(sel)) {
					mainGame->btnLoadSinglePlay->setText(gDataManager->GetSysString(1359).data());
					mainGame->btnLoadSinglePlay->setEnabled(true);
					break;
				} else
					mainGame->btnLoadSinglePlay->setText(gDataManager->GetSysString(1357).data());
				if(sel == -1)
					break;
				mainGame->btnLoadSinglePlay->setEnabled(mainGame->coreloaded);
				mainGame->btnDeleteSinglePlay->setEnabled(true);
				mainGame->btnRenameSinglePlay->setEnabled(true);
				mainGame->btnOpenSinglePlay->setEnabled(true);
				mainGame->btnShareSinglePlay->setEnabled(true);
				const wchar_t* name = mainGame->lstSinglePlayList->getListItem(mainGame->lstSinglePlayList->getSelected(), true);
				mainGame->stSinglePlayInfo->setText(Utils::ReadPuzzleMessage(name).data());
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_LISTBOX_SELECTED_AGAIN: {
			switch(id) {
			case LISTBOX_LAN_HOST: {
				int sel = mainGame->lstHostList->getSelected();
				if(sel == -1)
					break;
				try {
					const auto parsed = epro::Host::resolve(mainGame->ebJoinHost->getText(), mainGame->ebJoinPort->getText());
					gGameConfig->lasthost = mainGame->ebJoinHost->getText();
					gGameConfig->lastport = mainGame->ebJoinPort->getText();
					mainGame->dInfo.secret.pass = mainGame->ebJoinPass->getText();
					if(DuelClient::StartClient(parsed.address, parsed.port, 0, false)) {
						mainGame->btnCreateHost->setEnabled(false);
						mainGame->btnJoinHost->setEnabled(false);
						mainGame->btnJoinCancel->setEnabled(false);
					}
					break;
				}
				catch(...) {
					mainGame->PopupMessage(gDataManager->GetSysString(1412));
					break;
				}
			}
			case LISTBOX_REPLAY_LIST: {
                ////kdiy////////
                if(mainGame->wCharacterReplay->isVisible())
                     mainGame->HideElement(mainGame->wCharacterReplay);
                ////kdiy////////
				if(mainGame->lstReplayList->isDirectory(mainGame->lstReplayList->getSelected()))
					mainGame->lstReplayList->enterDirectory(mainGame->lstReplayList->getSelected());
				else
					LoadReplay();
				break;
			}
			case LISTBOX_SINGLEPLAY_LIST: {
				if(!mainGame->btnLoadSinglePlay->isEnabled())
					break;
				const auto& list = mainGame->lstSinglePlayList;
				const auto selected = list->getSelected();
				if(selected == -1)
					break;
				if(list->isDirectory(selected))
					list->enterDirectory(selected);
				else {
					SingleMode::singleSignal.SetNoWait(false);
					SingleMode::DuelOptions opts(BufferIO::EncodeUTF8(list->getListItem(selected, true)));
					SingleMode::StartPlay(std::move(opts));
				}
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_CHECKBOX_CHANGED: {
			switch(id) {
			/////kdiy/////
			case CHECKBOX_ENTERTAUNMENT_MODE_1_CHECK: {
				if(mainGame->chkEntertainmentMode_1Check->isChecked())
					mainGame->cbEntertainmentMode_1Bot->setEnabled(true);
				else
					mainGame->cbEntertainmentMode_1Bot->setEnabled(false);
				break;
			}
			case CHECKBOX_ENTERTAUNMENT_READY:{
				if(mainGame->chkEntertainmentPrepReady->isChecked()) {
					mainGame->lstEntertainmentPlayList->setEnabled(false);
					mainGame->chkEntertainmentMode_1Check->setEnabled(false);
					mainGame->cbEntertainmentMode_1Bot->setEnabled(false);
					mainGame->mode->SetRule(mainGame->mode->modeIndex);
					DuelClient::is_local_host = false;
					uint16_t host_port;
					try {
						host_port = static_cast<uint16_t>(std::stoul(mainGame->ebHostPort->getText()));
					}
					catch(...) {
						break;
					}
                    if(!NetServer::StartServer(host_port)) {
                        mainGame->mode->isMode = false;
                        break;
                    }
					const auto ip = 0x100007F; //127.0.0.1 in network byte order
					if(!DuelClient::StartClient({ &ip, epro::Address::INET }, host_port)) {
                        NetServer::StopServer();
                        mainGame->mode->isMode = false;
                        break;
                    }
					DuelClient::is_local_host = true;
					try {
						mainGame->mode->LoadWindBot(host_port, L"");
					}
					catch(...)
					{
						mainGame->PopupMessage(L"Failed to launch windbot");
                        mainGame->mode->isMode = false;
					}
                    break;
				} else {
					mainGame->lstEntertainmentPlayList->setEnabled(true);
					mainGame->btnEntertainmentStartGame->setEnabled(false);
					mainGame->chkEntertainmentMode_1Check->setEnabled(true);
					if(mainGame->chkEntertainmentMode_1Check->isChecked())
						mainGame->cbEntertainmentMode_1Bot->setEnabled(true);
					else
						mainGame->cbEntertainmentMode_1Bot->setEnabled(false);
					DuelClient::StopClient();
				}
				break;
			}
			case CHECKBOX_DEFAULT_LOCAL: {
				bool chk = true;
				if(mainGame->chkdefaultlocal->isChecked()) {
					chk = false;
                    mainGame->chkAI->setChecked(false);
                }
				mainGame->cbRule2->setEnabled(chk);
				mainGame->chkAI->setEnabled(chk);
				mainGame->ebTimeLimit2->setEnabled(chk);
				mainGame->cbDuelRule2->setEnabled(chk);
				mainGame->chkNoCheckDeckContent2->setEnabled(chk);
				mainGame->chkNoShuffleDeck2->setEnabled(chk);
				mainGame->chkNoLFlist2->setEnabled(chk);
				mainGame->chkTag->setEnabled(chk);
				mainGame->chkMatch->setEnabled(chk);
				mainGame->ebStartLP2->setEnabled(chk);
				mainGame->ebStartHand2->setEnabled(chk);
				mainGame->ebDrawCount2->setEnabled(chk);
				mainGame->ebJoinPass2->setEnabled(chk);
				break;
			}
			////kdiy///////
			case CHECK_SHOW_LOCKED_ROOMS: {
				ServerLobby::FillOnlineRooms();
				break;
			}
			case CHECK_SHOW_ACTIVE_ROOMS: {
				ServerLobby::FillOnlineRooms();
				//atm the behaviour for locked rooms is the same, so no need to refresh
				//ServerLobby::RefreshRooms();
				break;
			}
			case CHECKBOX_HP_READY: {
				if(!caller->isEnabled())
					break;
				mainGame->env->setFocus(mainGame->wHostPrepare);
				if(static_cast<irr::gui::IGUICheckBox*>(caller)->isChecked()) {
					const auto selected = mainGame->cbDeckSelect->getSelected();
					///////kdiy////
					//if(selected == -1 || !mainGame->deckBuilder.SetCurrentDeckFromFile(Utils::ToPathString(mainGame->cbDeckSelect->getItem(selected)))) {
					auto folder = Utils::ToPathString(mainGame->cbDeck2Select->getItem(mainGame->cbDeck2Select->getSelected()));
					if(mainGame->cbDeck2Select->getSelected() == -1 || selected == -1 || !mainGame->deckBuilder.SetCurrentDeckFromFile(folder + EPRO_TEXT("/") + Utils::ToPathString(mainGame->cbDeckSelect->getItem(selected)))) {
					///////kdiy////
						static_cast<irr::gui::IGUICheckBox*>(caller)->setChecked(false);
						break;
					}
					UpdateDeck();
					DuelClient::SendPacketToServer(CTOS_HS_READY);
					mainGame->cbDeckSelect->setEnabled(false);
					///////kdiy////
					mainGame->cbDeck2Select->setEnabled(false);
					///////kdiy////
					if(mainGame->dInfo.team1 + mainGame->dInfo.team2 > 2)
						mainGame->btnHostPrepDuelist->setEnabled(false);
				} else {
					DuelClient::SendPacketToServer(CTOS_HS_NOTREADY);
					mainGame->cbDeckSelect->setEnabled(true);
					///////kdiy////
					mainGame->cbDeck2Select->setEnabled(true);
					///////kdiy////
					if(mainGame->dInfo.team1 + mainGame->dInfo.team2 > 2)
						mainGame->btnHostPrepDuelist->setEnabled(true);
				}
				break;
			}
			case CHECKBOX_EXTRA_RULE: {
				mainGame->UpdateExtraRules();
				break;
			}
			case CHECKBOX_PZONE: {
				if(mainGame->chkCustomRules[3]->isChecked())
					mainGame->chkCustomRules[4]->setEnabled(true);
				else {
					mainGame->chkCustomRules[4]->setChecked(false);
					mainGame->chkCustomRules[4]->setEnabled(false);
				}
				break;
			}
			case TCG_SEGOC_NONPUBLIC: {
				const auto checked = static_cast<irr::gui::IGUICheckBox*>(caller)->isChecked();
				mainGame->chkTcgRulings->setChecked(checked);
				mainGame->chkCustomRules[TCG_SEGOC_NONPUBLIC - CHECKBOX_OBSOLETE]->setChecked(checked);
				if(checked)
					mainGame->duel_param |= DUEL_TCG_SEGOC_NONPUBLIC;
				else
					mainGame->duel_param &= ~DUEL_TCG_SEGOC_NONPUBLIC;
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_EDITBOX_CHANGED: {
			switch(id) {
            //kdiy///////
            case EDITBOX_REPLAYNAME: {
                auto& replay = ReplayMode::cur_replay;
				const auto& names1 = replay.GetPlayerNames();
                auto& names = replay.playersC;
                if(names.empty() || names.size() < 2)
                    names.assign(names1.begin(), names1.end());
				auto elem = static_cast<irr::gui::IGUIEditBox*>(event.GUIEvent.Caller);
                for(int i = 0; i < 6; i++) {
					if(elem == mainGame->ebName_replay[i]) {
						names[i] = epro::format(L"{}", elem->getText());
					}
                }
                std::wstring repinfo;
				time_t curtime = replay.pheader.base.timestamp;
				repinfo.append(epro::format(L"{:%Y/%m/%d %H:%M:%S}\n", fmt::localtime(curtime)));
				for(int i = 0; i < replay.GetPlayersCount(0); i++) {
					repinfo.append(names[i] + L"\n");
				}
				repinfo.append(L"===VS===\n");
				for(int i = 0; i < replay.GetPlayersCount(1); i++) {
					repinfo.append(names[i + replay.GetPlayersCount(0)] + L"\n");
				}
				if(replay.GetTurnsCount())
					repinfo.append(epro::format(L"\n{}: {}", gDataManager->GetSysString(2009), replay.GetTurnsCount()));
				mainGame->stReplayInfo->setText(repinfo.data());
				break;
			}
            //kdiy///////
			case EDITBOX_PORT_BOX: {
				const wchar_t* text = caller->getText();
				wchar_t filtered[20];
				int j = 0;
				bool changed = false;
				for(int i = 0; text[i] && j < 19; i++) {
					if(text[i] >= L'0' && text[i]<= L'9') {
						filtered[j] = text[i];
						j++;
						changed = true;
					}
				}
				filtered[j] = 0;
				text = filtered;
				if(BufferIO::GetVal(text) > 65535) {
					text = L"65535";
					changed = true;
				}
				if(changed)
					caller->setText(text);
				break;
			}
			case EDITBOX_TEAM_COUNT: {
				auto elem = static_cast<irr::gui::IGUIEditBox*>(event.GUIEvent.Caller);
				auto min = (elem == mainGame->ebOnlineTeam1 || elem == mainGame->ebOnlineTeam2) ? L"0" : L"1";
				auto text = elem->getText();
				auto len = wcslen(text);
				if(len < 1)
					break;
				if(text[len - 1] < min[0] || text[len - 1] > L'3') {
					elem->setText(min);
					break;
				}
				wchar_t string[] = { text[len - 1], 0 };
				elem->setText(string);
				break;
			}
			case EDITBOX_NICKNAME: {
				auto elem = static_cast<irr::gui::IGUIEditBox*>(event.GUIEvent.Caller);
				auto target = (elem == mainGame->ebNickNameOnline) ? mainGame->ebNickName : mainGame->ebNickNameOnline;
				target->setText(elem->getText());
				break;
			}
			}
			if(caller->getParent() == mainGame->wRoomListPlaceholder)
				ServerLobby::FillOnlineRooms();
			break;
		}
		//////kdiy////////
		case irr::gui::EGET_EDITBOX_ENTER: {
			switch(id) {
				case EDITBOX_PASSWORD: {
#ifdef Update_PW
					auto pw = mainGame->ebPw->getText();
					std::string t(Update_PW);
					std::wstring a(t.begin(), t.end());
					if(!Utils::FileExists(EPRO_TEXT("./updates/configs.json"))) {
						if(a.compare(pw) == 0) {
							gClientUpdater->StartUpdate(Game::UpdateDownloadBar, mainGame);
							mainGame->PopupElement(mainGame->updateWindow);
							break;
						} else update_trial++;
					} else
					    update_trial = 3;
					if(update_trial > 2) {
						mainGame->btnLanMode->setEnabled(false);
						if(!Utils::FileExists(EPRO_TEXT("./updates/configs.json")) && Utils::FileExists(EPRO_TEXT("./config/configs.json"))) {
							Utils::MakeDirectory(EPRO_TEXT("updates"));
						    Utils::FileCopy(EPRO_TEXT("./config/configs.json"), EPRO_TEXT("./updates/configs.json"));
						}
						mainGame->HideElement(mainGame->pwupdateWindow);
					}
#endif
					break;
				}
			}
		}
		//////kdiy////////
		case irr::gui::EGET_TAB_CHANGED: {
			switch(id) {
			case TAB_CONTROL_CREATE_HOST: {
				auto elem = static_cast<irr::gui::IGUITabControl*>(event.GUIEvent.Caller);
				auto curTab = elem->getActiveTab();
				if(curTab == 0) {
					mainGame->UpdateDuelParam();
				} else {
					const auto tcg = mainGame->duel_param & DUEL_TCG_SEGOC_NONPUBLIC;
	#define CHECK(MR) case (MR - 1):{ mainGame->duel_param = DUEL_MODE_MR##MR; mainGame->forbiddentypes = DUEL_MODE_MR##MR##_FORB; break; }
					switch (mainGame->cbDuelRule->getSelected()) {
					CHECK(1)
					CHECK(2)
					CHECK(3)
					CHECK(4)
					CHECK(5)
					case 5:	{
						mainGame->duel_param = DUEL_MODE_SPEED;
						mainGame->forbiddentypes = 0;
						break;
					}
					case 6:	{
						mainGame->duel_param = DUEL_MODE_RUSH;
						mainGame->forbiddentypes = 0;
						break;
					}
					case 7:	{
						mainGame->duel_param = DUEL_MODE_GOAT;
						mainGame->forbiddentypes = DUEL_MODE_MR1_FORB;
						break;
					}
					}
	#undef CHECK
					mainGame->duel_param |= tcg;
					for (auto i = 0u; i < sizeofarr(mainGame->chkCustomRules); ++i) {
						bool set = false;
						if(i == 19)
							set = mainGame->duel_param & DUEL_USE_TRAPS_IN_NEW_CHAIN;
						else if(i == 20)
							set = mainGame->duel_param & DUEL_6_STEP_BATLLE_STEP;
						else if(i == 21)
							set = mainGame->duel_param & DUEL_TRIGGER_WHEN_PRIVATE_KNOWLEDGE;
						else if(i > 21)
							set = mainGame->duel_param & 0x100ULL << (i - 3);
						else
							set = mainGame->duel_param & 0x100ULL << i;
						mainGame->chkCustomRules[i]->setChecked(set);
						if(i == 3)
							mainGame->chkCustomRules[4]->setEnabled(set);
					}
					static constexpr uint32_t limits[]{ TYPE_FUSION, TYPE_SYNCHRO, TYPE_XYZ, TYPE_PENDULUM, TYPE_LINK };
					for (auto i = 0u; i < sizeofarr(mainGame->chkTypeLimit); ++i)
							mainGame->chkTypeLimit[i]->setChecked(mainGame->forbiddentypes & limits[i]);
				}
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_COMBO_BOX_CHANGED: {
			switch (id) {
			case COMBOBOX_DUEL_RULE: {
				auto setDeckSizes = [&](const DeckSizes& size) {
					mainGame->ebMainMin->setText(epro::to_wstring<int>(size.main.min).data());
					mainGame->ebMainMax->setText(epro::to_wstring<int>(size.main.max).data());
					mainGame->ebExtraMin->setText(epro::to_wstring<int>(size.extra.min).data());
					mainGame->ebExtraMax->setText(epro::to_wstring<int>(size.extra.max).data());
					mainGame->ebSideMin->setText(epro::to_wstring<int>(size.side.min).data());
					mainGame->ebSideMax->setText(epro::to_wstring<int>(size.side.max).data());
				};
				static constexpr DeckSizes ocg_deck_sizes{ {40,60}, {0,15}, {0,15} };
				static constexpr DeckSizes rush_deck_sizes{ {40,60}, {0,15}, {0,15} };
				static constexpr DeckSizes speed_deck_sizes{ {20,30}, {0,6}, {0,6} };
				static constexpr DeckSizes goat_deck_sizes{ {40,60}, {0,999}, {0,15} };
				mainGame->chkTcgRulings->setChecked(false);
				auto combobox = static_cast<irr::gui::IGUIComboBox*>(event.GUIEvent.Caller);
#define CHECK(MR) case (MR - 1): { mainGame->duel_param = DUEL_MODE_MR##MR; mainGame->forbiddentypes = DUEL_MODE_MR##MR##_FORB;\
									setDeckSizes(ocg_deck_sizes); mainGame->ebStartHand->setText(L"5"); goto remove; }
				switch (combobox->getSelected()) {
				CHECK(1)
				CHECK(2)
				CHECK(3)
				CHECK(4)
				CHECK(5)
				case 5:	{
					mainGame->duel_param = DUEL_MODE_SPEED;
					setDeckSizes(speed_deck_sizes);
					mainGame->forbiddentypes = 0;
					mainGame->ebStartHand->setText(L"4");
					goto remove;
				}
				case 6:	{
					mainGame->duel_param = DUEL_MODE_RUSH;
					setDeckSizes(rush_deck_sizes);
					mainGame->forbiddentypes = 0;
					mainGame->ebStartHand->setText(L"4");
					goto remove;
				}
				case 7:	{
					mainGame->duel_param = DUEL_MODE_GOAT;
					setDeckSizes(goat_deck_sizes);
					mainGame->forbiddentypes = DUEL_MODE_MR1_FORB;
					mainGame->chkTcgRulings->setChecked(true);
					mainGame->ebStartHand->setText(L"5");
					goto remove;
				}
				default: break;
				remove:
				combobox->removeItem(8);
				mainGame->UpdateExtraRules();
				}
#undef CHECK
				for(auto i = 0u; i < sizeofarr(mainGame->chkCustomRules); ++i) {
					bool set = false;
					if(i == 19)
						set = mainGame->duel_param & DUEL_USE_TRAPS_IN_NEW_CHAIN;
					else if(i == 20)
						set = mainGame->duel_param & DUEL_6_STEP_BATLLE_STEP;
					else if(i == 21)
						set = mainGame->duel_param & DUEL_TRIGGER_WHEN_PRIVATE_KNOWLEDGE;
					else if(i > 21)
						set = mainGame->duel_param & 0x100ULL << (i - 3);
					else
						set = mainGame->duel_param & 0x100ULL << i;
					mainGame->chkCustomRules[i]->setChecked(set);
					if(i == 3)
						mainGame->chkCustomRules[4]->setEnabled(set);
				}
				static constexpr uint32_t limits[]{ TYPE_FUSION, TYPE_SYNCHRO, TYPE_XYZ, TYPE_PENDULUM, TYPE_LINK };
				for(auto i = 0u; i < sizeofarr(mainGame->chkTypeLimit); ++i)
					mainGame->chkTypeLimit[i]->setChecked(mainGame->forbiddentypes & limits[i]);
				break;
			}
			case COMBOBOX_BOT_DECK: {
				gGameConfig->lastBot = mainGame->gBot.CurrentIndex();
				///////kdiy//////////
				const auto& bot = mainGame->gBot.bots[mainGame->gBot.CurrentIndex()];
				if (bot.deck == L"AI_perfectdicky") {
					mainGame->gBot.aiDeckSelect->setVisible(true);
					mainGame->gBot.aiDeckSelect->setEnabled(true);
					mainGame->gBot.aiDeckSelect2->setVisible(true);
					mainGame->gBot.aiDeckSelect2->setEnabled(true);
				} else {
					mainGame->gBot.aiDeckSelect->setVisible(false);
					mainGame->gBot.aiDeckSelect->setEnabled(false);
					mainGame->gBot.aiDeckSelect2->setVisible(false);
					mainGame->gBot.aiDeckSelect2->setEnabled(false);
				}
				if (bot.deck == L"AI_perfectdicky" || bot.deck == L"AI_Numeron" || bot.deck == L"AI_Hope" || bot.deck == L"MokeyMokeyKing") {
					mainGame->gBot.chkSeed->setVisible(true);
					mainGame->gBot.chkSeed->setEnabled(true);
				} else {
					mainGame->gBot.chkSeed->setVisible(false);
					mainGame->gBot.chkSeed->setEnabled(false);
					mainGame->gBot.chkSeed->setSelected(0);
					gGameConfig->botSeed = 0;
				}
				///////kdiy//////////
				mainGame->gBot.UpdateEngine();
				break;
			}
			case COMBOBOX_BOT_ENGINE: {
				mainGame->gBot.UpdateDescription();
				break;
			}
			///////kdiy//////////
            case COMBOBOX_CHARACTER: {
#ifndef VIP
				break;
#endif
				bool filechk = mainGame->chantcheck();
				if(!filechk)
				    break;
                auto elem = static_cast<irr::gui::IGUIComboBox*>(event.GUIEvent.Caller);
				for(int i = 0; i < 6; ++i) {
                    if(elem == mainGame->ebCharacter[i]) {
                        int sel = mainGame->ebCharacter[i]->getSelected();
                        mainGame->charactcomboselect(i, 1, sel);
                    }
                    if(elem == mainGame->ebCharacter_replay[i]) {
                        int sel = mainGame->ebCharacter_replay[i]->getSelected();
                        mainGame->charactcomboselect(i, 2, sel);
                    }
                }

				int player = gSoundManager->character[mainGame->choose_player];
				if(player > 0 && gSoundManager->textcharacter[player-1].size() > 2) {
					mainGame->btnsubCharacterSelect[0]->setVisible(true);
					mainGame->btnsubCharacterSelect_replay[0]->setVisible(true);
					for(int i = 2; i < 6; i++) {
						if(gSoundManager->textcharacter[player-1].size() > i) {
							mainGame->btnsubCharacterSelect[i-1]->setVisible(true);
							mainGame->btnsubCharacterSelect_replay[i-1]->setVisible(true);
						} else {
							mainGame->btnsubCharacterSelect[i-1]->setVisible(false);
							mainGame->btnsubCharacterSelect_replay[i-1]->setVisible(false);
						}
					}
				} else {
					for(int i = 0; i < 5; i++) {
						mainGame->btnsubCharacterSelect[i]->setVisible(false);
						mainGame->btnsubCharacterSelect_replay[i]->setVisible(false);
					}
				}
				break;
			}
			case COMBOBOX_PICS: {
				int prevsel = gGameConfig->hdpic;
				gGameConfig->hdpic = mainGame->cbpics->getSelected();
				//mainGame->device->closeDevice();
				int sel = mainGame->cbpics->getSelected();
				if(sel > 0 && prevsel ==0) {
				    try {
						gGameConfig->dpi_scale = static_cast<uint32_t>(std::stol(mainGame->gSettings.ebDpiScale->getText())) / 100.0;
						mainGame->restart = true;
					} catch(...){}
				}
				break;
			}
			case COMBOBOX_aiDeck2: {
				int sel = mainGame->gBot.aiDeckSelect->getSelected();
				int sel2 = mainGame->gBot.aiDeckSelect2->getSelected();
				if(sel2 >= 0)
				    mainGame->RefreshDeck(mainGame->gBot.aiDeckSelect);
				break;
			}
			case COMBOBOX_cbDeckSelect: {
				int sel = mainGame->cbDeckSelect->getSelected();
				int sel2 = mainGame->cbDeck2Select->getSelected();
				if(sel2 >= 0)
				    mainGame->RefreshDeck(mainGame->cbDeckSelect);
				break;
			}
            ///////kdiy//////////
			case SERVER_CHOICE: {
				ServerLobby::RefreshRooms();
				return false;
			}
			default: break;
			}
			if(caller->getParent() == mainGame->wRoomListPlaceholder)
				ServerLobby::FillOnlineRooms();
			break;
		}
		case irr::gui::EGET_TABLE_SELECTED_AGAIN: {
			switch(id) {
			case TABLE_ROOMLIST: {
				if(wcslen(mainGame->ebNickNameOnline->getText()) <= 0) {
					mainGame->PopupMessage(gDataManager->GetSysString(1257), gDataManager->GetSysString(1256));
					break;
				}
				if(mainGame->roomListTable->getSelected() >= 0) {
					mainGame->HideElement(mainGame->wRoomListPlaceholder);
					ServerLobby::JoinServer(false);
				}
				break;
			}
			}
			break;
		}
		default: break;
		}
		break;
	}
	case irr::EET_KEY_INPUT_EVENT: {
		switch(event.KeyInput.Key) {
		case irr::KEY_KEY_R: {
			if(!event.KeyInput.PressedDown && !mainGame->HasFocus(irr::gui::EGUIET_EDIT_BOX))
				mainGame->textFont->setTransparency(true);
			break;
		}
		case irr::KEY_ESCAPE: {
			if(!mainGame->HasFocus(irr::gui::EGUIET_EDIT_BOX))
				mainGame->device->minimizeWindow();
			break;
		}
		case irr::KEY_F5: {
			if(!event.KeyInput.PressedDown && mainGame->wRoomListPlaceholder->isVisible())
				ServerLobby::RefreshRooms();
			break;
		}
		default: break;
		}
		break;
	}
#if !EDOPRO_ANDROID && !EDOPRO_IOS
	case irr::EET_DROP_EVENT: {
		static std::wstring to_open_file;
		switch(event.DropEvent.DropType) {
			case irr::DROP_START: {
				to_open_file.clear();
				break;
			}
			case irr::DROP_FILE: {
				to_open_file = event.DropEvent.Text;
				break;
			}
			case irr::DROP_END:	{
				if(to_open_file.size()) {
					auto extension = Utils::GetFileExtension(to_open_file);
					bool isMenu = !mainGame->wSinglePlay->isVisible() && !mainGame->wReplay->isVisible();
					if(extension == L"ydk" && isMenu && mainGame->deckBuilder.SetCurrentDeckFromFile(Utils::ToPathString(to_open_file))) {
						//////kdiy/////
						//mainGame->RefreshDeck(mainGame->cbDBDecks);
						mainGame->RefreshDeck(mainGame->cbDBDecks, true);
						//////kdiy/////
						auto name = Utils::GetFileName(to_open_file);
						mainGame->ebDeckname->setText(name.data());
						//////kdiy/////
						mainGame->cbDBDecks2->setSelected(-1);
                        mainGame->cbDBDecks22->setSelected(-1);
						//////kdiy/////
						mainGame->cbDBDecks->setSelected(-1);
						mainGame->HideElement(mainGame->wMainMenu);
						//////kdiy/////
#ifdef EK
						mainGame->HideElement(mainGame->wQQ);
#endif
						//////kdiy/////
						mainGame->deckBuilder.Initialize();
						return true;
					} else if(mainGame->coreloaded && extension == L"lua" && !mainGame->wReplay->isVisible()) {
						open_file = true;
						open_file_name = Utils::ToPathString(to_open_file);
						if(!mainGame->wSinglePlay->isVisible())
							ClickButton(mainGame->btnSingleMode);
						ClickButton(mainGame->btnLoadSinglePlay);
						return true;
					} else if(mainGame->coreloaded && (extension == L"yrpx" || extension == L"yrp") && !mainGame->wSinglePlay->isVisible()) {
						open_file = true;
						open_file_name = Utils::ToPathString(to_open_file);
						if(!mainGame->wReplay->isVisible())
							ClickButton(mainGame->btnReplayMode);
						ClickButton(mainGame->btnLoadReplay);
						return true;
					} else if(extension == L"pem" || extension == L"cer" || extension == L"crt") {
						gGameConfig->override_ssl_certificate_path = BufferIO::EncodeUTF8(to_open_file);
					}
					to_open_file.clear();
				}
				break;
			}
			default: break;
		}
		break;
	}
#endif
	default: break;
	}
	return false;
}

template<typename T>
static void Synchronize(const T& range, irr::gui::IGUICheckBox* elem) {
	auto checked = elem->isChecked();
	for(auto i = range.first; i != range.second; ++i)
		static_cast<irr::gui::IGUICheckBox*>(i->second)->setChecked(checked);
}
template<typename T>
static void Synchronize(const T& range, irr::gui::IGUIScrollBar* elem) {
	auto position = elem->getPos();
	for(auto i = range.first; i != range.second; ++i)
		static_cast<irr::gui::IGUIScrollBar*>(i->second)->setPos(position);
}

void MenuHandler::SynchronizeElement(irr::gui::IGUIElement* elem) const {
	const auto range = synchronized_elements.equal_range(elem->getID());
	if(range.first == range.second)
		return;
	switch(elem->getType()) {
	case irr::gui::EGUIET_CHECK_BOX:
		return Synchronize(range, static_cast<irr::gui::IGUICheckBox*>(elem));
	case irr::gui::EGUIET_SCROLL_BAR:
		return Synchronize(range, static_cast<irr::gui::IGUIScrollBar*>(elem));
	default:
		return;
	}
}

}
