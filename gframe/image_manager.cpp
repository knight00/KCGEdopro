#include "game_config.h"
#include <fmt/format.h>
#include "utils.h"
#include <IImage.h>
#include <IGUIImage.h>
#include <IVideoDriver.h>
#include <IrrlichtDevice.h>
#include <IReadFile.h>
#include "logging.h"
#include "image_manager.h"
#include "image_downloader.h"
#include "game.h"
#include "config.h"

#define BASE_PATH EPRO_TEXT("./textures/")

namespace ygo {
////////kdiy/////
#define TEXTURE_DECK				0
#define TEXTURE_MENU				1
#define TEXTURE_COVERS				2
#define TEXTURE_COVERO				3
#define TEXTURE_ATTACK				4
#define TEXTURE_ACTIVATE			5
#define TEXTURE_CHAIN			    6
#define TEXTURE_NEGATED			    7
#define TEXTURE_LP		            8
#define TEXTURE_LPf		            9
#define TEXTURE_MASK		        10
#define TEXTURE_EQUIP		        11
#define TEXTURE_TARGET		        12
#define TEXTURE_CHAINTARGET		    13
#define TEXTURE_F1		            14
#define TEXTURE_F2		            15
#define TEXTURE_F3		            16
#define TEXTURE_BACKGROUND		    17
#define TEXTURE_BACKGROUND_MENU		18
#define TEXTURE_BACKGROUND_DECK		19
#define TEXTURE_field2		        20
#define TEXTURE_field_transparent2	21
#define TEXTURE_field3		        22
#define TEXTURE_field_transparent3	23
#define TEXTURE_field		        24
#define TEXTURE_field_transparent	25
#define TEXTURE_field4		        26
#define TEXTURE_field_transparent4	27
#define TEXTURE_field_fieldSP2	    28
#define TEXTURE_field_transparentSP2 29
#define TEXTURE_fieldSP3            30
#define TEXTURE_field_transparentSP3 31
#define TEXTURE_fieldSP             32
#define TEXTURE_field_transparentSP 33
#define TEXTURE_fieldSP4            34
#define TEXTURE_field_transparentSP4 35
#define TEXTURE_UNKNOWN             36
#define TEXTURE_LIM                 37
#define TEXTURE_OT                  38
#define TEXTURE_SETTING             39

#define TEXTURE_MUTO                40
#define TEXTURE_ATEM                41
#define TEXTURE_KAIBA               42
#define TEXTURE_JOEY                43
#define TEXTURE_MARIK               44
#define TEXTURE_DARTZ               45
#define TEXTURE_BAKURA              46
#define TEXTURE_AIGAMI              47
#define TEXTURE_JUDAI               48
#define TEXTURE_MANJOME             49
#define TEXTURE_KAISA               50
#define TEXTURE_PHORNIX             51
#define TEXTURE_JOHN                52
#define TEXTURE_YUBEL               53
#define TEXTURE_YUSEI               54
#define TEXTURE_JACK                55
#define TEXTURE_ARKI                56
#define TEXTURE_CROW                57
#define TEXTURE_KIRYU               58
#define TEXTURE_PARADOX             59
#define TEXTURE_ZONE                60
#define TEXTURE_YUMA                61
#define TEXTURE_SHARK               62
#define TEXTURE_KAITO               63
#define TEXTURE_IV                  64
#define TEXTURE_DONTHOUSAND         65
#define TEXTURE_YUYA                66
#define TEXTURE_DECLAN              67
#define TEXTURE_SHAY                68
#define TEXTURE_PLAYMAKER           69
#define TEXTURE_SOULBURNER          70
#define TEXTURE_BLUEANGEL           71
////////kdiy/////

#define ASSERT_TEXTURE_LOADED(what, name) do { if(!what) { throw std::runtime_error("Couldn't load texture: " name); }} while(0)
#define ASSIGN_DEFAULT(obj) do { def_##obj=obj; } while(0)

namespace {
bool hasNPotSupport(irr::video::IVideoDriver* driver) {
	static const auto supported = [driver] {
		return driver->queryFeature(irr::video::EVDF_TEXTURE_NPOT);
	}();
	return supported;
}
// Compute next-higher power of 2 efficiently, e.g. for power-of-2 texture sizes.
// Public Domain: https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
inline irr::u32 npot2(irr::u32 orig) {
	orig--;
	orig |= orig >> 1;
	orig |= orig >> 2;
	orig |= orig >> 4;
	orig |= orig >> 8;
	orig |= orig >> 16;
	return orig + 1;
}
irr::s32 toPow2(irr::video::IVideoDriver* driver, irr::s32 size) {
	if(!hasNPotSupport(driver))
		return npot2(size);
	return size;
};
}

ImageManager::ImageManager() {
	stop_threads = false;
	obj_clear_thread = epro::thread(&ImageManager::ClearFutureObjects, this);
	load_threads.reserve(gGameConfig->imageLoadThreads);
	for(int i = 0; i < gGameConfig->imageLoadThreads; ++i)
		load_threads.emplace_back(&ImageManager::LoadPic, this);
}
ImageManager::~ImageManager() {
	stop_threads = true;
	obj_clear_lock.lock();
	cv_clear.notify_all();
	obj_clear_lock.unlock();
	obj_clear_thread.join();
	pic_load.lock();
	cv_load.notify_all();
	pic_load.unlock();
	for(auto& thread : load_threads)
		thread.join();
	for(auto& it : g_imgCache) {
		if(it.second)
			it.second->drop();
	}
	for(auto& it : g_txrCache) {
		if(it.second)
			driver->removeTexture(it.second);
	}
}
irr::video::ITexture* ImageManager::loadTextureFixedSize(epro::path_stringview texture_name, int width, int height) {
	width = mainGame->Scale(width);
	height = mainGame->Scale(height);
	irr::video::ITexture* ret = GetTextureFromFile(epro::format(EPRO_TEXT("{}{}.png"), textures_path, texture_name).data(), width, height);
	if(ret == nullptr)
		ret = GetTextureFromFile(epro::format(EPRO_TEXT("{}{}.jpg"), textures_path, texture_name).data(), width, height);
	return ret;
}
irr::video::ITexture* ImageManager::loadTextureAnySize(epro::path_stringview texture_name) {
	irr::video::ITexture* ret = driver->getTexture(epro::format(EPRO_TEXT("{}{}.png"), textures_path, texture_name).data());
	if(ret == nullptr)
		ret = driver->getTexture(epro::format(EPRO_TEXT("{}{}.jpg"), textures_path, texture_name).data());
	return ret;
}
bool ImageManager::Initial() {
	/////kdiy/////
	Utils::MakeDirectory(EPRO_TEXT("./textures/character"));
	std::vector<epro::path_string> searchPath;
	searchPath.push_back(EPRO_TEXT("./textures/character/muto"));
	searchPath.push_back(EPRO_TEXT("./textures/character/atem"));
	searchPath.push_back(EPRO_TEXT("./textures/character/kaiba"));
	searchPath.push_back(EPRO_TEXT("./textures/character/joey"));
	searchPath.push_back(EPRO_TEXT("./textures/character/marik"));
    searchPath.push_back(EPRO_TEXT("./textures/character/dartz"));
	searchPath.push_back(EPRO_TEXT("./textures/character/bakura"));
	searchPath.push_back(EPRO_TEXT("./textures/character/aigami"));
	searchPath.push_back(EPRO_TEXT("./textures/character/judai"));
	searchPath.push_back(EPRO_TEXT("./textures/character/manjome"));
	searchPath.push_back(EPRO_TEXT("./textures/character/kaisa"));
	searchPath.push_back(EPRO_TEXT("./textures/character/phoenix"));
	searchPath.push_back(EPRO_TEXT("./textures/character/john"));
	searchPath.push_back(EPRO_TEXT("./textures/character/yubel"));
	searchPath.push_back(EPRO_TEXT("./textures/character/yusei"));
	searchPath.push_back(EPRO_TEXT("./textures/character/jack"));
	searchPath.push_back(EPRO_TEXT("./textures/character/arki"));
	searchPath.push_back(EPRO_TEXT("./textures/character/crow"));
	searchPath.push_back(EPRO_TEXT("./textures/character/kiryu"));
	searchPath.push_back(EPRO_TEXT("./textures/character/paradox"));
	searchPath.push_back(EPRO_TEXT("./textures/character/zone"));
	searchPath.push_back(EPRO_TEXT("./textures/character/yuma"));
	searchPath.push_back(EPRO_TEXT("./textures/character/shark"));
	searchPath.push_back(EPRO_TEXT("./textures/character/kaito"));
	searchPath.push_back(EPRO_TEXT("./textures/character/iv"));
	searchPath.push_back(EPRO_TEXT("./textures/character/DonThousand"));
	searchPath.push_back(EPRO_TEXT("./textures/character/yuya"));
	searchPath.push_back(EPRO_TEXT("./textures/character/declan"));
	searchPath.push_back(EPRO_TEXT("./textures/character/shay"));
	searchPath.push_back(EPRO_TEXT("./textures/character/playmaker"));
	searchPath.push_back(EPRO_TEXT("./textures/character/soulburner"));
	searchPath.push_back(EPRO_TEXT("./textures/character/blueangel"));
	for (auto path : searchPath) {
		Utils::MakeDirectory(path);
		Utils::MakeDirectory(epro::format(EPRO_TEXT("{}/icon"), path));
		Utils::MakeDirectory(epro::format(EPRO_TEXT("{}/damage"), path));
	}
	RefreshRandomImageList();
	/////kdiy/////
	timestamp_id = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	textures_path = BASE_PATH;

	/////kdiy//////
    // tCover[0] = loadTextureFixedSize(EPRO_TEXT("cover"_sv), CARD_IMG_WIDTH, CARD_IMG_HEIGHT);
	// ASSERT_TEXTURE_LOADED(tCover[0], "cover");

	// tCover[1] = loadTextureFixedSize(EPRO_TEXT("cover2"_sv), CARD_IMG_WIDTH, CARD_IMG_HEIGHT);
	// if(!tCover[1])
	// 	tCover[1] = tCover[0];

	// tUnknown = loadTextureFixedSize(EPRO_TEXT("unknown"_sv), CARD_IMG_WIDTH, CARD_IMG_HEIGHT);
	// ASSERT_TEXTURE_LOADED(tUnknown, "unknown");

	// tAct = loadTextureAnySize(EPRO_TEXT("act"_sv));
	// ASSERT_TEXTURE_LOADED(tAct, "act");
	// ASSIGN_DEFAULT(tAct);

	// tAttack = loadTextureAnySize(EPRO_TEXT("attack"_sv));
	// ASSERT_TEXTURE_LOADED(tAttack, "attack");
	// ASSIGN_DEFAULT(tAttack);

	// tChain = loadTextureAnySize(EPRO_TEXT("chain"_sv));
	// ASSERT_TEXTURE_LOADED(tChain, "chain");
	// ASSIGN_DEFAULT(tChain);

	// tNegated = loadTextureFixedSize(EPRO_TEXT("negated"_sv), 128, 128);
	// ASSERT_TEXTURE_LOADED(tNegated, "negated");
	// ASSIGN_DEFAULT(tNegated);

	// tNumber = loadTextureFixedSize(EPRO_TEXT("number"_sv), 320, 256);
	// ASSERT_TEXTURE_LOADED(tNumber, "number");
	// ASSIGN_DEFAULT(tNumber);

	// tLPBar = loadTextureAnySize(EPRO_TEXT("lp"_sv));
	// ASSERT_TEXTURE_LOADED(tLPBar, "lp");
	// ASSIGN_DEFAULT(tLPBar);

	// tLPFrame = loadTextureAnySize(EPRO_TEXT("lpf"_sv));
	// ASSERT_TEXTURE_LOADED(tLPFrame, "lpf");
	// ASSIGN_DEFAULT(tLPFrame);

	// tMask = loadTextureFixedSize(EPRO_TEXT("mask"_sv), 254, 254);
	// ASSERT_TEXTURE_LOADED(tMask, "mask");
	// ASSIGN_DEFAULT(tMask);

	// tEquip = loadTextureAnySize(EPRO_TEXT("equip"_sv));
	// ASSERT_TEXTURE_LOADED(tEquip, "equip");
	// ASSIGN_DEFAULT(tEquip);

	// tTarget = loadTextureAnySize(EPRO_TEXT("target"_sv));
	// ASSERT_TEXTURE_LOADED(tTarget, "target");
	// ASSIGN_DEFAULT(tTarget);

	// tChainTarget = loadTextureAnySize(EPRO_TEXT("chaintarget"_sv));
	// ASSERT_TEXTURE_LOADED(tChainTarget, "chaintarget");
	// ASSIGN_DEFAULT(tChainTarget);

	// tLim = loadTextureAnySize(EPRO_TEXT("lim"_sv));
	// ASSERT_TEXTURE_LOADED(tLim, "lim");
	// ASSIGN_DEFAULT(tLim);

	// tOT = loadTextureAnySize(EPRO_TEXT("ot"_sv));
	// ASSERT_TEXTURE_LOADED(tOT, "ot");
	// ASSIGN_DEFAULT(tOT);

	// tHand[0] = loadTextureFixedSize(EPRO_TEXT("f1"_sv), 89, 128);
	// ASSERT_TEXTURE_LOADED(tHand[0], "f1");
	// ASSIGN_DEFAULT(tHand[0]);

	// tHand[1] = loadTextureFixedSize(EPRO_TEXT("f2"_sv), 89, 128);
	// ASSERT_TEXTURE_LOADED(tHand[1], "f2");
	// ASSIGN_DEFAULT(tHand[1]);

	// tHand[2] = loadTextureFixedSize(EPRO_TEXT("f3"_sv), 89, 128);
	// ASSERT_TEXTURE_LOADED(tHand[2], "f3");
	// ASSIGN_DEFAULT(tHand[2]);

	// tBackGround = loadTextureAnySize(EPRO_TEXT("bg"_sv));
	// ASSERT_TEXTURE_LOADED(tBackGround, "bg");
	// ASSIGN_DEFAULT(tBackGround);

	// tBackGround_menu = loadTextureAnySize(EPRO_TEXT("bg_menu"_sv));
	// if(!tBackGround_menu)
	// 	tBackGround_menu = tBackGround;
	// ASSIGN_DEFAULT(tBackGround_menu);

	// tBackGround_deck = loadTextureAnySize(EPRO_TEXT("bg_deck"_sv));
	// if(!tBackGround_deck)
	// 	tBackGround_deck = tBackGround;
	// ASSIGN_DEFAULT(tBackGround_deck);

	// tBackGround_duel_topdown = loadTextureAnySize(EPRO_TEXT("bg_duel_topdown"_sv));
	// if(!tBackGround_duel_topdown)
	// 	tBackGround_duel_topdown = tBackGround;
	// ASSIGN_DEFAULT(tBackGround_duel_topdown);

	// tField[0][0] = loadTextureAnySize(EPRO_TEXT("field2"_sv));
	// ASSERT_TEXTURE_LOADED(tField[0][0], "field2");
	// ASSIGN_DEFAULT(tField[0][0]);

	// tFieldTransparent[0][0] = loadTextureAnySize(EPRO_TEXT("field-transparent2"_sv));
	// ASSERT_TEXTURE_LOADED(tFieldTransparent[0][0], "field-transparent2");
	// ASSIGN_DEFAULT(tFieldTransparent[0][0]);

	// tField[0][1] = loadTextureAnySize(EPRO_TEXT("field3"_sv));
	// ASSERT_TEXTURE_LOADED(tField[0][1], "field3");
	// ASSIGN_DEFAULT(tField[0][1]);

	// tFieldTransparent[0][1] = loadTextureAnySize(EPRO_TEXT("field-transparent3"_sv));
	// ASSERT_TEXTURE_LOADED(tFieldTransparent[0][1], "field-transparent3");
	// ASSIGN_DEFAULT(tFieldTransparent[0][1]);

	// tField[0][2] = loadTextureAnySize(EPRO_TEXT("field"_sv));
	// ASSERT_TEXTURE_LOADED(tField[0][2], "field");
	// ASSIGN_DEFAULT(tField[0][2]);

	// tFieldTransparent[0][2] = loadTextureAnySize(EPRO_TEXT("field-transparent"_sv));
	// ASSERT_TEXTURE_LOADED(tFieldTransparent[0][2], "field-transparent");
	// ASSIGN_DEFAULT(tFieldTransparent[0][2]);

	// tField[0][3] = loadTextureAnySize(EPRO_TEXT("field4"_sv));
	// ASSERT_TEXTURE_LOADED(tField[0][3], "field4");
	// ASSIGN_DEFAULT(tField[0][3]);

	// tFieldTransparent[0][3] = loadTextureAnySize(EPRO_TEXT("field-transparent4"_sv));
	// ASSERT_TEXTURE_LOADED(tFieldTransparent[0][3], "field-transparent4");
	// ASSIGN_DEFAULT(tFieldTransparent[0][3]);

	// tField[1][0] = loadTextureAnySize(EPRO_TEXT("fieldSP2"_sv));
	// ASSERT_TEXTURE_LOADED(tField[1][0], "fieldSP2");
	// ASSIGN_DEFAULT(tField[1][0]);

	// tFieldTransparent[1][0] = loadTextureAnySize(EPRO_TEXT("field-transparentSP2"_sv));
	// ASSERT_TEXTURE_LOADED(tFieldTransparent[1][0], "field-transparentSP2");
	// ASSIGN_DEFAULT(tFieldTransparent[1][0]);

	// tField[1][1] = loadTextureAnySize(EPRO_TEXT("fieldSP3"_sv));
	// ASSERT_TEXTURE_LOADED(tField[1][1], "fieldSP3");
	// ASSIGN_DEFAULT(tField[1][1]);

	// tFieldTransparent[1][1] = loadTextureAnySize(EPRO_TEXT("field-transparentSP3"_sv));
	// ASSERT_TEXTURE_LOADED(tFieldTransparent[1][1], "field-transparentSP3");
	// ASSIGN_DEFAULT(tFieldTransparent[1][1]);

	// tField[1][2] = loadTextureAnySize(EPRO_TEXT("fieldSP"_sv));
	// ASSERT_TEXTURE_LOADED(tField[1][2], "fieldSP");
	// ASSIGN_DEFAULT(tField[1][2]);

	// tFieldTransparent[1][2] = loadTextureAnySize(EPRO_TEXT("field-transparentSP"_sv));
	// ASSERT_TEXTURE_LOADED(tFieldTransparent[1][2], "field-transparentSP");
	// ASSIGN_DEFAULT(tFieldTransparent[1][2]);

	// tField[1][3] = loadTextureAnySize(EPRO_TEXT("fieldSP4"_sv));
	// ASSERT_TEXTURE_LOADED(tField[1][3], "fieldSP4");
	// ASSIGN_DEFAULT(tField[1][3]);

	// tFieldTransparent[1][3] = loadTextureAnySize(EPRO_TEXT("field-transparentSP4"_sv));
	// ASSERT_TEXTURE_LOADED(tFieldTransparent[1][3], "field-transparentSP4");
	// ASSIGN_DEFAULT(tFieldTransparent[1][3]);

	// tSettings = loadTextureAnySize(EPRO_TEXT("settings"_sv));
	// ASSERT_TEXTURE_LOADED(tSettings, "settings");
	// ASSIGN_DEFAULT(tSettings);
    QQ = driver->getTexture(EPRO_TEXT("./textures/QQ.jpg"));
    ASSERT_TEXTURE_LOADED(QQ, "QQ");
    icon[0] = driver->getTexture(EPRO_TEXT("./textures/character/player/mini_icon.png"));
	character[0] = driver->getTexture(0);
	characterd[0] = driver->getTexture(0);
	for(uint8_t i = 0; i < 6; i++) {
        scharacter[i][0] = driver->getTexture(0);
        scharacter[i][1] = driver->getTexture(0);
        modeHead[i] = driver->getTexture(0);
    }
    head[0] = driver->getTexture(0);
    for(uint8_t i = 1; i <= CHARACTER_STORY; i++) {
        //1: Yusei
        //2: Darkman
        //3: Paradox
        head[i] = driver->getTexture(epro::format(EPRO_TEXT("./mode/story/head/{}.jpg"), i).c_str());
        if(head[i] == nullptr)
            head[i] = driver->getTexture(epro::format(EPRO_TEXT("./mode/story/head/{}.png"), i).c_str());
    }
#ifdef VIP
    RefreshKCGImage();
#else
    for(uint8_t playno = 1; playno < CHARACTER_VOICE; playno++) {
		icon[playno] = driver->getTexture(0);
	    character[playno] = driver->getTexture(0);
	    characterd[playno] = driver->getTexture(0);
	}
#endif
    tcharacterselect = driver->getTexture(EPRO_TEXT("./textures/character/left.png"));
	tcharacterselect2 = driver->getTexture(EPRO_TEXT("./textures/character/right.png"));
    GetRandomImage(tCover[0], TEXTURE_COVERS, CARD_IMG_WIDTH, CARD_IMG_HEIGHT);
    GetRandomImage(tCover[1], TEXTURE_COVERO, CARD_IMG_WIDTH, CARD_IMG_HEIGHT);
    if (!tCover[0])
	    tCover[0] = loadTextureFixedSize(EPRO_TEXT("cover"_sv), CARD_IMG_WIDTH, CARD_IMG_HEIGHT);
	ASSERT_TEXTURE_LOADED(tCover[0], "cover");

	if (!tCover[1])
	    tCover[1] = loadTextureFixedSize(EPRO_TEXT("cover2"_sv), CARD_IMG_WIDTH, CARD_IMG_HEIGHT);
	if(!tCover[1])
		tCover[1] = tCover[0];

	GetRandomImage(tUnknown, TEXTURE_UNKNOWN, CARD_IMG_WIDTH, CARD_IMG_HEIGHT);
	if (!tUnknown)
		tUnknown = loadTextureFixedSize(EPRO_TEXT("unknown"_sv), CARD_IMG_WIDTH, CARD_IMG_HEIGHT);
	ASSERT_TEXTURE_LOADED(tUnknown, "unknown");

	GetRandomImage(tAct, TEXTURE_ACTIVATE);
	GetRandomImage(tAttack, TEXTURE_ATTACK);
	if (!tAct) {
		tAct = loadTextureAnySize(EPRO_TEXT("act"_sv));
        ASSIGN_DEFAULT(tAct);
    }
	ASSERT_TEXTURE_LOADED(tAct, "act");

	if (!tAttack) {
		tAttack = loadTextureAnySize(EPRO_TEXT("attack"_sv));
        ASSIGN_DEFAULT(tAttack);
    }
	ASSERT_TEXTURE_LOADED(tAttack, "attack");

	GetRandomImage(tChain, TEXTURE_CHAIN);
	if (!tChain) {
		tChain = loadTextureAnySize(EPRO_TEXT("chain"_sv));
        ASSIGN_DEFAULT(tChain);
    }
	ASSERT_TEXTURE_LOADED(tChain, "chain");

	GetRandomImage(tNegated, TEXTURE_NEGATED, 128, 128);
	if (!tNegated) {
		tNegated = loadTextureFixedSize(EPRO_TEXT("negated"_sv), 128, 128);
        ASSIGN_DEFAULT(tNegated);
    }
	ASSERT_TEXTURE_LOADED(tNegated, "negated");

	tNumber = loadTextureFixedSize(EPRO_TEXT("number"_sv), 320, 256);
	ASSERT_TEXTURE_LOADED(tNumber, "number");
	ASSIGN_DEFAULT(tNumber);

	GetRandomImage(tLPBar, TEXTURE_LP);
	if (!tLPBar) {
		tLPBar = loadTextureAnySize(EPRO_TEXT("lp"_sv));
        ASSIGN_DEFAULT(tLPBar);
    }
	ASSERT_TEXTURE_LOADED(tLPBar, "lp");

	GetRandomImage(tLPFrame, TEXTURE_LPf);
	if (!tLPFrame) {
		tLPFrame = loadTextureAnySize(EPRO_TEXT("lpf"_sv));
        ASSIGN_DEFAULT(tLPFrame);
    }
	ASSERT_TEXTURE_LOADED(tLPFrame, "lpf");

	GetRandomImage(tMask, TEXTURE_MASK, 254, 254);
	if (!tMask) {
		tMask = loadTextureFixedSize(EPRO_TEXT("mask"_sv), 254, 254);
        ASSIGN_DEFAULT(tMask);
    }
	ASSERT_TEXTURE_LOADED(tMask, "mask");

	GetRandomImage(tEquip, TEXTURE_EQUIP);
	if (!tEquip) {
		tEquip = loadTextureAnySize(EPRO_TEXT("equip"_sv));
        ASSIGN_DEFAULT(tEquip);
    }
	ASSERT_TEXTURE_LOADED(tEquip, "equip");

	GetRandomImage(tTarget, TEXTURE_TARGET);
	if (!tTarget) {
		tTarget = loadTextureAnySize(EPRO_TEXT("target"_sv));
        ASSIGN_DEFAULT(tTarget);
    }
	ASSERT_TEXTURE_LOADED(tTarget, "target");

	GetRandomImage(tChainTarget, TEXTURE_CHAINTARGET);
	if (!tChainTarget) {
		tChainTarget = loadTextureAnySize(EPRO_TEXT("chaintarget"_sv));
        ASSIGN_DEFAULT(tChainTarget);
    }
	ASSERT_TEXTURE_LOADED(tChainTarget, "chaintarget");

	GetRandomImage(tLim, TEXTURE_LIM);
	if (!tLim) {
		tLim = loadTextureAnySize(EPRO_TEXT("lim"_sv));
        ASSIGN_DEFAULT(tLim);
    }
	ASSERT_TEXTURE_LOADED(tLim, "lim");

	GetRandomImage(tOT, TEXTURE_OT);
	if (!tOT) {
	    tOT = loadTextureAnySize(EPRO_TEXT("ot"_sv));
        ASSIGN_DEFAULT(tOT);
    }
	ASSERT_TEXTURE_LOADED(tOT, "ot");

	GetRandomImagef(89, 128);
	if (!tHand[0]) {
		tHand[0] = loadTextureFixedSize(EPRO_TEXT("f1"_sv), 89, 128);
        ASSIGN_DEFAULT(tHand[0]);
    }
	ASSERT_TEXTURE_LOADED(tHand[0], "f1");
	if (!tHand[1]) {
		tHand[1] = loadTextureFixedSize(EPRO_TEXT("f2"_sv), 89, 128);
        ASSIGN_DEFAULT(tHand[1]);
    }
	ASSERT_TEXTURE_LOADED(tHand[1], "f2");
	if (!tHand[2]) {
		tHand[2] = loadTextureFixedSize(EPRO_TEXT("f3"_sv), 89, 128);
        ASSIGN_DEFAULT(tHand[2]);
    }
	ASSERT_TEXTURE_LOADED(tHand[2], "f3");

	GetRandomImage(tBackGround, TEXTURE_BACKGROUND);
	if (!tBackGround) {
		tBackGround = loadTextureAnySize(EPRO_TEXT("bg"_sv));
        ASSIGN_DEFAULT(tBackGround);
    }
	ASSERT_TEXTURE_LOADED(tBackGround, "bg");

	GetRandomImage(tBackGround_menu, TEXTURE_BACKGROUND_MENU);
	if (!tBackGround_menu)
		tBackGround_menu = loadTextureAnySize(EPRO_TEXT("bg_menu"_sv));
	if(!tBackGround_menu) {
		tBackGround_menu = tBackGround;
        ASSIGN_DEFAULT(tBackGround_menu);
    }

	GetRandomImage(tBackGround_deck, TEXTURE_BACKGROUND_DECK);
	if (!tBackGround_deck)
		tBackGround_deck = loadTextureAnySize(EPRO_TEXT("bg_deck"_sv));
	if(!tBackGround_deck) {
		tBackGround_deck = tBackGround;
        ASSIGN_DEFAULT(tBackGround_deck);
    }

	tBackGround_duel_topdown = loadTextureAnySize(EPRO_TEXT("bg_duel_topdown"_sv));
	if(!tBackGround_duel_topdown) {
		tBackGround_duel_topdown = tBackGround;
        ASSIGN_DEFAULT(tBackGround_duel_topdown);
    }

	GetRandomImage(tField[0][0], TEXTURE_field2);
	if (!tField[0][0]) {
		tField[0][0] = loadTextureAnySize(EPRO_TEXT("field2"_sv));
        ASSIGN_DEFAULT(tField[0][0]);
    }
	ASSERT_TEXTURE_LOADED(tField[0][0], "field2");

	GetRandomImage(tFieldTransparent[0][0], TEXTURE_field_transparent2);
	if (!tFieldTransparent[0][0]) {
		tFieldTransparent[0][0] = loadTextureAnySize(EPRO_TEXT("field-transparent2"_sv));
        ASSIGN_DEFAULT(tFieldTransparent[0][0]);
    }
	ASSERT_TEXTURE_LOADED(tFieldTransparent[0][0], "field-transparent2");

	GetRandomImage(tField[0][1], TEXTURE_field3);
	if (!tField[0][1]) {
		tField[0][1] = loadTextureAnySize(EPRO_TEXT("field3"_sv));
        ASSIGN_DEFAULT(tField[0][1]);
    }
	ASSERT_TEXTURE_LOADED(tField[0][1], "field3");

	GetRandomImage(tFieldTransparent[0][1], TEXTURE_field_transparent3);
	if (!tFieldTransparent[0][1]) {
		tFieldTransparent[0][1] = loadTextureAnySize(EPRO_TEXT("field-transparent3"_sv));
        ASSIGN_DEFAULT(tFieldTransparent[0][1]);
    }
	ASSERT_TEXTURE_LOADED(tFieldTransparent[0][1], "field-transparent3");

	GetRandomImage(tField[0][2], TEXTURE_field);
	if (!tField[0][2]) {
		tField[0][2] = loadTextureAnySize(EPRO_TEXT("field"_sv));
        ASSIGN_DEFAULT(tField[0][2]);
    }
	ASSERT_TEXTURE_LOADED(tField[0][2], "field");

	GetRandomImage(tFieldTransparent[0][2], TEXTURE_field_transparent);
	if (!tFieldTransparent[0][2]) {
		tFieldTransparent[0][2] = loadTextureAnySize(EPRO_TEXT("field-transparent"_sv));
        ASSIGN_DEFAULT(tFieldTransparent[0][2]);
    }
	ASSERT_TEXTURE_LOADED(tFieldTransparent[0][2], "field-transparent");

	GetRandomImage(tField[0][3], TEXTURE_field4);
	if (!tField[0][3]) {
		tField[0][3] = loadTextureAnySize(EPRO_TEXT("field4"_sv));
        ASSIGN_DEFAULT(tField[0][3]);
    }
    ASSERT_TEXTURE_LOADED(tField[0][3], "field4");

	GetRandomImage(tFieldTransparent[0][3], TEXTURE_field_transparent4);
	if (!tFieldTransparent[0][3]) {
		tFieldTransparent[0][3] = loadTextureAnySize(EPRO_TEXT("field-transparent4"_sv));
        ASSIGN_DEFAULT(tFieldTransparent[0][3]);
    }
	ASSERT_TEXTURE_LOADED(tFieldTransparent[0][3], "field-transparent4");

	GetRandomImage(tField[1][0], TEXTURE_field_fieldSP2);
	if (!tField[1][0]) {
		tField[1][0] = loadTextureAnySize(EPRO_TEXT("fieldSP2"_sv));
        ASSIGN_DEFAULT(tField[1][0]);
    }
	ASSERT_TEXTURE_LOADED(tField[1][0], "fieldSP2");

	GetRandomImage(tFieldTransparent[1][0], TEXTURE_field_transparentSP2);
	if (!tFieldTransparent[1][0]) {
		tFieldTransparent[1][0] = loadTextureAnySize(EPRO_TEXT("field-transparentSP2"_sv));
        ASSIGN_DEFAULT(tFieldTransparent[1][0]);
    }
	ASSERT_TEXTURE_LOADED(tFieldTransparent[1][0], "field-transparentSP2");

	GetRandomImage(tField[1][1], TEXTURE_fieldSP3);
	if (!tField[1][1]) {
		tField[1][1] = loadTextureAnySize(EPRO_TEXT("fieldSP3"_sv));
        ASSIGN_DEFAULT(tField[1][1]);
    }
	ASSERT_TEXTURE_LOADED(tField[1][1], "fieldSP3");

	GetRandomImage(tFieldTransparent[1][1], TEXTURE_field_transparentSP3);
	if (!tFieldTransparent[1][1]) {
		tFieldTransparent[1][1] = loadTextureAnySize(EPRO_TEXT("field-transparentSP3"_sv));
        ASSIGN_DEFAULT(tFieldTransparent[1][1]);
    }
	ASSERT_TEXTURE_LOADED(tFieldTransparent[1][1], "field-transparentSP3");

	GetRandomImage(tField[1][2], TEXTURE_fieldSP);
	if (!tField[1][2]) {
		tField[1][2] = loadTextureAnySize(EPRO_TEXT("fieldSP"_sv));
        ASSIGN_DEFAULT(tField[1][2]);
    }
	ASSERT_TEXTURE_LOADED(tField[1][2], "fieldSP");

	GetRandomImage(tFieldTransparent[1][2], TEXTURE_field_transparentSP);
	if (!tFieldTransparent[1][2]) {
		tFieldTransparent[1][2] = loadTextureAnySize(EPRO_TEXT("field-transparentSP"_sv));
        ASSIGN_DEFAULT(tFieldTransparent[1][2]);
    }
	ASSERT_TEXTURE_LOADED(tFieldTransparent[1][2], "field-transparentSP");

	GetRandomImage(tField[1][3], TEXTURE_fieldSP4);
	if (!tField[1][3]) {
		tField[1][3] = loadTextureAnySize(EPRO_TEXT("fieldSP4"_sv));
        ASSIGN_DEFAULT(tField[1][3]);
    }
	ASSERT_TEXTURE_LOADED(tField[1][3], "fieldSP4");

	GetRandomImage(tFieldTransparent[1][3], TEXTURE_field_transparentSP4);
	if (!tFieldTransparent[1][3]) {
		tFieldTransparent[1][3] = loadTextureAnySize(EPRO_TEXT("field-transparentSP4"_sv));
        ASSIGN_DEFAULT(tFieldTransparent[1][3]);
    }
	ASSERT_TEXTURE_LOADED(tFieldTransparent[1][3], "field-transparentSP4");

	GetRandomImage(tSettings, TEXTURE_SETTING);
	if (!tSettings) {
	    tSettings = loadTextureAnySize(EPRO_TEXT("settings"_sv));
        ASSIGN_DEFAULT(tSettings);
    }
	ASSERT_TEXTURE_LOADED(tSettings, "settings");

    char buff[100];
	for (int i = 0; i < 14; i++) {
		snprintf(buff, 100, "textures/pscale/rscale_%d.png", i);
		tRScale[i] = driver->getTexture(buff);
	}
	for (int i = 0; i < 14; i++) {
		snprintf(buff, 100, "textures/pscale/lscale_%d.png", i);
		tLScale[i] = driver->getTexture(buff);
	}
    modeBody[0] = driver->getTexture(0);
	for (uint32_t i = 1; i <= CHAPTER; i++) {
		snprintf(buff, 100, "./mode/story/body/%d.png", i);
        if(Utils::FileExists(Utils::ToPathString(buff)))
		    modeBody[i] = driver->getTexture(buff);
        if(modeBody[i] == nullptr) {
            snprintf(buff, 100, "./mode/story/body/%d.jpg", i);
			if(Utils::FileExists(Utils::ToPathString(buff)))
		        modeBody[i] = driver->getTexture(buff);
        }
	}
    ///kdiy/////

	// Not required to be present
	tCheckBox[0] = loadTextureAnySize(EPRO_TEXT("checkbox_16"_sv));
	ASSIGN_DEFAULT(tCheckBox[0]);

	tCheckBox[1] = loadTextureAnySize(EPRO_TEXT("checkbox_32"_sv));
	ASSIGN_DEFAULT(tCheckBox[1]);

	tCheckBox[2] = loadTextureAnySize(EPRO_TEXT("checkbox_64"_sv));
	ASSIGN_DEFAULT(tCheckBox[2]);


	sizes[0].first = sizes[1].first = toPow2(driver, CARD_IMG_WIDTH * gGameConfig->dpi_scale);
	sizes[0].second = sizes[1].second = toPow2(driver, CARD_IMG_HEIGHT * gGameConfig->dpi_scale);
	sizes[2].first = toPow2(driver, CARD_THUMB_WIDTH * gGameConfig->dpi_scale);
	sizes[2].second = toPow2(driver, CARD_THUMB_HEIGHT * gGameConfig->dpi_scale);
	return true;
}
//////kdiy//////
void ImageManager::SetAvatar(int player, const wchar_t *avatar) {
	auto string = EPRO_TEXT("./textures/character/custom/") + Utils::ToPathString(avatar) + EPRO_TEXT(".png");
    auto* tmp = driver->getTexture(string.c_str());
    if(tmp != nullptr) {
        if (scharacter[player][0])
            driver->removeTexture(scharacter[player][0]);
        scharacter[player][0] = tmp;
        if (scharacter[player][1])
            driver->removeTexture(scharacter[player][1]);
	    scharacter[player][1] = tmp;
    }
}
void ImageManager::RefreshRandomImageList() {
	RefreshImageDir(EPRO_TEXT("bg_deck"), TEXTURE_DECK);
	RefreshImageDir(EPRO_TEXT("bg_menu"), TEXTURE_MENU);
	RefreshImageDir(EPRO_TEXT("cover"), TEXTURE_COVERS);
	RefreshImageDir(EPRO_TEXT("cover2"), TEXTURE_COVERO);
	RefreshImageDir(EPRO_TEXT("attack"), TEXTURE_ATTACK);
	RefreshImageDir(EPRO_TEXT("act"), TEXTURE_ACTIVATE);
	RefreshImageDir(EPRO_TEXT("chain"), TEXTURE_CHAIN);
	RefreshImageDir(EPRO_TEXT("negated"), TEXTURE_NEGATED);
	RefreshImageDir(EPRO_TEXT("lp"), TEXTURE_LP);
	RefreshImageDir(EPRO_TEXT("lpf"), TEXTURE_LPf);
	RefreshImageDir(EPRO_TEXT("mask"), TEXTURE_MASK);
	RefreshImageDir(EPRO_TEXT("equip"), TEXTURE_EQUIP);
	RefreshImageDir(EPRO_TEXT("target"), TEXTURE_TARGET);
	RefreshImageDir(EPRO_TEXT("chaintarget"), TEXTURE_CHAINTARGET);
	RefreshImageDirf();
	RefreshImageDir(EPRO_TEXT("bg"), TEXTURE_BACKGROUND);
	RefreshImageDir(EPRO_TEXT("bg_menu"), TEXTURE_BACKGROUND_MENU);
	RefreshImageDir(EPRO_TEXT("bg_deck"), TEXTURE_BACKGROUND_DECK);
	RefreshImageDir(EPRO_TEXT("field2"), TEXTURE_field2);
	RefreshImageDir(EPRO_TEXT("field-transparent2"), TEXTURE_field_transparent2);
	RefreshImageDir(EPRO_TEXT("field3"), TEXTURE_field3);
	RefreshImageDir(EPRO_TEXT("field-transparent3"), TEXTURE_field_transparent3);
	RefreshImageDir(EPRO_TEXT("field"), TEXTURE_field);
	RefreshImageDir(EPRO_TEXT("field-transparent"), TEXTURE_field_transparent);
	RefreshImageDir(EPRO_TEXT("field4"), TEXTURE_field4);
	RefreshImageDir(EPRO_TEXT("field-transparent4"), TEXTURE_field_transparent4);
	RefreshImageDir(EPRO_TEXT("field-fieldSP2"), TEXTURE_field_fieldSP2);
	RefreshImageDir(EPRO_TEXT("field-transparentSP2"), TEXTURE_field_transparentSP2);
	RefreshImageDir(EPRO_TEXT("fieldSP3"), TEXTURE_fieldSP3);
	RefreshImageDir(EPRO_TEXT("field-transparentSP3"), TEXTURE_field_transparentSP3);
	RefreshImageDir(EPRO_TEXT("field-transparentSP"), TEXTURE_field_transparentSP);
	RefreshImageDir(EPRO_TEXT("fieldSP4"), TEXTURE_fieldSP4);
	RefreshImageDir(EPRO_TEXT("field-transparentSP4"), TEXTURE_field_transparentSP4);
	RefreshImageDir(EPRO_TEXT("unknown"), TEXTURE_UNKNOWN);
	RefreshImageDir(EPRO_TEXT("lim"), TEXTURE_LIM);
	RefreshImageDir(EPRO_TEXT("ot"), TEXTURE_OT);
	RefreshImageDir(EPRO_TEXT("settings"), TEXTURE_SETTING);

    RefreshImageDir(EPRO_TEXT("character/muto/icon"), TEXTURE_MUTO);
	RefreshImageDir(EPRO_TEXT("character/atem/icon"), TEXTURE_ATEM);
	RefreshImageDir(EPRO_TEXT("character/kaiba/icon"), TEXTURE_KAIBA);
	RefreshImageDir(EPRO_TEXT("character/joey/icon"), TEXTURE_JOEY);
	RefreshImageDir(EPRO_TEXT("character/marik/icon"), TEXTURE_MARIK);
    RefreshImageDir(EPRO_TEXT("character/dartz/icon"), TEXTURE_DARTZ);
	RefreshImageDir(EPRO_TEXT("character/bakura/icon"), TEXTURE_BAKURA);
	RefreshImageDir(EPRO_TEXT("character/aigami/icon"), TEXTURE_AIGAMI);
	RefreshImageDir(EPRO_TEXT("character/judai/icon"), TEXTURE_JUDAI);
	RefreshImageDir(EPRO_TEXT("character/manjome/icon"), TEXTURE_MANJOME);
	RefreshImageDir(EPRO_TEXT("character/kaisa/icon"), TEXTURE_KAISA);
	RefreshImageDir(EPRO_TEXT("character/phoenix/icon"), TEXTURE_PHORNIX);
	RefreshImageDir(EPRO_TEXT("character/john/icon"), TEXTURE_JOHN);
	RefreshImageDir(EPRO_TEXT("character/yubel/icon"), TEXTURE_YUBEL);
	RefreshImageDir(EPRO_TEXT("character/yusei/icon"), TEXTURE_YUSEI);
	RefreshImageDir(EPRO_TEXT("character/jack/icon"), TEXTURE_JACK);
	RefreshImageDir(EPRO_TEXT("character/arki/icon"), TEXTURE_ARKI);
	RefreshImageDir(EPRO_TEXT("character/crow/icon"), TEXTURE_CROW);
	RefreshImageDir(EPRO_TEXT("character/kiryu/icon"), TEXTURE_KIRYU);
	RefreshImageDir(EPRO_TEXT("character/paradox/icon"), TEXTURE_PARADOX);
	RefreshImageDir(EPRO_TEXT("character/zone/icon"), TEXTURE_ZONE);
	RefreshImageDir(EPRO_TEXT("character/yuma/icon"), TEXTURE_YUMA);
	RefreshImageDir(EPRO_TEXT("character/shark/icon"), TEXTURE_SHARK);
	RefreshImageDir(EPRO_TEXT("character/kaito/icon"), TEXTURE_KAITO);
	RefreshImageDir(EPRO_TEXT("character/iv/icon"), TEXTURE_IV);
	RefreshImageDir(EPRO_TEXT("character/Donthousand/icon"), TEXTURE_DONTHOUSAND);
	RefreshImageDir(EPRO_TEXT("character/yuya/icon"), TEXTURE_YUYA);
	RefreshImageDir(EPRO_TEXT("character/declan/icon"), TEXTURE_DECLAN);
	RefreshImageDir(EPRO_TEXT("character/shay/icon"), TEXTURE_SHAY);
	RefreshImageDir(EPRO_TEXT("character/playmaker/icon"), TEXTURE_PLAYMAKER);
	RefreshImageDir(EPRO_TEXT("character/soulburner/icon"), TEXTURE_SOULBURNER);
	RefreshImageDir(EPRO_TEXT("character/blueangel/icon"), TEXTURE_BLUEANGEL);
	
    RefreshImageDir(EPRO_TEXT("character/muto/damage"), TEXTURE_MUTO + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/atem/damage"), TEXTURE_ATEM + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/kaiba/damage"), TEXTURE_KAIBA + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/joey/damage"), TEXTURE_JOEY + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/marik/damage"), TEXTURE_MARIK + CHARACTER_VOICE - 1);
    RefreshImageDir(EPRO_TEXT("character/dartz/damage"), TEXTURE_DARTZ + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/bakura/damage"), TEXTURE_BAKURA + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/aigami/damage"), TEXTURE_AIGAMI + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/judai/damage"), TEXTURE_JUDAI + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/manjome/damage"), TEXTURE_MANJOME + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/kaisa/damage"), TEXTURE_KAISA + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/phoenix/damage"), TEXTURE_PHORNIX + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/john/damage"), TEXTURE_JOHN + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/yubel/damage"), TEXTURE_YUBEL + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/yusei/damage"), TEXTURE_YUSEI + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/jack/damage"), TEXTURE_JACK + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/arki/damage"), TEXTURE_ARKI + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/crow/damage"), TEXTURE_CROW + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/kiryu/damage"), TEXTURE_KIRYU + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/paradox/damage"), TEXTURE_PARADOX + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/zone/damage"), TEXTURE_ZONE + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/yuma/damage"), TEXTURE_YUMA + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/shark/damage"), TEXTURE_SHARK + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/kaito/damage"), TEXTURE_KAITO + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/iv/damage"), TEXTURE_IV + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/Donthousand/damage"), TEXTURE_DONTHOUSAND + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/yuya/damage"), TEXTURE_YUYA + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/declan/damage"), TEXTURE_DECLAN + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/shay/damage"), TEXTURE_SHAY + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/playmaker/damage"), TEXTURE_PLAYMAKER + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/soulburner/damage"), TEXTURE_SOULBURNER + CHARACTER_VOICE - 1);
	RefreshImageDir(EPRO_TEXT("character/blueangel/damage"), TEXTURE_BLUEANGEL + CHARACTER_VOICE - 1);

	for(int i = 0; i < 40 + CHARACTER_VOICE + CHARACTER_VOICE -2; ++i)
		saved_image_id[i] = -1;
}
void ImageManager::RefreshImageDir(epro::path_string path, int image_type) {
	for(auto file : Utils::FindFiles(BASE_PATH + path, { EPRO_TEXT("jpg"), EPRO_TEXT("png") }))
		ImageList[image_type].push_back(epro::format(EPRO_TEXT("{}{}/{}"), BASE_PATH, path, file));
}
void ImageManager::RefreshImageDirf() {
	for(auto& _folder : Utils::FindSubfolders(epro::format(EPRO_TEXT("{}morra/"), BASE_PATH), 1, false)) {
        bool f1 = false; bool f2 = false; bool f3 = false;
		auto folder = epro::format(EPRO_TEXT("{}morra/{}/"), BASE_PATH, _folder);
		for(auto file : Utils::FindFiles(folder, { EPRO_TEXT("jpg"), EPRO_TEXT("png") })) {
			if(Utils::ToUTF8IfNeeded(Utils::GetFileName(file)) == epro::format("f1"))
				f1 = true;
			if(Utils::ToUTF8IfNeeded(Utils::GetFileName(file)) == epro::format("f2"))
				f2 = true;
			if(Utils::ToUTF8IfNeeded(Utils::GetFileName(file)) == epro::format("f3"))
				f3 = true;
		}
        if(f1 && f2 && f3)
            ImageList[TEXTURE_F1].push_back(folder);
	}
}
void ImageManager::GetRandomImage(irr::video::ITexture*& src, int image_type, bool force_random) {
    int count = ImageList[image_type].size();
	if((!gGameConfig->randomtexture && !force_random) || count <= 0) {
        src = NULL;
        return;
    }
    if(saved_image_id[image_type] == -1)
		saved_image_id[image_type] = rand() % count;
	int image_id = saved_image_id[image_type];
	auto name = ImageList[image_type][image_id];
    irr::video::ITexture* tmp = driver->getTexture(name.c_str());
    if(tmp == nullptr)
        return;
	if(src != tmp) {
		driver->removeTexture(src);
		src = tmp;
	}
}
void ImageManager::GetRandomImage(irr::video::ITexture*& src, int image_type, int width, int height, bool force_random) {
	int count = ImageList[image_type].size();
	if((!gGameConfig->randomtexture && !force_random) || count <= 0) {
        src = NULL;
        return;
    }
    if(saved_image_id[image_type] == -1)
		saved_image_id[image_type] = rand() % count;
	int image_id = saved_image_id[image_type];
	auto name = ImageList[image_type][image_id];
    irr::video::ITexture* tmp = GetTextureFromFile(name.c_str(), width, height);
    if(tmp == nullptr)
        return;
	if(src != tmp) {
		driver->removeTexture(src);
		src = tmp;
	}
}
void ImageManager::GetRandomImagef(int width, int height) {
	int count = ImageList[TEXTURE_F1].size();
	if(!gGameConfig->randomtexture || count <= 0) {
		tHand[0] = NULL;
		tHand[1] = NULL;
		tHand[2] = NULL;
        return;
    }
    if(saved_image_id[TEXTURE_F1] == -1)
		saved_image_id[TEXTURE_F1] = rand() % count;
	int image_id = saved_image_id[TEXTURE_F1];
	for(auto file : Utils::FindFiles(ImageList[TEXTURE_F1][image_id], { EPRO_TEXT("jpg"), EPRO_TEXT("png") })) {
        irr::video::ITexture* tmp = GetTextureFromFile((ImageList[TEXTURE_F1][image_id] + file).c_str(), width, height);
        if(Utils::ToUTF8IfNeeded(Utils::GetFileName(file)) == epro::format("f1")) {
			if(tHand[0] != tmp) {
				driver->removeTexture(tHand[0]);
				tHand[0] = tmp;
			}
        }
        if(Utils::ToUTF8IfNeeded(Utils::GetFileName(file)) == epro::format("f2")) {
			if(tHand[1] != tmp) {
				driver->removeTexture(tHand[1]);
				tHand[1] = tmp;
			}
        }
        if(Utils::ToUTF8IfNeeded(Utils::GetFileName(file)) == epro::format("f3")) {
			if(tHand[2] != tmp) {
				driver->removeTexture(tHand[2]);
				tHand[2] = tmp;
			}
        }
	}
}
void ImageManager::RefreshKCGImage() {
	const wchar_t* textcharacter[] = {L"muto",L"atem",L"kaiba",L"joey",L"marik",L"dartz",L"bakura",L"aigami",L"judai",L"manjome",L"kaisa",L"phoenix",L"john",L"yubel",L"yusei",L"jack",L"arki",L"crow",L"kiryu",L"paradox",L"zone",L"yuma",L"shark",L"kaito",L"iv",L"DonThousand",L"yuya",L"declan",L"shay",L"playmaker",L"soulburner",L"blueangel"};
	int imgcharacter[] = {TEXTURE_MUTO,TEXTURE_ATEM,TEXTURE_KAIBA,TEXTURE_JOEY,TEXTURE_MARIK,TEXTURE_DARTZ,TEXTURE_BAKURA,TEXTURE_AIGAMI,TEXTURE_JUDAI,TEXTURE_MANJOME,TEXTURE_KAISA,TEXTURE_PHORNIX,TEXTURE_JOHN,TEXTURE_YUBEL,TEXTURE_YUSEI,TEXTURE_JACK,TEXTURE_ARKI,TEXTURE_CROW,TEXTURE_KIRYU,TEXTURE_PARADOX,TEXTURE_ZONE,TEXTURE_YUMA,TEXTURE_SHARK,TEXTURE_KAITO,TEXTURE_IV,TEXTURE_DONTHOUSAND,TEXTURE_YUYA,TEXTURE_DECLAN,TEXTURE_SHAY,TEXTURE_PLAYMAKER,TEXTURE_SOULBURNER,TEXTURE_BLUEANGEL};
    //if(!avatar_only) {
    for(uint8_t playno = 1; playno < CHARACTER_VOICE; playno++) {
        icon[playno] = driver->getTexture((EPRO_TEXT("./textures/character/") + Utils::ToPathString(textcharacter[playno-1]) + EPRO_TEXT("/mini_icon.png")).c_str());
        GetRandomImage(character[playno], imgcharacter[playno-1], true);
        if(!character[playno])
            character[playno] = driver->getTexture(0);
        GetRandomImage(characterd[playno], imgcharacter[playno-1] + CHARACTER_VOICE - 1, true);
        if(!characterd[playno]) {
            if(!character[playno])
                characterd[playno] = driver->getTexture(0);
            else
                characterd[playno] = character[playno];
        }
    }
    //} 
    // else if(gGameConfig->randomtexture && avataricon1 > 0 && avataricon2 > 0) {
    //     for(int i = 1; i < 3; i++) {
    //         int playno = avataricon1;
    //         if(i == 2) playno = avataricon2;
    //         GetRandomImage(character[playno], imgcharacter[playno-1], true);
    //         saved_image_id[imgcharacter[playno-1]] = -1;
    //         if(!character[playno])
    //             character[playno] = driver->getTexture(0);
    //         GetRandomImage(characterd[playno], imgcharacter[playno-1] + CHARACTER_VOICE - 1, true);
    //         saved_image_id[imgcharacter[playno-1] + CHARACTER_VOICE - 1] = -1;
    //         if(!characterd[playno]) {
    //             if(!character[playno])
    //                 characterd[playno] = driver->getTexture(0);
    //             else
    //                 characterd[playno] = character[playno];
    //         }
    //     }
    // }
}
//////kdiy//////
void ImageManager::replaceTextureLoadingFixedSize(irr::video::ITexture*& texture, irr::video::ITexture* fallback, epro::path_stringview texture_name, int width, int height) {
	auto* tmp = loadTextureFixedSize(texture_name, width, height);
	if(!tmp)
		tmp = fallback;
	if(texture != fallback)
		driver->removeTexture(texture);
	texture = tmp;
}
void ImageManager::replaceTextureLoadingAnySize(irr::video::ITexture*& texture, irr::video::ITexture* fallback, epro::path_stringview texture_name) {
	auto* tmp = loadTextureAnySize(texture_name);
	if(!tmp)
		tmp = fallback;
	if(texture != fallback)
		driver->removeTexture(texture);
	texture = tmp;
}
#define REPLACE_TEXTURE_WITH_FIXED_SIZE(obj,name,w,h) replaceTextureLoadingFixedSize(obj, def_##obj, EPRO_TEXT(name) ""_sv, w, h)
#define REPLACE_TEXTURE_ANY_SIZE(obj,name) replaceTextureLoadingAnySize(obj, def_##obj, EPRO_TEXT(name) ""_sv)

void ImageManager::ChangeTextures(epro::path_stringview _path) {
	/////kdiy//////
	// if(_path == textures_path)
	// 	return;
	// textures_path.assign(_path.data(), _path.size());
	if (!gGameConfig->randomtexture)
        return;
	/////kdiy//////
	const bool is_base = textures_path == BASE_PATH;
	/////kdiy//////
    // REPLACE_TEXTURE_ANY_SIZE(tAct, "act");
	// REPLACE_TEXTURE_ANY_SIZE(tAttack, "attack");
	// REPLACE_TEXTURE_ANY_SIZE(tChain, "chain");
	// REPLACE_TEXTURE_WITH_FIXED_SIZE(tNegated, "negated", 128, 128);
	// REPLACE_TEXTURE_WITH_FIXED_SIZE(tNumber, "number", 320, 256);
	// REPLACE_TEXTURE_ANY_SIZE(tLPBar, "lp");
	// REPLACE_TEXTURE_ANY_SIZE(tLPFrame, "lpf");
	// REPLACE_TEXTURE_WITH_FIXED_SIZE(tMask, "mask", 254, 254);
	// REPLACE_TEXTURE_ANY_SIZE(tEquip, "equip");
	// REPLACE_TEXTURE_ANY_SIZE(tTarget, "target");
	// REPLACE_TEXTURE_ANY_SIZE(tChainTarget, "chaintarget");
	// REPLACE_TEXTURE_ANY_SIZE(tLim, "lim");
	// REPLACE_TEXTURE_ANY_SIZE(tOT, "ot");
	// REPLACE_TEXTURE_WITH_FIXED_SIZE(tHand[0], "f1", 89, 128);
	// REPLACE_TEXTURE_WITH_FIXED_SIZE(tHand[1], "f2", 89, 128);
	// REPLACE_TEXTURE_WITH_FIXED_SIZE(tHand[2], "f3", 89, 128);
	// REPLACE_TEXTURE_ANY_SIZE(tBackGround, "bg");
	// REPLACE_TEXTURE_ANY_SIZE(tBackGround_menu, "bg_menu");
	// if(!is_base && tBackGround != def_tBackGround && tBackGround_menu == def_tBackGround_menu)
	// 	tBackGround_menu = tBackGround;
	// REPLACE_TEXTURE_ANY_SIZE(tBackGround_deck, "bg_deck");
	// if(!is_base && tBackGround != def_tBackGround && tBackGround_deck == def_tBackGround_deck)
	// 	tBackGround_deck = tBackGround;
	// REPLACE_TEXTURE_ANY_SIZE(tBackGround_duel_topdown, "bg_duel_topdown");
	// if(!is_base && tBackGround != def_tBackGround && tBackGround_duel_topdown == def_tBackGround_duel_topdown)
	// 	tBackGround_duel_topdown = tBackGround;
	// REPLACE_TEXTURE_ANY_SIZE(tField[0][0], "field2");
	// REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[0][0], "field-transparent2");
	// REPLACE_TEXTURE_ANY_SIZE(tField[0][1], "field3");
	// REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[0][1], "field-transparent3");
	// REPLACE_TEXTURE_ANY_SIZE(tField[0][2], "field");
	// REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[0][2], "field-transparent");
	// REPLACE_TEXTURE_ANY_SIZE(tField[0][3], "field4");
	// REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[0][3], "field-transparent4");
	// REPLACE_TEXTURE_ANY_SIZE(tField[1][0], "fieldSP2");
	// REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[1][0], "field-transparentSP2");
	// REPLACE_TEXTURE_ANY_SIZE(tField[1][1], "fieldSP3");
	// REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[1][1], "field-transparentSP3");
	// REPLACE_TEXTURE_ANY_SIZE(tField[1][2], "fieldSP");
	// REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[1][2], "field-transparentSP");
	// REPLACE_TEXTURE_ANY_SIZE(tField[1][3], "fieldSP4");
	// REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[1][3], "field-transparentSP4");
	// REPLACE_TEXTURE_ANY_SIZE(tSettings, "settings");
#ifdef VIP
    RefreshKCGImage();
#endif
    GetRandomImage(tAct, TEXTURE_ACTIVATE);
	if(!tAct)
	    REPLACE_TEXTURE_ANY_SIZE(tAct, "act");
	GetRandomImage(tAttack, TEXTURE_ATTACK);
	if(!tAttack)
	    REPLACE_TEXTURE_ANY_SIZE(tAttack, "attack");
	GetRandomImage(tChain, TEXTURE_CHAIN);
	if(!tChain)
	    REPLACE_TEXTURE_ANY_SIZE(tChain, "chain");
	GetRandomImage(tNegated, TEXTURE_NEGATED, 128, 128);
	if(!tNegated)
	    REPLACE_TEXTURE_WITH_FIXED_SIZE(tNegated, "negated", 128, 128);
	GetRandomImage(tLPBar, TEXTURE_LP);
	if(!tLPBar)
	    REPLACE_TEXTURE_ANY_SIZE(tLPBar, "lp");
	GetRandomImage(tLPFrame, TEXTURE_LPf);
	if(!tLPFrame)
	    REPLACE_TEXTURE_ANY_SIZE(tLPFrame, "lpf");
	GetRandomImage(tMask, TEXTURE_MASK, 254, 254);
	if(!tMask)
	    REPLACE_TEXTURE_WITH_FIXED_SIZE(tMask, "mask", 254, 254);
	GetRandomImage(tEquip, TEXTURE_EQUIP);
	if(!tEquip)
	    REPLACE_TEXTURE_ANY_SIZE(tEquip, "equip");
	GetRandomImage(tTarget, TEXTURE_TARGET);
	if(!tTarget)
	    REPLACE_TEXTURE_ANY_SIZE(tTarget, "target");
	GetRandomImage(tChainTarget, TEXTURE_CHAINTARGET);
	if(!tChainTarget)
	    REPLACE_TEXTURE_ANY_SIZE(tChainTarget, "chaintarget");
	GetRandomImage(tLim, TEXTURE_LIM);
	if(!tLim)
	    REPLACE_TEXTURE_ANY_SIZE(tLim, "lim");
	GetRandomImage(tOT, TEXTURE_OT);
	if(!tOT)
	    REPLACE_TEXTURE_ANY_SIZE(tOT, "ot");
	GetRandomImagef(89, 128);
	if(!tHand[0])
	    REPLACE_TEXTURE_WITH_FIXED_SIZE(tHand[0], "f1", 89, 128);
	if(!tHand[1])
	    REPLACE_TEXTURE_WITH_FIXED_SIZE(tHand[1], "f2", 89, 128);
	if(!tHand[2])
	    REPLACE_TEXTURE_WITH_FIXED_SIZE(tHand[2], "f3", 89, 128);
	GetRandomImage(tBackGround, TEXTURE_BACKGROUND);
	if(!tBackGround)
	    REPLACE_TEXTURE_ANY_SIZE(tBackGround, "bg");
	GetRandomImage(tBackGround_menu, TEXTURE_BACKGROUND_MENU);
	if (!tBackGround_menu)
		tBackGround_menu = loadTextureAnySize(EPRO_TEXT("bg_menu"_sv));
	if(!tBackGround_menu) {
		tBackGround_menu = tBackGround;
    }

	GetRandomImage(tBackGround_deck, TEXTURE_BACKGROUND_DECK);
	if (!tBackGround_deck)
		tBackGround_deck = loadTextureAnySize(EPRO_TEXT("bg_deck"_sv));
	if(!tBackGround_deck) {
		tBackGround_deck = tBackGround;
    }

	GetRandomImage(tField[0][0], TEXTURE_field2);
	if(!tField[0][0])
	    REPLACE_TEXTURE_ANY_SIZE(tField[0][0], "field2");
	GetRandomImage(tFieldTransparent[0][0], TEXTURE_field_transparent2);
	if(!tFieldTransparent[0][0])
	    REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[0][0], "field-transparent2");
	GetRandomImage(tField[0][1], TEXTURE_field3);
	if(!tField[0][1])
	    REPLACE_TEXTURE_ANY_SIZE(tField[0][1], "field3");
	GetRandomImage(tFieldTransparent[0][1], TEXTURE_field_transparent3);
	if(!tFieldTransparent[0][1])
	    REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[0][1], "field-transparent3");
	GetRandomImage(tField[0][2], TEXTURE_field);
	if(!tField[0][2])
	    REPLACE_TEXTURE_ANY_SIZE(tField[0][2], "field");
	GetRandomImage(tFieldTransparent[0][2], TEXTURE_field_transparent);
	if(!tFieldTransparent[0][2])
	    REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[0][2], "field-transparent");
	GetRandomImage(tField[0][3], TEXTURE_field4);
	if(!tField[0][3])
	    REPLACE_TEXTURE_ANY_SIZE(tField[0][3], "field4");
	GetRandomImage(tFieldTransparent[0][3], TEXTURE_field_transparent4);
	if(!tFieldTransparent[0][3])
	    REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[0][3], "field-transparent4");
	GetRandomImage(tField[1][0], TEXTURE_field_fieldSP2);
	if(!tField[1][0])
	    REPLACE_TEXTURE_ANY_SIZE(tField[1][0], "fieldSP2");
	GetRandomImage(tFieldTransparent[1][0], TEXTURE_field_transparentSP2);
	if(!tFieldTransparent[1][0])
	    REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[1][0], "field-transparentSP2");
	GetRandomImage(tField[1][1], TEXTURE_fieldSP3);
	if(!tField[1][1])
	    REPLACE_TEXTURE_ANY_SIZE(tField[1][1], "fieldSP3");
	GetRandomImage(tFieldTransparent[1][1], TEXTURE_field_transparentSP3);
	if(!tFieldTransparent[1][1])
	    REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[1][1], "field-transparentSP3");
	GetRandomImage(tField[1][2], TEXTURE_fieldSP);
	if(!tField[1][2])
	    REPLACE_TEXTURE_ANY_SIZE(tField[1][2], "fieldSP");
	GetRandomImage(tFieldTransparent[1][2], TEXTURE_field_transparentSP);
	if(!tFieldTransparent[1][2])
	    REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[1][2], "field-transparentSP");
	GetRandomImage(tField[1][3], TEXTURE_fieldSP4);
	if(!tField[1][3])
	    REPLACE_TEXTURE_ANY_SIZE(tField[1][3], "fieldSP4");
	GetRandomImage(tFieldTransparent[1][3], TEXTURE_field_transparentSP4);
	if(!tFieldTransparent[1][3])
	    REPLACE_TEXTURE_ANY_SIZE(tFieldTransparent[1][3], "field-transparentSP4");
	GetRandomImage(tSettings, TEXTURE_SETTING);
	if(!tSettings)
	    REPLACE_TEXTURE_ANY_SIZE(tSettings, "settings");
	REPLACE_TEXTURE_WITH_FIXED_SIZE(tNumber, "number", 320, 256);
	REPLACE_TEXTURE_ANY_SIZE(tBackGround_duel_topdown, "bg_duel_topdown");
	if(!tBackGround_duel_topdown)
		tBackGround_duel_topdown = tBackGround;
    /////kdiy//////
	REPLACE_TEXTURE_ANY_SIZE(tCheckBox[0], "checkbox_16");
	REPLACE_TEXTURE_ANY_SIZE(tCheckBox[1], "checkbox_32");
	REPLACE_TEXTURE_ANY_SIZE(tCheckBox[2], "checkbox_64");
	RefreshCovers();
}
#undef REPLACE_TEXTURE_ANY_SIZE
#undef REPLACE_TEXTURE_WITH_FIXED_SIZE
void ImageManager::ResetTextures() {
	ChangeTextures(BASE_PATH);
}
void ImageManager::SetDevice(irr::IrrlichtDevice* dev) {
	device = dev;
	driver = dev->getVideoDriver();
}
void ImageManager::ClearTexture(bool resize) {
	auto ClearMap = [&](texture_map &map) {
		for(const auto& tit : map) {
			if(tit.second.texture) {
				driver->removeTexture(tit.second.texture);
			}
		}
		map.clear();
	};
	if(resize) {
		const auto card_sizes = mainGame->imgCard->getRelativePosition().getSize();
		sizes[1].first = toPow2(driver, card_sizes.Width);
		sizes[1].second = toPow2(driver, card_sizes.Height);
		sizes[2].first = toPow2(driver, CARD_THUMB_WIDTH * mainGame->window_scale.X * gGameConfig->dpi_scale);
		sizes[2].second = toPow2(driver, CARD_THUMB_HEIGHT * mainGame->window_scale.Y * gGameConfig->dpi_scale);
		RefreshCovers();
	} else
		ClearCachedTextures();
	ClearMap(tMap[0]);
	ClearMap(tMap[1]);
	ClearMap(tThumb);
	ClearMap(tCovers);
	for(const auto& tit : tFields) {
		if(tit.second) {
			driver->removeTexture(tit.second);
		}
	}
	tFields.clear();
	/////////kdiy////
	for(const auto& tit : tCloseup) {
		if(tit.second) {
			driver->removeTexture(tit.second);
		}
	}
	tCloseup.clear();
	/////////kdiy////
}
void ImageManager::RefreshCachedTextures() {
	auto LoadTexture = [this](int index, texture_map& dest, auto& size, imgType type) {
		auto& src = loaded_pics[index];
		std::vector<uint32_t> readd;
		for(int i = 0; i < gGameConfig->maxImagesPerFrame; i++) {
			std::unique_lock<epro::mutex> lck(pic_load);
			if(src.empty())
				break;
			auto loaded = std::move(src.front());
			src.pop_front();
			lck.unlock();
			auto& map_elem = dest[loaded.code];
			if(loaded.status == loadStatus::WAIT_DOWNLOAD) {
				map_elem.preload_status = preloadStatus::WAIT_DOWNLOAD;
				continue;
			}
			auto& ret_texture = map_elem.texture;
			map_elem.preload_status = preloadStatus::LOADED;
			if(loaded.status == loadStatus::LOAD_FAIL) {
				ret_texture = nullptr;
				continue;
			}
			auto* texture = loaded.texture;
			if(texture->getDimension().Width != static_cast<irr::u32>(size.first) || texture->getDimension().Height != static_cast<irr::u32>(size.second)) {
				readd.push_back(loaded.code);
				ret_texture = nullptr;
				continue;
			}
			ret_texture = driver->addTexture({ loaded.path.data(), static_cast<irr::u32>(loaded.path.size()) }, texture);
			texture->drop();
		}
		if(readd.size()) {
			std::lock_guard<epro::mutex> lck(pic_load);
			for(auto& code : readd)
				to_load.emplace_front(code, type, index, std::ref(size.first), std::ref(size.second), timestamp_id, std::ref(timestamp_id));
			cv_load.notify_all();
		}
	};
	LoadTexture(0, tMap[0], sizes[0], imgType::ART);
	LoadTexture(1, tMap[1], sizes[1], imgType::ART);
	LoadTexture(2, tThumb, sizes[2], imgType::THUMB);
	LoadTexture(3, tCovers, sizes[1], imgType::COVER);
}
void ImageManager::ClearFutureObjects() {
	Utils::SetThreadName("ImgObjsClear");
	while(!stop_threads) {
		std::unique_lock<epro::mutex> lck(obj_clear_lock);
		while(to_clear.empty()) {
			cv_clear.wait(lck);
			if(stop_threads)
				return;
		}
		auto img = std::move(to_clear.front());
		to_clear.pop_front();
		lck.unlock();
		if(img.texture)
			img.texture->drop();
	}
}

void ImageManager::RefreshCovers() {
	const auto is_base_path = textures_path == BASE_PATH;
	auto reloadTextureWithNewSizes = [this, is_base_path, width = (int)sizes[1].first, height = (int)sizes[1].second](auto*& texture, epro::path_stringview texture_name) {
		auto new_texture = loadTextureFixedSize(texture_name, width, height);
		if(!new_texture && !is_base_path) {
			const auto old_textures_path = std::exchange(textures_path, BASE_PATH);
			new_texture = loadTextureFixedSize(texture_name, width, height);
			textures_path = old_textures_path;
		}
		if(!new_texture)
			return;
		driver->removeTexture(std::exchange(texture, new_texture));
	};
	reloadTextureWithNewSizes(tCover[0], EPRO_TEXT("cover"_sv));
	driver->removeTexture(std::exchange(tCover[1], nullptr));
	reloadTextureWithNewSizes(tCover[1], EPRO_TEXT("cover2"_sv));
	if(!tCover[1])
		tCover[1] = tCover[0];
	reloadTextureWithNewSizes(tUnknown, EPRO_TEXT("unknown"_sv));
}
void ImageManager::LoadPic() {
	Utils::SetThreadName("PicLoader");
	while(!stop_threads) {
		std::unique_lock<epro::mutex> lck(pic_load);
		while(to_load.empty()) {
			cv_load.wait(lck);
			if(stop_threads) {
				return;
			}
		}
		auto loaded = std::move(to_load.front());
		to_load.pop_front();
		lck.unlock();
		auto load_status = LoadCardTexture(loaded.code, loaded.type, loaded.reference_width, loaded.reference_height, loaded.timestamp, loaded.reference_timestamp);
		lck.lock();
		loaded_pics[loaded.index].push_front(std::move(load_status));
	}
}
void ImageManager::ClearCachedTextures() {
	timestamp_id = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	std::lock_guard<epro::mutex> lck(obj_clear_lock);
	{
		std::lock_guard<epro::mutex> lck2(pic_load);
		for(auto& map : loaded_pics) {
			to_clear.insert(to_clear.end(), std::make_move_iterator(map.begin()), std::make_move_iterator(map.end()));
			map.clear();
		}
		to_load.clear();
	}
	cv_clear.notify_one();
}
// function by Warr1024, from https://github.com/minetest/minetest/issues/2419 , modified
bool ImageManager::imageScaleNNAA(irr::video::IImage* src, irr::video::IImage* dest, chrono_time timestamp_id, const std::atomic<chrono_time>& source_timestamp_id) {
	// Cache rectsngle boundaries.
	auto& sdim = src->getDimension();
	const double sw = sdim.Width;
	const double sh = sdim.Height;

	// Walk each destination image pixel.
	// Note: loop y around x for better cache locality.
	const auto& dim = dest->getDimension();
	const auto divw = sw / dim.Width;
	const auto divh = sh / dim.Height;
	irr::u32 dy = 0;
	for(; dy < dim.Height && timestamp_id == source_timestamp_id; dy++) {
		for(irr::u32 dx = 0; dx < dim.Width; dx++) {
			// Calculate floating-point source rectangle bounds.
			const double minsx = dx * divw;
			const double maxsx = minsx + divw;
			const double minsy = dy * divh;
			const double maxsy = minsy + divh;

			// Total area, and integral of r, g, b values over that area,
			// initialized to zero, to be summed up in next loops.
			double area = 0, ra = 0, ga = 0, ba = 0, aa = 0;
			irr::video::SColor pxl;
			const auto csy = std::floor(minsy);
			const auto csx = std::floor(minsx);
			// Loop over the integral pixel positions described by those bounds.
			for(double sy = csy; sy < maxsy; sy++)
				for(double sx = csx; sx < maxsx; sx++) {
					// Calculate width, height, then area of dest pixel
					// that's covered by this source pixel.
					double pw = 1;
					if(minsx > sx)
						pw += sx - minsx;
					if(maxsx < (sx + 1))
						pw += maxsx - sx - 1;
					double ph = 1;
					if(minsy > sy)
						ph += sy - minsy;
					if(maxsy < (sy + 1))
						ph += maxsy - sy - 1;
					const double pa = pw * ph;

					// Get source pixel and add it to totals, weighted
					// by covered area and alpha.
					pxl = src->getPixel(sx, sy);
					area += pa;
					ra += pa * pxl.getRed();
					ga += pa * pxl.getGreen();
					ba += pa * pxl.getBlue();
					aa += pa * pxl.getAlpha();
				}

			// Set the destination image pixel to the average color.
			if(area > 0) {
				pxl.setRed(ra / area + 0.5);
				pxl.setGreen(ga / area + 0.5);
				pxl.setBlue(ba / area + 0.5);
				pxl.setAlpha(aa / area + 0.5);
			} else {
				pxl.setRed(0);
				pxl.setGreen(0);
				pxl.setBlue(0);
				pxl.setAlpha(0);
			}
			dest->setPixel(dx, dy, pxl);
		}
	}
	return dy == dim.Height;
}
irr::video::IImage* ImageManager::GetScaledImage(irr::video::IImage* srcimg, int width, int height, chrono_time call_timestamp_id, const std::atomic<chrono_time>& source_timestamp_id) {
	if(width <= 0 || height <= 0)
		return nullptr;
	if(!srcimg || call_timestamp_id != source_timestamp_id.load())
		return nullptr;
	const irr::core::dimension2d<irr::u32> dim(width, height);
	if(srcimg->getDimension() == dim) {
		srcimg->grab();
		return srcimg;
	} else {
		irr::video::IImage* destimg = driver->createImage(srcimg->getColorFormat(), dim);
		if(call_timestamp_id != source_timestamp_id || !imageScaleNNAA(srcimg, destimg, call_timestamp_id, source_timestamp_id)) {
			destimg->drop();
			destimg = nullptr;
		}
		return destimg;
	}
}
irr::video::ITexture* ImageManager::GetTextureFromFile(const irr::io::path& file, int width, int height) {
	auto img = GetScaledImageFromFile(file, width, height);
	if(img) {
		auto texture = driver->addTexture(file, img);
		img->drop();
		if(texture)
			return texture;
	}
	return driver->getTexture(file);
}
ImageManager::load_return ImageManager::LoadCardTexture(uint32_t code, imgType type, const std::atomic<irr::s32>& _width, const std::atomic<irr::s32>& _height, chrono_time call_timestamp_id, const std::atomic<chrono_time>& source_timestamp_id) {
	int width = _width;
	int height = _height;
	if(type == imgType::THUMB)
		type = imgType::ART;
	load_return ret{ loadStatus::LOAD_FAIL, code };
	auto LoadImg = [&](irr::video::IImage* base_img)->irr::video::IImage* {
		if(!base_img)
			return nullptr;
		if(width != _width || height != _height) {
			width = _width;
			height = _height;
		}
		while(const auto img = GetScaledImage(base_img, width, height, call_timestamp_id, source_timestamp_id)) {
			if(call_timestamp_id != source_timestamp_id.load()) {
				img->drop();
				base_img->drop();
				return nullptr;
			}
			if(width != _width || height != _height) {
				img->drop();
				width = _width;
				height = _height;
				continue;
			}
			base_img->drop();
			return img;
		}
		base_img->drop();
		return nullptr;
	};
	irr::video::IImage* img;
	auto status = gImageDownloader->GetDownloadStatus(code, type);
	if(status == ImageDownloader::downloadStatus::DOWNLOADED) {
		if(call_timestamp_id != source_timestamp_id.load())
			return ret;
		const auto file = gImageDownloader->GetDownloadPath(code, type);
		if((img = LoadImg(driver->createImageFromFile({ file.data(), static_cast<irr::u32>(file.size()) }))) != nullptr) {
			ret.status = loadStatus::LOAD_OK;
			ret.path = epro::path_string{ file };
			ret.texture = img;
		}
		return ret;
	} else if(status == ImageDownloader::downloadStatus::NONE) {
		for(auto& path : (type == imgType::ART) ? mainGame->pic_dirs : mainGame->cover_dirs) {
			for(auto extension : { EPRO_TEXT(".png"), EPRO_TEXT(".jpg") }) {
				if(call_timestamp_id != source_timestamp_id.load())
					return ret;
				irr::video::IImage* base_img = nullptr;
				epro::path_string file;
				if(path == EPRO_TEXT("archives")) {
					auto archiveFile = Utils::FindFileInArchives(
						(type == imgType::ART) ? EPRO_TEXT("pics/") : EPRO_TEXT("pics/cover/"),
						epro::format(EPRO_TEXT("{}{}"), code, extension));
					if(!archiveFile)
						continue;
					const auto& name = archiveFile->getFileName();
					file = { name.c_str(), name.size() };
					base_img = driver->createImageFromFile(archiveFile);
					archiveFile->drop();
				} else {
					file = epro::format(EPRO_TEXT("{}{}{}"), path, code, extension);
					base_img = driver->createImageFromFile({ file.data(), static_cast<irr::u32>(file.size()) });
				}
				if((img = LoadImg(base_img)) != nullptr) {
					ret.status = loadStatus::LOAD_OK;
					ret.path = file;
					ret.texture = img;
					return ret;
				}
			}
		}
		gImageDownloader->AddToDownloadQueue(code, type);
		ret.status = loadStatus::WAIT_DOWNLOAD;
		return ret;
	}
	return ret;
}
irr::video::ITexture* ImageManager::GetTextureCard(uint32_t code, imgType type, bool wait, bool fit, int* chk) {
	if(chk)
		*chk = 1;
	irr::video::ITexture* ret_unk = tUnknown;
	int index;
	int size_index;
	auto& map = [&]()->texture_map& {
		switch(type) {
			case imgType::ART: {
				index = fit ? 1 : 0;
				size_index = index;
				return tMap[fit ? 1 : 0];
			}
			case imgType::THUMB: {
				index = 2;
				size_index = index;
				return tThumb;
			}
			case imgType::COVER: {
				ret_unk = tCover[0];
				index = 3;
				size_index = 0;
				return tCovers;
			}
			default:
				unreachable();
		}
	}();
	if(code == 0)
		return ret_unk;
	auto& elem = map[code];
	if(elem.preload_status != preloadStatus::LOADED) {
		auto status = gImageDownloader->GetDownloadStatus(code, type);
		if(status == ImageDownloader::downloadStatus::DOWNLOADING) {
			if(chk)
				*chk = 2;
			return ret_unk;
		}
		//pic will be loaded below instead
		/*if(status == ImageDownloader::DOWNLOADED) {
			map[code] = driver->getTexture(gImageDownloader->GetDownloadPath(code, type).data());
			return map[code] ? map[code] : ret_unk;
		}*/
		if(status == ImageDownloader::downloadStatus::DOWNLOAD_ERROR) {
			map[code].texture = nullptr;
			return ret_unk;
		}
		if(chk)
			*chk = 2;
		if(elem.preload_status == preloadStatus::NONE || (elem.preload_status == preloadStatus::WAIT_DOWNLOAD && status == ImageDownloader::downloadStatus::DOWNLOADED)) {
			elem.preload_status = preloadStatus::LOADING;
			if(wait) {
				auto load_result = LoadCardTexture(code, type, sizes[size_index].first, sizes[size_index].second, timestamp_id, timestamp_id);
				auto& rmap = map[code].texture;
				if(load_result.status == loadStatus::LOAD_OK) {
					rmap = driver->addTexture(load_result.path.data(), load_result.texture);
					load_result.texture->drop();
					if(chk)
						*chk = 1;
				} else {
					rmap = nullptr;
					if(chk)
						*chk = 0;
				}
				return (rmap) ? rmap : ret_unk;
			} else {
				std::lock_guard<epro::mutex> lck(pic_load);
				to_load.emplace_front(code, type, index, std::ref(sizes[size_index].first), std::ref(sizes[size_index].second), timestamp_id.load(), std::ref(timestamp_id));
				cv_load.notify_one();
			}
		}
		return ret_unk;
	}
	auto* texture = elem.texture;
	if(chk && texture == nullptr)
		*chk = 0;
	if(texture)
		return texture;
	return ret_unk;
}
irr::video::ITexture* ImageManager::GetTextureField(uint32_t code) {
	if(code == 0)
		return nullptr;
	auto tit = tFields.find(code);
	if(tit != tFields.end())
		return tit->second;
	auto status = gImageDownloader->GetDownloadStatus(code, imgType::FIELD);
	if(status != ImageDownloader::downloadStatus::NONE) {
		if(status == ImageDownloader::downloadStatus::DOWNLOADED) {
			const auto path = gImageDownloader->GetDownloadPath(code, imgType::FIELD);
			auto downloaded = driver->getTexture({ path.data(), static_cast<irr::u32>(path.size()) });
			tFields.emplace(code, downloaded);
			return downloaded;
		}
		return nullptr;
	}
	for(auto& path : mainGame->field_dirs) {
		for(auto extension : { EPRO_TEXT(".png"), EPRO_TEXT(".jpg") }) {
			irr::video::ITexture* img;
			if(path == EPRO_TEXT("archives")) {
				auto archiveFile = Utils::FindFileInArchives(EPRO_TEXT("pics/field/"), epro::format(EPRO_TEXT("{}{}"), code, extension));
				if(!archiveFile)
					continue;
				img = driver->getTexture(archiveFile);
				archiveFile->drop();
			} else
				img = driver->getTexture(epro::format(EPRO_TEXT("{}{}{}"), path, code, extension).data());
			if(img) {
				tFields.emplace(code, img);
				return img;
			}
		}
	}
	gImageDownloader->AddToDownloadQueue(code, imgType::FIELD);
	return nullptr;
}
/////////kdiy////
irr::video::ITexture* ImageManager::GetTextureCloseup(uint32_t code) {
	if(code == 0 || !gGameConfig->closeup)
		return nullptr;
	auto tit = tCloseup.find(code);
	if(tit != tCloseup.end())
		return tit->second;
	auto status = gImageDownloader->GetDownloadStatus(code, imgType::CLOSEUP);
	if(status != ImageDownloader::downloadStatus::NONE) {
		if(status == ImageDownloader::downloadStatus::DOWNLOADED) {
			const auto path = gImageDownloader->GetDownloadPath(code, imgType::CLOSEUP);
			auto downloaded = driver->getTexture({ path.data(), static_cast<irr::u32>(path.size()) });
			tCloseup.emplace(code, downloaded);
			return downloaded;
		}
		return nullptr;
	}
	for(auto& path : mainGame->closeup_dirs) {
		for(auto extension : { EPRO_TEXT(".png"), EPRO_TEXT(".jpg") }) {
			irr::video::ITexture* img;
			if(path == EPRO_TEXT("archives")) {
				auto archiveFile = Utils::FindFileInArchives(EPRO_TEXT("pics/closeup/"), epro::format(EPRO_TEXT("{}{}"), code, extension));
				if(!archiveFile)
					continue;
				img = driver->getTexture(archiveFile);
				archiveFile->drop();
			} else
				img = driver->getTexture(epro::format(EPRO_TEXT("{}{}{}"), path, code, extension).data());
			if(img) {
				tCloseup.emplace(code, img);
				return img;
			}
		}
	}
	gImageDownloader->AddToDownloadQueue(code, imgType::CLOSEUP);
	return nullptr;
}
/////////kdiy////

irr::video::ITexture* ImageManager::GetCheckboxScaledTexture(float scale) {
	if(scale > 3.5f && tCheckBox[2])
			return tCheckBox[2];
	if(scale > 2.0f && tCheckBox[1])
		return tCheckBox[1];
	return tCheckBox[0];
}


/*
From minetest: Copyright (C) 2015 Aaron Suen <warr1024@gmail.com>
https://github.com/minetest/minetest/blob/5506e97ed897dde2d4820fe1b021a4622bae03b3/src/client/guiscalingfilter.cpp
originally under LGPL2.1+
*/



/* Fill in RGB values for transparent pixels, to correct for odd colors
 * appearing at borders when blending.  This is because many PNG optimizers
 * like to discard RGB values of transparent pixels, but when blending then
 * with non-transparent neighbors, their RGB values will shpw up nonetheless.
 *
 * This function modifies the original image in-place.
 *
 * Parameter "threshold" is the alpha level below which pixels are considered
 * transparent.  Should be 127 for 3d where alpha is threshold, but 0 for
 * 2d where alpha is blended.
 */
static void imageCleanTransparent(irr::video::IImage* src, irr::u32 threshold) {
	const auto& dim = src->getDimension();

	// Walk each pixel looking for fully transparent ones.
	// Note: loop y around x for better cache locality.
	for(irr::u32 ctry = 0; ctry < dim.Height; ctry++)
		for(irr::u32 ctrx = 0; ctrx < dim.Width; ctrx++) {

			// Ignore opaque pixels.
			auto c = src->getPixel(ctrx, ctry);
			if(c.getAlpha() > threshold)
				continue;

			// Sample size and total weighted r, g, b values.
			irr::u32 ss = 0, sr = 0, sg = 0, sb = 0;

			// Walk each neighbor pixel (clipped to image bounds).
			for(irr::u32 sy = (ctry < 1) ? 0 : (ctry - 1);
				sy <= (ctry + 1) && sy < dim.Height; sy++)
				for(irr::u32 sx = (ctrx < 1) ? 0 : (ctrx - 1);
					sx <= (ctrx + 1) && sx < dim.Width; sx++) {

				// Ignore transparent pixels.
				const auto d = src->getPixel(sx, sy);
				if(d.getAlpha() <= threshold)
					continue;

				// Add RGB values weighted by alpha.
				const auto a = d.getAlpha();
				ss += a;
				sr += a * d.getRed();
				sg += a * d.getGreen();
				sb += a * d.getBlue();
			}

			// If we found any neighbor RGB data, set pixel to average
			// weighted by alpha.
			if(ss > 0) {
				c.setRed(sr / ss);
				c.setGreen(sg / ss);
				c.setBlue(sb / ss);
				src->setPixel(ctrx, ctry, c);
			}
		}
}

/* Scale a region of an image into another image, using nearest-neighbor with
 * anti-aliasing; treat pixels as crisp rectangles, but blend them at boundaries
 * to prevent non-integer scaling ratio artifacts.  Note that this may cause
 * some blending at the edges where pixels don't line up perfectly, but this
 * filter is designed to produce the most accurate results for both upscaling
 * and downscaling.
 */
static void imageScaleNNAAUnthreaded(irr::video::IImage* src, const irr::core::rect<irr::s32>& srcrect, irr::video::IImage* dest) {
	// Cache rectangle boundaries.
	const double sox = srcrect.UpperLeftCorner.X;
	const double soy = srcrect.UpperLeftCorner.Y;
	const double sw = srcrect.getWidth();
	const double sh = srcrect.getHeight();

	// Walk each destination image pixel.
	// Note: loop y around x for better cache locality.
	const auto& dim = dest->getDimension();
	const auto divw = sw / dim.Width;
	const auto divh = sh / dim.Height;
	for(irr::u32 dy = 0; dy < dim.Height; dy++)
		for(irr::u32 dx = 0; dx < dim.Width; dx++) {

			// Calculate floating-point source rectangle bounds.
			// Do some basic clipping, and for mirrored/flipped rects,
			// make sure min/max are in the right order.
			auto minsx = std::min(std::max(sox + (dx * divw), 0.0), sw + sox);
			auto maxsx = std::min(std::max(minsx + divw, 0.0), sw + sox);
			if(minsx > maxsx)
				std::swap(minsx, maxsx);
			auto minsy = std::min(std::max(soy + (dy * divh), 0.0), sh + soy);
			auto maxsy = std::min(std::max(minsy + divh, 0.0), sh + soy);
			if(minsy > maxsy)
				std::swap(minsy, maxsy);

			const auto csy = std::floor(minsy);
			const auto csx = std::floor(minsx);

			// Total area, and integral of r, g, b values over that area,
			// initialized to zero, to be summed up in next loops.
			double area = 0, ra = 0, ga = 0, ba = 0, aa = 0;
			irr::video::SColor pxl;

			// Loop over the integral pixel positions described by those bounds.
			for(double sy = csy; sy < maxsy; sy++)
				for(double sx = csx; sx < maxsx; sx++) {
					// Calculate width, height, then area of dest pixel
					// that's covered by this source pixel.

					double pw = 1.0;
					if(minsx > sx)
						pw += sx - minsx;
					if(maxsx < (sx + 1))
						pw += maxsx - sx - 1;
					double ph = 1.0;
					if(minsy > sy)
						ph += sy - minsy;
					if(maxsy < (sy + 1))
						ph += maxsy - sy - 1;
					const double pa = pw * ph;

					// Get source pixel and add it to totals, weighted
					// by covered area and alpha.
					pxl = src->getPixel(sx, sy);
					area += pa;
					ra += pa * pxl.getRed();
					ga += pa * pxl.getGreen();
					ba += pa * pxl.getBlue();
					aa += pa * pxl.getAlpha();
				}

			// Set the destination image pixel to the average color.
			if(area > 0) {
				pxl.setRed(ra / area + 0.5);
				pxl.setGreen(ga / area + 0.5);
				pxl.setBlue(ba / area + 0.5);
				pxl.setAlpha(aa / area + 0.5);
			} else {
				pxl.setRed(0);
				pxl.setGreen(0);
				pxl.setBlue(0);
				pxl.setAlpha(0);
			}
			dest->setPixel(dx, dy, pxl);
		}
}
/* Get a cached, high-quality pre-scaled texture for display purposes.  If the
 * texture is not already cached, attempt to create it.  Returns a pre-scaled texture,
 * or the original texture if unable to pre-scale it.
 */
irr::video::ITexture* ImageManager::guiScalingResizeCached(irr::video::ITexture* src, const irr::core::rect<irr::s32> &srcrect,
											const irr::core::rect<irr::s32> &destrect) {
	if(!src)
		return src;

	const auto& origname = src->getName().getPath();
	// Calculate scaled texture name.
	const auto scale_name = epro::format(EPRO_TEXT("{}@guiScalingFilter:{}:{}:{}:{}:{}:{}"),
						 origname,
						 srcrect.UpperLeftCorner.X,
						 srcrect.UpperLeftCorner.Y,
						 srcrect.getWidth(),
						 srcrect.getHeight(),
						 destrect.getWidth(),
						 destrect.getHeight());

	// Search for existing scaled texture.
	irr::video::ITexture*& scaled = g_txrCache[scale_name];
	if(scaled)
		return scaled;

	// Try to find the texture converted to an image in the cache.
	// If the image was not found, try to extract it from the texture.
	irr::video::IImage* srcimg = g_imgCache[origname];
	if(!srcimg) {
		srcimg = driver->createImageFromData(src->getColorFormat(),
											 src->getSize(), src->lock(), false);
		src->unlock();
		g_imgCache[origname] = srcimg;
	}

	// Create a new destination image and scale the source into it.
	imageCleanTransparent(srcimg, 0);
	irr::video::IImage* destimg = driver->createImage(src->getColorFormat(),
													  irr::core::dimension2d<irr::u32>((irr::u32)destrect.getWidth(),
													 (irr::u32)destrect.getHeight()));
	imageScaleNNAAUnthreaded(srcimg, srcrect, destimg);

	// Some platforms are picky about textures being powers of 2, so expand
	// the image dimensions to the next power of 2, if necessary.
	if(!hasNPotSupport(driver)) {
		irr::video::IImage *po2img = driver->createImage(src->getColorFormat(),
														 irr::core::dimension2d<irr::u32>(npot2((irr::u32)destrect.getWidth()),
																		   npot2((irr::u32)destrect.getHeight())));
		po2img->fill(irr::video::SColor(0, 0, 0, 0));
		destimg->copyTo(po2img);
		destimg->drop();
		destimg = po2img;
	}

	// Convert the scaled image back into a texture.
	scaled = driver->addTexture({ scale_name.data(), static_cast<irr::u32>(scale_name.size()) }, destimg);
	destimg->drop();

	return scaled;
}
void ImageManager::draw2DImageFilterScaled(irr::video::ITexture* txr,
							 const irr::core::rect<irr::s32>& destrect, const irr::core::rect<irr::s32>& srcrect,
							 const irr::core::rect<irr::s32>* cliprect, const irr::video::SColor* const colors,
							 bool usealpha) {
	// Attempt to pre-scale image in software in high quality.
	irr::video::ITexture* scaled = guiScalingResizeCached(txr, srcrect, destrect);
	if(!scaled)
		return;

	// Correct source rect based on scaled image.
	const auto mysrcrect = (scaled != txr)
		? irr::core::rect<irr::s32>(0, 0, destrect.getWidth(), destrect.getHeight())
		: srcrect;

	driver->draw2DImage(scaled, destrect, mysrcrect, cliprect, colors, usealpha);
}
irr::video::IImage* ImageManager::GetScaledImageFromFile(const irr::io::path& file, int width, int height) {
	if(width <= 0 || height <= 0)
		return nullptr;

	auto* srcimg = driver->createImageFromFile(file);
	if(!srcimg)
		return nullptr;

	const irr::core::dimension2d<irr::u32> dim(width, height);
	const auto& srcdim = srcimg->getDimension();
	if(srcdim == dim) {
		return srcimg;
	} else {
		auto* destimg = driver->createImage(srcimg->getColorFormat(), dim);
		imageScaleNNAAUnthreaded(srcimg, { 0, 0, (irr::s32)srcdim.Width, (irr::s32)srcdim.Height }, destimg);
		srcimg->drop();
		return destimg;
	}
}

}
