#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <memory>
#include "RNG/mt19937.h"
#include <map>
#include "text_types.h"
#include "sound_backend.h"
///kdiy////////
#include "common.h"
///kdiy////////

namespace ygo {

class SoundManager {
public:
	enum SFX {
		SUMMON,
		SPECIAL_SUMMON,
        ///kdiy////////
        FUSION_SUMMON,
        SYNCHRO_SUMMON,
        XYZ_SUMMON,
        PENDULUM_SUMMON,
        LINK_SUMMON,
        RITUAL_SUMMON,
        SUMMON_DARK,
        SUMMON_DIVINE,
        SUMMON_EARTH,
        SUMMON_FIRE,
        SUMMON_LIGHT,
        SUMMON_WATER,
        SUMMON_WIND,
        SPECIAL_SUMMON_DARK,
        SPECIAL_SUMMON_DIVINE,
        SPECIAL_SUMMON_EARTH,
        SPECIAL_SUMMON_FIRE,
        SPECIAL_SUMMON_LIGHT,
        SPECIAL_SUMMON_WATER,
        SPECIAL_SUMMON_WIND,
        OVERLAY,
        NEGATE,
        ATTACK_DISABLED,
        POS_CHANGE,
        ///kdiy////////
		ACTIVATE,
		SET,
		FLIP,
		REVEAL,
		EQUIP,
		DESTROYED,
		BANISHED,
		TOKEN,
		ATTACK,
		DIRECT_ATTACK,
		DRAW,
		SHUFFLE,
		DAMAGE,
		RECOVER,
		COUNTER_ADD,
		COUNTER_REMOVE,
		COIN,
		DICE,
		NEXT_TURN,
		PHASE,
		PLAYER_ENTER,
		CHAT,
		SFX_TOTAL_SIZE
	};
	enum BGM {
		ALL,
		DUEL,
		MENU,
		DECK,
		ADVANTAGE,
		DISADVANTAGE,
		WIN,
		LOSE
	};
	enum class CHANT {
		//////kdiy/////
		SET,
		DESTROY,
		DRAW,
		DAMAGE,
		RECOVER,
		NEXTTURN,
		STARTUP,
		BORED,
		PENDULUM,
        PSCALE,
		OPPCOUNTER,
		SELFCOUNTER,
		RELEASE,
		BATTLEPHASE,
        TURNEND,
		WIN,
		LOSE,
		//////kdiy/////
		SUMMON,
		ATTACK,
		ACTIVATE
	};
	SoundManager(double sounds_volume, double music_volume, bool sounds_enabled, bool music_enabled);
	bool IsUsable();
	void RefreshBGMList();
	void RefreshZipChants(epro::path_stringview folder, std::vector<std::string> &list, int character);
	void RefreshChants(epro::path_stringview folder, std::vector<std::string> &list);
	void RefreshZipCards(epro::path_stringview folder, std::map<std::pair<CHANT, uint32_t>, std::vector<std::string>>& list, CHANT chant, int character);
	void RefreshCards(epro::path_stringview folder, std::map<std::pair<CHANT, uint32_t>, std::vector<std::string>>& list, CHANT chant, int character);
	void RefreshChantsList();
	void PlaySoundEffect(SFX sound);
	void PlayBGM(BGM scene, bool loop = true);
	////////kdiy////////
	//bool PlayChant(CHANT chant, uint32_t code);
	bool PlayCardBGM(uint32_t code, uint32_t code2);
	void PlayCustomMusic(std::string num);
	void PlayCustomBGM(std::string num);
    bool PlayFieldSound();
	void AddtoZipChantSPList(CHANT chant, uint16_t extra, size_t j, std::vector<std::string>& chantlist, std::vector<std::string>& list, std::vector<std::string>& list2, int character);
	void AddtoChantSPList(CHANT chant, uint16_t extra, size_t j, std::vector<std::string>& chantlist, std::vector<std::string>& list, std::vector<std::string>& list2);
	void AddtoZipChantList(std::vector<std::string>& chantlist, int i, std::vector<std::string>& list, std::vector<std::string>& list2, int character);
	void AddtoChantList(std::vector<std::string>& chantlist, int i, std::vector<std::string>& list, std::vector<std::string>& list2);
	bool PlayZipChants(CHANT chant, std::string file, std::vector<std::string>& sound, uint8_t player);
	bool PlayChants(CHANT chant, std::string file, std::vector<std::string>& sound, uint8_t player);
	bool PlayChant(CHANT chant, uint32_t code, uint32_t code2, uint8_t player, uint16_t extra = 0, uint8_t player2 = 0);
	void PlayStartupChant(uint8_t player, std::vector<uint8_t> team);
	uint8_t character[6] = {0,0,0,0,0,0}; //0: empty, 1: muto, 2: atem,...
	uint8_t subcharacter[CHARACTER_VOICE] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	std::vector <std::vector<epro::path_stringview>> textcharacter = { {EPRO_TEXT("muto")},{EPRO_TEXT("atem")},{EPRO_TEXT("kaiba")},{EPRO_TEXT("joey"),EPRO_TEXT("joey/joey_dm"),EPRO_TEXT("joey/joey_dsod")},{EPRO_TEXT("anzu")},{EPRO_TEXT("mai")},{EPRO_TEXT("pegasus")},{EPRO_TEXT("marik")},{EPRO_TEXT("dartz")},{EPRO_TEXT("bakura")},{EPRO_TEXT("aigami")},{EPRO_TEXT("honda")},{EPRO_TEXT("solomon")},{EPRO_TEXT("mokuba")},{EPRO_TEXT("ryuji")},{EPRO_TEXT("haga")},{EPRO_TEXT("lyuzaki")},{EPRO_TEXT("ryota")},{EPRO_TEXT("keith")},{EPRO_TEXT("maze")},{EPRO_TEXT("psychic")},{EPRO_TEXT("ghost")},{EPRO_TEXT("pandora")},{EPRO_TEXT("ishizu")},{EPRO_TEXT("odion")},{EPRO_TEXT("shalla")},{EPRO_TEXT("judai")},{EPRO_TEXT("manjome")},{EPRO_TEXT("asuka")},{EPRO_TEXT("kaisa")},{EPRO_TEXT("phoenix")},{EPRO_TEXT("john")},{EPRO_TEXT("cronos")},{EPRO_TEXT("sartorius")},{EPRO_TEXT("yubel")},{EPRO_TEXT("darkness")},{EPRO_TEXT("yusei")},{EPRO_TEXT("jack")},{EPRO_TEXT("arki")},{EPRO_TEXT("lua")},{EPRO_TEXT("luka")},{EPRO_TEXT("crow")},{EPRO_TEXT("kiryu")},{EPRO_TEXT("carly")},{EPRO_TEXT("rex")},{EPRO_TEXT("apolia")},{EPRO_TEXT("paradox")},{EPRO_TEXT("antinomy")},{EPRO_TEXT("zone")},{EPRO_TEXT("yuma")},{EPRO_TEXT("shark")},{EPRO_TEXT("kaito")},{EPRO_TEXT("iii")},{EPRO_TEXT("iv")},{EPRO_TEXT("v")},{EPRO_TEXT("anna")},{EPRO_TEXT("rio")},{EPRO_TEXT("DonThousand")},{EPRO_TEXT("yuya")},{EPRO_TEXT("declan")},{EPRO_TEXT("shay")},{EPRO_TEXT("sora")},{EPRO_TEXT("yuzu")},{EPRO_TEXT("playmaker")},{EPRO_TEXT("soulburner")},{EPRO_TEXT("blueangel")},{EPRO_TEXT("gore")}, {EPRO_TEXT("darksiner")} };
	std::vector<std::string> soundcount;
	int32_t GetSoundDuration();
	int PlayModeSound();
    void PlayMode(bool lock=false);
	void EnableSummonSounds(bool enable);
	void EnableActivateSounds(bool enable);
	void EnableAttackSounds(bool enable);
	////////kdiy////////
	void SetSoundVolume(double volume);
	void SetMusicVolume(double volume);
	void EnableSounds(bool enable);
	void EnableMusic(bool enable);
	void StopSounds();
	void StopMusic();
	void PauseMusic(bool pause);
	void Tick();

private:
	std::vector<std::string> BGMList[8];
	////////kdiy////
	//std::string SFXList[SFX::SFX_TOTAL_SIZE];
	//std::map<std::pair<CHANT, uint32_t>, std::string> ChantsList;
    std::vector<std::string> SFXList[SFX::SFX_TOTAL_SIZE];
	std::map<uint32_t, std::string> ChantBGM;
	//card chant, Chantcard2 for zipped
	std::map<std::pair<CHANT, uint32_t>, std::vector<std::string>> Chantcard[CHARACTER_VOICE + CHARACTER_STORY_ONLY];
	std::map<std::pair<CHANT, uint32_t>, std::vector<std::string>> Chantcard2[CHARACTER_VOICE + CHARACTER_STORY_ONLY];
	//action chant, Chantaction2 for zipped
	std::vector<std::string> Chantaction[20][CHARACTER_VOICE + CHARACTER_STORY_ONLY][14][CHARACTER_VOICE + CHARACTER_STORY_ONLY]; //1st: action no., 2nd: character(0=no character), 3rd: subaction, 4th: opponent character
	std::vector<std::string> Chantaction2[20][CHARACTER_VOICE + CHARACTER_STORY_ONLY][14][CHARACTER_VOICE + CHARACTER_STORY_ONLY];
	std::string bgm_now = "";
	////////kdiy////
	int bgm_scene{ -1 };
	RNG::mt19937 rnd;
	std::unique_ptr<SoundBackend> mixer{ nullptr };
	void RefreshSoundsList();
	void RefreshBGMDir(epro::path_stringview path, BGM scene);
	bool soundsEnabled{ false };
	bool musicEnabled{ false };
	std::string working_dir{ "./" };
	bool succesfully_initied{ false };
};

extern SoundManager* gSoundManager;

}

#endif //SOUNDMANAGER_H
