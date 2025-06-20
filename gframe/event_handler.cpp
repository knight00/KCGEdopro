#include <algorithm>
#include <cassert>
#include "utils.h"
#include "game_config.h"
#include "client_field.h"
#include "client_card.h"
#include "math.h"
#include "network.h"
#include "game.h"
#include "duelclient.h"
#include "data_manager.h"
#include "image_manager.h"
#include "replay_mode.h"
#include "single_mode.h"
#include "materials.h"
#include "progressivebuffer.h"
#include "utils_gui.h"
#include "sound_manager.h"
#include "CGUIImageButton/CGUIImageButton.h"
#include "CGUITTFont/CGUITTFont.h"
#include "custom_skin_enum.h"
#include "Base64.h"
#include <IrrlichtDevice.h>
#include <ISceneManager.h>
#include <ICameraSceneNode.h>
#include <ISceneManager.h>
#include <ISceneCollisionManager.h>
#include <IrrlichtDevice.h>
#include <IGUIEnvironment.h>
#include <IGUIWindow.h>
#include <IGUIStaticText.h>
#include <IGUIEditBox.h>
#include <IGUIComboBox.h>
#include <IGUIListBox.h>
#include <IGUICheckBox.h>
#include <IGUIContextMenu.h>
#include <IGUITabControl.h>
#include <IGUIScrollBar.h>
#include "joystick_wrapper.h"
#include "porting.h"
#include "config.h"
#include "fmt.h"

namespace {

#if EDOPRO_ANDROID || EDOPRO_IOS
inline bool TransformEvent(const irr::SEvent& event, bool& stopPropagation) {
	return porting::transformEvent(event, stopPropagation);
}
#else
inline constexpr bool TransformEvent(const irr::SEvent&, bool&) {
	return false;
}
#endif
}

namespace ygo {

std::string showing_repo = "";

bool ClientField::OnEvent(const irr::SEvent& event) {
	bool stopPropagation = false;
	if(OnCommonEvent(event, stopPropagation))
		return stopPropagation;
	switch(event.EventType) {
	case irr::EET_GUI_EVENT: {
		int id = event.GUIEvent.Caller->getID();
		switch(event.GUIEvent.EventType) {
		case irr::gui::EGET_BUTTON_CLICKED: {
			switch(id) {
			///////kdiy///////
			case BUTTON_CARDINFO: {
				auto elem = static_cast<irr::gui::IGUIButton*>(event.GUIEvent.Caller);
				for (int i = 0; i < 8; i++) {
					if(elem == mainGame->CardInfo[i]) {
						mainGame->stText->setText(mainGame->effectText[i].data());
						break;
					}
				}
				break;
			}
			case BUTTON_AVATAR_CARD0: {
				mainGame->cardbutton[0]->setPressed();
				mainGame->cardbutton[1]->setPressed(false);
				mainGame->cardbutton[2]->setPressed(false);
                mainGame->cardbutton[0]->setImage(mainGame->imageManager.cardchant0);
				mainGame->cardbutton[1]->setImage(mainGame->imageManager.cardchant01);
				mainGame->cardbutton[2]->setImage(mainGame->imageManager.cardchant02);
				break;
			}
			case BUTTON_AVATAR_CARD1: {
				mainGame->cardbutton[0]->setPressed(false);
				mainGame->cardbutton[1]->setPressed();
				mainGame->cardbutton[2]->setPressed(false);
                mainGame->cardbutton[0]->setImage(mainGame->imageManager.cardchant00);
				mainGame->cardbutton[1]->setImage(mainGame->imageManager.cardchant1);
				mainGame->cardbutton[2]->setImage(mainGame->imageManager.cardchant02);
				break;
			}
			case BUTTON_AVATAR_CARD2: {
				mainGame->cardbutton[0]->setPressed(false);
				mainGame->cardbutton[1]->setPressed(false);
				mainGame->cardbutton[2]->setPressed();
                mainGame->cardbutton[0]->setImage(mainGame->imageManager.cardchant00);
				mainGame->cardbutton[1]->setImage(mainGame->imageManager.cardchant01);
				mainGame->cardbutton[2]->setImage(mainGame->imageManager.cardchant2);
				break;
			}
			case BUTTON_PLAY_CARD: {
				auto pointer = gDataManager->GetCardData(mainGame->showingcard);
				if(!pointer) {
					pointer = gDataManager->GetCardData(mainGame->showingcardalias);
					if(!pointer) break;
				}
				uint16_t extra = 0;
				uint32_t type = pointer->type;
           		if(type & TYPE_MONSTER) {
                	if(mainGame->cardbutton[0]->isPressed()) {
                    	if(type & TYPE_PENDULUM) extra |= 0x20;
                    	if(type & TYPE_LINK) extra |= 0x8;
                    	if(type & TYPE_XYZ) extra |= 0x4;
                    	if(type & TYPE_SYNCHRO) extra |= 0x2;
                    	if(type & TYPE_FUSION) extra |= 0x1;
                    	if(type & TYPE_RITUAL) extra |= 0x10;
						if(type & SUMMON_TYPE_MAXIMUM) extra |= 0x800;
						if((pointer->level >= 5) && !(type & (TYPE_PENDULUM | TYPE_LINK | TYPE_XYZ | TYPE_SYNCHRO | TYPE_FUSION | TYPE_RITUAL | TYPE_SPSUMMON))) extra |= 0x400;
                    	gSoundManager->PlayChant(SoundManager::CHANT::SUMMON, mainGame->showingcard, mainGame->showingcardalias, 0, 0, extra);
                	} else if(mainGame->cardbutton[1]->isPressed()) {
                    	extra = 0x1;
				    	gSoundManager->PlayChant(SoundManager::CHANT::ATTACK, mainGame->showingcard, mainGame->showingcardalias, 0, 0, extra);
                	} else if(mainGame->cardbutton[2]->isPressed()) {
                    	extra = 0x801;
                    	gSoundManager->PlayChant(SoundManager::CHANT::ACTIVATE, mainGame->showingcard, mainGame->showingcardalias, 0, 0, extra);
                	}
            	} else {
                	extra = 0x1;
                	if(type & TYPE_SPELL) {
                    	if(!(type & (TYPE_FIELD | TYPE_EQUIP | TYPE_CONTINUOUS | TYPE_RITUAL | TYPE_QUICKPLAY | TYPE_PENDULUM))) extra |= 0x4;
                    	if(type & TYPE_QUICKPLAY) extra |= 0x8;
                    	if(type & TYPE_CONTINUOUS) extra |= 0x10;
                    	if(type & TYPE_EQUIP) extra |= 0x20;
                    	if(type & TYPE_RITUAL) extra |= 0x40;
                    	if(type & TYPE_FIELD) extra |= 0x1000;
                    	if(type & TYPE_ACTION) extra |= 0x4000;
                	}
                	if(type & TYPE_TRAP) {
                    	if(!(type & (TYPE_COUNTER | TYPE_CONTINUOUS))) extra |= 0x80;
                    	if(type & TYPE_CONTINUOUS) extra |= 0x100;
                    	if(type & TYPE_COUNTER) extra |= 0x200;
                	}
					gSoundManager->PlayChant(SoundManager::CHANT::ACTIVATE, mainGame->showingcard, mainGame->showingcardalias, 0, 0, extra);
            	}
				break;
			}
			case BUTTON_AVATAR_BORED0: {
				gSoundManager->PlayChant(SoundManager::CHANT::BORED, 0, 0, 0, mainGame->avataricon1, 0, mainGame->avataricon2);
				break;
			}
			case BUTTON_AVATAR_BORED1: {
				gSoundManager->PlayChant(SoundManager::CHANT::BORED, 0, 0, 1, mainGame->avataricon2, 0, mainGame->avataricon1);
				break;
			}
			case BUTTON_ENTERTAUNMENT_PLOAT_CLOSE: { //story start after click ok
				mainGame->mode->NextPlot(); //ploatstep 0->1
				break;
			}
			/////kdiy/////
			case BUTTON_HAND1:
			case BUTTON_HAND2:
			case BUTTON_HAND3: {
				mainGame->wHand->setVisible(false);
				SendRPSResult(id - BUTTON_HAND1 + 1);
				break;
			}
			case BUTTON_FIRST:
			case BUTTON_SECOND: {
				mainGame->HideElement(mainGame->wFTSelect);
				CTOS_TPResult cstr;
				cstr.res = BUTTON_SECOND - id;
				DuelClient::SendPacketToServer(CTOS_TP_RESULT, cstr);
				break;
			}
			case BUTTON_REPLAY_START: {
				if(!mainGame->dInfo.isReplay)
					break;
				mainGame->btnReplayStart->setVisible(false);
				mainGame->btnReplayPause->setVisible(true);
				mainGame->btnReplayStep->setVisible(false);
				mainGame->btnReplayUndo->setVisible(false);
				ReplayMode::Pause(false, false);
				break;
			}
			case BUTTON_REPLAY_PAUSE: {
				if(!mainGame->dInfo.isReplay)
					break;
				mainGame->btnReplayStart->setVisible(true);
				mainGame->btnReplayPause->setVisible(false);
				mainGame->btnReplayStep->setVisible(true);
				mainGame->btnReplayUndo->setVisible(true);
				ReplayMode::Pause(true, false);
				break;
			}
			case BUTTON_REPLAY_STEP: {
				if(!mainGame->dInfo.isReplay)
					break;
				ReplayMode::Pause(false, true);
				break;
			}
			case BUTTON_REPLAY_EXIT: {
				if(!mainGame->dInfo.isReplay)
					break;
				ReplayMode::StopReplay();
				break;
			}
			case BUTTON_REPLAY_SWAP: {
				if(mainGame->dInfo.isReplay)
					ReplayMode::SwapField();
				else if (mainGame->dInfo.player_type == 7)
					DuelClient::SwapField();
				break;
			}
			case BUTTON_REPLAY_UNDO: {
				if(!mainGame->dInfo.isReplay)
					break;
				ReplayMode::Undo();
				break;
			}
			case BUTTON_FILE_SAVE: {
				if(mainGame->ebFileSaveName->getText()[0] == 0)
					break;
				mainGame->saveReplay = true;
				mainGame->HideElement(mainGame->wFileSave);
				mainGame->replaySignal.Set();
				break;
			}
			case BUTTON_FILE_CANCEL: {
				mainGame->saveReplay = false;
				mainGame->HideElement(mainGame->wFileSave);
				mainGame->replaySignal.Set();
				break;
			}
			case BUTTON_LEAVE_GAME: {
			    ////kdiy////////
				mainGame->StopVideo(false, true);
                mainGame->isEvent = false;
        		mainGame->bodycharacter[0] = 0;
        		mainGame->bodycharacter[1] = 0;
				for(int i = 0; i < 3; i++) {
        			mainGame->cutincharacter[0][i] = 0;
        			mainGame->cutincharacter[1][i] = 0;
				}
        		mainGame->lpcharacter[0] = 0;
        		mainGame->lpcharacter[1] = 0;
				mainGame->chantsound.stop();
                gSoundManager->soundcount.clear();
                mainGame->animecount.clear();
				if(mainGame->dInfo.isReplay) {
					ReplayMode::StopReplay();
					break;
				}
                ////kdiy////////
				if(mainGame->dInfo.isSingleMode) {
					SingleMode::singleSignal.SetNoWait(true);
					SingleMode::StopPlay(false);
					break;
				}
				if(mainGame->dInfo.player_type == 7) {
					if(mainGame->wFileSave->isVisible()) {
						mainGame->saveReplay = false;
						mainGame->HideElement(mainGame->wFileSave);
						mainGame->replaySignal.Set();
					}
					DuelClient::StopClient();
					mainGame->dInfo.isInDuel = false;
					mainGame->dInfo.isStarted = false;
					gSoundManager->StopSounds();
					mainGame->device->setEventReceiver(&mainGame->menuHandler);
					mainGame->mTopMenu->setVisible(true);
					mainGame->stTip->setVisible(false);
					mainGame->stHintMsg->setVisible(false);
					mainGame->wCardImg->setVisible(false);
					mainGame->wInfos->setVisible(false);
					mainGame->wPhase->setVisible(false);
					mainGame->btnLeaveGame->setVisible(false);
					///////kdiy///////
					for(int i = 0; i < 8; i++)
					    mainGame->CardInfo[i]->setVisible(false);
					mainGame->wBtnShowCard->setVisible(false);
                    mainGame->wLocation->setVisible(false);
					///////kdiy///////
					mainGame->btnSpectatorSwap->setVisible(false);
					mainGame->wChat->setVisible(false);
					mainGame->btnCreateHost->setEnabled(true);
					mainGame->btnJoinHost->setEnabled(true);
					mainGame->btnJoinCancel->setEnabled(true);
					if(mainGame->isHostingOnline) {
						mainGame->ShowElement(mainGame->wRoomListPlaceholder);
					} else {
						mainGame->ShowElement(mainGame->wLanWindow);
					}
					mainGame->SetMessageWindow();
				} else {
					DuelClient::SendPacketToServer(CTOS_SURRENDER);
				}
				break;
			}
			case BUTTON_CHAIN_IGNORE:
			case BUTTON_CHAIN_ALWAYS:
			case BUTTON_CHAIN_WHENAVAIL: {
				UpdateChainButtons(event.GUIEvent.Caller);
				break;
			}
			case BUTTON_CANCEL_OR_FINISH: {
				CancelOrFinish();
				break;
			}
			case BUTTON_RESTART_SINGLE: {
                ////kdiy////////
				mainGame->StopVideo(false, true);
                mainGame->isEvent = false;
        		mainGame->bodycharacter[0] = 0;
        		mainGame->bodycharacter[1] = 0;
				for(int i = 0; i < 3; i++) {
        			mainGame->cutincharacter[0][i] = 0;
        			mainGame->cutincharacter[1][i] = 0;
				}
        		mainGame->lpcharacter[0] = 0;
        		mainGame->lpcharacter[1] = 0;
				mainGame->chantsound.stop();
                gSoundManager->soundcount.clear();
                mainGame->animecount.clear();
			    ////kdiy////////
				if(mainGame->dInfo.isSingleMode)
					SingleMode::Restart();
				break;
			}
			case BUTTON_MSG_OK: {
				mainGame->HideElement(mainGame->wMessage);
				mainGame->actionSignal.Set();
				break;
			}
			case BUTTON_YES: {
				if(mainGame->dInfo.checkRematch) {
					mainGame->dInfo.checkRematch = false;
					mainGame->HideElement(mainGame->wQuery);
					CTOS_RematchResponse crr;
					crr.rematch = true;
					DuelClient::SendPacketToServer(CTOS_REMATCH_RESPONSE, crr);
					break;
				}
				switch(mainGame->dInfo.curMsg) {
				case MSG_SELECT_YESNO:
				case MSG_SELECT_EFFECTYN: {
					if(highlighting_card)
					    ////kdiy///////////
						//highlighting_card->is_highlighting = false;
						highlighting_card->is_activable = false;
						////kdiy///////////
					highlighting_card = 0;
					DuelClient::SetResponseI(1);
					mainGame->HideElement(mainGame->wQuery, true);
					break;
				}
				case MSG_SELECT_CARD:
				case MSG_SELECT_TRIBUTE: {
					mainGame->HideElement(mainGame->wQuery);
					break;
				}
				case MSG_SELECT_CHAIN: {
					mainGame->HideElement(mainGame->wQuery);
					if (!chain_forced) {
						ShowCancelOrFinishButton(1);
					}
					break;
				}
				default: {
					mainGame->HideElement(mainGame->wQuery);
					break;
				}
				}
				break;
			}
			case BUTTON_NO: {
				if(mainGame->dInfo.checkRematch) {
					mainGame->dInfo.checkRematch = false;
					mainGame->HideElement(mainGame->wQuery);
					CTOS_RematchResponse crr;
					crr.rematch = false;
					DuelClient::SendPacketToServer(CTOS_REMATCH_RESPONSE, crr);
					break;
				}
				switch(mainGame->dInfo.curMsg) {
				case MSG_SELECT_YESNO:
				case MSG_SELECT_EFFECTYN: {
					if(highlighting_card)
						////kdiy///////////
						//highlighting_card->is_highlighting = false;
						highlighting_card->is_activable = false;
						////kdiy///////////
					highlighting_card = 0;
					////kdiy///////////
        			if(mainGame->dField.attacker && mainGame->dField.attacker->is_attack) {
            			mainGame->dField.attacker->is_attack = false;
            			mainGame->dField.attacker->curRot = mainGame->dField.attacker->attRot;
        			}
        			mainGame->dField.attacker->is_attacked = false;
					////kdiy///////////
					DuelClient::SetResponseI(0);
					mainGame->HideElement(mainGame->wQuery, true);
					break;
				}
				case MSG_SELECT_CHAIN: {
					DuelClient::SetResponseI(-1);
					mainGame->HideElement(mainGame->wQuery, true);
					ShowCancelOrFinishButton(0);
					break;
				}
				case MSG_SELECT_CARD:
				case MSG_SELECT_TRIBUTE: {
					SetResponseSelectedCards();
					ShowCancelOrFinishButton(0);
					mainGame->HideElement(mainGame->wQuery, true);
					break;
				}
				default: {
					mainGame->HideElement(mainGame->wQuery);
					break;
				}
				}
				break;
			}
			case BUTTON_POS_AU: {
				DuelClient::SetResponseI(POS_FACEUP_ATTACK);
				mainGame->HideElement(mainGame->wPosSelect, true);
				break;
			}
			case BUTTON_POS_AD: {
				DuelClient::SetResponseI(POS_FACEDOWN_ATTACK);
				mainGame->HideElement(mainGame->wPosSelect, true);
				break;
			}
			case BUTTON_POS_DU: {
				DuelClient::SetResponseI(POS_FACEUP_DEFENSE);
				mainGame->HideElement(mainGame->wPosSelect, true);
				break;
			}
			case BUTTON_POS_DD: {
				DuelClient::SetResponseI(POS_FACEDOWN_DEFENSE);
				mainGame->HideElement(mainGame->wPosSelect, true);
				break;
			}
			case BUTTON_OPTION_PREV: {
				selected_option--;
				mainGame->btnOptionn->setVisible(true);
				if(selected_option == 0)
					mainGame->btnOptionp->setVisible(false);
				mainGame->stOptions->setText(gDataManager->GetDesc(select_options[selected_option], mainGame->dInfo.compat_mode).data());
				break;
			}
			case BUTTON_OPTION_NEXT: {
				selected_option++;
				mainGame->btnOptionp->setVisible(true);
				if(selected_option == select_options.size() - 1)
					mainGame->btnOptionn->setVisible(false);
				mainGame->stOptions->setText(gDataManager->GetDesc(select_options[selected_option], mainGame->dInfo.compat_mode).data());
				break;
			}
			case BUTTON_OPTION_0:
			case BUTTON_OPTION_1:
			case BUTTON_OPTION_2:
			case BUTTON_OPTION_3:
			case BUTTON_OPTION_4: {
				int step = mainGame->scrOption->isVisible() ? mainGame->scrOption->getPos() : 0;
				selected_option = id - BUTTON_OPTION_0 + step;
				SetResponseSelectedOption();
				break;
			}
			case BUTTON_OPTION_OK: {
				SetResponseSelectedOption();
				break;
			}
			case BUTTON_ANNUMBER_OK: {
				DuelClient::SetResponseI(mainGame->cbANNumber->getSelected());
				mainGame->HideElement(mainGame->wANNumber, true);
				break;
			}
			case BUTTON_ANCARD_OK: {
				int sel = mainGame->lstANCard->getSelected();
				if(sel == -1)
					break;
				DuelClient::SetResponseI(ancard[sel]);
				mainGame->HideElement(mainGame->wANCard, true);
				break;
			}
			case BUTTON_CMD_SHUFFLE: {
				mainGame->btnShuffle->setVisible(false);
				DuelClient::SetResponseI(8);
				DuelClient::SendResponse();
				break;
			}
			case BUTTON_CMD_ACTIVATE:
			case BUTTON_CMD_RESET: {
				mainGame->wCmdMenu->setVisible(false);
				ShowCancelOrFinishButton(0);
				if(!list_command) {
					int index = -1;
					select_options.clear();
					for (size_t i = 0; i < activatable_cards.size(); ++i) {
						if (activatable_cards[i] == clicked_card) {
							if(activatable_descs[i].second == EFFECT_CLIENT_MODE_RESOLVE)
								continue;
							if(activatable_descs[i].second == EFFECT_CLIENT_MODE_RESET) {
								if(id == BUTTON_CMD_ACTIVATE) continue;
							} else {
								if(id == BUTTON_CMD_RESET) continue;
							}
							select_options.push_back(activatable_descs[i].first);
							if (index == -1) index = static_cast<int>(i);
						}
					}
					if (select_options.size() == 1) {
						if (mainGame->dInfo.curMsg == MSG_SELECT_IDLECMD) {
							DuelClient::SetResponseI((index << 16) + 5);
						} else if (mainGame->dInfo.curMsg == MSG_SELECT_BATTLECMD) {
							DuelClient::SetResponseI(index << 16);
						} else {
							DuelClient::SetResponseI(index);
						}
						DuelClient::SendResponse();
					} else {
						command_card = clicked_card;
						ShowSelectOption();
					}
				} else {
					selectable_cards.clear();
					conti_selecting = false;
					switch(command_location) {
					case LOCATION_DECK: {
						for(size_t i = 0; i < deck[command_controler].size(); ++i)
							if(deck[command_controler][i]->cmdFlag & COMMAND_ACTIVATE)
								selectable_cards.push_back(deck[command_controler][i]);
						break;
					}
					case LOCATION_GRAVE: {
						for(size_t i = 0; i < grave[command_controler].size(); ++i)
							if(grave[command_controler][i]->cmdFlag & COMMAND_ACTIVATE)
								selectable_cards.push_back(grave[command_controler][i]);
						break;
					}
					case LOCATION_REMOVED: {
						for(size_t i = 0; i < remove[command_controler].size(); ++i)
							if(remove[command_controler][i]->cmdFlag & COMMAND_ACTIVATE)
								selectable_cards.push_back(remove[command_controler][i]);
						break;
					}
					case LOCATION_EXTRA: {
						for(size_t i = 0; i < extra[command_controler].size(); ++i)
							if(extra[command_controler][i]->cmdFlag & COMMAND_ACTIVATE)
								selectable_cards.push_back(extra[command_controler][i]);
						break;
					}
					case POSITION_HINT: {
						selectable_cards = conti_cards;
						std::sort(selectable_cards.begin(), selectable_cards.end());
						auto eit = std::unique(selectable_cards.begin(), selectable_cards.end());
						selectable_cards.erase(eit, selectable_cards.end());
						conti_selecting = true;
						break;
					}
					}
					if(!conti_selecting) {
						mainGame->wCardSelect->setText(gDataManager->GetSysString(566).data());
						list_command = COMMAND_ACTIVATE;
					} else {
						mainGame->wCardSelect->setText(gDataManager->GetSysString(568).data());
						list_command = COMMAND_OPERATION;
					}
					std::sort(selectable_cards.begin(), selectable_cards.end(), ClientCard::client_card_sort);
					ShowSelectCard(true, true);
				}
				break;
			}
			case BUTTON_CMD_SUMMON: {
				mainGame->wCmdMenu->setVisible(false);
				if(!clicked_card)
					break;
				for(size_t i = 0; i < summonable_cards.size(); ++i) {
					if(summonable_cards[i] == clicked_card) {
						ClearCommandFlag();
						DuelClient::SetResponseI(static_cast<uint32_t>(i) << 16);
						DuelClient::SendResponse();
						break;
					}
				}
				break;
			}
			case BUTTON_CMD_SPSUMMON: {
				mainGame->wCmdMenu->setVisible(false);
				if(!list_command) {
					if(!clicked_card)
						break;
					for(size_t i = 0; i < spsummonable_cards.size(); ++i) {
						if(spsummonable_cards[i] == clicked_card) {
							ClearCommandFlag();
							DuelClient::SetResponseI((static_cast<uint32_t>(i) << 16) + 1);
							DuelClient::SendResponse();
							break;
						}
					}
				} else {
					selectable_cards.clear();
					switch(command_location) {
					case LOCATION_DECK: {
						for(size_t i = 0; i < deck[command_controler].size(); ++i)
							if(deck[command_controler][i]->cmdFlag & COMMAND_SPSUMMON)
								selectable_cards.push_back(deck[command_controler][i]);
						break;
					}
					case LOCATION_GRAVE: {
						for(size_t i = 0; i < grave[command_controler].size(); ++i)
							if(grave[command_controler][i]->cmdFlag & COMMAND_SPSUMMON)
								selectable_cards.push_back(grave[command_controler][i]);
						break;
					}
					case LOCATION_REMOVED: {
						for (size_t i = 0; i < remove[command_controler].size(); ++i)
							if (remove[command_controler][i]->cmdFlag & COMMAND_SPSUMMON)
								selectable_cards.push_back(remove[command_controler][i]);
						break;
					}
					case LOCATION_EXTRA: {
						for(size_t i = 0; i < extra[command_controler].size(); ++i)
							if(extra[command_controler][i]->cmdFlag & COMMAND_SPSUMMON)
								selectable_cards.push_back(extra[command_controler][i]);
						break;
					}
					}
					list_command = COMMAND_SPSUMMON;
					mainGame->wCardSelect->setText(gDataManager->GetSysString(509).data());
					ShowSelectCard();
					select_ready = false;
					ShowCancelOrFinishButton(1);
				}
				break;
			}
			case BUTTON_CMD_MSET: {
				mainGame->wCmdMenu->setVisible(false);
				if(!clicked_card)
					break;
				for(size_t i = 0; i < msetable_cards.size(); ++i) {
					if(msetable_cards[i] == clicked_card) {
						DuelClient::SetResponseI((static_cast<uint32_t>(i) << 16) + 3);
						DuelClient::SendResponse();
						break;
					}
				}
				break;
			}
			case BUTTON_CMD_SSET: {
				mainGame->wCmdMenu->setVisible(false);
				if(!clicked_card)
					break;
				for(size_t i = 0; i < ssetable_cards.size(); ++i) {
					if(ssetable_cards[i] == clicked_card) {
						DuelClient::SetResponseI((static_cast<uint32_t>(i) << 16) + 4);
						DuelClient::SendResponse();
						break;
					}
				}
				break;
			}
			case BUTTON_CMD_REPOS: {
				mainGame->wCmdMenu->setVisible(false);
				if(!clicked_card)
					break;
				for(size_t i = 0; i < reposable_cards.size(); ++i) {
					if(reposable_cards[i] == clicked_card) {
						DuelClient::SetResponseI((static_cast<uint32_t>(i) << 16) + 2);
						DuelClient::SendResponse();
						break;
					}
				}
				break;
			}
			case BUTTON_CMD_ATTACK: {
				mainGame->wCmdMenu->setVisible(false);
				if(!clicked_card)
					break;
				for(size_t i = 0; i < attackable_cards.size(); ++i) {
					if(attackable_cards[i] == clicked_card) {
						DuelClient::SetResponseI((static_cast<uint32_t>(i) << 16) + 1);
						DuelClient::SendResponse();
						break;
					}
				}
				break;
			}
			case BUTTON_CMD_SHOWLIST: {
				mainGame->wCmdMenu->setVisible(false);
				selectable_cards.clear();
				switch(command_location) {
				case LOCATION_DECK: {
					for(int32_t i = (int32_t)deck[command_controler].size() - 1; i >= 0 ; --i)
						selectable_cards.push_back(deck[command_controler][i]);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1000), deck[command_controler].size()).data());
					break;
				}
				case LOCATION_MZONE: {
					ClientCard* pcard = mzone[command_controler][command_sequence];
					for(auto& _pcard : pcard->overlayed)
						selectable_cards.push_back(_pcard);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1007), pcard->overlayed.size()).data());
					break;
				}
				case LOCATION_SZONE: {
					ClientCard* pcard = szone[command_controler][command_sequence];
					for (auto& _pcard : pcard->overlayed)
						selectable_cards.push_back(_pcard);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1007), pcard->overlayed.size()).data());
					break;
				}
				case LOCATION_GRAVE: {
					for(int32_t i = (int32_t)grave[command_controler].size() - 1; i >= 0 ; --i)
						selectable_cards.push_back(grave[command_controler][i]);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1004), grave[command_controler].size()).data());
					break;
				}
				case LOCATION_REMOVED: {
					for(int32_t i = (int32_t)remove[command_controler].size() - 1; i >= 0 ; --i)
						selectable_cards.push_back(remove[command_controler][i]);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1005), remove[command_controler].size()).data());
					break;
				}
				case LOCATION_EXTRA: {
					for(int32_t i = (int32_t)extra[command_controler].size() - 1; i >= 0 ; --i)
						selectable_cards.push_back(extra[command_controler][i]);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1006), extra[command_controler].size()).data());
					break;
				}
				}
				list_command = COMMAND_LIST;
				std::sort(selectable_cards.begin(), selectable_cards.end(), ClientCard::client_card_sort);
				ShowSelectCard(true);
				break;
			}
			case BUTTON_BP: {
				if(mainGame->dInfo.curMsg == MSG_SELECT_IDLECMD) {
					DuelClient::SetResponseI(6);
					DuelClient::SendResponse();
				}
				break;
			}
			case BUTTON_M2: {
				if(mainGame->dInfo.curMsg == MSG_SELECT_BATTLECMD) {
					DuelClient::SetResponseI(2);
					DuelClient::SendResponse();
				}
				break;
			}
			case BUTTON_EP: {
				if(mainGame->dInfo.curMsg == MSG_SELECT_BATTLECMD) {
					DuelClient::SetResponseI(3);
					DuelClient::SendResponse();
				} else if(mainGame->dInfo.curMsg == MSG_SELECT_IDLECMD) {
					DuelClient::SetResponseI(7);
					DuelClient::SendResponse();
				}
				break;
			}
			case BUTTON_CARD_0:
			case BUTTON_CARD_1:
			case BUTTON_CARD_2:
			case BUTTON_CARD_3:
			case BUTTON_CARD_4: {
				if(mainGame->dInfo.isReplay)
					break;
				mainGame->stCardListTip->setVisible(false);
				switch(mainGame->dInfo.curMsg) {
				case MSG_SELECT_IDLECMD:
				case MSG_SELECT_BATTLECMD:
				case MSG_SELECT_CHAIN: {
					if(list_command == COMMAND_LIST)
						break;
					if(list_command == COMMAND_SPSUMMON) {
						command_card = selectable_cards[id - BUTTON_CARD_0 + mainGame->scrCardList->getPos() / 10];
						int index = 0;
						while(spsummonable_cards[index] != command_card) index++;
						DuelClient::SetResponseI((index << 16) + 1);
						mainGame->HideElement(mainGame->wCardSelect, true);
                        //////////kdiy/////////
                        for(int i = 0; i < 5; ++i)
                            mainGame->selectedcard[i]->setVisible(false);
                        //////////kdiy/////////
						ShowCancelOrFinishButton(0);
						break;
					}
					if(list_command == COMMAND_ACTIVATE || list_command == COMMAND_OPERATION) {
						int index = -1;
						command_card = selectable_cards[id - BUTTON_CARD_0 + mainGame->scrCardList->getPos() / 10];
						select_options.clear();
						for (size_t i = 0; i < activatable_cards.size(); ++i) {
							if (activatable_cards[i] == command_card) {
								if(activatable_descs[i].second == EFFECT_CLIENT_MODE_RESOLVE) {
									if(list_command == COMMAND_ACTIVATE) continue;
								} else {
									if(list_command == COMMAND_OPERATION) continue;
								}
								select_options.push_back(activatable_descs[i].first);
								if (index == -1) index = static_cast<int>(i);
							}
						}
						if (select_options.size() == 1) {
							if (mainGame->dInfo.curMsg == MSG_SELECT_IDLECMD) {
								DuelClient::SetResponseI((index << 16) + 5);
							} else if (mainGame->dInfo.curMsg == MSG_SELECT_BATTLECMD) {
								DuelClient::SetResponseI(index << 16);
							} else {
								DuelClient::SetResponseI(index);
							}
							mainGame->HideElement(mainGame->wCardSelect, true);
                            //////////kdiy/////////
                            for(int i = 0; i < 5; ++i)
                                mainGame->selectedcard[i]->setVisible(false);
                            //////////kdiy/////////
						} else {
							mainGame->stOptions->setText(gDataManager->GetDesc(select_options[0], mainGame->dInfo.compat_mode).data());
							selected_option = 0;
							mainGame->wCardSelect->setVisible(false);
                            //////////kdiy/////////
                            for(int i = 0; i < 5; ++i)
                                mainGame->selectedcard[i]->setVisible(false);
                            //////////kdiy/////////
							ShowSelectOption();
						}
						break;
					}
					break;
				}
				case MSG_SELECT_CARD: {
					command_card = selectable_cards[id - BUTTON_CARD_0 + mainGame->scrCardList->getPos() / 10];
					if (command_card->is_selected) {
						command_card->is_selected = false;
                        //////////kdiy/////////
                        mainGame->selectedcard[id - BUTTON_CARD_0]->setVisible(false);
                        //////////kdiy/////////
						auto it = std::find(selected_cards.begin(), selected_cards.end(), command_card);
						selected_cards.erase(it);
						if(command_card->controler)
							mainGame->stCardPos[id - BUTTON_CARD_0]->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
						else mainGame->stCardPos[id - BUTTON_CARD_0]->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
					} else {
						command_card->is_selected = true;
                        //////////kdiy/////////
                        mainGame->selectedcard[id - BUTTON_CARD_0]->setVisible(true);
                        //////////kdiy/////////
						mainGame->stCardPos[id - BUTTON_CARD_0]->setBackgroundColor(skin::DUELFIELD_CARD_SELECTED_WINDOW_BACKGROUND_VAL);
						selected_cards.push_back(command_card);
					}
					auto sel = selected_cards.size();
					if (sel >= select_max) {
						SetResponseSelectedCards();
						ShowCancelOrFinishButton(0);
						mainGame->HideElement(mainGame->wCardSelect, true);
                        //////////kdiy/////////
                        for(int i = 0; i < 5; ++i)
                            mainGame->selectedcard[i]->setVisible(false);
                        //////////kdiy/////////
					} else if (sel >= select_min) {
						select_ready = true;
						mainGame->btnSelectOK->setVisible(true);
						ShowCancelOrFinishButton(2);
					} else {
						select_ready = false;
						mainGame->btnSelectOK->setVisible(false);
						if (select_cancelable && sel == 0)
							ShowCancelOrFinishButton(1);
						else
							ShowCancelOrFinishButton(0);
					}
					break;
				}
				case MSG_SELECT_UNSELECT_CARD: {
					command_card = selectable_cards[id - BUTTON_CARD_0 + mainGame->scrCardList->getPos() / 10];
					if (command_card->is_selected) {
						command_card->is_selected = false;
                        //////////kdiy/////////
                        mainGame->selectedcard[id - BUTTON_CARD_0]->setVisible(false);
                        //////////kdiy/////////
						if(command_card->controler)
							mainGame->stCardPos[id - BUTTON_CARD_0]->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
						else mainGame->stCardPos[id - BUTTON_CARD_0]->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
					} else {
						command_card->is_selected = true;
                        //////////kdiy/////////
                        mainGame->selectedcard[id - BUTTON_CARD_0]->setVisible(true);
                        //////////kdiy/////////
						mainGame->stCardPos[id - BUTTON_CARD_0]->setBackgroundColor(skin::DUELFIELD_CARD_SELECTED_WINDOW_BACKGROUND_VAL);
					}
					selected_cards.push_back(command_card);
					if (selected_cards.size() > 0) {
						SetResponseSelectedCards();
						ShowCancelOrFinishButton(0);
						mainGame->HideElement(mainGame->wCardSelect, true);
                        //////////kdiy/////////
                        for(int i = 0; i < 5; ++i)
                            mainGame->selectedcard[i]->setVisible(false);
                        //////////kdiy/////////
					}
					break;
				}
				case MSG_SELECT_SUM: {
					command_card = selectable_cards[id - BUTTON_CARD_0 + mainGame->scrCardList->getPos() / 10];
					if (std::find(must_select_cards.begin(), must_select_cards.end(), command_card) != must_select_cards.end())
						break;
					if (command_card->is_selected) {
						command_card->is_selected = false;
                        //////////kdiy/////////
                        mainGame->selectedcard[id - BUTTON_CARD_0]->setVisible(false);
                        //////////kdiy/////////
						auto it = std::find(selected_cards.begin(), selected_cards.end(), command_card);
						selected_cards.erase(it);
					} else
						selected_cards.push_back(command_card);
					ShowSelectSum();
					break;
				}
				case MSG_SORT_CHAIN:
				case MSG_SORT_CARD: {
					int offset = mainGame->scrCardList->getPos() / 10;
					int sel_seq = id - BUTTON_CARD_0 + offset;
					if(sort_list[sel_seq]) {
						select_min--;
						int sel = sort_list[sel_seq];
						sort_list[sel_seq] = 0;
						for(uint32_t i = 0; i < select_max; ++i)
							if(sort_list[i] > sel)
								sort_list[i]--;
						for(uint32_t i = 0; i < 5; ++i) {
							if(offset + i >= select_max)
								break;
							if(sort_list[offset + i]) {
								mainGame->stCardPos[i]->setText(epro::to_wstring(sort_list[offset + i]).data());
							} else mainGame->stCardPos[i]->setText(L"");
						}
					} else {
						select_min++;
						sort_list[sel_seq] = select_min;
						mainGame->stCardPos[id - BUTTON_CARD_0]->setText(epro::to_wstring(select_min).data());
						if(select_min == select_max) {
							uint8_t respbuf[64];
							for(uint32_t i = 0; i < select_max; ++i)
								respbuf[i] = sort_list[i] - 1;
							DuelClient::SetResponseB(respbuf, select_max);
							mainGame->HideElement(mainGame->wCardSelect, true);
                            //////////kdiy/////////
                            for(int i = 0; i < 5; ++i)
                                mainGame->selectedcard[i]->setVisible(false);
                            //////////kdiy/////////
							sort_list.clear();
						}
					}
					break;
				}
				}
				break;
			}
			case BUTTON_CARD_SEL_OK: {
				// Space and Return trigger the last focused button so the hide animation can be triggered again
				// even if it's already hidden along with its child OK button
				auto HideCardSelectIfVisible = [](bool setAction = false) {
					if (mainGame->wCardSelect->isVisible())
						mainGame->HideElement(mainGame->wCardSelect, setAction);
                    //////////kdiy/////////
                    for(int i = 0; i < 5; ++i)
                        mainGame->selectedcard[i]->setVisible(false);
                    //////////kdiy/////////
				};
 				mainGame->stCardListTip->setVisible(false);
				if(mainGame->dInfo.isReplay) {
					HideCardSelectIfVisible();
					break;
				}
				if(mainGame->dInfo.curMsg == MSG_SELECT_CARD || mainGame->dInfo.curMsg == MSG_SELECT_SUM) {
					if(select_ready) {
						SetResponseSelectedCards();
						ShowCancelOrFinishButton(0);
						HideCardSelectIfVisible(true);
					}
					break;
				} else if(mainGame->dInfo.curMsg == MSG_CONFIRM_CARDS) {
					HideCardSelectIfVisible();
					mainGame->actionSignal.Set();
					break;
				} else if(mainGame->dInfo.curMsg == MSG_SELECT_UNSELECT_CARD){
					DuelClient::SetResponseI(-1);
					ShowCancelOrFinishButton(0);
					HideCardSelectIfVisible(true);
				} else {
					HideCardSelectIfVisible();
					if (mainGame->dInfo.curMsg == MSG_SELECT_CHAIN && !chain_forced)
						ShowCancelOrFinishButton(1);
					break;
				}
				break;
			}
			case BUTTON_CARD_DISP_OK: {
				mainGame->HideElement(mainGame->wCardDisplay);
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_CHECKBOX_CHANGED: {
			switch(id) {
			case CHECK_ATTRIBUTE: {
				int att = 0, filter = 0x1, count = 0;
				/////zdiy/////
				//for(int i = 0; i < 7; ++i, filter <<= 1) {
				for(int i = 0; i < 8; ++i, filter <<= 1) {
				/////zdiy/////
					if(mainGame->chkAttribute[i]->isChecked()) {
						att |= filter;
						count++;
					}
				}
				if(count == announce_count) {
					DuelClient::SetResponseI(att);
					mainGame->HideElement(mainGame->wANAttribute, true);
				}
				break;
			}
			case CHECK_RACE: {
				uint64_t rac = 0, filter = 0x1, count = 0;
				for(size_t i = 0; i < sizeofarr(mainGame->chkRace); ++i, filter <<= 1) {
					if(mainGame->chkRace[i]->isChecked()) {
						rac |= filter;
						count++;
					}
				}
				/////zdiy/////
				uint64_t z_filter = 0x10000000000000;
				for(int i = 25; i < 36;  ++i, z_filter <<= 1) {
					if(mainGame->chkRace[i]->isChecked()) {
						rac |= z_filter;
						count++;
					}
				}
				/////zdiy/////
				if(count == announce_count) {
					if(mainGame->dInfo.legacy_race_size)
						DuelClient::SetResponse<int32_t>(rac);
					else
						DuelClient::SetResponse<uint64_t>(rac);
					mainGame->HideElement(mainGame->wANRace, true);
				}
				break;
			}
			case CHECKBOX_CHAIN_BUTTONS: {
				if(mainGame->dInfo.isStarted && !mainGame->dInfo.isReplay && mainGame->dInfo.player_type < 7) {
					const bool checked = !mainGame->tabSettings.chkHideChainButtons->isChecked();
					mainGame->btnChainIgnore->setVisible(checked);
					mainGame->btnChainAlways->setVisible(checked);
					mainGame->btnChainWhenAvail->setVisible(checked);
				}
				break;
			}
			case CHECKBOX_DOTTED_LINES: {
				gGameConfig->dotted_lines = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_LISTBOX_CHANGED: {
			switch(id) {
			case LISTBOX_ANCARD: {
				int sel = mainGame->lstANCard->getSelected();
				if(sel != -1) {
					mainGame->ShowCardInfo(ancard[sel]);
				}
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_SCROLL_BAR_CHANGED: {
			switch(id) {
			case SCROLL_OPTION_SELECT: {
				int step = mainGame->scrOption->isVisible() ? mainGame->scrOption->getPos() : 0;
				for(int i = 0; i < 5; i++) {
					mainGame->btnOption[i]->setText(gDataManager->GetDesc(select_options[i + step], mainGame->dInfo.compat_mode).data());
				}

				break;
			}
			case SCROLL_CARD_SELECT: {
				int pos = mainGame->scrCardList->getPos() / 10;
				for(int i = 0; i < 5; ++i) {
					auto& curcard = selectable_cards[i + pos];
					auto& curstring = mainGame->stCardPos[i];
					// draw selectable_cards[i + pos] in btnCardSelect[i]
					curstring->enableOverrideColor(false);
					// image
					if(curcard->code)
						mainGame->imageLoading[mainGame->btnCardSelect[i]] = curcard->code;
					else if(conti_selecting)
						mainGame->imageLoading[mainGame->btnCardSelect[i]] = curcard->chain_code;
					else
						mainGame->btnCardSelect[i]->setImage(mainGame->imageManager.tCover[0]);
					mainGame->btnCardSelect[i]->setRelativePosition(mainGame->Scale(30 + i * 125, 55, 30 + 120 + i * 125, 225));
					// text
					std::wstring text = L"";
					if(sort_list.size()) {
						if(sort_list[pos + i] > 0)
							text = epro::to_wstring(sort_list[pos + i]);
					} else {
						if(conti_selecting)
							text = std::wstring{ DataManager::unknown_string };
						else if(curcard->location == LOCATION_OVERLAY) {
							text = epro::format(L"{}[{}]({})", gDataManager->FormatLocation(curcard->overlayTarget->location, curcard->overlayTarget->sequence),
								curcard->overlayTarget->sequence + 1, curcard->sequence + 1);
						} else if(curcard->location != 0) {
							text = epro::format(L"{}[{}]", gDataManager->FormatLocation(curcard->location, curcard->sequence),
								curcard->sequence + 1);
						}
					}
					curstring->setText(text.data());
                    //////////kdiy/////////
					if(curcard->is_selected)
                        mainGame->selectedcard[i]->setVisible(true);
                    else
                        mainGame->selectedcard[i]->setVisible(false);
                    //////////kdiy/////////
					// color
					if(conti_selecting)
						curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
					else if(curcard->location == LOCATION_OVERLAY) {
						if(curcard->owner != curcard->overlayTarget->controler)
							curstring->setOverrideColor(skin::DUELFIELD_CARD_SELECT_WINDOW_OVERLAY_TEXT_VAL);
						if(curcard->is_selected)
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELECTED_WINDOW_BACKGROUND_VAL);
						else if(curcard->overlayTarget->controler)
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
						else
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
					} else if(curcard->location == LOCATION_DECK || curcard->location == LOCATION_EXTRA || curcard->location == LOCATION_REMOVED) {
						if(curcard->position & POS_FACEDOWN)
							curstring->setOverrideColor(skin::DUELFIELD_CARD_SELECT_WINDOW_SET_TEXT_VAL);
						if(curcard->is_selected)
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELECTED_WINDOW_BACKGROUND_VAL);
						else if(curcard->controler)
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
						else
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
					} else {
						if(curcard->is_selected)
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELECTED_WINDOW_BACKGROUND_VAL);
						else if(curcard->controler)
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
						else
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
					}
				}
				break;
			}
			case SCROLL_CARD_DISPLAY: {
				int pos = mainGame->scrDisplayList->getPos() / 10;
				for(int i = 0; i < 5; ++i) {
					auto& curcard = display_cards[i + pos];
					auto& curstring = mainGame->stDisplayPos[i];
					// draw display_cards[i + pos] in btnCardDisplay[i]
					curstring->enableOverrideColor(false);
					if(curcard->code)
						mainGame->imageLoading[mainGame->btnCardDisplay[i]] = curcard->code;
					else
						mainGame->btnCardDisplay[i]->setImage(mainGame->imageManager.tCover[0]);
					mainGame->btnCardDisplay[i]->setRelativePosition(mainGame->Scale(30 + i * 125, 55, 30 + 120 + i * 125, 225));
					std::wstring text;
					if(curcard->location == LOCATION_OVERLAY) {
						text = epro::format(L"{}[{}]({})", gDataManager->FormatLocation(curcard->overlayTarget->location, curcard->overlayTarget->sequence),
							curcard->overlayTarget->sequence + 1, curcard->sequence + 1);
					} else {
						text = epro::format(L"{}[{}]", gDataManager->FormatLocation(curcard->location, curcard->sequence),
							curcard->sequence + 1);
					}
					curstring->setText(text.data());
					if(curcard->location == LOCATION_OVERLAY) {
						if(curcard->owner != curcard->overlayTarget->controler)
							curstring->setOverrideColor(skin::DUELFIELD_CARD_SELECT_WINDOW_OVERLAY_TEXT_VAL);
						// BackgroundColor: controller of the xyz monster
						if(curcard->overlayTarget->controler)
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
						else
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
					} else if(curcard->location == LOCATION_EXTRA || curcard->location == LOCATION_REMOVED) {
						if(curcard->position & POS_FACEDOWN)
							curstring->setOverrideColor(skin::DUELFIELD_CARD_SELECT_WINDOW_SET_TEXT_VAL);
						if(curcard->controler)
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
						else
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
					} else {
						if(curcard->controler)
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_OPPONENT_WINDOW_BACKGROUND_VAL);
						else
							curstring->setBackgroundColor(skin::DUELFIELD_CARD_SELF_WINDOW_BACKGROUND_VAL);
					}
				}
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_EDITBOX_CHANGED: {
			switch(id) {
			case EDITBOX_ANCARD: {
				UpdateDeclarableList();
				break;
			}
			case EDITBOX_FILE_NAME: {
				mainGame->ValidateName(event.GUIEvent.Caller);
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_EDITBOX_ENTER: {
			switch(id) {
			case EDITBOX_ANCARD: {
				UpdateDeclarableList(true);
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_ELEMENT_HOVERED: {
			if(id >= BUTTON_CARD_0 && id <= BUTTON_CARD_4) {
				int pos = mainGame->scrCardList->getPos() / 10;
				ClientCard* mcard = selectable_cards[id - BUTTON_CARD_0 + pos];
				if(mcard) {
					SetShowMark(mcard, true);
					ShowCardInfoInList(mcard, mainGame->btnCardSelect[id - BUTTON_CARD_0], mainGame->wCardSelect);
					if(mcard->code) {
                        ///////kdiy/////////
						//mainGame->ShowCardInfo(mcard->code);
                        mainGame->ShowCardInfo(mcard->code, false, imgType::ART, mcard);
                        ///////kdiy/////////
					} else {
						if(mcard->cover)
							mainGame->ShowCardInfo(mcard->cover, false, imgType::COVER);
						else
							mainGame->ClearCardInfo(mcard->controler);
					}
				}
			}
			if(id >= BUTTON_DISPLAY_0 && id <= BUTTON_DISPLAY_4) {
				int pos = mainGame->scrDisplayList->getPos() / 10;
				ClientCard* mcard = display_cards[id - BUTTON_DISPLAY_0 + pos];
				if(mcard) {
					SetShowMark(mcard, true);
					ShowCardInfoInList(mcard, mainGame->btnCardDisplay[id - BUTTON_DISPLAY_0], mainGame->wCardDisplay);
					if(mcard->code) {
						///////kdiy/////////
						//mainGame->ShowCardInfo(mcard->code);
                        mainGame->ShowCardInfo(mcard->code, false, imgType::ART, mcard);
                        ///////kdiy/////////
					} else {
						if(mcard->cover)
							mainGame->ShowCardInfo(mcard->cover, false, imgType::COVER);
						else
							mainGame->ClearCardInfo(mcard->controler);
					}
				}
			}
			if(id == TEXT_CARD_LIST_TIP) {
				mainGame->stCardListTip->setVisible(true);
			}
			break;
		}
		case irr::gui::EGET_ELEMENT_LEFT: {
			if(mainGame->stCardListTip->isVisible()) {
				if(id >= BUTTON_CARD_0 && id <= BUTTON_CARD_4) {
					int pos = mainGame->scrCardList->getPos() / 10;
					ClientCard* mcard = selectable_cards[id - BUTTON_CARD_0 + pos];
					if(mcard)
						SetShowMark(mcard, false);
					mainGame->stCardListTip->setVisible(false);
				}
				if(id >= BUTTON_DISPLAY_0 && id <= BUTTON_DISPLAY_4) {
					int pos = mainGame->scrDisplayList->getPos() / 10;
					ClientCard* mcard = display_cards[id - BUTTON_DISPLAY_0 + pos];
					if(mcard)
						SetShowMark(mcard, false);
					mainGame->stCardListTip->setVisible(false);
				}
			}
			if(id == TEXT_CARD_LIST_TIP) {
				mainGame->stCardListTip->setVisible(false);
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case irr::EET_MOUSE_INPUT_EVENT: {
		bool isroot = [&event,root=mainGame->env->getRootGUIElement()] {
			const auto elem = root->getElementFromPoint({ event.MouseInput.X, event.MouseInput.Y });
			return elem == root || elem == mainGame->wPhase;
		}();
		switch(event.MouseInput.Event) {
		case irr::EMIE_LMOUSE_DOUBLE_CLICK: {
			if(mainGame->dInfo.isReplay)
				break;
			if(mainGame->dInfo.player_type == 7)
				break;
			if(!mainGame->dInfo.isInDuel)
				break;
			if(mainGame->wCardDisplay->isVisible())
				break;
			/////kdiy/////
			if(mainGame->isAnime) {
				mainGame->StopVideo(false, true);
			    break;
			}
            if(mainGame->isEvent) {
				mainGame->isEvent = false;
				mainGame->cv->notify_one();
				mainGame->chantsound.stop();
            }
			if(mainGame->mode->isMode && mainGame->mode->isPlot) {
				if(mainGame->mode->plotStep < 1) break;
                if(!mainGame->dInfo.isStarted)
				    mainGame->mode->isStartDuel = true;
				mainGame->mode->NextPlot(2); //skip continuous ploat
				break;
			}
			/////kdiy/////
			irr::core::vector2di mousepos(event.MouseInput.X, event.MouseInput.Y);
			irr::gui::IGUIElement* root = mainGame->env->getRootGUIElement();
			if(root->getElementFromPoint(mousepos) != root)
				break;
			irr::core::vector2di pos = mainGame->Resize(event.MouseInput.X, event.MouseInput.Y, true);
			if(pos.X < 300)
				break;
			GetHoverField(mousepos);
			if((hovered_location & (LOCATION_DECK | LOCATION_GRAVE | LOCATION_REMOVED | LOCATION_EXTRA)) == 0)
				break;
			if(hovered_location == LOCATION_DECK && !mainGame->dInfo.isSingleMode)
				break;
			ShowPileDisplayCards(hovered_location, hovered_controler);
			break;
		}
		case irr::EMIE_LMOUSE_LEFT_UP: {
			if(!mainGame->dInfo.isInDuel)
				break;
			/////kdiy/////
			if(mainGame->mode->isMode && mainGame->mode->isPlot){
				if(mainGame->mode->plotStep < 1) break;
                if(mainGame->dInfo.isStarted) {
					mainGame->isEvent = false;
                    mainGame->cv->notify_one();
					mainGame->chantsound.stop();
                } else if(!(mainGame->dInfo.isStarted && mainGame->mode->isStartEvent))
                    mainGame->mode->NextPlot();
				break;
			}
			/////kdiy/////
			hovered_location = 0;
			irr::core::vector2di pos = mainGame->Resize(event.MouseInput.X, event.MouseInput.Y, true);
			irr::core::vector2di mousepos(event.MouseInput.X, event.MouseInput.Y);
			irr::s32 x = pos.X;
			irr::s32 y = pos.Y;
			if(mainGame->always_chain) {
				mainGame->always_chain = false;
				mainGame->ignore_chain = event.MouseInput.isRightPressed();
				mainGame->chain_when_avail = false;
				UpdateChainButtons();
			}
			if(x < 300)
				break;
			if(mainGame->wCmdMenu->isVisible() && !mainGame->wCmdMenu->getRelativePosition().isPointInside(mousepos))
				mainGame->wCmdMenu->setVisible(false);
			if(panel && panel->isVisible())
				break;
			GetHoverField(mousepos);
			if(hovered_location & 0xe)
				clicked_card = GetCard(hovered_controler, hovered_location, hovered_sequence);
			else clicked_card = 0;
			if(mainGame->dInfo.isReplay) {
				if(mainGame->wCardSelect->isVisible())
					break;
				selectable_cards.clear();
				switch(hovered_location) {
				case LOCATION_DECK: {
					if(deck[hovered_controler].size() == 0)
						break;
					for(int32_t i = (int32_t)deck[hovered_controler].size() - 1; i >= 0 ; --i)
						selectable_cards.push_back(deck[hovered_controler][i]);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1000), deck[hovered_controler].size()).data());
					break;
				}
				case LOCATION_MZONE: {
					if(!clicked_card || clicked_card->overlayed.size() == 0)
						break;
					for(auto& pcard : clicked_card->overlayed)
						selectable_cards.push_back(pcard);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1007), clicked_card->overlayed.size()).data());
					break;
				}
				//////kdiy///////////
				case LOCATION_SZONE: {
					if(!clicked_card || clicked_card->overlayed.size() == 0)
						break;
					for(auto& pcard : clicked_card->overlayed)
						selectable_cards.push_back(pcard);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1008), clicked_card->overlayed.size()).c_str());
					break;
				}
				//////kdiy///////////
				case LOCATION_GRAVE: {
					if(grave[hovered_controler].size() == 0)
						break;
					for(int32_t i = (int32_t)grave[hovered_controler].size() - 1; i >= 0 ; --i)
						selectable_cards.push_back(grave[hovered_controler][i]);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1004), grave[hovered_controler].size()).data());
					break;
				}
				case LOCATION_REMOVED: {
					if(remove[hovered_controler].size() == 0)
						break;
					for(int32_t i = (int32_t)remove[hovered_controler].size() - 1; i >= 0 ; --i)
						selectable_cards.push_back(remove[hovered_controler][i]);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1005), remove[hovered_controler].size()).data());
					break;
				}
				case LOCATION_EXTRA: {
					if(extra[hovered_controler].size() == 0)
						break;
					for(int32_t i = (int32_t)extra[hovered_controler].size() - 1; i >= 0 ; --i)
						selectable_cards.push_back(extra[hovered_controler][i]);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1006), extra[hovered_controler].size()).data());
					break;
				}
				}
				if(selectable_cards.size())
					ShowSelectCard(true);
				break;
			}
			if(mainGame->dInfo.player_type == 7) {
				if(mainGame->wCardSelect->isVisible())
					break;
				selectable_cards.clear();
				switch(hovered_location) {
				case LOCATION_MZONE: {
					if(!clicked_card || clicked_card->overlayed.size() == 0)
						break;
					for(int32_t i = 0; i < (int32_t)clicked_card->overlayed.size(); ++i)
						selectable_cards.push_back(clicked_card->overlayed[i]);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1007), clicked_card->overlayed.size()).data());
					break;
				}
				////////kdiy/////////////
				case LOCATION_SZONE: {
					if(!clicked_card || clicked_card->overlayed.size() == 0)
						break;
					for(int32_t i = 0; i < (int32_t)clicked_card->overlayed.size(); ++i)
						selectable_cards.push_back(clicked_card->overlayed[i]);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1008), clicked_card->overlayed.size()).c_str());
					break;
				}
				////////kdiy/////////////
				case LOCATION_GRAVE: {
					if(grave[hovered_controler].size() == 0)
						break;
					for(int32_t i = (int32_t)grave[hovered_controler].size() - 1; i >= 0 ; --i)
						selectable_cards.push_back(grave[hovered_controler][i]);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1004), grave[hovered_controler].size()).data());
					break;
				}
				case LOCATION_REMOVED: {
					if (remove[hovered_controler].size() == 0)
						break;
					for (int32_t i = (int32_t)remove[hovered_controler].size() - 1; i >= 0; --i)
						selectable_cards.push_back(remove[hovered_controler][i]);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1005), remove[hovered_controler].size()).data());
					break;
				}
				case LOCATION_EXTRA: {
					if (extra[hovered_controler].size() == 0)
						break;
					for (int32_t i = (int32_t)extra[hovered_controler].size() - 1; i >= 0; --i)
						selectable_cards.push_back(extra[hovered_controler][i]);
					mainGame->wCardSelect->setText(epro::format(L"{}({})", gDataManager->GetSysString(1006), extra[hovered_controler].size()).data());
					break;
				}
				}
				if(selectable_cards.size())
					ShowSelectCard(true);
				break;
			}
			command_controler = hovered_controler;
			command_location = hovered_location;
			command_sequence = hovered_sequence;
			switch(mainGame->dInfo.curMsg) {
			case MSG_WAITING: {
				switch(hovered_location) {
				case LOCATION_MZONE:
				case LOCATION_SZONE: {
					if(!clicked_card || clicked_card->overlayed.size() == 0)
						break;
					ShowMenu(COMMAND_LIST, x, y);
					break;
				}
				case LOCATION_GRAVE: {
					if(grave[hovered_controler].size() == 0)
						break;
					ShowMenu(COMMAND_LIST, x, y);
					break;
				}
				case LOCATION_REMOVED: {
					if(remove[hovered_controler].size() == 0)
						break;
					ShowMenu(COMMAND_LIST, x, y);
					break;
				}
				case LOCATION_EXTRA: {
					if(extra[hovered_controler].size() == 0)
						break;
					ShowMenu(COMMAND_LIST, x, y);
					break;
				}
				}
				break;
			}
			case MSG_SELECT_BATTLECMD:
			case MSG_SELECT_IDLECMD:
			case MSG_SELECT_CHAIN: {
				switch(hovered_location) {
				case LOCATION_DECK: {
					int command_flag = 0;
					if(deck[hovered_controler].size() == 0)
						break;
					for(size_t i = 0; i < deck[hovered_controler].size(); ++i)
						command_flag |= deck[hovered_controler][i]->cmdFlag;
					if(mainGame->dInfo.isSingleMode)
						command_flag |= COMMAND_LIST;
					list_command = 1;
					ShowMenu(command_flag, x, y);
					break;
				}
				case LOCATION_HAND:
				case LOCATION_MZONE:
				case LOCATION_SZONE: {
					if(!clicked_card)
						break;
					int command_flag = clicked_card->cmdFlag;
					if(clicked_card->overlayed.size())
						command_flag |= COMMAND_LIST;
					list_command = 0;
					ShowMenu(command_flag, x, y);
					break;
				}
				case LOCATION_GRAVE: {
					int command_flag = 0;
					if(grave[hovered_controler].size() == 0)
						break;
					for(size_t i = 0; i < grave[hovered_controler].size(); ++i)
						command_flag |= grave[hovered_controler][i]->cmdFlag;
					command_flag |= COMMAND_LIST;
					list_command = 1;
					ShowMenu(command_flag, x, y);
					break;
				}
				case LOCATION_REMOVED: {
					int command_flag = 0;
					if(remove[hovered_controler].size() == 0)
						break;
					for(size_t i = 0; i < remove[hovered_controler].size(); ++i)
						command_flag |= remove[hovered_controler][i]->cmdFlag;
					command_flag |= COMMAND_LIST;
					list_command = 1;
					ShowMenu(command_flag, x, y);
					break;
				}
				case LOCATION_EXTRA: {
					int command_flag = 0;
					if(extra[hovered_controler].size() == 0)
						break;
					for(size_t i = 0; i < extra[hovered_controler].size(); ++i)
						command_flag |= extra[hovered_controler][i]->cmdFlag;
					command_flag |= COMMAND_LIST;
					list_command = 1;
					ShowMenu(command_flag, x, y);
					break;
				}
				case POSITION_HINT: {
					int command_flag = 0;
					if(conti_cards.size() == 0)
						break;
					command_flag |= COMMAND_OPERATION;
					list_command = 1;
					ShowMenu(command_flag, x, y);
					break;
				}
				}
				break;
			}
			case MSG_SELECT_PLACE:
			case MSG_SELECT_DISFIELD: {
				if (!(hovered_location & LOCATION_ONFIELD))
					break;
				uint32_t flag = 1 << (hovered_sequence + (hovered_controler << 4) + ((hovered_location == LOCATION_MZONE) ? 0 : 8));
				if(hovered_location == LOCATION_MZONE && (hovered_sequence == 5 || hovered_sequence == 6)) {
					if((flag & selectable_field) == 0 && selectable_field & 0x600000)
						flag = 1 << ((11 - hovered_sequence) + (1 << 4));
				}
				if (flag & selectable_field) {
					if (flag & selected_field) {
						selected_field &= ~flag;
						select_min++;
					} else {
						selected_field |= flag;
						select_min--;
						if (select_min == 0) {
							uint8_t respbuf[80];
							int filter = 1;
							int p = 0;
							for (int i = 0; i < 7; ++i, filter <<= 1) {
								if (selected_field & filter) {
									respbuf[p] = mainGame->LocalPlayer(0);
									respbuf[p + 1] = LOCATION_MZONE;
									respbuf[p + 2] = i;
									p += 3;
								}
							}
							filter = 0x100;
							for (int i = 0; i < 8; ++i, filter <<= 1) {
								if (selected_field & filter) {
									respbuf[p] = mainGame->LocalPlayer(0);
									respbuf[p + 1] = LOCATION_SZONE;
									respbuf[p + 2] = i;
									p += 3;
								}
							}
							filter = 0x10000;
							for (int i = 0; i < 7; ++i, filter <<= 1) {
								if (selected_field & filter) {
									respbuf[p] = mainGame->LocalPlayer(1);
									respbuf[p + 1] = LOCATION_MZONE;
									respbuf[p + 2] = i;
									p += 3;
								}
							}
							filter = 0x1000000;
							for (int i = 0; i < 8; ++i, filter <<= 1) {
								if (selected_field & filter) {
									respbuf[p] = mainGame->LocalPlayer(1);
									respbuf[p + 1] = LOCATION_SZONE;
									respbuf[p + 2] = i;
									p += 3;
								}
							}
							selectable_field = 0;
							selected_field = 0;
							DuelClient::SetResponseB(respbuf, p);
							DuelClient::SendResponse();
						}
					}
				}
				break;
			}
			case MSG_SELECT_CARD:
			case MSG_SELECT_TRIBUTE: {
				if (!(hovered_location & 0xe) || !clicked_card || !clicked_card->is_selectable)
					break;
				if (clicked_card->is_selected) {
					clicked_card->is_selected = false;
					auto it = std::find(selected_cards.begin(), selected_cards.end(), clicked_card);
					selected_cards.erase(it);
				} else {
					clicked_card->is_selected = true;
					selected_cards.push_back(clicked_card);
				}
				uint32_t min = static_cast<uint32_t>(selected_cards.size()), max = 0;
				if (mainGame->dInfo.curMsg == MSG_SELECT_CARD) {
					max = static_cast<uint32_t>(selected_cards.size());
				} else {
					for(size_t i = 0; i < selected_cards.size(); ++i)
						max += selected_cards[i]->opParam;
				}
				if (min >= select_max) {
					SetResponseSelectedCards();
					ShowCancelOrFinishButton(0);
					DuelClient::SendResponse();
				} else if (max >= select_min) {
					if(selected_cards.size() == selectable_cards.size()) {
						SetResponseSelectedCards();
						ShowCancelOrFinishButton(0);
						DuelClient::SendResponse();
					} else {
						select_ready = true;
						ShowCancelOrFinishButton(2);
					}
				} else {
					select_ready = false;
					if (select_cancelable && min == 0)
						ShowCancelOrFinishButton(1);
					else
						ShowCancelOrFinishButton(0);
				}
				break;
			}
			case MSG_SELECT_UNSELECT_CARD: {
				if (!(hovered_location & 0xe) || !clicked_card || !clicked_card->is_selectable)
					break;
				if (clicked_card->is_selected) {
					clicked_card->is_selected = false;
				} else {
					clicked_card->is_selected = true;
				}
				selected_cards.push_back(clicked_card);
				if (selected_cards.size() > 0) {
					ShowCancelOrFinishButton(0);
					SetResponseSelectedCards();
					DuelClient::SendResponse();
				}
				break;
			}
			case MSG_SELECT_COUNTER: {
				if (!clicked_card || !clicked_card->is_selectable)
					break;
				clicked_card->opParam--;
				if ((clicked_card->opParam & 0xffff) == 0)
					clicked_card->is_selectable = false;
				select_counter_count--;
				if (select_counter_count == 0) {
					uint16_t respbuf[32];
					for(size_t i = 0; i < selectable_cards.size(); ++i)
						respbuf[i] = (selectable_cards[i]->opParam >> 16) - (selectable_cards[i]->opParam & 0xffff);
					mainGame->stHintMsg->setVisible(false);
					ClearSelect();
					DuelClient::SetResponseB(respbuf, selectable_cards.size() * 2);
					DuelClient::SendResponse();
				} else {
					mainGame->stHintMsg->setText(epro::sprintf(gDataManager->GetSysString(204), select_counter_count, gDataManager->GetCounterName(select_counter_type)).data());
				}
				break;
			}
			case MSG_SELECT_SUM: {
				if (!clicked_card || !clicked_card->is_selectable || (std::find(must_select_cards.begin(), must_select_cards.end(), clicked_card) != must_select_cards.end()))
					break;
				if (clicked_card->is_selected) {
					clicked_card->is_selected = false;
					auto it = std::find(selected_cards.begin(), selected_cards.end(), clicked_card);
					selected_cards.erase(it);
				} else
					selected_cards.push_back(clicked_card);
				ShowSelectSum();
				break;
			}
			}
			break;
		}
		case irr::EMIE_RMOUSE_LEFT_UP: {
			if(mainGame->dInfo.isReplay)
				break;
			/////kdiy/////
			if(mainGame->mode->isMode && mainGame->mode->isPlot)
				break;
			/////kdiy/////
			auto x = event.MouseInput.X;
			auto y = event.MouseInput.Y;
			irr::core::vector2di pos(x, y);
			if(mainGame->dInfo.isInDuel && mainGame->ignore_chain) {
				mainGame->ignore_chain = false;
				mainGame->always_chain = event.MouseInput.isLeftPressed();
				mainGame->chain_when_avail = false;
				UpdateChainButtons();
			}
			mainGame->wCmdMenu->setVisible(false);
			if(mainGame->fadingList.size())
				break;
			CancelOrFinish();
			break;
		}
		case irr::EMIE_MOUSE_MOVED: {
			if(!mainGame->dInfo.isInDuel)
				break;
			/////kdiy/////
			if(mainGame->mode->isMode && mainGame->mode->isPlot)
			    break;
			/////kdiy/////
			bool should_show_tip = false;
			irr::core::vector2di pos = mainGame->Resize(event.MouseInput.X, event.MouseInput.Y, true);
			irr::core::vector2di mousepos(event.MouseInput.X, event.MouseInput.Y);
			irr::s32 x = pos.X;
			irr::s32 y = pos.Y;
			if(x < 300) {
				mainGame->stTip->setVisible(should_show_tip);
				/////kdiy/////
				if(x < 160)
				/////kdiy/////
				break;
			}
			hovered_location = 0;
			ClientCard* mcard = 0;
			uint8_t mplayer = 2;
			if(!panel || !panel->isVisible() || !panel->getRelativePosition().isPointInside(mousepos)) {
				GetHoverField(mousepos);
				if(hovered_location & 0xe)
					mcard = GetCard(hovered_controler, hovered_location, hovered_sequence);
				else if(hovered_location == LOCATION_GRAVE) {
					if(grave[hovered_controler].size())
						mcard = grave[hovered_controler].back();
				} else if(hovered_location == LOCATION_REMOVED) {
					if(remove[hovered_controler].size()) {
						mcard = remove[hovered_controler].back();
					}
				} else if(hovered_location == LOCATION_EXTRA) {
					if(extra[hovered_controler].size()) {
						mcard = extra[hovered_controler].back();
					}
				} else if(hovered_location == LOCATION_DECK) {
					if(deck[hovered_controler].size())
						mcard = deck[hovered_controler].back();
				} else if(hovered_location == LOCATION_SKILL) {
					mcard = skills[hovered_controler];
				} else {
					const auto& self = mainGame->dInfo.isTeam1 ? mainGame->dInfo.selfnames : mainGame->dInfo.opponames;
					const auto& oppo = mainGame->dInfo.isTeam1 ? mainGame->dInfo.opponames : mainGame->dInfo.selfnames;
					/////kdiy/////////
					//if(mainGame->Resize(327, 8, 630, 51 + static_cast<irr::s32>(23 * (self.size() - 1))).isPointInside(mousepos))
					if(mainGame->Resize(161, 553, 350, 640 + static_cast<irr::s32>(23 * (self.size() - 1))).isPointInside(mousepos))
					/////kdiy/////////
						mplayer = 0;
					/////kdiy/////////
					//else if(mainGame->Resize(689, 8, 991, 51 + static_cast<irr::s32>(23 * (oppo.size() - 1))).isPointInside(mousepos))
					else if(mainGame->Resize(691, 48, 900, 135 + static_cast<irr::s32>(23 * (oppo.size() - 1))).isPointInside(mousepos))
					/////kdiy/////////
						mplayer = 1;
				}
			}
			if(hovered_location == LOCATION_HAND && (mainGame->dInfo.is_shuffling || mainGame->dInfo.curMsg == MSG_SHUFFLE_HAND))
				mcard = 0;
			if(mcard == 0 && mplayer > 1)
				should_show_tip = false;
			else if(mcard == hovered_card && mplayer == hovered_player) {
				if(mainGame->stTip->isVisible()) {
					should_show_tip = true;
					irr::core::recti tpos = mainGame->stTip->getRelativePosition();
					mainGame->stTip->setRelativePosition(irr::core::vector2di(mousepos.X - tpos.getWidth() - mainGame->Scale(10), mcard ? mousepos.Y - tpos.getHeight() - mainGame->Scale(10) : y + mainGame->Scale(10)));
				}
			}
			if(mcard != hovered_card) {
				if(hovered_card) {
					if(hovered_card->location == LOCATION_HAND && !mainGame->dInfo.is_shuffling && mainGame->dInfo.curMsg != MSG_SHUFFLE_HAND) {
						hovered_card->is_hovered = false;
						MoveCard(hovered_card, 5);
						if(hovered_controler == 0)
							mainGame->hideChat = false;
					}
					SetShowMark(hovered_card, false);
				}
				if(mcard) {
					if(mcard != clicked_card)
						mainGame->wCmdMenu->setVisible(false);
					if(hovered_location == LOCATION_HAND) {
						mcard->is_hovered = true;
						MoveCard(mcard, 5);
						if(hovered_controler == 0)
							mainGame->hideChat = true;
					}
					SetShowMark(mcard, true);
                    ///////kdiy/////////
					//if(mcard->code) {
						//mainGame->ShowCardInfo(mcard->code);
                    if(mcard->code && (mcard->type || mainGame->dInfo.isSingleMode)) {
						mainGame->ShowCardInfo(mcard->code, false, imgType::ART, mcard);
                    ///////kdiy/////////
						if(mcard->location & (LOCATION_HAND | LOCATION_MZONE | LOCATION_SZONE | LOCATION_SKILL)) {
							///////kdiy/////////
							//std::wstring str(gDataManager->GetName(mcard->code));
							//if(mcard->alias != 0 && !CardDataC::IsInArtworkOffsetRange(mcard) && str != gDataManager->GetName(mcard->alias)) {
								//str.append(epro::format(L"\n({})", gDataManager->GetName(mcard->alias)));
							std::wstring str(gDataManager->GetName(mcard));
							if(mcard->alias != 0 && !CardDataC::IsInArtworkOffsetRange(mcard) && gDataManager->GetName(mcard, true).substr(0,gDataManager->GetName(mcard->alias, true).length()) != gDataManager->GetName(mcard->alias, true) && !mcard->is_real) {
								str.append(epro::format(L"\n({})", gDataManager->GetName(mcard->alias)));
							///////kdiy/////////
							}
							if(mcard->type & TYPE_MONSTER) {
								//////kdiy/////
								//if (mcard->type & TYPE_LINK) {
								// 	str.append(epro::format(L"\n{}/Link {}\n{}/{}", mcard->atkstring, mcard->link, gDataManager->FormatRace(mcard->race),
								// 		gDataManager->FormatAttribute(mcard->attribute)));
								// } else {
								// 	str.append(epro::format(L"\n{}/{}", mcard->atkstring, mcard->defstring));
								// 	if(mcard->rank && mcard->level)
								// 	    str.append(epro::format(L"\n\u2606{}/\u2605{} {}/{}", mcard->level, mcard->rank, gDataManager->FormatRace(mcard->race), gDataManager->FormatAttribute(mcard->attribute)));
								// 	else {
								// 		str.append(epro::format(L"\n{}{} {}/{}", (mcard->level ? L"\u2605" : L"\u2606"), (mcard->level ? mcard->level : mcard->rank), gDataManager->FormatRace(mcard->race), gDataManager->FormatAttribute(mcard->attribute)));
								// 	}
								//has lv, rk, lk
								if ((mcard->level != 0 || (mcard->type & (TYPE_FUSION | TYPE_SYNCHRO | TYPE_RITUAL))) && (mcard->rank != 0 || (mcard->type & TYPE_XYZ)) && (mcard->link != 0 || (mcard->type & TYPE_LINK))) {
									str.append(epro::format(L"\n\u2605{}/\u2606{} {}/{}", mcard->level, mcard->rank, gDataManager->FormatRace(mcard->race), gDataManager->FormatAttribute(mcard->attribute)));
									str.append(epro::format(L"\n{}/Link {}", mcard->atkstring, mcard->link));
								//has lk, rk/lv
								} else if ((mcard->link != 0 || (mcard->type & TYPE_LINK)) && ((mcard->rank != 0 || (mcard->type & TYPE_XYZ)) || (mcard->level != 0 || (mcard->type & (TYPE_FUSION | TYPE_SYNCHRO | TYPE_RITUAL))))) {
									str.append(epro::format(L"\n{}{} {}/{}", (mcard->type & TYPE_XYZ) ? L"\u2606" : L"\u2605", (mcard->type & TYPE_XYZ) ? mcard->rank : mcard->level, gDataManager->FormatRace(mcard->race), gDataManager->FormatAttribute(mcard->attribute)));
									str.append(epro::format(L"\n{}/Link {}", mcard->atkstring, mcard->link));
								//has lv, rk
								} else if ((mcard->level != 0 || (mcard->type & (TYPE_FUSION | TYPE_SYNCHRO | TYPE_RITUAL))) && (mcard->rank != 0 || (mcard->type & TYPE_XYZ))) {
									str.append(epro::format(L"\n\u2605{}/\u2606{} {}/{}", mcard->level, mcard->rank, gDataManager->FormatRace(mcard->race), gDataManager->FormatAttribute(mcard->attribute)));
									str.append(epro::format(L"\n{}/{}", mcard->atkstring, mcard->defstring));
								//has rk/lv
								} else if ((mcard->rank != 0 || (mcard->type & TYPE_XYZ)) || (mcard->level != 0 || !(mcard->type & (TYPE_XYZ | TYPE_LINK)))) {
									str.append(epro::format(L"\n{}{} {}/{}", (mcard->type & TYPE_XYZ)  ? L"\u2606" : L"\u2605", (mcard->type & TYPE_XYZ) ? mcard->rank : mcard->level, gDataManager->FormatRace(mcard->race), gDataManager->FormatAttribute(mcard->attribute)));
									str.append(epro::format(L"\n{}/{}", mcard->atkstring, mcard->defstring));
								//has lk
								} else if (mcard->link != 0 || (mcard->type & TYPE_LINK))
								    str.append(epro::format(L"\n{}/Link {}\n{}/{}", mcard->atkstring, mcard->link, gDataManager->FormatRace(mcard->race), gDataManager->FormatAttribute(mcard->attribute)));
								///////kdiy/////////
							}
							if((mcard->location & (LOCATION_HAND | LOCATION_SZONE)) != 0 && (mcard->type & TYPE_PENDULUM)) {
								str.append(epro::format(L"\n{}/{}", mcard->lscale, mcard->rscale));
							}
							for(auto ctit = mcard->counters.begin(); ctit != mcard->counters.end(); ++ctit) {
								str.append(epro::format(L"\n[{}]: {}", gDataManager->GetCounterName(ctit->first), ctit->second));
							}
							if(mcard->cHint && mcard->chValue && (mcard->location & LOCATION_ONFIELD)) {
								if(mcard->cHint == CHINT_TURN)
									str.append(epro::format(L"\n{}{}", gDataManager->GetSysString(211), mcard->chValue));
								else if(mcard->cHint == CHINT_CARD)
									str.append(epro::format(L"\n{}{}", gDataManager->GetSysString(212), gDataManager->GetName(mcard->chValue)));
								else if(mcard->cHint == CHINT_RACE)
								    str.append(epro::format(L"\n{}{}", gDataManager->GetSysString(213), gDataManager->FormatRace(mcard->chValue)));
								else if(mcard->cHint == CHINT_ATTRIBUTE)
									str.append(epro::format(L"\n{}{}", gDataManager->GetSysString(214), gDataManager->FormatAttribute(mcard->chValue)));
								else if(mcard->cHint == CHINT_NUMBER)
									str.append(epro::format(L"\n{}{}", gDataManager->GetSysString(215), mcard->chValue));
							}
							for(auto iter = mcard->desc_hints.begin(); iter != mcard->desc_hints.end(); ++iter) {
								str.append(epro::format(L"\n*{}", gDataManager->GetDesc(iter->first, mainGame->dInfo.compat_mode)));
							}
							should_show_tip = true;
							auto dtip = mainGame->textFont->getDimensionustring(str) + mainGame->Scale(irr::core::dimension2d<uint32_t>(10, 10));
							mainGame->stTip->setRelativePosition(irr::core::recti(mousepos.X - mainGame->Scale(10) - dtip.Width, mousepos.Y - mainGame->Scale(10) - dtip.Height, mousepos.X - mainGame->Scale(10), mousepos.Y - mainGame->Scale(10)));
							mainGame->stTip->setText(str.data());
						}
					} else {
						should_show_tip = false;
						if(mcard->cover)
							mainGame->ShowCardInfo(mcard->cover, false, imgType::COVER);
						else
							mainGame->ClearCardInfo(mcard->controler);
					}
				}
				hovered_card = mcard;
			}
			if(mplayer != hovered_player) {
				if(mplayer < 2) {
					/////kdiy/////
					// std::wstring player_name;
					// auto& self = mainGame->dInfo.isTeam1 ? mainGame->dInfo.selfnames : mainGame->dInfo.opponames;
					// auto& oppo = mainGame->dInfo.isTeam1 ? mainGame->dInfo.opponames : mainGame->dInfo.selfnames;
					// if (mplayer == 0)
					// 	player_name = self[mainGame->dInfo.current_player[mplayer]];
					// else
					// 	player_name = oppo[mainGame->dInfo.current_player[mplayer]];
					// for(const auto& hint : player_desc_hints[mplayer]) {
					// 	player_name.append(epro::format(L"\n*{}", gDataManager->GetDesc(hint.first, mainGame->dInfo.compat_mode)));
					// }
					// should_show_tip = true;
					// auto dtip = mainGame->textFont->getDimensionustring(player_name) + mainGame->Scale(irr::core::dimension2d<uint32_t>(10, 10));
					// mainGame->stTip->setRelativePosition(irr::core::recti(mousepos.X - mainGame->Scale(10) - dtip.Width, mousepos.Y + mainGame->Scale(10), mousepos.X - mainGame->Scale(10), mousepos.Y + mainGame->Scale(10) + dtip.Height));
					// mainGame->stTip->setText(player_name.data());
					mainGame->ShowPlayerInfo(mplayer);
					/////kdiy/////
				}
				hovered_player = mplayer;
			}
			mainGame->stTip->setVisible(should_show_tip);
			break;
		}
		case irr::EMIE_MOUSE_WHEEL: {
			break;
		}
		case irr::EMIE_LMOUSE_PRESSED_DOWN: {
			if(!mainGame->dInfo.isInDuel || !isroot || event.MouseInput.X <= 300)
				break;
			/////kdiy/////
			if(mainGame->mode->isMode && mainGame->mode->isPlot)
				break;
			/////kdiy/////
			mainGame->always_chain = true;
			mainGame->ignore_chain = false;
			mainGame->chain_when_avail = false;
			UpdateChainButtons();
			break;
		}
		case irr::EMIE_RMOUSE_PRESSED_DOWN: {
			if(!mainGame->dInfo.isInDuel || !isroot || event.MouseInput.X <= 300)
				break;
			/////kdiy/////
			if(mainGame->mode->isMode && mainGame->mode->isPlot)
				break;
			/////kdiy/////
			mainGame->ignore_chain = true;
			mainGame->always_chain = false;
			mainGame->chain_when_avail = false;
			UpdateChainButtons();
			break;
		}
		default:
			break;
		}
		break;
	}
	case irr::EET_KEY_INPUT_EVENT: {
		switch(event.KeyInput.Key) {
		case irr::KEY_KEY_A: {
			if(!mainGame->HasFocus(irr::gui::EGUIET_EDIT_BOX)) {
				mainGame->always_chain = event.KeyInput.PressedDown;
				mainGame->ignore_chain = false;
				mainGame->chain_when_avail = false;
				UpdateChainButtons();
			}
			break;
		}
		case irr::KEY_KEY_S: {
			if(!mainGame->HasFocus(irr::gui::EGUIET_EDIT_BOX)) {
				mainGame->ignore_chain = event.KeyInput.PressedDown;
				mainGame->always_chain = false;
				mainGame->chain_when_avail = false;
				UpdateChainButtons();
			}
			break;
		}
		case irr::KEY_KEY_D: {
			if(!mainGame->HasFocus(irr::gui::EGUIET_EDIT_BOX)) {
				mainGame->chain_when_avail = event.KeyInput.PressedDown;
				mainGame->always_chain = false;
				mainGame->ignore_chain = false;
				UpdateChainButtons();
			}
			break;
		}
		case irr::KEY_F1:
		case irr::KEY_F2:
		case irr::KEY_F3:
		case irr::KEY_F4:
		case irr::KEY_F5:
		case irr::KEY_F6:
		case irr::KEY_F7:
		case irr::KEY_F8: {
			if(!event.KeyInput.PressedDown && !mainGame->dInfo.isReplay && mainGame->dInfo.player_type != 7 && mainGame->dInfo.isInDuel
					&& !mainGame->wCardDisplay->isVisible() && !mainGame->HasFocus(irr::gui::EGUIET_EDIT_BOX)) {
				switch(event.KeyInput.Key) {
					case irr::KEY_F1:
						ShowPileDisplayCards(LOCATION_GRAVE, 0);
						break;
					case irr::KEY_F2:
						ShowPileDisplayCards(LOCATION_REMOVED, 0);
						break;
					case irr::KEY_F3:
						ShowPileDisplayCards(LOCATION_EXTRA, 0);
						break;
					case irr::KEY_F4:
						ShowPileDisplayCards(LOCATION_OVERLAY, 0);
						break;
					case irr::KEY_F5:
						ShowPileDisplayCards(LOCATION_GRAVE, 1);
						break;
					case irr::KEY_F6:
						ShowPileDisplayCards(LOCATION_REMOVED, 1);
						break;
					case irr::KEY_F7:
						ShowPileDisplayCards(LOCATION_EXTRA, 1);
						break;
					case irr::KEY_F8:
						ShowPileDisplayCards(LOCATION_OVERLAY, 1);
						break;
					default: break;
				}
			}
			break;
		}
		default: break;
		}
		break;
	}
	default: break;
	}
	return false;
}
static bool IsTrulyVisible(const irr::gui::IGUIElement* elem) {
	while(elem->isVisible()) {
		elem = elem->getParent();
		if(!elem)
			return true;
	}
	return false;
}
bool ClientField::OnCommonEvent(const irr::SEvent& event, bool& stopPropagation) {
	static irr::u32 buttonstates = 0;
	static uint8_t resizestate = gGameConfig->fullscreen ? 2 : 0;
	if(TransformEvent(event, stopPropagation))
		return true;
	switch(event.EventType) {
	case irr::EET_GUI_EVENT: {
		auto id = event.GUIEvent.Caller->getID();
		if(mainGame->menuHandler.IsSynchronizedElement(id))
			mainGame->menuHandler.SynchronizeElement(event.GUIEvent.Caller);
		switch(event.GUIEvent.EventType) {
		case irr::gui::EGET_ELEMENT_HOVERED: {
			// Set cursor to an I-Beam if hovering over an edit box
			if (event.GUIEvent.Caller->getType() == irr::gui::EGUIET_EDIT_BOX && event.GUIEvent.Caller->isEnabled()) {
				GUIUtils::ChangeCursor(mainGame->device, irr::gui::ECI_IBEAM);
				return true;
			}
			break;
		}
		case irr::gui::EGET_ELEMENT_LEFT: {
			// Set cursor to normal if left an edit box
			if (event.GUIEvent.Caller->getType() == irr::gui::EGUIET_EDIT_BOX) {
				GUIUtils::ChangeCursor(mainGame->device, irr::gui::ECI_NORMAL);
				return true;
			}
			break;
		}
		case irr::gui::EGET_ELEMENT_CLOSED: {
			if(event.GUIEvent.Caller == mainGame->gSettings.window) {
				stopPropagation = true;
				mainGame->HideElement(event.GUIEvent.Caller);
				return true;
			}
			break;
		}
		case irr::gui::EGET_BUTTON_CLICKED: {
			if(mainGame->fadingList.size() || !IsTrulyVisible(event.GUIEvent.Caller)) {
				stopPropagation = true;
				return true;
			}
			switch(id) {
			case BUTTON_CLEAR_LOG: {
				mainGame->lstLog->clear();
				mainGame->logParam.clear();
				return true;
			}
			case BUTTON_CLEAR_CHAT: {
				mainGame->lstChat->clear();
				for(int i = 0; i < 8; i++) {
					mainGame->chatMsg[i].clear();
					mainGame->chatType[i] = 0;
					mainGame->chatTiming[i] = 0;
				}
				return true;
			}
			case BUTTON_EXPAND_INFOBOX: {
				mainGame->infosExpanded = mainGame->infosExpanded ? 0 : 1;
				mainGame->btnExpandLog->setText(mainGame->infosExpanded ? gDataManager->GetSysString(2044).data() : gDataManager->GetSysString(2043).data());
				mainGame->btnExpandChat->setText(mainGame->infosExpanded ? gDataManager->GetSysString(2044).data() : gDataManager->GetSysString(2043).data());
				{
					auto wInfosSize = mainGame->wInfos->getRelativePosition();
					wInfosSize.LowerRightCorner.X = mainGame->ResizeX(mainGame->infosExpanded ? 1023 : 301);
					mainGame->wInfos->setRelativePosition(wInfosSize);
				}
				auto lstsSize = mainGame->Resize(10, 10, mainGame->infosExpanded ? 1012 : 290, 0);
				lstsSize.LowerRightCorner.Y = mainGame->ResizeY(300 - mainGame->Scale(7)) - mainGame->Scale(10);
				mainGame->lstLog->setRelativePosition(lstsSize);
				mainGame->lstChat->setRelativePosition(lstsSize);
				return true;
			}
			case BUTTON_REPO_CHANGELOG:	{
				irr::gui::IGUIButton* button = (irr::gui::IGUIButton*)event.GUIEvent.Caller;
				for(auto& repo : mainGame->repoInfoGui) {
					if(repo.second.history_button1 == button || repo.second.history_button2 == button) {
						showing_repo = repo.first;
						mainGame->stCommitLog->setText(mainGame->chkCommitLogExpand->isChecked() ? repo.second.commit_history_full.data() : repo.second.commit_history_partial.data());
						mainGame->SetCentered(mainGame->wCommitsLog);
						mainGame->PopupElement(mainGame->wCommitsLog);
						break;
					}
				}
				return true;
			}
			case BUTTON_REPO_CHANGELOG_EXIT: {
				mainGame->HideElement(mainGame->wCommitsLog);
				return true;
			}
			case BUTTON_RELOAD_SKIN: {
				mainGame->should_reload_skin = true;
				break;
			}
			case BUTTON_SHOW_SETTINGS: {
				if (!mainGame->gSettings.window->isVisible())
					mainGame->PopupElement(mainGame->gSettings.window);
				mainGame->env->setFocus(mainGame->gSettings.window);
				break;
			}
			//////kdiy///////
			case BUTTON_SHOW_CARD: {
				if(!mainGame->wCardImg->isVisible())
					mainGame->wCardImg->setVisible(true);
                else
					mainGame->HideElement(mainGame->wCardImg);
				break;
			}
			case BUTTON_CHATLOG: {
				if (!mainGame->wInfos->isVisible()) {
					mainGame->wInfos->setVisible(true);
					mainGame->env->getRootGUIElement()->bringToFront(mainGame->wInfos);
				} else
					mainGame->HideElement(mainGame->wInfos);
				break;
			}
			case BUTTON_CARDLOC: {
				if(!mainGame->wLocation->isVisible()) {
					mainGame->wLocation->setVisible(true);
					mainGame->HideElement(mainGame->wCardImg);
					mainGame->HideElement(mainGame->wInfos);
                    for(int i = 0; i < 8; i++)
                        mainGame->CardInfo[i]->setVisible(false);
				} else
					mainGame->HideElement(mainGame->wLocation);
				break;
			}
			case BUTTON_LOCATION_0: {
				mainGame->btnLocation[0]->setPressed();
				for(int i = 1; i < 6; ++i)
					mainGame->btnLocation[i]->setPressed(false);
				break;
			}
			case BUTTON_LOCATION_1: {
				mainGame->btnLocation[1]->setPressed();
				for(int i = 0; i < 6; ++i) {
					if(i == 1) continue;
					mainGame->btnLocation[i]->setPressed(false);
				}
				break;
			}
			case BUTTON_LOCATION_2: {
				mainGame->btnLocation[2]->setPressed();
				for(int i = 0; i < 6; ++i) {
					if(i == 2) continue;
					mainGame->btnLocation[i]->setPressed(false);
				}
				break;
			}
			case BUTTON_LOCATION_3: {
				mainGame->btnLocation[3]->setPressed();
				for(int i = 0; i < 6; ++i) {
					if(i == 3) continue;
					mainGame->btnLocation[i]->setPressed(false);
				}
				break;
			}
			case BUTTON_LOCATION_4: {
				mainGame->btnLocation[4]->setPressed();
				for(int i = 0; i < 6; ++i) {
					if(i == 4) continue;
					mainGame->btnLocation[i]->setPressed(false);
				}
				break;
			}
			case BUTTON_LOCATION_5: {
				mainGame->btnLocation[5]->setPressed();
				for(int i = 0; i < 6; ++i) {
					if(i == 5) continue;
					mainGame->btnLocation[i]->setPressed(false);
				}
				break;
			}
            case BUTTON_REPO_FILE:	{
				irr::gui::IGUIButton* button = (irr::gui::IGUIButton*)event.GUIEvent.Caller;
				for(auto& repo : mainGame->repoInfoGui) {
					if(repo.second.file_button == button) {
						mainGame->stACMessage->setText(gDataManager->GetSysString(8005).data());
						mainGame->PopupElement(mainGame->wACMessage, 90);
						Utils::SystemOpen(Utils::ToPathString(repo.second.path), Utils::OPEN_FILE);
					}
                }
				break;
			}
			case BUTTON_HOME: {
                Utils::SystemOpen(EPRO_TEXT("https://afdian.com/a/Edokcg/"), Utils::OPEN_URL);
				break;
			}
			case BUTTON_FOLDER: {
                Utils::SystemOpen(EPRO_TEXT("https://afdian.com/p/4a2966dcbb0d11edaaa052540025c377/"), Utils::OPEN_URL);
				break;
			}
			case BUTTON_PICDL: {
                Utils::SystemOpen(EPRO_TEXT("https://afdian.com/p/350597f2a3e711efb15f52540025c377/"), Utils::OPEN_URL);
				break;
			}
			case BUTTON_MOVIEDL: {
                Utils::SystemOpen(EPRO_TEXT("https://afdian.com/p/8c3fb8e8a3df11efba4752540025c377/"), Utils::OPEN_URL);
				break;
			}
            case BUTTON_SOUNDDL: {
                Utils::SystemOpen(EPRO_TEXT("https://afdian.com/p/ad6b923aa3df11ef9a3b5254001e7c00/"), Utils::OPEN_URL);
				break;
			}
			case BUTTON_INTRO: {
                Utils::SystemOpen(EPRO_TEXT("https://www.bilibili.com/read/cv8171279/"), Utils::OPEN_URL);
				break;
			}
			case BUTTON_TUT: {
                Utils::SystemOpen(EPRO_TEXT("https://www.bilibili.com/video/BV1Ey4y1q7pr/"), Utils::OPEN_URL);
				break;
			}
			case BUTTON_TUT2: {
                Utils::SystemOpen(EPRO_TEXT("https://www.bilibili.com/video/BV1a54y127Xx?p=2"), Utils::OPEN_URL);
				break;
			}
			case BUTTON_TEXTURE: {
				break;
			}
			case BUTTON_TEXTURE_SELECT2: {
				if(mainGame->gSettings.clickedindex < 0) break;
				int selected = mainGame->gSettings.cbName_texture[mainGame->gSettings.clickedindex]->getSelected();
				if(mainGame->gSettings.cbName_texture[mainGame->gSettings.clickedindex]->getItemCount() < 2) break;
				if(selected >= mainGame->gSettings.cbName_texture[mainGame->gSettings.clickedindex]->getItemCount() - 1) selected = 0;
				else selected++;
				mainGame->gSettings.cbName_texture[mainGame->gSettings.clickedindex]->setSelected(selected);
				std::wstring filepath = std::wstring(mainGame->gSettings.cbName_texture[mainGame->gSettings.clickedindex]->getItem(selected));
				auto tTexture = mainGame->imageManager.UpdatetTexture(mainGame->gSettings.clickedindex, filepath);
				if(tTexture != nullptr) {
					mainGame->gSettings.btnrandomtexture->setImage(tTexture);
					mainGame->gSettings.btnrandomtexture2->setImage(tTexture);
					if(mainGame->gSettings.clickedindex == 18) mainGame->btnSettings->setImage(mainGame->imageManager.tSettings);
					if(mainGame->gSettings.clickedindex == 19) {
						mainGame->btnHand[0]->setImage(mainGame->imageManager.tHand[0]);
						mainGame->btnHand[1]->setImage(mainGame->imageManager.tHand[1]);
						mainGame->btnHand[2]->setImage(mainGame->imageManager.tHand[2]);
					}
					mainGame->driver->removeTexture(tTexture);
					tTexture = nullptr;
				}
				break;
			}
			case BUTTON_TEXTURE_SELECT: {
				if(mainGame->gSettings.clickedindex < 0) break;
				int selected = mainGame->gSettings.cbName_texture[mainGame->gSettings.clickedindex]->getSelected();
				if(mainGame->gSettings.cbName_texture[mainGame->gSettings.clickedindex]->getItemCount() < 2) break;
				if(selected == 0) selected = mainGame->gSettings.cbName_texture[mainGame->gSettings.clickedindex]->getItemCount() - 1;
				else selected--;
				mainGame->gSettings.cbName_texture[mainGame->gSettings.clickedindex]->setSelected(selected);
				std::wstring filepath = std::wstring(mainGame->gSettings.cbName_texture[mainGame->gSettings.clickedindex]->getItem(selected));
				auto tTexture = mainGame->imageManager.UpdatetTexture(mainGame->gSettings.clickedindex, filepath);
				if(tTexture != nullptr) {
					mainGame->gSettings.btnrandomtexture->setImage(tTexture);
					mainGame->gSettings.btnrandomtexture2->setImage(tTexture);
					if(mainGame->gSettings.clickedindex == 18) mainGame->btnSettings->setImage(mainGame->imageManager.tSettings);
					if(mainGame->gSettings.clickedindex == 19) {
						mainGame->btnHand[0]->setImage(mainGame->imageManager.tHand[0]);
						mainGame->btnHand[1]->setImage(mainGame->imageManager.tHand[1]);
						mainGame->btnHand[2]->setImage(mainGame->imageManager.tHand[2]);
					}
					mainGame->driver->removeTexture(tTexture);
					tTexture = nullptr;
				}
				break;
			}
			case BUTTON_TEXTURE_OK: {
				mainGame->HideElement(mainGame->gSettings.wRandomTexture);
				mainGame->ShowElement(mainGame->gSettings.window);
				break;
			}
			case BUTTON_TEXTURE_OK2: {
				mainGame->HideElement(mainGame->gSettings.wRandomWallpaper);
				mainGame->ShowElement(mainGame->gSettings.window);
				break;
			}
			//////kdiy///////
			case BUTTON_APPLY_RESTART: {
				try {
					gGameConfig->dpi_scale = static_cast<uint32_t>(std::stol(mainGame->gSettings.ebDpiScale->getText())) / 100.0;
					//////kdiy///////
					gGameConfig->textfont.size = static_cast<uint32_t>(std::stol(mainGame->gSettings.ebFontSize->getText()));
					//////kdiy///////
					mainGame->restart = true;
					//mainGame->device->closeDevice();
				} catch(...){}
				break;
			}
			case BUTTON_FPS_CAP: {
				try {
					gGameConfig->maxFPS = static_cast<int32_t>(std::stol(mainGame->gSettings.ebFPSCap->getText()));
				} catch (...) {
					mainGame->gSettings.ebFPSCap->setText(epro::to_wstring(gGameConfig->maxFPS).data());
				}
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_LISTBOX_CHANGED: {
			switch(id) {
			case LISTBOX_LOG: {
				int sel = mainGame->lstLog->getSelected();
				if(sel != -1 && (int)mainGame->logParam.size() >= sel && mainGame->logParam[sel]) {
					mainGame->ShowCardInfo(mainGame->logParam[sel]);
				}
				return true;
			}
			}
			break;
		}
		case irr::gui::EGET_LISTBOX_SELECTED_AGAIN: {
			switch(id) {
			case LISTBOX_LOG: {
				int sel = mainGame->lstLog->getSelected();
				if(sel != -1 && (int)mainGame->logParam.size() >= sel && mainGame->logParam[sel]) {
					mainGame->wInfos->setActiveTab(0);
				}
				return true;
			}
			}
			break;
		}
		case irr::gui::EGET_SCROLL_BAR_CHANGED: {
			switch(id) {
			case SCROLL_MUSIC_VOLUME: {
				gGameConfig->musicVolume = static_cast<irr::gui::IGUIScrollBar*>(event.GUIEvent.Caller)->getPos();
				gSoundManager->SetMusicVolume(gGameConfig->musicVolume / 100.0);
				return true;
			}
			case SCROLL_SOUND_VOLUME: {
				gGameConfig->soundVolume = static_cast<irr::gui::IGUIScrollBar*>(event.GUIEvent.Caller)->getPos();
				gSoundManager->SetSoundVolume(gGameConfig->soundVolume / 100.0);
				return true;
			}
			}
			break;
		}
		case irr::gui::EGET_CHECKBOX_CHANGED: {
			switch (id) {
			case CHECKBOX_ENABLE_MUSIC: {
				gGameConfig->enablemusic = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				gSoundManager->EnableMusic(gGameConfig->enablemusic);
				return true;
			}
			case CHECKBOX_ENABLE_SOUND: {
				gGameConfig->enablesound = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				gSoundManager->EnableSounds(gGameConfig->enablesound);
				return true;
			}
			/////kdiy/////////
			case CHECKBOX_ENABLE_SSOUND: {
				gGameConfig->enablessound = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				if(gGameConfig->enablessound)
					mainGame->chantcheck();
				return true;
			}
			case CHECKBOX_ENABLE_CSOUND: {
				gGameConfig->enablecsound = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				if(gGameConfig->enablecsound)
					mainGame->chantcheck();
				return true;
			}
			case CHECKBOX_ENABLE_ASOUND: {
				gGameConfig->enableasound = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				if(gGameConfig->enableasound)
					mainGame->chantcheck();
				return true;
			}
			case CHECKBOX_ENABLE_ANIME: {
				gGameConfig->enableanime = static_cast<irr::gui::IGUICheckBox *>(event.GUIEvent.Caller)->isChecked();
				if(gGameConfig->enableanime)
					mainGame->moviecheck();
				return true;
			}
			case CHECKBOX_ENABLE_SANIME: {
				gGameConfig->enablesanime = static_cast<irr::gui::IGUICheckBox *>(event.GUIEvent.Caller)->isChecked();
				if(gGameConfig->enablesanime)
                    mainGame->moviecheck(0);
				return true;
			}
			case CHECKBOX_ENABLE_CANIME: {
				gGameConfig->enablecanime = static_cast<irr::gui::IGUICheckBox *>(event.GUIEvent.Caller)->isChecked();
				if(gGameConfig->enablecanime)
                    mainGame->moviecheck(1);
				return true;
			}
			case CHECKBOX_ENABLE_AANIME: {
				gGameConfig->enableaanime = static_cast<irr::gui::IGUICheckBox *>(event.GUIEvent.Caller)->isChecked();
				if(gGameConfig->enableaanime)
                    mainGame->moviecheck(2);
				return true;
			}
			case CHECKBOX_ENABLE_FANIME: {
				gGameConfig->enablefanime = static_cast<irr::gui::IGUICheckBox *>(event.GUIEvent.Caller)->isChecked();
				if(gGameConfig->enablefanime)
                    mainGame->moviecheck(3);
				return true;
			}
			case CHECKBOX_ANIME_FULL: {
				gGameConfig->animefull = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				return true;
			}
			case CHECKBOX_HIDE_NAME_TAG:{
				gGameConfig->chkHideNameTag = mainGame->gSettings.chkHideNameTag->isChecked();
				return true;
			}
            case CHECKBOX_RANDOM_TEXTURE:{
				gGameConfig->randomtexture = mainGame->gSettings.chkRandomtexture->isChecked();
				if(!gGameConfig->randomtexture) {
					mainGame->HideElement(mainGame->gSettings.window);
					mainGame->ShowElement(mainGame->gSettings.wRandomTexture);
				}
				return true;
			}
            case CHECKBOX_RANDOM_WALLPAPER:{
				gGameConfig->randomwallpaper = mainGame->gSettings.chkRandomwallpaper->isChecked();
				if(!gGameConfig->randomwallpaper) {
					mainGame->HideElement(mainGame->gSettings.window);
					mainGame->ShowElement(mainGame->gSettings.wRandomWallpaper);
				}
				return true;
			}
            case CHECKBOX_TEXTURE:{
				mainGame->gSettings.clickedchkbox = true;
				auto checkboxtexture = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller);
				int k = 0;
				bool visible = false;
				std::wstring filepath = L"";
				for(int i = 0; i < 20; i++) {
					if(mainGame->gSettings.chktexture[i] == checkboxtexture) {
					    k = i;
						std::tie(visible, filepath) = mainGame->SetRandTexture(i, 1);
						break;
					}
				}
				mainGame->gSettings.clickedindex = k;
				mainGame->gSettings.cbName_texture[k]->setVisible(!visible);
				if(!visible) {
					int selected = mainGame->gSettings.cbName_texture[k]->getSelected();
					if (selected < 0) {
						selected = 0;
						mainGame->gSettings.cbName_texture[k]->setSelected(0);
					}
					if(filepath == L"") filepath = std::wstring(mainGame->gSettings.cbName_texture[k]->getItem(0));
					auto tTexture = mainGame->imageManager.UpdatetTexture(k, filepath);
					if(tTexture != nullptr) {
						mainGame->gSettings.btnrandomtexture->setImage(tTexture);
						mainGame->gSettings.btnrandomtexture2->setImage(tTexture);
						if(k == 18) mainGame->btnSettings->setImage(mainGame->imageManager.tSettings);
						if(k == 19) {
							mainGame->btnHand[0]->setImage(mainGame->imageManager.tHand[0]);
							mainGame->btnHand[1]->setImage(mainGame->imageManager.tHand[1]);
							mainGame->btnHand[2]->setImage(mainGame->imageManager.tHand[2]);
						}
						mainGame->driver->removeTexture(tTexture);
						tTexture = nullptr;
					}
				} else {
				    mainGame->gSettings.btnrandomtexture->setImage(0);
					mainGame->gSettings.btnrandomtexture2->setImage(0);
				}
				return true;
			}
            case CHECKBOX_VWALLPAPER:{
				gGameConfig->videowallpaper = mainGame->gSettings.chkVideowallpaper->isChecked();
				mainGame->gSettings.chkRandomVideowallpaper->setEnabled(gGameConfig->videowallpaper);
				mainGame->gSettings.cbVideowallpaper->setEnabled(gGameConfig->videowallpaper && !gGameConfig->randomvideowallpaper);
				return true;
			}
            case CHECKBOX_RANDOM_VWALLPAPER:{
				gGameConfig->randomvideowallpaper = mainGame->gSettings.chkRandomVideowallpaper->isChecked();
				if(gGameConfig->randomvideowallpaper) mainGame->imageManager.GetRandomVWallpaper();
				mainGame->gSettings.cbVideowallpaper->setEnabled(gGameConfig->videowallpaper && !gGameConfig->randomvideowallpaper);
				return true;
			}
            case CHECKBOX_CLOSEUP:{
				gGameConfig->closeup = mainGame->gSettings.chkCloseup->isChecked();
				return true;
			}
            case CHECKBOX_PAINTING:{
				gGameConfig->painting = mainGame->gSettings.chkPainting->isChecked();
				return true;
			}
            case CHECKBOX_PAUSE_DUEL: {
				gGameConfig->pauseduel = mainGame->gSettings.chkPauseduel->isChecked();
				break;
			}
			// case CHECKBOX_HIDE_PASSCODE_SCOPE: {
			// 	gGameConfig->hidePasscodeScope = mainGame->gSettings.chkHidePasscodeScope->isChecked();
			// 	mainGame->stPasscodeScope->setVisible(!gGameConfig->hidePasscodeScope);
			// 	mainGame->RefreshCardInfoTextPositions();
			// 	return true;
			// }
			/////kdiy/////////
			case CHECKBOX_QUICK_ANIMATION: {
				gGameConfig->quick_animation = mainGame->tabSettings.chkQuickAnimation->isChecked();
				return true;
			}
			case CHECKBOX_ALTERNATIVE_PHASE_LAYOUT: {
				gGameConfig->alternative_phase_layout = mainGame->tabSettings.chkAlternativePhaseLayout->isChecked();
				mainGame->SetPhaseButtons(true);
				return true;
			}
			case CHECKBOX_HIDE_ARCHETYPES: {
				gGameConfig->chkHideSetname = mainGame->gSettings.chkHideSetname->isChecked();
				mainGame->stSetName->setVisible(!gGameConfig->chkHideSetname);
				mainGame->RefreshCardInfoTextPositions();
				return true;
			}
			case CHECKBOX_SHOW_SCOPE_LABEL: {
				gGameConfig->showScopeLabel = mainGame->gSettings.chkShowScopeLabel->isChecked();
				return true;
			}
			case CHECKBOX_IGNORE_DECK_CONTENTS: {
				gGameConfig->ignoreDeckContents = mainGame->gSettings.chkIgnoreDeckContents->isChecked();
				return true;
			}
			case CHECKBOX_ADD_CARD_NAME_TO_DECK_LIST: {
				gGameConfig->addCardNamesToDeckList = mainGame->gSettings.chkAddCardNamesInDeckList->isChecked();
				return true;
			}
			case CHECKBOX_SHOW_FPS: {
				gGameConfig->showFPS = mainGame->gSettings.chkShowFPS->isChecked();
				mainGame->fpsCounter->setVisible(gGameConfig->showFPS);
				return true;
			}
			case CHECKBOX_DRAW_FIELD_SPELLS: {
				gGameConfig->draw_field_spell = mainGame->gSettings.chkDrawFieldSpells->isChecked();
				return true;
			}
			case CHECKBOX_FILTER_BOT: {
				gGameConfig->filterBot = mainGame->gSettings.chkFilterBot->isChecked();
				mainGame->gBot.Refresh(gGameConfig->filterBot * (mainGame->cbDuelRule->getSelected() + 1), gGameConfig->lastBot);
				return true;
			}
			case CHECKBOX_FULLSCREEN: {
				GUIUtils::ToggleFullscreen(mainGame->device, gGameConfig->fullscreen);
				return true;
			}
			case CHECKBOX_SCALE_BACKGROUND: {
				gGameConfig->scale_background = mainGame->gSettings.chkScaleBackground->isChecked();
				return true;
			}
			case CHECKBOX_ACCURATE_BACKGROUND_RESIZE: {
				gGameConfig->accurate_bg_resize = mainGame->gSettings.chkAccurateBackgroundResize->isChecked();
				return true;
			}
			case CHECKBOX_CONFIRM_DECK_CLEAR: {
				gGameConfig->confirm_clear_deck = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				return true;
			}
			case BUTTON_REPO_CHANGELOG_EXPAND: {
				auto& repo = mainGame->repoInfoGui[showing_repo];
				mainGame->stCommitLog->setText(mainGame->chkCommitLogExpand->isChecked() ? repo.commit_history_full.data() : repo.commit_history_partial.data());
				return true;
			}
			case CHECKBOX_SAVE_HAND_TEST_REPLAY: {
				gGameConfig->saveHandTest = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				return true;
			}
			case CHECKBOX_LOOP_MUSIC: {
				gGameConfig->loopMusic = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				break;
			}
#ifdef DISCORD_APP_ID
			case CHECKBOX_DISCORD_INTEGRATION: {
				gGameConfig->discordIntegration = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				mainGame->discord.UpdatePresence(gGameConfig->discordIntegration ? DiscordWrapper::INITIALIZE : DiscordWrapper::TERMINATE);
				break;
			}
#endif
			case CHECKBOX_LOG_DOWNLOAD_ERRORS: {
				gGameConfig->logDownloadErrors = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				break;
			}
#if EDOPRO_ANDROID
			case CHECKBOX_NATIVE_KEYBOARD: {
				gGameConfig->native_keyboard = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				break;
			}
			case CHECKBOX_NATIVE_MOUSE: {
				gGameConfig->native_mouse = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				break;
			}
#endif
			case CHECKBOX_HIDE_HANDS_REPLAY: {
				gGameConfig->hideHandsInReplays = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				break;
			}
			case CHECKBOX_TOPDOWN: {
				mainGame->current_topdown = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				break;
			}
			case CHECKBOX_KEEP_FIELD_ASPECT_RATIO: {
				mainGame->current_keep_aspect_ratio = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				break;
			}
			case CHECKBOX_KEEP_CARD_ASPECT_RATIO: {
				const auto checked = static_cast<irr::gui::IGUICheckBox*>(event.GUIEvent.Caller)->isChecked();
				gGameConfig->keep_cardinfo_aspect_ratio = checked;
				mainGame->ResizeCardinfoWindow(checked);
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_EDITBOX_ENTER: {
			switch(id) {
			case EDITBOX_CHAT: {
				if(mainGame->dInfo.isReplay)
					break;
				const wchar_t* input = mainGame->ebChatInput->getText();
				if(input[0]) {
					uint16_t msgbuf[256];
					int len = BufferIO::EncodeUTF16(input, msgbuf, 256);
					DuelClient::SendBufferToServer(CTOS_CHAT, msgbuf, len * sizeof(uint16_t));
					mainGame->ebChatInput->setText(L"");
				}
				return true;
			}
			}
			break;
		}
		case irr::gui::EGET_EDITBOX_CHANGED: {
			switch(id) {
			case EDITBOX_NUMERIC: {
				std::wstring tmp(event.GUIEvent.Caller->getText());
				if(Utils::KeepOnlyDigits(tmp))
					event.GUIEvent.Caller->setText(tmp.data());
				break;
			}
			case EDITBOX_FPS_CAP: {
				std::wstring tmp(event.GUIEvent.Caller->getText());
				if(Utils::KeepOnlyDigits(tmp, true) || tmp.size() > 1) {
					if(tmp.size() > 1) {
						if(tmp[0] == L'-' && (tmp[1] != L'1' || tmp.size() != 2)) {
							event.GUIEvent.Caller->setText(L"-");
							break;
						}
					}
					event.GUIEvent.Caller->setText(tmp.data());
				}
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_TAB_CHANGED: {
			if(event.GUIEvent.Caller == mainGame->wInfos) {
				auto curTab = mainGame->wInfos->getTab(mainGame->wInfos->getActiveTab());
				if((curTab != mainGame->tabLog && curTab != mainGame->tabChat) && mainGame->infosExpanded) {
					if(mainGame->infosExpanded == 1) {
						auto wInfosSize = mainGame->wInfos->getRelativePosition();
						wInfosSize.LowerRightCorner.X = mainGame->ResizeX(301);
						mainGame->wInfos->setRelativePosition(wInfosSize);
					}
					mainGame->infosExpanded = 2;
				} else if(mainGame->infosExpanded) {
					if(mainGame->infosExpanded == 2) {
						auto wInfosSize = mainGame->wInfos->getRelativePosition();
						wInfosSize.LowerRightCorner.X = mainGame->ResizeX(1023);
						mainGame->wInfos->setRelativePosition(wInfosSize);
					}
					mainGame->infosExpanded = 1;
				}
				return true;
			}
			break;
		}
		case irr::gui::EGET_COMBO_BOX_CHANGED: {
			switch(id) {
			case COMBOBOX_VSYNC: {
				gGameConfig->vsync = mainGame->gSettings.cbVSync->getSelected();
				GUIUtils::ToggleSwapInterval(mainGame->driver, gGameConfig->vsync);
				return true;
			}
			case COMBOBOX_CURRENT_SKIN: {
				auto newskin = Utils::ToPathString(mainGame->gSettings.cbCurrentSkin->getItem(mainGame->gSettings.cbCurrentSkin->getSelected()));
				mainGame->should_reload_skin = newskin != gGameConfig->skin;
				return true;
			}
			case COMBOBOX_CURRENT_LOCALE: {
				mainGame->ApplyLocale(mainGame->gSettings.cbCurrentLocale->getSelected());
				return true;
			}
			///kdiy///////
			case COMBOBOX_CURRENT_FONT: {
				int selected = mainGame->gSettings.cbCurrentFont->getSelected();
				if (selected < 0) return true;
				gGameConfig->textfont.font = Utils::ToPathString(epro::format(L"fonts/{}", mainGame->gSettings.cbCurrentFont->getItem(selected)));
				return true;
			}
			case COMBOBOX_TEXTURE: {
				mainGame->gSettings.clickedchkbox = false;
				auto comboboxtexture = static_cast<irr::gui::IGUIComboBox*>(event.GUIEvent.Caller);
				int k = 0;
				bool visible = false;
				std::wstring setfilepath = L"", filepath = L"";
				for(int i = 0; i < 20; i++) {
					if(mainGame->gSettings.cbName_texture[i] == comboboxtexture) {
						k = i;
						int selected = mainGame->gSettings.cbName_texture[k]->getSelected();
						if (selected < 0) {
							selected = 0;
							mainGame->gSettings.cbName_texture[k]->setSelected(0);
						}
						setfilepath = std::wstring(mainGame->gSettings.cbName_texture[i]->getItem(selected));
						std::tie(visible, filepath) = mainGame->SetRandTexture(i, 2, setfilepath);
						break;
					}
				}
				mainGame->gSettings.clickedindex = k;
				auto tTexture = mainGame->imageManager.UpdatetTexture(k, filepath);
				if(k == 18) mainGame->btnSettings->setImage(mainGame->imageManager.tSettings);
				if(k == 19) {
					mainGame->btnHand[0]->setImage(mainGame->imageManager.tHand[0]);
					mainGame->btnHand[1]->setImage(mainGame->imageManager.tHand[1]);
					mainGame->btnHand[2]->setImage(mainGame->imageManager.tHand[2]);
				}
				if(tTexture != nullptr) {
					mainGame->gSettings.btnrandomtexture->setImage(tTexture);
					mainGame->gSettings.btnrandomtexture2->setImage(tTexture);
					mainGame->driver->removeTexture(tTexture);
					tTexture = nullptr;
				}
				return true;
			}
			case COMBOBOX_VIDEO_WALLPAPER: {
				int selected = mainGame->gSettings.cbVideowallpaper->getSelected();
				if (selected < 0) return true;
				gGameConfig->videowallpapertexture = Utils::ToUTF8IfNeeded(mainGame->gSettings.cbVideowallpaper->getItem(selected));
				mainGame->videowallpaper_path = "wallpaper/" + gGameConfig->videowallpapertexture;
				return true;
			}
			///kdiy///////
			case COMBOBOX_CORE_LOG_OUTPUT: {
				int selected = mainGame->gSettings.cbCoreLogOutput->getSelected();
				if (selected < 0) return true;
				gGameConfig->coreLogOutput = mainGame->gSettings.cbCoreLogOutput->getItemData(selected);
				return true;
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
		case irr::KEY_KEY_C: {
			if(event.KeyInput.Control && mainGame->HasFocus(irr::gui::EGUIET_LIST_BOX)) {
				auto focus = static_cast<irr::gui::IGUIListBox*>(mainGame->env->getFocus());
				int sel = focus->getSelected();
				if(sel != -1) {
					Utils::OSOperator->copyToClipboard(focus->getListItem(sel));
					return true;
				}
			}
			return false;
		}
		case irr::KEY_KEY_R: {
			if(event.KeyInput.Control) {
				mainGame->should_reload_skin = true;
				return true;
			}
			if(!event.KeyInput.PressedDown && !mainGame->HasFocus(irr::gui::EGUIET_EDIT_BOX))
				mainGame->textFont->setTransparency(true);
			return true;
		}
		case irr::KEY_KEY_O: {
			if (event.KeyInput.Control && !event.KeyInput.PressedDown) {
				auto window = mainGame->gSettings.window;
				if(window->isVisible())
					mainGame->HideElement(window);
				else
					mainGame->PopupElement(window);
			}
			return true;
		}
		case irr::KEY_ESCAPE: {
			if(!mainGame->HasFocus(irr::gui::EGUIET_EDIT_BOX))
				mainGame->device->minimizeWindow();
			return true;
		}
		///kdiy///////
		// case irr::KEY_F9: {
		// 	if (!event.KeyInput.PressedDown) {
		// 		const auto new_val = !mainGame->current_topdown;
		// 		GUIUtils::SetCheckbox(mainGame->device, mainGame->gSettings.chkTopdown, new_val);
		// 		GUIUtils::SetCheckbox(mainGame->device, mainGame->gSettings.chkKeepFieldRatio, new_val);
		// 	}
		// 	return true;
		// }
		///kdiy///////
		case irr::KEY_F10: {
			if(event.KeyInput.Shift)
				gGameConfig->windowStruct.clear();
			else
				gGameConfig->windowStruct = GUIUtils::SerializeWindowPosition(mainGame->device);
			return true;
		}
		case irr::KEY_F11: {
			if(!event.KeyInput.PressedDown) {
				GUIUtils::ToggleFullscreen(mainGame->device, gGameConfig->fullscreen);
				mainGame->gSettings.chkFullscreen->setChecked(gGameConfig->fullscreen);
			}
			return true;
		}
		case irr::KEY_F12: {
			if (!event.KeyInput.PressedDown)
				GUIUtils::TakeScreenshot(mainGame->device);
			return true;
		}
		case irr::KEY_KEY_1: {
			if (event.KeyInput.Control && !event.KeyInput.PressedDown)
				mainGame->wInfos->setActiveTab(0);
			break;
		}
		case irr::KEY_KEY_2: {
			if (event.KeyInput.Control && !event.KeyInput.PressedDown)
				mainGame->wInfos->setActiveTab(1);
			break;
		}
		case irr::KEY_KEY_3: {
			if (event.KeyInput.Control && !event.KeyInput.PressedDown)
				mainGame->wInfos->setActiveTab(2);
			break;
		}
		case irr::KEY_KEY_4: {
			if (event.KeyInput.Control && !event.KeyInput.PressedDown)
				mainGame->wInfos->setActiveTab(3);
			break;
		}
		/////kdiy/////////
		// case irr::KEY_KEY_5: {
		// 	if (event.KeyInput.Control && !event.KeyInput.PressedDown)
		// 		mainGame->wInfos->setActiveTab(4);
		// 	break;
		// }
		/////kdiy/////////
		default: break;
		}
		break;
	}
	case irr::EET_MOUSE_INPUT_EVENT: {
		auto SimulateMouse = [device=mainGame->device, &event](irr::EMOUSE_INPUT_EVENT type) {
			irr::SEvent simulated = event;
			simulated.MouseInput.Event = type;
			device->postEventFromUser(simulated);
			return true;
		};
		switch (event.MouseInput.Event) {
			case irr::EMIE_MOUSE_WHEEL: {
				irr::gui::IGUIElement* root = mainGame->env->getRootGUIElement();
				irr::gui::IGUIElement* elem = root->getElementFromPoint({ event.MouseInput.X, event.MouseInput.Y });
				auto IsStaticText = [](irr::gui::IGUIElement* elem) -> bool {
					return elem && elem->getType() == irr::gui::EGUIET_STATIC_TEXT;
				};
				auto IsScrollBar = [](irr::gui::IGUIElement* elem) -> bool {
					return elem && (elem->getType() == irr::gui::EGUIET_SCROLL_BAR);
				};
				auto IsScrollBarButton = [&IsScrollBar](irr::gui::IGUIElement* elem) -> bool {
					return elem && (elem->getType() == irr::gui::EGUIET_BUTTON) && IsScrollBar(elem->getParent());
				};
				if(IsStaticText(elem) || IsScrollBar(elem) || IsScrollBarButton(elem)) {
					if(elem->OnEvent(event)) {
						stopPropagation = true;
						return true;
					}
				}
				break;
			}
			case irr::EMIE_MOUSE_MOVED: {
				if((buttonstates & 0xffff) && !event.MouseInput.ButtonStates) {
					auto _event = event;
					_event.MouseInput.ButtonStates = buttonstates & 0xffff;
					_event.MouseInput.Control = buttonstates & (1 << 30);
					_event.MouseInput.Shift = buttonstates & (1 << 29);
					mainGame->device->postEventFromUser(_event);
					stopPropagation = true;
					return true;
				}
				break;
			}
			default: break;
		}
		if(gGameConfig->ctrlClickIsRMB && event.MouseInput.Control) {
			switch(event.MouseInput.Event) {
#define REMAP(TYPE) case irr::EMIE_LMOUSE_##TYPE: return SimulateMouse(irr::EMIE_RMOUSE_##TYPE)
				REMAP(PRESSED_DOWN);
				REMAP(LEFT_UP);
				REMAP(DOUBLE_CLICK);
				REMAP(TRIPLE_CLICK);
#undef REMAP
				default: break;
			}
		}
		if(event.MouseInput.Event == irr::EMIE_LMOUSE_PRESSED_DOWN && mainGame->showingcard) {
			irr::gui::IGUIElement* root = mainGame->env->getRootGUIElement();
			irr::gui::IGUIElement* elem = root->getElementFromPoint({ event.MouseInput.X, event.MouseInput.Y });
            if(elem == mainGame->stName) {
				auto path = mainGame->FindScript(epro::format(EPRO_TEXT("c{}.lua"), mainGame->showingcard));
				if(path.empty()) {
					auto cd = gDataManager->GetCardData(mainGame->showingcard);
					if(cd && cd->IsInArtworkOffsetRange())
						path = mainGame->FindScript(epro::format(EPRO_TEXT("c{}.lua"), cd->alias));
				}
				if(path.size() && path != EPRO_TEXT("archives"))
					Utils::SystemOpen(path, Utils::OPEN_FILE);
			} else if(elem == mainGame->stPasscodeScope) {
                ///kdiy///////
				//Utils::OSOperator->copyToClipboard(epro::format(L"{}", mainGame->showingcard).data());
                auto cd = gDataManager->GetCardData(mainGame->showingcard);
                if(cd && (cd->IsInArtworkOffsetRange() || (cd->alias && cd->alias > 0)))
                    Utils::OSOperator->copyToClipboard(epro::format(L"{}", cd->alias).data());
                else
                    Utils::OSOperator->copyToClipboard(epro::format(L"{}", mainGame->showingcard).data());
                ///kdiy///////
			}
		}
        ///kdiy///////
        if(event.MouseInput.Event == irr::EMIE_LMOUSE_DOUBLE_CLICK && mainGame->showingcard) {
			irr::gui::IGUIElement* root = mainGame->env->getRootGUIElement();
			irr::gui::IGUIElement* elem = root->getElementFromPoint({ event.MouseInput.X, event.MouseInput.Y });
            if(elem == mainGame->stPasscodeScope) {
                Utils::OSOperator->copyToClipboard(epro::format(L"{}", mainGame->showingcard).data());
			}
        }
        ///kdiy///////
		break;
	}
	case irr::EET_JOYSTICK_INPUT_EVENT: {
		if(!gGameConfig->controller_input || !mainGame->device->isWindowFocused())
			break;
		auto& jevent = event.JoystickEvent;
		irr::f32 moveHorizontal = 0.f; // Range is -1.f for full left to +1.f for full right
		irr::f32 moveVertical = 0.f; // -1.f for full down to +1.f for full up.
		const irr::f32 DEAD_ZONE = 0.07f;

		moveHorizontal = (irr::f32)jevent.Axis[JWrapper::Axis::LEFTX] / 32767.f;
		if(fabs(moveHorizontal) < DEAD_ZONE)
			moveHorizontal = 0.f;

		moveVertical = (irr::f32)jevent.Axis[JWrapper::Axis::LEFTY] / 32767.f;
		if(fabs(moveVertical) < DEAD_ZONE)
			moveVertical = 0.f;
		const irr::f32 MOVEMENT_SPEED = 0.001f;
		auto cursor = mainGame->device->getCursorControl();
		auto pos = cursor->getRelativePosition();
		if(!irr::core::equals(moveHorizontal, 0.f) || !irr::core::equals(moveVertical, 0.f)) {
			pos.X += MOVEMENT_SPEED * mainGame->delta_time * moveHorizontal;
			pos.Y += MOVEMENT_SPEED * mainGame->delta_time * moveVertical;
			auto clamp = [](auto& val) { val = (val < 0.f) ? 0.f : (1.f < val) ? 1.f : val;	};
			clamp(pos.X);
			clamp(pos.Y);
			cursor->setPosition(pos.X, pos.Y);
		}
		buttonstates = 0;
		if(jevent.ButtonStates & JWrapper::Buttons::A)
			buttonstates |= irr::E_MOUSE_BUTTON_STATE_MASK::EMBSM_LEFT;
		if(jevent.ButtonStates & JWrapper::Buttons::B)
			buttonstates |= irr::E_MOUSE_BUTTON_STATE_MASK::EMBSM_RIGHT;
		if(jevent.ButtonStates & JWrapper::Buttons::Y)
			buttonstates |= irr::E_MOUSE_BUTTON_STATE_MASK::EMBSM_MIDDLE;
		irr::SEvent simulated{};
		simulated.EventType = irr::EET_MOUSE_INPUT_EVENT;
		simulated.MouseInput.ButtonStates = buttonstates;
		simulated.MouseInput.Control = jevent.ButtonStates & JWrapper::Buttons::LEFTSHOULDER;
		simulated.MouseInput.Shift = jevent.ButtonStates & JWrapper::Buttons::RIGHTSHOULDER;
		simulated.MouseInput.X = irr::core::round32(pos.X * mainGame->window_size.Width);
		simulated.MouseInput.Y = irr::core::round32(pos.Y * mainGame->window_size.Height);

		buttonstates |= (simulated.MouseInput.Control) ? 1 << 30 : 0;
		buttonstates |= (simulated.MouseInput.Shift) ? 1 << 29 : 0;

		auto& changed = jevent.POV;

		auto CheckAndPost = [device=mainGame->device, &simulated, &changed, &states=jevent.ButtonStates](int button, irr::EMOUSE_INPUT_EVENT type) {
			if(changed & button) {
				simulated.MouseInput.Event = (states & button) ? type : (irr::EMOUSE_INPUT_EVENT)(type + 3);
				device->postEventFromUser(simulated);
			}
		};

		CheckAndPost(JWrapper::Buttons::A, irr::EMIE_LMOUSE_PRESSED_DOWN);
		CheckAndPost(JWrapper::Buttons::B, irr::EMIE_RMOUSE_PRESSED_DOWN);
		CheckAndPost(JWrapper::Buttons::Y, irr::EMIE_MMOUSE_PRESSED_DOWN);

		moveVertical = (irr::f32)jevent.Axis[JWrapper::Axis::RIGHTY] / -32767.f;
		if(fabs(moveVertical) < DEAD_ZONE)
			moveVertical = 0.f;

		if(!irr::core::equals(moveVertical, 0.f)) {
			simulated.MouseInput.Wheel = moveVertical;
			simulated.MouseInput.Event = irr::EMIE_MOUSE_WHEEL;
			mainGame->device->postEventFromUser(simulated);
		}
		if(changed & JWrapper::Buttons::X && !(jevent.ButtonStates & JWrapper::Buttons::X)) {
			resizestate = (resizestate + 1) % 3;
			switch(resizestate) {
			case 0: {
				if(gGameConfig->fullscreen)
					GUIUtils::ToggleFullscreen(mainGame->device, gGameConfig->fullscreen);
				mainGame->device->restoreWindow();
				break;
			}
			case 1: {
				if(gGameConfig->fullscreen)
					GUIUtils::ToggleFullscreen(mainGame->device, gGameConfig->fullscreen);
				mainGame->device->maximizeWindow();
				break;
			}
			case 2: {
				if(!gGameConfig->fullscreen)
					GUIUtils::ToggleFullscreen(mainGame->device, gGameConfig->fullscreen);
				break;
			}
			}
		}
		return true;
	}
#if IRRLICHT_VERSION_MAJOR == 1 && IRRLICHT_VERSION_MINOR == 9
	case irr::EET_TOUCH_INPUT_EVENT: {
		if(event.TouchInput.touchedCount != 3)
			return false;
		if(event.TouchInput.Event != irr::ETIE_LEFT_UP)
			return false;
		auto window = mainGame->gSettings.window;
		if(window->isVisible())
			mainGame->HideElement(window);
		else
			mainGame->PopupElement(window);
		return true;
	}
#endif
	default: break;
	}
	return false;
}

irr::core::vector3df MouseToPlane(const irr::core::vector2d<irr::s32>& mouse, const irr::core::plane3d<irr::f32>& plane) {
	const auto collmanager = mainGame->smgr->getSceneCollisionManager();
	irr::core::line3df line = collmanager->getRayFromScreenCoordinates(mouse);
	irr::core::vector3d<irr::f32> startintersection;
	plane.getIntersectionWithLimitedLine(line.start, line.end, startintersection);
	return startintersection;
}

inline irr::core::vector3df MouseToField(const irr::core::vector2d<irr::s32>& mouse) {
	const auto& vec = matManager.getExtra()[0];
	return MouseToPlane(mouse, { vec[0].Pos, vec[1].Pos, vec[2].Pos });
}

bool CheckHand(const irr::core::vector2d<irr::s32>& mouse, const std::vector<ClientCard*>& hand) {
	if(hand.empty()) return false;
	irr::core::recti rect{ hand.front()->hand_collision.UpperLeftCorner, hand.back()->hand_collision.LowerRightCorner };
	if(!rect.isValid())
		rect = { hand.back()->hand_collision.UpperLeftCorner, hand.front()->hand_collision.LowerRightCorner };
	return rect.isPointInside(mouse);
}

void ClientField::GetHoverField(const irr::core::vector2d<irr::s32>& mouse) {
	const int three_columns = mainGame->dInfo.HasFieldFlag(DUEL_3_COLUMNS_FIELD);
	const int not_separate_pzones = !mainGame->dInfo.HasFieldFlag(DUEL_SEPARATE_PZONE);
	if(CheckHand(mouse, hand[0])) {
		hovered_controler = 0;
		hovered_location = LOCATION_HAND;
		for(auto it = hand[0].rbegin(); it != hand[0].rend(); it++) {
			if((*it)->hand_collision.isPointInside(mouse)) {
				hovered_sequence = (*it)->sequence;
				return;
			}
		}
		hovered_location = 0;
	} else if(CheckHand(mouse, hand[1])) {
		hovered_controler = 1;
		hovered_location = LOCATION_HAND;
		for(auto it = hand[1].begin(); it != hand[1].end(); it++) {
			if((*it)->hand_collision.isPointInside(mouse)) {
				hovered_sequence = (*it)->sequence;
				return;
			}
		}
		hovered_location = 0;
	} else {
		const auto coords = MouseToField(mouse);
		const auto& boardx = coords.X;
		const auto& boardy = coords.Y;
		hovered_location = 0;
		if(boardx >= matManager.getExtra()[0][0].Pos.X && boardx <= matManager.getExtra()[0][1].Pos.X) {
			if(boardy >= matManager.getExtra()[0][0].Pos.Y && boardy <= matManager.getExtra()[0][2].Pos.Y) {
				hovered_controler = 0;
				hovered_location = LOCATION_EXTRA;
			} else if(boardy >= matManager.getSzone()[0][5][0].Pos.Y && boardy <= matManager.getSzone()[0][5][2].Pos.Y) {//field
				hovered_controler = 0;
				hovered_location = LOCATION_SZONE;
				hovered_sequence = 5;
			} else if(!not_separate_pzones && boardy >= matManager.getSzone()[0][6][0].Pos.Y && boardy <= matManager.getSzone()[0][6][2].Pos.Y) {
				hovered_controler = 0;
				hovered_location = LOCATION_SZONE;
				hovered_sequence = 6;
			} else if(not_separate_pzones && boardy >= matManager.getRemove()[1][2].Pos.Y && boardy <= matManager.getRemove()[1][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_REMOVED;
			} else if(!not_separate_pzones && boardy >= matManager.getSzone()[1][7][2].Pos.Y && boardy <= matManager.getSzone()[1][7][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_SZONE;
				hovered_sequence = 7;
			} else if(boardy >= matManager.getGrave()[1][2].Pos.Y && boardy <= matManager.getGrave()[1][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_GRAVE;
			} else if(boardy >= matManager.getDeck()[1][2].Pos.Y && boardy <= matManager.getDeck()[1][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_DECK;
			} else if(not_separate_pzones && boardy >= matManager.getSkill()[0][0].Pos.Y && boardy <= matManager.getSkill()[0][2].Pos.Y) {
				hovered_controler = 0;
				hovered_location = LOCATION_SKILL;
			}
		} else if(!not_separate_pzones && boardx >= matManager.getRemove()[1][1].Pos.X && boardx <= matManager.getRemove()[1][0].Pos.X) {
			if(boardy >= matManager.getRemove()[1][2].Pos.Y && boardy <= matManager.getRemove()[1][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_REMOVED;
			} else if(boardy >= matManager.vFieldContiAct[three_columns][0].Y && boardy <= matManager.vFieldContiAct[three_columns][2].Y) {
				hovered_controler = 0;
				hovered_location = POSITION_HINT;
			} else if(boardy >= matManager.getSkill()[0][0].Pos.Y && boardy <= matManager.getSkill()[0][2].Pos.Y) {
				hovered_controler = 0;
				hovered_location = LOCATION_SKILL;
			}
		} else if(three_columns && boardx >= matManager.getSkill()[0][1].Pos.X && boardx <= matManager.getSkill()[0][2].Pos.X &&
				  boardy >= matManager.getSkill()[0][0].Pos.Y && boardy <= matManager.getSkill()[0][2].Pos.Y) {
			hovered_controler = 0;
			hovered_location = LOCATION_SKILL;
		} else if(not_separate_pzones && boardx >= matManager.getSzone()[1][7][1].Pos.X && boardx <= matManager.getSzone()[1][7][2].Pos.X) {
			if(boardy >= matManager.getSzone()[1][7][2].Pos.Y && boardy <= matManager.getSzone()[1][7][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_SZONE;
				hovered_sequence = 7;
			} else if(boardy >= matManager.vFieldContiAct[three_columns][0].Y && boardy <= matManager.vFieldContiAct[three_columns][2].Y) {
				hovered_controler = 0;
				hovered_location = POSITION_HINT;
			}
		} else if(boardx >= matManager.getDeck()[0][0].Pos.X && boardx <= matManager.getDeck()[0][1].Pos.X) {
			if(boardy >= matManager.getDeck()[0][0].Pos.Y && boardy <= matManager.getDeck()[0][2].Pos.Y) {
				hovered_controler = 0;
				hovered_location = LOCATION_DECK;
			} else if(boardy >= matManager.getGrave()[0][0].Pos.Y && boardy <= matManager.getGrave()[0][2].Pos.Y) {
				hovered_controler = 0;
				hovered_location = LOCATION_GRAVE;
			} else if(!not_separate_pzones && boardy >= matManager.getSzone()[1][6][2].Pos.Y && boardy <= matManager.getSzone()[1][6][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_SZONE;
				hovered_sequence = 6;
			} else if(!not_separate_pzones && boardy >= matManager.getSzone()[0][7][0].Pos.Y && boardy <= matManager.getSzone()[0][7][2].Pos.Y) {
				hovered_controler = 0;
				hovered_location = LOCATION_SZONE;
				hovered_sequence = 7;
			} else if(not_separate_pzones && boardy >= matManager.getRemove()[0][0].Pos.Y && boardy <= matManager.getRemove()[0][2].Pos.Y) {
				hovered_controler = 0;
				hovered_location = LOCATION_REMOVED;
			} else if(boardy >= matManager.getSzone()[1][5][2].Pos.Y && boardy <= matManager.getSzone()[1][5][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_SZONE;
				hovered_sequence = 5;
			} else if(boardy >= matManager.getExtra()[1][2].Pos.Y && boardy <= matManager.getExtra()[1][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_EXTRA;
			} else if(not_separate_pzones && boardy >= matManager.getSkill()[1][2].Pos.Y && boardy <= matManager.getSkill()[1][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_SKILL;
			}
		} else if(!three_columns && not_separate_pzones && boardx >= matManager.getSzone()[0][7][1].Pos.X && boardx <= matManager.getSzone()[0][7][0].Pos.X) {
			if(boardy >= matManager.getSzone()[0][7][0].Pos.Y && boardy <= matManager.getSzone()[0][7][2].Pos.Y) {
				hovered_controler = 0;
				hovered_location = LOCATION_SZONE;
				hovered_sequence = 7;
			}
		} else if(!not_separate_pzones && boardx >= matManager.getRemove()[0][0].Pos.X && boardx <= matManager.getRemove()[0][1].Pos.X) {
			if(boardy >= matManager.getRemove()[0][0].Pos.Y && boardy <= matManager.getRemove()[0][2].Pos.Y) {
				hovered_controler = 0;
				hovered_location = LOCATION_REMOVED;
			} else if(!not_separate_pzones && boardy >= matManager.getSkill()[1][2].Pos.Y && boardy <= matManager.getSkill()[1][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_SKILL;
			}
		} else if(not_separate_pzones && three_columns && boardx >= matManager.getSkill()[1][1].Pos.X && boardx <= matManager.getSkill()[1][0].Pos.X){
			if(boardy >= matManager.getSkill()[1][2].Pos.Y && boardy <= matManager.getSkill()[1][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_SKILL;
			}
		} else if(boardx >= matManager.vFieldMzone[0][0][0].Pos.X && boardx <= matManager.vFieldMzone[0][4][1].Pos.X) {
			int sequence = (boardx - matManager.vFieldMzone[0][0][0].Pos.X) / (matManager.vFieldMzone[0][0][1].Pos.X - matManager.vFieldMzone[0][0][0].Pos.X);
			if(sequence > 4)
				sequence = 4;
			if(three_columns && (sequence == 0 || sequence== 4))
				hovered_location = 0;
			else if(boardy > matManager.getSzone()[0][0][0].Pos.Y && boardy <= matManager.getSzone()[0][0][2].Pos.Y) {
				hovered_controler = 0;
				hovered_location = LOCATION_SZONE;
				hovered_sequence = sequence;
			} else if(boardy >= matManager.vFieldMzone[0][0][0].Pos.Y && boardy <= matManager.vFieldMzone[0][0][2].Pos.Y) {
				hovered_controler = 0;
				hovered_location = LOCATION_MZONE;
				hovered_sequence = sequence;
			} else if(boardy >= matManager.vFieldMzone[0][5][0].Pos.Y && boardy <= matManager.vFieldMzone[0][5][2].Pos.Y) {
				if(sequence == 1) {
					if(!mzone[1][6]) {
						hovered_controler = 0;
						hovered_location = LOCATION_MZONE;
						hovered_sequence = 5;
					} else {
						hovered_controler = 1;
						hovered_location = LOCATION_MZONE;
						hovered_sequence = 6;
					}
				} else if(sequence == 3) {
					if(!mzone[1][5]) {
						hovered_controler = 0;
						hovered_location = LOCATION_MZONE;
						hovered_sequence = 6;
					} else {
						hovered_controler = 1;
						hovered_location = LOCATION_MZONE;
						hovered_sequence = 5;
					}
				}
			} else if(boardy >= matManager.vFieldMzone[1][0][2].Pos.Y && boardy <= matManager.vFieldMzone[1][0][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_MZONE;
				hovered_sequence = 4 - sequence;
			} else if(boardy >= matManager.getSzone()[1][0][2].Pos.Y && boardy < matManager.getSzone()[1][0][0].Pos.Y) {
				hovered_controler = 1;
				hovered_location = LOCATION_SZONE;
				hovered_sequence = 4 - sequence;
			}
		}
	}
}
void ClientField::ShowMenu(int flag, int x, int y) {
	if(!flag) {
		mainGame->wCmdMenu->setVisible(false);
		return;
	}
	int height = mainGame->Scale(1);
	auto increase = mainGame->Scale(21);
	if(flag & COMMAND_ACTIVATE) {
		mainGame->btnActivate->setVisible(true);
		mainGame->btnActivate->setRelativePosition(irr::core::vector2di(1, height));
		height += increase;
	} else mainGame->btnActivate->setVisible(false);
	if(flag & COMMAND_SUMMON) {
		mainGame->btnSummon->setVisible(true);
		mainGame->btnSummon->setRelativePosition(irr::core::vector2di(1, height));
		height += increase;
	} else mainGame->btnSummon->setVisible(false);
	if(flag & COMMAND_SPSUMMON) {
		mainGame->btnSPSummon->setVisible(true);
		mainGame->btnSPSummon->setRelativePosition(irr::core::vector2di(1, height));
		height += increase;
	} else mainGame->btnSPSummon->setVisible(false);
	if(flag & COMMAND_MSET) {
		mainGame->btnMSet->setVisible(true);
		mainGame->btnMSet->setRelativePosition(irr::core::vector2di(1, height));
		height += increase;
	} else mainGame->btnMSet->setVisible(false);
	if(flag & COMMAND_SSET) {
		if(!(clicked_card->type & TYPE_MONSTER))
			mainGame->btnSSet->setText(gDataManager->GetSysString(1153).data());
		else
			mainGame->btnSSet->setText(gDataManager->GetSysString(1159).data());
		mainGame->btnSSet->setVisible(true);
		mainGame->btnSSet->setRelativePosition(irr::core::vector2di(1, height));
		height += increase;
	} else mainGame->btnSSet->setVisible(false);
	if(flag & COMMAND_REPOS) {
		if(clicked_card->position & POS_FACEDOWN)
			mainGame->btnRepos->setText(gDataManager->GetSysString(1154).data());
		else if(clicked_card->position & POS_ATTACK)
			mainGame->btnRepos->setText(gDataManager->GetSysString(1155).data());
		else
			mainGame->btnRepos->setText(gDataManager->GetSysString(1156).data());
		mainGame->btnRepos->setVisible(true);
		mainGame->btnRepos->setRelativePosition(irr::core::vector2di(1, height));
		height += increase;
	} else mainGame->btnRepos->setVisible(false);
	if(flag & COMMAND_ATTACK) {
		mainGame->btnAttack->setVisible(true);
		mainGame->btnAttack->setRelativePosition(irr::core::vector2di(1, height));
		height += increase;
	} else mainGame->btnAttack->setVisible(false);
	if(flag & COMMAND_LIST) {
		mainGame->btnShowList->setVisible(true);
		mainGame->btnShowList->setRelativePosition(irr::core::vector2di(1, height));
		height += increase;
	} else mainGame->btnShowList->setVisible(false);
	if(flag & COMMAND_OPERATION) {
		mainGame->btnOperation->setVisible(true);
		mainGame->btnOperation->setRelativePosition(irr::core::vector2di(1, height));
		height += increase;
	} else mainGame->btnOperation->setVisible(false);
	if(flag & COMMAND_RESET) {
		mainGame->btnReset->setVisible(true);
		mainGame->btnReset->setRelativePosition(irr::core::vector2di(1, height));
		height += increase;
	} else mainGame->btnReset->setVisible(false);
	panel = mainGame->wCmdMenu;
	mainGame->wCmdMenu->setVisible(true);
	irr::core::vector2di mouse = mainGame->Resize(x, y);
	x = mouse.X;
	y = mouse.Y;
	mainGame->wCmdMenu->setRelativePosition(irr::core::recti(x - mainGame->Scale(20), y - mainGame->Scale(20) - height, x + mainGame->Scale(80), y - mainGame->Scale(20)));
}
void ClientField::UpdateChainButtons(irr::gui::IGUIElement* caller) {
	if(!caller) {
		if(mainGame->ignore_chain || mainGame->always_chain || mainGame->chain_when_avail) {
			mainGame->btnChainIgnore->setPressed(mainGame->ignore_chain);
			mainGame->btnChainAlways->setPressed(mainGame->always_chain);
			mainGame->btnChainWhenAvail->setPressed(mainGame->chain_when_avail);
			return;
		}
		mainGame->btnChainIgnore->setPressed(mainGame->btnChainIgnore->isSubElement());
		mainGame->btnChainAlways->setPressed(mainGame->btnChainAlways->isSubElement());
		mainGame->btnChainWhenAvail->setPressed(mainGame->btnChainWhenAvail->isSubElement());
	} else {
		auto SetButton = [caller=(irr::gui::IGUIButton*)caller](irr::gui::IGUIButton* button) {
			const auto press = caller == button && caller->isPressed();
            ////kdiy////////
            if(press)
                button->setImage(mainGame->imageManager.tButtonpress);
            else
                button->setImage(mainGame->imageManager.tButton);
            ////kdiy////////
			button->setPressed(press);
			button->setSubElement(press);
		};
		SetButton(mainGame->btnChainIgnore);
		SetButton(mainGame->btnChainAlways);
		SetButton(mainGame->btnChainWhenAvail);
	}
}
void ClientField::ShowCancelOrFinishButton(int buttonOp) {
	if (!mainGame->dInfo.isReplay) {
		switch (buttonOp) {
		case 1:
			mainGame->btnCancelOrFinish->setText(gDataManager->GetSysString(1295).data());
			mainGame->btnCancelOrFinish->setVisible(true);
			break;
		case 2:
			mainGame->btnCancelOrFinish->setText(gDataManager->GetSysString(1296).data());
			mainGame->btnCancelOrFinish->setVisible(true);
			break;
		case 0:
		default:
			mainGame->btnCancelOrFinish->setVisible(false);
			break;
		}
	}
	else {
		mainGame->btnCancelOrFinish->setVisible(false);
	}
}
void ClientField::SetShowMark(ClientCard* pcard, bool enable) {
	if(pcard->equipTarget)
		pcard->equipTarget->is_showequip = enable;
	for(auto cit = pcard->equipped.begin(); cit != pcard->equipped.end(); ++cit)
		(*cit)->is_showequip = enable;
	for(auto cit = pcard->cardTarget.begin(); cit != pcard->cardTarget.end(); ++cit)
		(*cit)->is_showtarget = enable;
	for(auto cit = pcard->ownerTarget.begin(); cit != pcard->ownerTarget.end(); ++cit)
		(*cit)->is_showtarget = enable;
	for(auto chit = chains.begin(); chit != chains.end(); ++chit) {
		if(pcard == chit->chain_card) {
			for(auto tgit = chit->target.begin(); tgit != chit->target.end(); ++tgit)
				(*tgit)->is_showchaintarget = enable;
		}
		if(chit->target.find(pcard) != chit->target.end())
			chit->chain_card->is_showchaintarget = enable;
	}
}
void ClientField::ShowCardInfoInList(ClientCard* pcard, irr::gui::IGUIElement* element, irr::gui::IGUIElement* parent) {
	std::wstring str(L"");
	if(pcard->code) {
		////kdiy///////////
		//str.append(gDataManager->GetName(pcard->code).data());
		str.append(gDataManager->GetName(pcard).data());
		////kdiy///////////
	}
	if((pcard->status & STATUS_PROC_COMPLETE)
		&& (pcard->type & (TYPE_RITUAL | TYPE_FUSION | TYPE_SYNCHRO | TYPE_XYZ | TYPE_LINK | TYPE_SPSUMMON)))
		str.append(L"\n").append(gDataManager->GetSysString(224).data());
	for(size_t i = 0; i < chains.size(); ++i) {
		auto chit = chains[i];
		if(pcard == chit.chain_card) {
			str.append(L"\n").append(epro::sprintf(gDataManager->GetSysString(216), i + 1));
		}
		if(chit.target.find(pcard) != chit.target.end()) {
			////kdiy///////////
			//str.append(L"\n").append(epro::sprintf(gDataManager->GetSysString(217), i + 1, gDataManager->GetName(chit.chain_card->code)));
			str.append(L"\n").append(epro::sprintf(gDataManager->GetSysString(217), i + 1, gDataManager->GetName(chit.chain_card)));
			////kdiy///////////
		}
	}
	if(str.length() > 0) {
		parent->addChild(mainGame->stCardListTip);
		irr::core::recti ePos = element->getRelativePosition();
		irr::s32 x = (ePos.UpperLeftCorner.X + ePos.LowerRightCorner.X) / 2;
		irr::s32 y = ePos.LowerRightCorner.Y;
		mainGame->stCardListTip->setText(str.data());
		auto dTip = mainGame->guiFont->getDimension(mainGame->stCardListTip->getText()) + mainGame->Scale(irr::core::dimension2d<uint32_t>(10, 10));
		irr::s32 w = dTip.Width / 2;
		if(x - w < mainGame->Scale(10))
			x = w + mainGame->Scale(10);
		if(x + w > mainGame->Scale(670))
			x = mainGame->Scale(670) - w;
		mainGame->stCardListTip->setRelativePosition(irr::core::recti(x - w, y - mainGame->Scale(10), x + w, y - mainGame->Scale(10) + dTip.Height));
		mainGame->stCardListTip->setVisible(true);
	}
}
static int GetSuitableReturn(uint32_t maxseq, uint32_t size) {
	using nl8 = std::numeric_limits<uint8_t>;
	using nl16 = std::numeric_limits<uint16_t>;
	using nl32 = std::numeric_limits<uint32_t>;
	if(maxseq < nl8::max()) {
		if(maxseq >= size * nl8::digits)
			return 2;
	} else if(maxseq < nl16::max()) {
		if(maxseq >= size * nl16::digits)
			return 1;
	}
	else if(maxseq < nl32::max()) {
		if(maxseq >= size * nl32::digits)
			return 0;
	}
	return 3;
}
template<typename T>
static inline void WriteCard(ProgressiveBuffer& buffer, uint32_t i, uint32_t value) {
	static constexpr auto off = 8 >> (sizeof(T) / 2);
	buffer.set<T>(i + off, static_cast<T>(value));
}
void ClientField::SetResponseSelectedCards() const {
	if (!mainGame->dInfo.compat_mode) {
		if(mainGame->dInfo.curMsg == MSG_SELECT_UNSELECT_CARD) {
			uint32_t respbuf[] = { 1, selected_cards[0]->select_seq };
			DuelClient::SetResponseB(respbuf, sizeof(respbuf));
		} else {
			uint32_t maxseq = 0;
			uint32_t size = static_cast<uint32_t>(selected_cards.size());
			for(auto& c : selected_cards) {
				maxseq = std::max(maxseq, c->select_seq);
			}
			ProgressiveBuffer ret;
			switch(GetSuitableReturn(maxseq, size)) {
				case 3: {
					ret.set<int32_t>(0, 3);
					for(auto c : selected_cards)
						ret.bitToggle(c->select_seq + (sizeof(int32_t) * 8), true);
					break;
				}
				case 2:	{
					ret.set<int32_t>(0, 2);
					ret.set<uint32_t>(1, size);
					for(uint32_t i = 0; i < size; ++i)
						WriteCard<uint8_t>(ret, i, selected_cards[i]->select_seq);
					break;
				}
				case 1:	{
					ret.set<int32_t>(0, 1);
					ret.set<uint32_t>(1, size);
					for(uint32_t i = 0; i < size; ++i)
						WriteCard<uint16_t>(ret, i, selected_cards[i]->select_seq);
					break;
				}
				case 0:	{
					ret.set<int32_t>(0, 0);
					ret.set<uint32_t>(1, size);
					for(uint32_t i = 0; i < size; ++i)
						WriteCard<uint32_t>(ret, i, selected_cards[i]->select_seq);
					break;
				}
				default:
					unreachable();
			}
			DuelClient::SetResponseB(ret.data.data(), ret.data.size());
		}
	} else {
		uint8_t respbuf[64];
		respbuf[0] = static_cast<uint8_t>(selected_cards.size() + must_select_cards.size());
		auto offset = must_select_cards.size() + 1;
		for(size_t i = 0; i < selected_cards.size(); ++i)
			respbuf[i + offset] = static_cast<uint8_t>(selected_cards[i]->select_seq);
		DuelClient::SetResponseB(respbuf, selected_cards.size() + must_select_cards.size() + 1);
	}
}
void ClientField::SetResponseSelectedOption() const {
	if(mainGame->dInfo.curMsg == MSG_SELECT_OPTION) {
		DuelClient::SetResponseI(static_cast<uint32_t>(selected_option));
	} else {
		int index = 0;
		while(activatable_cards[index] != command_card || activatable_descs[index].first != select_options[selected_option]) index++;
		if(mainGame->dInfo.curMsg == MSG_SELECT_IDLECMD) {
			DuelClient::SetResponseI((index << 16) + 5);
		} else if(mainGame->dInfo.curMsg == MSG_SELECT_BATTLECMD) {
			DuelClient::SetResponseI(index << 16);
		} else {
			DuelClient::SetResponseI(index);
		}
	}
	mainGame->HideElement(mainGame->wOptions, true);
}
void ClientField::CancelOrFinish() {
	if(mainGame->dInfo.checkRematch) {
		mainGame->dInfo.checkRematch = false;
		mainGame->HideElement(mainGame->wQuery);
		CTOS_RematchResponse crr;
		crr.rematch = false;
		DuelClient::SendPacketToServer(CTOS_REMATCH_RESPONSE, crr);
		return;
	}
    //////////kdiy/////////
    for(int i = 0; i < 5; ++i)
        mainGame->selectedcard[i]->setVisible(false);
    //////////kdiy/////////
	switch (mainGame->dInfo.curMsg) {
	case MSG_WAITING: {
		if (mainGame->wCardSelect->isVisible()) {
			mainGame->HideElement(mainGame->wCardSelect);
			ShowCancelOrFinishButton(0);
		}
		break;
	}
	case MSG_SELECT_BATTLECMD: {
		if (mainGame->wCardSelect->isVisible()) {
			mainGame->HideElement(mainGame->wCardSelect);
			ShowCancelOrFinishButton(0);
		}
		if (mainGame->wOptions->isVisible()) {
			mainGame->HideElement(mainGame->wOptions);
			ShowCancelOrFinishButton(0);
		}
		break;
	}
	case MSG_SELECT_IDLECMD: {
		if (mainGame->wCardSelect->isVisible()) {
			mainGame->HideElement(mainGame->wCardSelect);
			ShowCancelOrFinishButton(0);
		}
		if (mainGame->wOptions->isVisible()) {
			mainGame->HideElement(mainGame->wOptions);
			ShowCancelOrFinishButton(0);
		}
		break;
	}
	case MSG_SELECT_YESNO:
	case MSG_SELECT_EFFECTYN: {
		if (highlighting_card)
			////kdiy///////////
			//highlighting_card->is_highlighting = false;
			highlighting_card->is_activable = false;
			////kdiy///////////
		highlighting_card = 0;
		////kdiy///////////
        if(mainGame->dField.attacker && mainGame->dField.attacker->is_attack) {
            	mainGame->dField.attacker->is_attack = false;
            mainGame->dField.attacker->curRot = mainGame->dField.attacker->attRot;
        }
        mainGame->dField.attacker->is_attacked = false;
		////kdiy///////////
		DuelClient::SetResponseI(0);
		mainGame->HideElement(mainGame->wQuery, true);
		break;
	}
	case MSG_SELECT_CARD:
	case MSG_SELECT_TRIBUTE: {
		if (selected_cards.size() == 0) {
			if(select_cancelable) {
				DuelClient::SetResponseI(-1);
				ShowCancelOrFinishButton(0);
				if(mainGame->wCardSelect->isVisible())
					mainGame->HideElement(mainGame->wCardSelect, true);
				else
					DuelClient::SendResponse();
			}
			break;
		}
		if (mainGame->wQuery->isVisible()) {
			SetResponseSelectedCards();
			ShowCancelOrFinishButton(0);
			mainGame->HideElement(mainGame->wQuery, true);
			break;
		}
		if (select_ready) {
			SetResponseSelectedCards();
			ShowCancelOrFinishButton(0);
			if (mainGame->wCardSelect->isVisible())
				mainGame->HideElement(mainGame->wCardSelect, true);
			else
				DuelClient::SendResponse();
		}
		break;
	}
	case MSG_SELECT_UNSELECT_CARD: {
		if(select_cancelable) {
			DuelClient::SetResponseI(-1);
			ShowCancelOrFinishButton(0);
			if(mainGame->wCardSelect->isVisible())
				mainGame->HideElement(mainGame->wCardSelect, true);
			else
				DuelClient::SendResponse();
		}
		break;
	}
	case MSG_SELECT_SUM: {
		if(select_ready) {
			SetResponseSelectedCards();
			ShowCancelOrFinishButton(0);
			if (mainGame->wCardSelect->isVisible())
				mainGame->HideElement(mainGame->wCardSelect, true);
			else
				DuelClient::SendResponse();
			break;
		}
		break;
	}
	case MSG_SELECT_CHAIN: {
		if (chain_forced)
			break;
		if (mainGame->wCardSelect->isVisible()) {
			mainGame->HideElement(mainGame->wCardSelect);
			break;
		}
		if (mainGame->wQuery->isVisible()) {
			DuelClient::SetResponseI(-1);
			ShowCancelOrFinishButton(0);
			mainGame->HideElement(mainGame->wQuery, true);
		}
		else {
			mainGame->PopupElement(mainGame->wQuery);
			ShowCancelOrFinishButton(0);
		}
		if (mainGame->wOptions->isVisible()) {
			DuelClient::SetResponseI(-1);
			ShowCancelOrFinishButton(0);
			mainGame->HideElement(mainGame->wOptions);
		}
		break;
	}
	case MSG_SORT_CHAIN:
	case MSG_SORT_CARD: {
		if (mainGame->wCardSelect->isVisible()) {
			DuelClient::SetResponseI(-1);
			mainGame->HideElement(mainGame->wCardSelect, true);
		}
		break;
	}
	}
}
void ClientField::ShowPileDisplayCards(int location, int player) {
	int loc_id = 0;
	switch(location) {
	case LOCATION_DECK:
		loc_id = 1000;
		display_cards.assign(deck[player].crbegin(), deck[player].crend());
		break;
	case LOCATION_GRAVE:
		loc_id = 1004;
		display_cards.assign(grave[player].crbegin(), grave[player].crend());
		break;
	case LOCATION_REMOVED:
		loc_id = 1005;
		display_cards.assign(remove[player].crbegin(), remove[player].crend());
		break;
	case LOCATION_EXTRA:
		loc_id = 1006;
		display_cards.assign(extra[player].crbegin(), extra[player].crend());
		break;
	case LOCATION_OVERLAY:
		loc_id = 1007;
		display_cards.clear();
		for(const auto& pcard : mzone[player]) {
			if(pcard)
				display_cards.insert(display_cards.end(), pcard->overlayed.begin(), pcard->overlayed.end());
		}
		//kdiy/////////
		for(const auto& pcard : szone[player]) {
			if(pcard)
				display_cards.insert(display_cards.end(), pcard->overlayed.begin(), pcard->overlayed.end());
		}
		//kdiy/////////
		break;
	}
	if(display_cards.size()) {
		mainGame->wCardDisplay->setText(epro::format(L"{}({})", gDataManager->GetSysString(loc_id), display_cards.size()).data());
		ShowLocationCard();
	}
}
void ClientField::SendRPSResult(uint8_t i) {
	assert((1 <= i) && (i <= 3));
	if(mainGame->dInfo.curMsg == MSG_ROCK_PAPER_SCISSORS) {
		DuelClient::SetResponseI(i);
		DuelClient::SendResponse();
	} else {
		mainGame->stHintMsg->setText(L"");
		mainGame->stHintMsg->setVisible(true);
		CTOS_HandResult cshr;
		cshr.res = i;
		DuelClient::SendPacketToServer(CTOS_HAND_RESULT, cshr);
	}
}
}
