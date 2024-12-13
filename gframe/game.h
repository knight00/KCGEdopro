#ifndef GAME_H
#define GAME_H

#include <unordered_map>
#include <vector>
#include <list>
#include <atomic>
#include "materials.h"
#include "settings_window.h"
#include "config.h"
#include "common.h"
#include "mysignal.h"
#include <SColor.h>
#include <rect.h>
#include <EGUIElementTypes.h>
#include "image_manager.h"
#include "client_field.h"
#include "deck_con.h"
#include "menu_handler.h"
#include "discord_wrapper.h"
#include "windbot_panel.h"
#include "ocgapi_types.h"
/////kdiy/////
#include "client_card.h"
#include "network.h"
/////ktest//////
//#include <opencv2/opencv.hpp>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}
#include <sfAudio/Audio.hpp>
/////kdiy/////
struct unzip_payload;
class CGUISkinSystem;
class IProgressBar;
namespace irr {
	class IrrlichtDevice;
	namespace gui {
		class IGUIElement;
		class CGUIFileSelectListBox;
		class CGUITTFont;
		class CGUIImageButton;
		class CGUIWindowedTabControl;
		class Panel;
		class IGUIComboBox;
		class IGUIContextMenu;
		class IGUIEditBox;
		class IGUITabControl;
		class IGUIEnvironment;
		class IGUITable;
		class IGUITab;
		class IGUIScrollBar;
		class IGUIListBox;
		class IGUIButton;
		class IGUIScrollBar;
		class IGUIImage;
	}
	namespace video {
		class IVideoDriver;
		struct S3DVertex;
		class ITexture;
	}
	namespace io {
		class IFileSystem;
	}
	namespace scene {
		class ISceneManager;
		class ICameraSceneNode;
	}
}
namespace ygo {

class GitRepo;

struct DuelInfo {
	bool isInDuel;
	bool isStarted;
	bool isReplay;
	bool isOldReplay;
	bool isCatchingUp;
	bool checkRematch;
	bool isFirst;
	bool isTeam1;
	bool isRelay;
	bool isInLobby;
	bool isSingleMode;
	bool isHandTest;
	bool compat_mode;
	bool legacy_race_size;
	bool is_shuffling;
	int current_player[2];
	int lp[2];
	int startlp;
	int duel_field;
	uint64_t duel_params;
	int turn;
	uint8_t curMsg;
	int team1;
	int team2;
	int best_of;
	std::vector<std::wstring> selfnames;
	std::vector<std::wstring> opponames;
	std::wstring strLP[2];
	std::wstring vic_string;
	uint8_t player_type;
	uint8_t time_player;
	uint16_t time_limit;
	uint16_t time_left[2];
	DiscordWrapper::DiscordSecret secret;
	bool isReplaySwapped;
	bool HasFieldFlag(uint64_t flag) const {
		return (flag & duel_params) == flag;
	}
	uint8_t GetPzoneIndex(uint8_t seq) const {
		if(seq > 1)
			return 0;
		if(HasFieldFlag(DUEL_SEPARATE_PZONE)) // 6 and 7
			return seq + 6;
		if(HasFieldFlag(DUEL_3_COLUMNS_FIELD))// 1 and 3
			return seq * 2 + 1;
		return seq * 4;					      // 0 and 4
	}
};

struct FadingUnit {
	bool signalAction;
	bool isFadein;
	float fadingFrame;
	int autoFadeoutFrame;
	irr::gui::IGUIElement* guiFading;
	irr::core::recti fadingSize;
	irr::core::vector2di fadingUL;
	irr::core::vector2di fadingLR;
	irr::core::vector2di fadingDest;
	bool wasEnabled;
};
/////kdiy/////
class Mode {
public:
	struct ModeText {//the text of mode meun
		std::wstring name;
		std::wstring des;
	};
	struct ModePloat {//the ploat text of mode-story
		uint8_t index;
		int control = 0;
        uint8_t icon = 0;
		std::wstring title = L"";
		std::wstring ploat = L"";
        bool isStartEvent = true;
        bool isStartDuel = false;
        bool isWinDuel = false;
        bool isLoseDuel = false;
        bool sextramonster = false;
        bool smonster = false;
        bool activate = false;
        bool attackeff = false;
        uint32_t code = 0;
	};
	std::vector<ModeText>* modeTexts;//vector modetext
	std::vector<ModePloat>* modePloats[CHAPTER];//vector modeploat
	std::vector<WindBot> bots;//all mode will load windbots from this
    std::map<uint8_t, std::wstring> playerNames;
	std::map<uint8_t, std::vector<std::wstring>> aiNames;//player+bot names when duel start
    std::map<uint8_t, std::wstring> aideck;
    std::map<uint8_t, std::vector<uint8_t>> iconlist;
    std::map<uint8_t, uint8_t> storyrule;
    std::map<uint8_t, bool> rush;
    std::map<uint8_t, int32_t> team1;
    std::map<uint8_t, int32_t> team2;
    std::map<uint8_t, bool> relay;
    std::map<uint8_t, uint32_t> lua;
	int modeIndex;//decide to play what kind of mode rule,from meun-list getSelected
	bool isMode;//the duel is mode?
	bool isPlot;//is showing ploat, ignore mouse/keyboard inputs
    bool isModeEvent;
	bool isStartEvent;//ploating, locking flow, allow left mouse click notify_one() to skip continuous ploat
	bool isStartDuel, isDuelEnd, isStoryStart;
	bool flag_100000155;//card 100000155 play sound
	uint8_t rule;//the rule of duel,zcg|story......
    uint8_t chapter;//story chapter
	uint8_t plotStep;//0: initial ploat, 1: normal ploat, 2: stop ploat
 	uint8_t plotIndex;//the index of plot,decide to set text plot
	uint8_t character[6] = { 0,0,0,0,0,0 };
	void InitializeMode();
	void DestoryMode();
	void RefreshEntertainmentPlay(std::vector<ModeText>* modeTexts);
	void RefreshControlState(uint8_t state);
	void SetControlState(uint8_t index);
	void ModePlayerReady(bool isAi);
	void SetRule(uint8_t index);
	bool LoadWindBot(int port, epro::wstringview pass);
	bool IsModeBot(std::wstring mode);
	void NextPlot(uint8_t step = 0, uint32_t code = 0); //step: plotStep, index: ploat.json index
    void PlayNextPlot(uint32_t code);
	std::wstring GetPloat(uint32_t code = 0);
	Mode();
	void LoadJsonInfo();
	~Mode();
private:
	void SetCurrentDeck();
	bool LoadJson(epro::path_string path, uint8_t index, uint8_t chapter = 0);
};
/////kdiy/////

struct info_panel_elements {
	//card image
	irr::gui::IGUIStaticText* wCardImg;
	irr::gui::IGUIImage* imgCard;
	//infos
	int infosExpanded; //0: not expanded, 1: expanded and to be shown as expanded, 2: expanded but not to be shown as expanded
	irr::gui::IGUITabControl* wInfos;
	irr::gui::IGUIStaticText* stName;
	irr::gui::IGUIStaticText* stInfo;
	irr::gui::IGUIStaticText* stDataInfo;
	irr::gui::IGUIStaticText* stSetName;
	irr::gui::IGUIStaticText* stPasscodeScope;
	irr::gui::IGUIStaticText* stText;

	irr::gui::IGUITab* tabLog;
	irr::gui::IGUIListBox* lstLog;
	irr::gui::IGUITab* tabChat;
	irr::gui::IGUIListBox* lstChat;
	irr::gui::IGUIButton* btnClearLog;
	irr::gui::IGUIButton* btnExpandLog;
	irr::gui::IGUIButton* btnClearChat;
	irr::gui::IGUIButton* btnExpandChat;
	irr::gui::IGUITab* tabRepositories;
	irr::gui::IGUIContextMenu* mTabRepositories;
	irr::gui::Panel* tabSystem;
	SettingsPane tabSettings;
	irr::gui::IGUIButton* btnTabShowSettings;
};

struct main_menu_panel_elements {
	//main menu
	int mainMenuLeftX;
	int mainMenuRightX;
	irr::gui::IGUIWindow* wMainMenu;
	/////kdiy/////////
	irr::gui::IGUIWindow* wQQ;
	irr::gui::CGUIImageButton* btnQQ;
	irr::gui::IGUIButton* btnQQ2;
	irr::gui::IGUIButton* btnQQ22;
	irr::gui::IGUIButton* btnSettings2;
	/////kdiy/////////
	irr::gui::IGUIWindow* wCommitsLog;
	irr::gui::IGUIContextMenu* mTopMenu;
	irr::gui::IGUIContextMenu* mRepositoriesInfo;
	irr::gui::IGUIContextMenu* mAbout;
	irr::gui::IGUIWindow* wAbout;
	irr::gui::IGUIStaticText* stAbout;
	irr::gui::IGUIContextMenu* mVersion;
	irr::gui::IGUIWindow* wVersion;
	irr::gui::IGUIStaticText* stVersion;
	irr::gui::IGUIStaticText* stCoreVersion;
	irr::gui::IGUIStaticText* stExpectedCoreVersion;
	irr::gui::IGUIStaticText* stCompatVersion;
	irr::gui::IGUIButton* btnOnlineMode;
	irr::gui::IGUIButton* btnLanMode;
	irr::gui::IGUIButton* btnSingleMode;
	irr::gui::IGUIButton* btnReplayMode;
	irr::gui::IGUIButton* btnTestMode;
	irr::gui::IGUIButton* btnDeckEdit;
	irr::gui::IGUIButton* btnModeExit;
	irr::gui::IGUIButton* btnCommitLogExit;
	irr::gui::IGUIStaticText* stCommitLog;
	irr::gui::IGUICheckBox* chkCommitLogExpand;
};

struct lan_panel_elements {
	//lan
	irr::gui::IGUIWindow* wLanWindow;
	irr::gui::IGUIEditBox* ebNickName;
	irr::gui::IGUIListBox* lstHostList;
	irr::gui::IGUIButton* btnLanRefresh;
	irr::gui::IGUIEditBox* ebJoinHost;
	irr::gui::IGUIEditBox* ebJoinPort;
	irr::gui::IGUIEditBox* ebJoinPass;
	irr::gui::IGUIButton* btnJoinHost;
	irr::gui::IGUIButton* btnJoinCancel;
	irr::gui::IGUIButton* btnCreateHost;
};

struct host_creation_panel_elements {
	//create host
	irr::gui::CGUIWindowedTabControl* wtcCreateHost;
	irr::gui::IGUIWindow* wCreateHost;
	irr::gui::IGUIComboBox* cbHostLFList;
	irr::gui::IGUIButton* btnRelayMode;
	irr::gui::IGUIComboBox* cbMatchMode;
	irr::gui::IGUIComboBox* cbRule;
	irr::gui::IGUIEditBox* ebTimeLimit;
	irr::gui::IGUIEditBox* ebTeam1;
	irr::gui::IGUIEditBox* ebTeam2;
	irr::gui::IGUIEditBox* ebBestOf;
	irr::gui::IGUIEditBox* ebOnlineTeam1;
	irr::gui::IGUIEditBox* ebOnlineTeam2;
	irr::gui::IGUIEditBox* ebOnlineBestOf;
	irr::gui::IGUIEditBox* ebStartLP;
	irr::gui::IGUIEditBox* ebStartHand;
	irr::gui::IGUIEditBox* ebDrawCount;
	irr::gui::IGUIEditBox* ebServerName;
	irr::gui::IGUIEditBox* ebServerPass;
	irr::gui::IGUIButton* btnRuleCards;
	irr::gui::IGUIWindow* wRules;
	/////kdiy///////
	//irr::gui::IGUICheckBox* chkRules[13];
	irr::gui::IGUICheckBox* chkRules[15];
	/////kdiy///////
	irr::gui::IGUIButton* btnRulesOK;
	irr::gui::IGUIComboBox* cbDuelRule;
	irr::gui::IGUICheckBox* chkCustomRules[7+12+8+2];
	irr::gui::IGUICheckBox* chkTypeLimit[5];
	irr::gui::IGUICheckBox* chkNoCheckDeckContent;
	irr::gui::IGUICheckBox* chkNoCheckDeckSize;
	irr::gui::IGUICheckBox* chkNoShuffleDeck;
	irr::gui::IGUICheckBox* chkTcgRulings;
	irr::gui::IGUIButton* btnHostConfirm;
	irr::gui::IGUIButton* btnHostCancel;
	irr::gui::IGUIStaticText* stHostPort;
	irr::gui::IGUIEditBox* ebHostPort;
	irr::gui::IGUIStaticText* stHostNotes;
	irr::gui::IGUIEditBox* ebHostNotes;
	irr::gui::IGUIStaticText* stVersus;
	irr::gui::IGUIStaticText* stBestof;
	///////kdiy//////////
	irr::gui::IGUIWindow* wCreateHost2;
	irr::gui::IGUICheckBox* chkdefaultlocal;
	irr::gui::IGUICheckBox* chkAI;
	irr::gui::IGUIComboBox* cbHostLFList2;
	irr::gui::IGUIComboBox* cbRule2;
	irr::gui::IGUIEditBox* ebTimeLimit2;
	irr::gui::IGUIEditBox* ebStartLP2;
	irr::gui::IGUIEditBox* ebStartHand2;
	irr::gui::IGUIEditBox* ebDrawCount2;
	irr::gui::IGUIEditBox* ebJoinPass2;
	irr::gui::IGUIComboBox* cbDuelRule2;
	irr::gui::IGUICheckBox* chkNoCheckDeckContent2;
	irr::gui::IGUICheckBox* chkNoShuffleDeck2;
	irr::gui::IGUICheckBox* chkNoLFlist2;
	irr::gui::IGUICheckBox* chkTag;
	irr::gui::IGUICheckBox* chkMatch;
	irr::gui::IGUIButton* btnSimpleJoinHost;
	irr::gui::IGUIButton* btnHostConfirm2;
	irr::gui::IGUIButton* btnHostCancel2;
	uint8_t character[6] = {0,0,0,0,0,0};
	uint8_t choose_player = -1; //0-5th players
	irr::gui::IGUIWindow* wCharacter;
	irr::gui::CGUIImageButton* btnCharacter;
	irr::gui::CGUIImageButton* btnCharacterSelect;
	irr::gui::CGUIImageButton* btnCharacterSelect2;
    irr::gui::IGUIWindow* wCharacterReplay;
	irr::gui::IGUIButton* btnCharacterSelect_replay;
	irr::gui::CGUIImageButton* btnCharacter_replay;
	irr::gui::CGUIImageButton* btnCharacterSelect1_replay;
	irr::gui::CGUIImageButton* btnCharacterSelect2_replay;
    irr::gui::IGUIButton* btnCharacterSelect_replayclose;
    irr::gui::IGUIButton* btnCharacterSelect_replayreset[6];
    irr::gui::IGUIComboBox* ebCharacter_replay[6];
    irr::gui::IGUIEditBox* ebName_replay[6];
	irr::gui::IGUIStaticText* stCharacterReplay;
	irr::gui::IGUIWindow* wAvatar[2];
	irr::gui::CGUIImageButton* avatarbutton[2];
	irr::gui::IGUIButton* cardbutton[3];
    int avataricon1 = 0;
    int avataricon2 = 0;
	///////kdiy//////////
	//deck options
	irr::gui::IGUICheckBox* chkNoCheckDeckContentSecondary;
	irr::gui::IGUICheckBox* chkNoCheckDeckSizeSecondary;
	irr::gui::IGUICheckBox* chkNoShuffleDeckSecondary;
	irr::gui::IGUIEditBox* ebMainMin;
	irr::gui::IGUIEditBox* ebMainMax;
	irr::gui::IGUIEditBox* ebExtraMin;
	irr::gui::IGUIEditBox* ebExtraMax;
	irr::gui::IGUIEditBox* ebSideMin;
	irr::gui::IGUIEditBox* ebSideMax;
};

struct host_panel_elements {
	//host panel
	irr::gui::IGUIWindow* wHostPrepare;
	irr::gui::IGUIWindow* wHostPrepareR;
	irr::gui::IGUIWindow* wHostPrepareL;
	WindBotPanel gBot;
	irr::gui::IGUIStaticText* stHostCardRule;
	irr::gui::IGUIButton* btnHostPrepDuelist;
	irr::gui::IGUIButton* btnHostPrepWindBot;
	irr::gui::IGUIButton* btnHostPrepOB;
	irr::gui::IGUIStaticText* stHostPrepDuelist[6];
	irr::gui::IGUICheckBox* chkHostPrepReady[6];
	irr::gui::IGUIButton* btnHostPrepKick[6];
	irr::gui::IGUIComboBox* cbDeckSelect;
	irr::gui::IGUIStaticText* stHostPrepRule;
	irr::gui::IGUIStaticText* stHostPrepRuleR;
	irr::gui::IGUIStaticText* stHostPrepRuleL;
	irr::gui::IGUIStaticText* stHostPrepOB;
	irr::gui::IGUIStaticText* stDeckSelect;
	//////////kdiy/////////
	irr::gui::IGUIButton* selectedcard[5];
    irr::gui::IGUIComboBox* ebCharacter[6];
	irr::gui::IGUIComboBox* ebCharacterDeck;
    irr::gui::IGUIComboBox* cbDBDecks2;
    irr::gui::IGUIComboBox* cbDBDecks22;
	irr::gui::IGUICheckBox* chkHandTestOpponentDeck;
    irr::gui::IGUIComboBox* cbHandTestDecks;
    irr::gui::IGUIComboBox* cbHandTestDecks2;
	irr::gui::CGUIImageButton* icon[6];
    irr::gui::CGUIImageButton* icon2[6];
	irr::gui::IGUIComboBox* cbDeck2Select;
    irr::gui::IGUIButton* btnPicDL;
	irr::gui::IGUIButton* btnMovieDL;
    irr::gui::IGUIButton* btnSoundDL;
	irr::gui::IGUIButton* btnIntro;
	irr::gui::IGUIButton* btnTut;
	irr::gui::IGUIButton* btnTut2;
	irr::gui::IGUIButton* btHome;
	irr::gui::IGUIButton* btnFolder;
    irr::gui::IGUIStaticText* stpics;
	irr::gui::IGUIComboBox* cbpics;
	//////////kdiy/////////
	irr::gui::IGUIButton* btnHostPrepReady;
	irr::gui::IGUIButton* btnHostPrepNotReady;
	irr::gui::IGUIButton* btnHostPrepStart;
	irr::gui::IGUIButton* btnHostPrepCancel;
};

struct replay_panel_elements {
	//replay
	irr::gui::IGUIWindow* wReplay;
	irr::gui::CGUIFileSelectListBox* lstReplayList;
	irr::gui::IGUIStaticText* stReplayInfo;
	irr::gui::IGUICheckBox* chkYrp;
	irr::gui::IGUIButton* btnLoadReplay;
	irr::gui::IGUIButton* btnDeleteReplay;
	irr::gui::IGUIButton* btnRenameReplay;
	irr::gui::IGUIButton* btnReplayCancel;
	irr::gui::IGUIButton* btnExportDeck;
	irr::gui::IGUIButton* btnShareReplay;
	irr::gui::IGUIEditBox* ebRepStartTurn;
};

struct puzzle_panel_elements {
	//puzzle mode
	irr::gui::IGUIWindow* wSinglePlay;
	irr::gui::CGUIFileSelectListBox* lstSinglePlayList;
	irr::gui::IGUIStaticText* stSinglePlayInfo;
	irr::gui::IGUIButton* btnLoadSinglePlay;
	irr::gui::IGUIButton* btnDeleteSinglePlay;
	irr::gui::IGUIButton* btnRenameSinglePlay;
	irr::gui::IGUIButton* btnOpenSinglePlay;
	irr::gui::IGUIButton* btnShareSinglePlay;
	irr::gui::IGUIButton* btnSinglePlayCancel;
	//////////kdiy/////////
	Mode* mode = new Mode();
	irr::gui::IGUICheckBox* chkEntertainmentPrepReady;
	irr::gui::IGUICheckBox* chkEntertainmentMode_1Check;
	irr::gui::IGUIComboBox* cbEntertainmentMode_1Bot;
	irr::gui::IGUIWindow* wEntertainmentPlay;
	irr::gui::CGUIFileSelectListBox* lstEntertainmentPlayList;
	irr::gui::IGUIStaticText* stEntertainmentPlayInfo;
	irr::gui::IGUIButton* btnEntertainmentMode;
	irr::gui::IGUIButton* btnEntertainmentStartGame;
	irr::gui::IGUIButton* btnEntertainmentExitGame;

	irr::gui::IGUIWindow* wBody;
	irr::gui::CGUIImageButton* btnBody;

	irr::gui::IGUIWindow* wPloat;
	irr::gui::IGUIStaticText* stPloatInfo;
	irr::gui::IGUIButton* btnPloat;

	irr::gui::IGUIWindow* wChPloatBody[2];
	irr::gui::IGUIStaticText* stChPloatInfo[2];
    //////////kdiy/////////
};

struct deck_edit_page_elements {
	//deck edit
	irr::gui::IGUIStaticText* wDeckEdit;
	irr::gui::IGUIComboBox* cbDBLFList;
	irr::gui::IGUIComboBox* cbDBDecks;

	irr::gui::IGUIButton* btnHandTest;
	irr::gui::IGUIButton* btnHandTestSettings;
	irr::gui::IGUIStaticText* stHandTestSettings;
	irr::gui::IGUIWindow* wHandTest;
	irr::gui::IGUIButton* btnYdkeManage;
	irr::gui::IGUIWindow* wYdkeManage;
	irr::gui::IGUICheckBox* chkHandTestNoOpponent;
	irr::gui::IGUICheckBox* chkHandTestNoShuffle;
	irr::gui::IGUIEditBox* ebHandTestStartHand;
	irr::gui::IGUIComboBox* cbHandTestDuelRule;
	irr::gui::IGUICheckBox* chkHandTestSaveReplay;

	irr::gui::IGUIButton* btnClearDeck;
	irr::gui::IGUIButton* btnSortDeck;
	irr::gui::IGUIButton* btnShuffleDeck;
	irr::gui::IGUIButton* btnSaveDeck;
	irr::gui::IGUIButton* btnDeleteDeck;
	irr::gui::IGUIButton* btnSaveDeckAs;
	irr::gui::IGUIButton* btnRenameDeck;
	irr::gui::IGUIButton* btnSideOK;
	irr::gui::IGUIButton* btnSideShuffle;
	irr::gui::IGUIButton* btnSideSort;
	irr::gui::IGUIButton* btnSideReload;
	irr::gui::IGUIEditBox* ebDeckname;
	irr::gui::IGUIStaticText* stBanlist;
	irr::gui::IGUIStaticText* stDeck;
	irr::gui::IGUIStaticText* stCategory;
	irr::gui::IGUIStaticText* stLimit;
	irr::gui::IGUIStaticText* stAttribute;
	irr::gui::IGUIStaticText* stRace;
	irr::gui::IGUIStaticText* stAttack;
	irr::gui::IGUIStaticText* stDefense;
	irr::gui::IGUIStaticText* stStar;
	irr::gui::IGUIStaticText* stSearch;
	irr::gui::IGUIStaticText* stScale;
	//filter
	irr::gui::IGUIStaticText* wFilter;
	irr::gui::IGUIScrollBar* scrFilter;
	irr::gui::IGUIComboBox* cbCardType;
	irr::gui::IGUIComboBox* cbCardType2;
	irr::gui::IGUIComboBox* cbRace;
	irr::gui::IGUIComboBox* cbAttribute;
	irr::gui::IGUIComboBox* cbLimit;
	irr::gui::IGUIEditBox* ebStar;
	irr::gui::IGUIEditBox* ebScale;
	irr::gui::IGUIEditBox* ebAttack;
	irr::gui::IGUIEditBox* ebDefense;
	irr::gui::IGUIEditBox* ebCardName;
	irr::gui::IGUIButton* btnEffectFilter;
	irr::gui::IGUIButton* btnStartFilter;
	irr::gui::IGUIButton* btnClearFilter;
	irr::gui::IGUIWindow* wCategories;
	irr::gui::IGUICheckBox* chkCategory[32];
	irr::gui::IGUIButton* btnCategoryOK;
	irr::gui::IGUIButton* btnMarksFilter;
	irr::gui::IGUIWindow* wLinkMarks;
	irr::gui::IGUIButton* btnMark[8];
	irr::gui::IGUIButton* btnMarksOK;
	irr::gui::IGUICheckBox* chkAnime;
	//sort type
	irr::gui::IGUIStaticText* wSort;
	irr::gui::IGUIComboBox* cbSortType;
};

struct server_lobby_page_elements {
	//server lobby
	bool isHostingOnline;
	irr::gui::IGUITable* roomListTable;
	irr::gui::IGUIStaticText* wRoomListPlaceholder;
	irr::gui::IGUIComboBox* serverChoice;
	/////kdiy/////////
	irr::gui::IGUIComboBox* serverChoice2;
	/////kdiy/////////
	irr::gui::IGUIEditBox* ebNickNameOnline;
	irr::gui::IGUIButton* btnCreateHost2;
	irr::gui::IGUIComboBox* cbFilterRule;
	irr::gui::IGUIComboBox* cbFilterBanlist;
	irr::gui::IGUIStaticText* ebRoomNameText;
	irr::gui::IGUIEditBox* ebRoomName;
	irr::gui::IGUICheckBox* chkShowPassword;
	irr::gui::IGUICheckBox* chkShowActiveRooms;
	irr::gui::IGUIButton* btnLanRefresh2;
	irr::gui::IGUIWindow* wRoomPassword;
	irr::gui::IGUIEditBox* ebRPName;
	irr::gui::IGUIButton* btnFilterRelayMode;
	irr::gui::IGUIButton* btnRPYes;
	irr::gui::IGUIButton* btnRPNo;
	irr::gui::IGUIButton* btnJoinHost2;
	irr::gui::IGUIButton* btnJoinCancel2;
};

struct game_field_elements {
	//hand
	irr::gui::IGUIWindow* wHand;
	irr::gui::CGUIImageButton* btnHand[3];
	//
	irr::gui::IGUIWindow* wFTSelect;
	irr::gui::IGUIButton* btnFirst;
	irr::gui::IGUIButton* btnSecond;
	//message
	irr::gui::IGUIWindow* wMessage;
	irr::gui::IGUIStaticText* stMessage;
	irr::gui::IGUIButton* btnMsgOK;
	//auto close message
	irr::gui::IGUIWindow* wACMessage;
	irr::gui::IGUIStaticText* stACMessage;
	//yes/no
	irr::gui::IGUIWindow* wQuery;
	irr::gui::IGUIStaticText* stQMessage;
	irr::gui::IGUIButton* btnYes;
	irr::gui::IGUIButton* btnNo;
	//options
	irr::gui::IGUIWindow* wOptions;
	irr::gui::IGUIStaticText* stOptions;
	irr::gui::IGUIButton* btnOptionp;
	irr::gui::IGUIButton* btnOptionn;
	irr::gui::IGUIButton* btnOptionOK;
	irr::gui::IGUIButton* btnOption[5];
	irr::gui::IGUIScrollBar* scrOption;
	//pos selection
	irr::gui::IGUIWindow* wPosSelect;
	irr::gui::CGUIImageButton* btnPSAU;
	irr::gui::CGUIImageButton* btnPSAD;
	irr::gui::CGUIImageButton* btnPSDU;
	irr::gui::CGUIImageButton* btnPSDD;
	//card selection
	irr::gui::IGUIWindow* wCardSelect;
	irr::gui::CGUIImageButton* btnCardSelect[5];
	irr::gui::IGUIStaticText* stCardPos[5];
	irr::gui::IGUIScrollBar* scrCardList;
	irr::gui::IGUIButton* btnSelectOK;
	//card display
	irr::gui::IGUIWindow* wCardDisplay;
	irr::gui::CGUIImageButton* btnCardDisplay[5];
	irr::gui::IGUIStaticText* stDisplayPos[5];
	irr::gui::IGUIScrollBar* scrDisplayList;
	irr::gui::IGUIButton* btnDisplayOK;
	//announce number
	irr::gui::IGUIWindow* wANNumber;
	irr::gui::IGUIComboBox* cbANNumber;
	irr::gui::IGUIButton* btnANNumberOK;
	//announce card
	irr::gui::IGUIWindow* wANCard;
	irr::gui::IGUIEditBox* ebANCard;
	irr::gui::IGUIListBox* lstANCard;
	irr::gui::IGUIButton* btnANCardOK;
	//announce attribute
	irr::gui::IGUIWindow* wANAttribute;
	/////zdiy/////
	//irr::gui::IGUICheckBox* chkAttribute[7];
	irr::gui::IGUICheckBox* chkAttribute[8];
	/////zdiy/////
	//announce race
	irr::gui::IGUIWindow* wANRace;
	irr::gui::IGUICheckBox* chkRace[64];
	//cmd menu
	irr::gui::IGUIWindow* wCmdMenu;
	irr::gui::IGUIButton* btnActivate;
	irr::gui::IGUIButton* btnSummon;
	irr::gui::IGUIButton* btnSPSummon;
	irr::gui::IGUIButton* btnMSet;
	irr::gui::IGUIButton* btnSSet;
	irr::gui::IGUIButton* btnRepos;
	irr::gui::IGUIButton* btnAttack;
	irr::gui::IGUIButton* btnShowList;
	irr::gui::IGUIButton* btnOperation;
	irr::gui::IGUIButton* btnReset;
	irr::gui::IGUIButton* btnShuffle;
	//chat window
	irr::gui::IGUIWindow* wChat;
	irr::gui::IGUIListBox* lstChatLog;
	irr::gui::IGUIEditBox* ebChatInput;
	//phase button
	irr::gui::IGUIStaticText* wPhase;
	irr::gui::IGUIButton* btnDP;
	irr::gui::IGUIButton* btnSP;
	irr::gui::IGUIButton* btnM1;
	irr::gui::IGUIButton* btnBP;
	irr::gui::IGUIButton* btnM2;
	irr::gui::IGUIButton* btnEP;
	//replay control
	/////kdiy///////
	//irr::gui::IGUIStaticText* wReplayControl;
	irr::gui::IGUIWindow* wReplayControl;
	/////kdiy///////
	irr::gui::IGUIButton* btnReplayStart;
	irr::gui::IGUIButton* btnReplayPause;
	irr::gui::IGUIButton* btnReplayStep;
	irr::gui::IGUIButton* btnReplayUndo;
	irr::gui::IGUIButton* btnReplayExit;
	irr::gui::IGUIButton* btnReplaySwap;
	//surrender/leave
	irr::gui::IGUIButton* btnLeaveGame;
	//restart
	irr::gui::IGUIButton* btnRestartSingle;
	//swap
	irr::gui::IGUIButton* btnSpectatorSwap;
	//chain control
	irr::gui::IGUIButton* btnChainIgnore;
	irr::gui::IGUIButton* btnChainAlways;
	irr::gui::IGUIButton* btnChainWhenAvail;

	//cancel or finish
	irr::gui::IGUIButton* btnCancelOrFinish;
};

class Game final : public info_panel_elements, public main_menu_panel_elements, public lan_panel_elements, public host_creation_panel_elements,
					public host_panel_elements, public replay_panel_elements, public puzzle_panel_elements, public deck_edit_page_elements,
					public server_lobby_page_elements, public game_field_elements {

public:
	~Game();
	void Initialize();
	bool LoadCore();
#ifdef YGOPRO_BUILD_DLL
	void LoadCoreFromRepos();
#endif
	bool MainLoop();
	bool ApplySkin(const epro::path_string& skin, bool reload = false, bool firstrun = false);
	////////kdiy////////
	//void RefreshDeck(irr::gui::IGUIComboBox* cbDeck);
    void RefreshDeck();
	void RefreshDeck(irr::gui::IGUIComboBox* cbDeck, bool refresh_folder=false);
	void* ReadCardDataToCore();
	void ReloadCBpic();
	bool moviecheck();
	bool chantcheck();
	void charactselect(uint8_t player, int sel);
    std::vector<std::wstring>& GetPlayerReplayNames();
	bool damcharacter[2] = { false,false };
	int replay_player[2];
    int replay_team1;
    int replay_team2;
    bool replayswap;
	////////kdiy////////
	void RefreshLFLists();
	void RefreshAiDecks();
	void RefreshReplay();
	void RefreshSingleplay();
	void DrawSelectionLine(const Materials::QuadVertex vec, bool strip, int width, irr::video::SColor color);
	void DrawBackGround();
	void DrawLinkedZones(ClientCard* pcard);
	void DrawCards();
	void DrawCard(ClientCard* pcard);
	void DrawMisc();
	//kidy///////
	//void DrawStatus(ClientCard* pcard);
	void DrawStatus(ClientCard* pcard, bool attackonly = false);
	//kidy///////
	void DrawPendScale(ClientCard* pcard);
	void DrawStackIndicator(epro::wstringview text, const Materials::QuadVertex v, bool opponent);
	void DrawGUI();
	void DrawSpec();
	void DrawBackImage(irr::video::ITexture* texture, bool resized);
	void ShowElement(irr::gui::IGUIElement* element, int autoframe = 0);
	void HideElement(irr::gui::IGUIElement* element, bool set_action = false);
	void PopupElement(irr::gui::IGUIElement* element, int hideframe = 0);
    void WaitFrameSignal(int frame, std::unique_lock<epro::mutex>& _lck);
	void DrawThumb(const CardDataC* cp, irr::core::vector2di pos, LFList* lflist, bool drag = false, const irr::core::recti* cliprect = nullptr, bool loadimage = true);
	//kidy///////
	void DrawThumb2(uint32_t code, irr::core::vector2di pos);
	//kidy///////
	void DrawDeckBd();
	void SaveConfig();
	struct RepoGui {
		std::string path;
		IProgressBar* progress1;
		IProgressBar* progress2;
		irr::gui::IGUIButton* history_button1;
        //kidy///////
        irr::gui::IGUIButton* file_button;
        //kidy///////
		irr::gui::IGUIButton* history_button2;
		std::wstring commit_history_full;
		std::wstring commit_history_partial;
	};
	RepoGui* AddGithubRepositoryStatusWindow(const GitRepo* repo);
	void LoadGithubRepositories();
	void ParseGithubRepositories(const std::vector<const GitRepo*>& repos);
	void UpdateRepoInfo(const GitRepo* repo, RepoGui* grepo);
	void LoadServers();
	///kdiy//////////
	void LoadLocalServers();
	void ReloadLocalCBDuelRule();
	void ReloadLocalCBRule();
	//void ShowCardInfo(uint32_t code, bool resize = false, imgType type = imgType::ART);
    void ShowCardInfo(uint32_t code, bool resize = false, imgType type = imgType::ART, ClientCard* pcard = nullptr);
    void ShowPlayerInfo(uint8_t player);
	///kdiy//////////
	void RefreshCardInfoTextPositions();
	void ClearCardInfo(int player = 0);
	void AddChatMsg(epro::wstringview msg, int player, int type);
	void AddChatMsg(epro::wstringview name, epro::wstringview msg, int type);
	void AddLog(epro::wstringview msg, int param = 0);
	void ClearChatMsg();
	void AddDebugMsg(epro::stringview msg);
	void ClearTextures();
	void CloseDuelWindow();
	void PopupMessage(epro::wstringview text, epro::wstringview caption = L"");
	void PopupSaveWindow(epro::wstringview caption, epro::wstringview text, epro::wstringview hint);

	uint8_t LocalPlayer(uint8_t player);
	void UpdateDuelParam();
	void UpdateExtraRules(bool set = false);
	int GetMasterRule(uint64_t param, uint32_t forbidden = 0, int* truerule = 0);
	void ResizePhaseButtons();
	void SetPhaseButtons(bool visibility = false);
	void SetMessageWindow();
	void ResizeCardinfoWindow(bool keep_ratio);

	bool HasFocus(irr::gui::EGUI_ELEMENT_TYPE type) const;

	void RefreshUICoreVersion();
	std::wstring GetLocalizedExpectedCore();
	std::wstring GetLocalizedCompatVersion();
	void ReloadCBSortType();
	void ReloadCBCardType();
	void ReloadCBCardType2();
	void ReloadCBLimit();
	void ReloadCBAttribute();
	void ReloadCBRace();
	void ReloadCBFilterRule();
	void ReloadCBDuelRule(irr::gui::IGUIComboBox* cb = nullptr);
	void ReloadCBRule();
	void ReloadCBCurrentSkin();
	void ReloadCBCoreLogOutput();
	void ReloadCBVsync();
	void ReloadElementsStrings();

	void OnResize();
	template<typename T>
	T Scale(T val) const;
	template<typename T, typename T2, typename T3, typename T4>
	irr::core::rect<T> Scale(T x, T2 y, T3 x2, T4 y2) const;
	template<typename T>
	irr::core::rect<T> Scale(const irr::core::rect<T>& rect) const;
	template<typename T>
	irr::core::vector2d<T> Scale(const irr::core::vector2d<T>& vec) const;
	template<typename T>
	T ResizeX(T x) const;
	template<typename T>
	T ResizeY(T y) const;
	template<typename T, typename T2>
	irr::core::vector2d<T> Scale(T x, T2 y) const;
	irr::core::recti Resize(irr::s32 x, irr::s32 y, irr::s32 x2, irr::s32 y2) const;
	irr::core::recti Resize(irr::s32 x, irr::s32 y, irr::s32 x2, irr::s32 y2, irr::s32 dx, irr::s32 dy, irr::s32 dx2, irr::s32 dy2) const;
	irr::core::vector2d<irr::s32> Resize(irr::s32 x, irr::s32 y, bool reverse = false) const;
	irr::core::recti ResizeWithCappedWidth(irr::s32 x, irr::s32 y, irr::s32 x2, irr::s32 y2, float targetAspectRatio, bool scale = true) const;
	irr::core::recti ResizeElem(irr::s32 x, irr::s32 y, irr::s32 x2, irr::s32 y2, bool scale = true) const;
	irr::core::recti ResizePhaseHint(irr::s32 x, irr::s32 y, irr::s32 x2, irr::s32 y2, irr::s32 width) const;
	irr::core::recti ResizeWinFromCenter(irr::s32 x, irr::s32 y, irr::s32 x2, irr::s32 y2, irr::s32 xoff = 0, irr::s32 yoff = 0) const;
	irr::core::recti ResizeWin(irr::s32 x, irr::s32 y, irr::s32 x2, irr::s32 y2, bool chat = false) const;
	void SetCentered(irr::gui::IGUIElement* elem, bool use_offset = true) const;
	void ValidateName(irr::gui::IGUIElement* box);

	OCG_Duel SetupDuel(OCG_DuelOptions opts);
	epro::path_string FindScript(epro::path_stringview script_name, irr::io::IReadFile** retarchive = nullptr);
	std::vector<char> FindAndReadScript(epro::stringview script_name);
	std::vector<char> ReadScript(epro::path_stringview script_name, irr::io::IReadFile* archive = nullptr);
	bool LoadScript(OCG_Duel pduel, epro::stringview script_name);
	static int ScriptReader(void* payload, OCG_Duel duel, const char* name);
	static void MessageHandler(void* payload, const char* string, int type);
	static void UpdateDownloadBar(int percentage, int cur, int tot, const char* filename, bool is_new, void* payload);
	static void UpdateUnzipBar(unzip_payload* payload);

	epro::mutex gMutex;
	Signal frameSignal;
	Signal actionSignal;
	Signal replaySignal;
	std::atomic<bool> closeDuelWindow{ false };
	Signal closeDoneSignal;
	DuelInfo dInfo;
	DiscordWrapper discord{};
	ImageManager imageManager;
#ifdef YGOPRO_BUILD_DLL
	void* ocgcore;
	bool coreJustLoaded;
#endif
	bool coreloaded;
	std::wstring corename;
	bool restart = false;
	std::list<FadingUnit> fadingList;
	std::vector<int> logParam;
	std::wstring chatMsg[8];
	std::map<std::string, RepoGui> repoInfoGui;

	uint32_t delta_time;
	uint32_t delta_frames;
	int hideChatTimer;
	bool hideChat;
	float chatTiming[8];
	int chatType[8];
	uint16_t linePatternD3D;
	uint16_t linePatternGL;
	float waitFrame;
	uint32_t signalFrame;
	bool saveReplay;
	int showcard;
	uint32_t showcardcode;
	float showcarddif;
	float showcardp;
	bool is_attacking;
	float attack_sv;
	irr::core::vector3df atk_r;
	irr::core::vector3df atk_t;
	float atkdy;
    ////kdiy//////
	float atk2dy;
	float atkdy2;
	float atk2dy2;
	float atkdy3;
	double angle = 0.0f;
    ////kdiy//////
	int lpframe;
	float lpd;
	int lpplayer;
	int lpccolor;
	int lpcalpha;
	std::wstring lpcstring;
	bool always_chain;
	bool ignore_chain;
	bool chain_when_avail;

	bool is_building;
	bool is_siding;
	uint32_t forbiddentypes;
	uint16_t extra_rules;
	uint64_t duel_param;
	uint32_t showingcard;
	bool cardimagetextureloading;
	float dpi_scale;


	irr::core::dimension2d<irr::u32> window_size;
	irr::core::vector2d<irr::f32> window_scale;

	CGUISkinSystem* skinSystem;

	ClientField dField;
	DeckBuilder deckBuilder;
	MenuHandler menuHandler;
	irr::IrrlichtDevice* device;
	irr::video::IVideoDriver* driver;
	irr::scene::ISceneManager* smgr;
	irr::scene::ICameraSceneNode* camera;
	irr::io::IFileSystem* filesystem;
	void PopulateResourcesDirectories();
	std::vector<epro::path_string> field_dirs;
	std::vector<epro::path_string> pic_dirs;
	std::vector<epro::path_string> cover_dirs;
	///kdiy////////
    bool git_update = false;
    bool git_error = false;
    bool first_play = false;
    uint32_t showcardalias;
    bool chklast = true;
	bool isEvent;//locking flow, allow left mouse click notify_one() to skip
	epro::condition_variable* cv;//should lock thread when play mode-story sound,this cv is in duelclient.cpp
	bool haloNodeexist[2][12][10];
    std::vector<irr::core::vector3df> haloNode[2][12][10];
    //ktest////////
    std::string currentVideo;
    std::string newVideo;
	bool videostart = false;
	bool isAnime = false;
    sf::Sound sound;
	sf::SoundBuffer soundBuffer;
    // cv::VideoCapture cap;
    irr::video::ITexture* videotexture = nullptr;
	// cv::Mat frame;
    //double totalFrames = 0;
    AVFormatContext* formatCtx = nullptr;
    AVCodecContext* videoCodecCtx = nullptr;
    AVCodecContext* audioCodecCtx = nullptr;
	AVPacket packet;
	AVFrame* videoFrame;
    AVFrame* audioFrame;
    int videoStreamIndex = -1, audioStreamIndex = -1;
	bool frameReady = false;
	double timeAccumulated = 0.0, lastAudioProcessedTime = 0.0; // Accumulate time to ensure smooth frame skipping
	double videoFrameDuration = 1.0, audioFrameDuration = 1.0;
	std::vector<int16_t> audioBuffer;
	bool openVideo(std::string videoName);
	bool PlayVideo(bool loop = false);
    void StopVideo(bool close = false, bool reset = true);
    //ktest////////
	std::vector<epro::path_string> closeup_dirs;
	///kdiy////////
	std::vector<epro::path_string> script_dirs;
	std::vector<epro::path_string> init_scripts;
	std::vector<epro::path_string> cores_to_load;
	void PopulateLocales();
	void ApplyLocale(size_t index, bool forced = false);
	using locale_entry_t = std::pair<epro::path_string, std::vector<epro::path_string>>;
	std::vector<locale_entry_t> locales;
	epro::mutex popupCheck;
	std::wstring queued_msg;
	std::wstring queued_caption;
	bool should_reload_skin;
	bool should_refresh_hands;
	bool current_topdown;
	bool current_keep_aspect_ratio;
	bool needs_to_acknowledge_discord_host{ false };
	//GUI
	irr::gui::IGUIEnvironment* env;
	irr::gui::CGUITTFont* guiFont;
	irr::gui::CGUITTFont* textFont;
	irr::gui::CGUITTFont* numFont;
	irr::gui::CGUITTFont* adFont;
	irr::gui::CGUITTFont* lpcFont;
	std::map<irr::gui::CGUIImageButton*, uint32_t> imageLoading;
	//card image
	/////kdiy/////////
	irr::gui::CGUITTFont* atkFont;
	irr::gui::CGUITTFont* defFont;
	irr::gui::CGUITTFont* numFont0;
	irr::gui::CGUITTFont* adFont0;
	irr::gui::CGUITTFont* lpFont;
	irr::gui::CGUITTFont* nameFont;
	irr::gui::CGUITTFont* turnFont;
	//irr::gui::IGUIStaticText* wCardImg;
	/////kdiy/////////
	//hint text
	irr::gui::IGUIStaticText* stHintMsg;
	irr::gui::IGUIStaticText* stTip;
	irr::gui::IGUIStaticText* stCardListTip;

	void PopulateGameHostWindows();
	void PopulateAIBotWindow();
	void PopulateTabSettingsWindow();
	void PopulateSettingsWindow();
	SettingsWindow gSettings;
	irr::gui::IGUIWindow* wBtnSettings;
	irr::gui::CGUIImageButton* btnSettings;

	irr::gui::IGUIWindow* updateWindow;
	irr::gui::IGUIStaticText* updateProgressText;
	IProgressBar* updateProgressTop;
	irr::gui::IGUIStaticText* updateSubprogressText;
	IProgressBar* updateProgressBottom;
    /////kdiy/////////
	irr::gui::IGUIWindow* wCardImg;
	irr::gui::IGUIButton* CardInfo[8];
	irr::gui::IGUIStaticText* stCardInfo[8];
	std::wstring effectText[8];
    irr::gui::IGUIStaticText* wCardInfo;
	irr::gui::IGUIWindow* wBtnShowCard;
	irr::gui::IGUIButton* btnShowCard;
	irr::gui::IGUIButton* btnChatLog;
	irr::gui::IGUIButton* btnCardLoc;
	irr::gui::IGUIStaticText* stPasscodeScope2;
	irr::gui::IGUIStaticText* stInfo2;
	irr::gui::IGUIWindow* wLocation;
	irr::gui::IGUIButton* btnLocation[6];
	irr::gui::IGUIStaticText* stLocation[6];
	irr::gui::IGUIWindow* pwupdateWindow;
	irr::gui::IGUIStaticText* updatePwText;
	irr::gui::IGUIEditBox* ebPw;
	irr::gui::IGUIButton* btnPw;
	/////kdiy/////////
	struct ProgressBarStatus {
		bool newFile;
		std::wstring progressText;
		std::wstring subProgressText;
		irr::s32 progressTop;
		irr::s32 progressBottom;
	};

	epro::mutex progressStatusLock;
	ProgressBarStatus progressStatus;

#define sizeofarr(arr) (sizeof(arr)/sizeof(decltype(*arr)))

	//file save
	irr::gui::IGUIWindow* wFileSave;
	irr::gui::IGUIStaticText* stFileSaveHint;
	irr::gui::IGUIEditBox* ebFileSaveName;
	irr::gui::IGUIButton* btnFileSaveYes;
	irr::gui::IGUIButton* btnFileSaveNo;
	irr::gui::IGUIStaticText* fpsCounter;
	std::vector<std::pair<irr::gui::IGUIElement*, uint32_t>> defaultStrings;
};

extern Game* mainGame;

template<typename T>
inline irr::core::vector2d<T> Game::Scale(const irr::core::vector2d<T>& vec) const {
	return Scale<T>(vec.X, vec.Y);
}
template<typename T>
inline T Game::ResizeX(T x) const {
	return Scale<T>(x * window_scale.X);
}
template<typename T>
inline T Game::ResizeY(T y) const {
	return Scale<T>(y * window_scale.Y);
}
template<typename T, typename T2>
inline irr::core::vector2d<T> Game::Scale(T x, T2 y) const {
	return { Scale<T>(x), Scale<T>(y) };
}
template<typename T>
inline T Game::Scale(T val) const {
	return T(val * dpi_scale);
}
template<typename T, typename T2, typename T3, typename T4>
inline irr::core::rect<T> Game::Scale(T x, T2 y, T3 x2, T4 y2) const {
	const auto& scale = dpi_scale;
	return { (T)std::roundf(x * scale),(T)std::roundf(y * scale), (T)std::roundf(x2 * scale), (T)std::roundf(y2 * scale) };
}
template<typename T>
inline irr::core::rect<T> Game::Scale(const irr::core::rect<T>& rect) const {
	return Scale(rect.UpperLeftCorner.X, rect.UpperLeftCorner.Y, rect.LowerRightCorner.X, rect.LowerRightCorner.Y);
}

}

#define UEVENT_EXIT			0x1
#define UEVENT_TOWINDOW		0x2

#define COMMAND_ACTIVATE	0x0001
#define COMMAND_SUMMON		0x0002
#define COMMAND_SPSUMMON	0x0004
#define COMMAND_MSET		0x0008
#define COMMAND_SSET		0x0010
#define COMMAND_REPOS		0x0020
#define COMMAND_ATTACK		0x0040
#define COMMAND_LIST		0x0080
#define COMMAND_OPERATION	0x0100
#define COMMAND_RESET		0x0200

#define POSITION_HINT		0x8000

#define DECK_SEARCH_SCROLL_STEP		100

constexpr float FIELD_X = 4.2f;
constexpr float FIELD_Y = 8.0f;
constexpr float FIELD_Z = 7.8f;
constexpr float FIELD_ANGLE = 0.798055715f; //(std::atan(FIELD_Y / FIELD_Z))

constexpr float CAMERA_LEFT = -0.90f;
constexpr float CAMERA_RIGHT = 0.45f;
constexpr float CAMERA_BOTTOM = -0.42f;
constexpr float CAMERA_TOP = 0.42f;

#endif // GAME_H
