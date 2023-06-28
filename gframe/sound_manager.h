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
		OPPCOUNTER,
		RELEASE,
		BATTLEPHASE,
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
	void RefreshChantsList();
	void PlaySoundEffect(SFX sound);
	void PlayBGM(BGM scene, bool loop = true);
	////////kdiy////////
	bool PlayCardBGM(uint32_t code, uint32_t code2);
	void PlayCustomMusic(std::string num);
	void PlayCustomBGM(std::string num);
    bool PlayFieldSound();
	//bool PlayChant(CHANT chant, uint32_t code);
	bool PlayChant(CHANT chant, uint32_t code, uint32_t code2, uint8_t player, uint16_t extra = 0);
	uint8_t character[6] = {0,0,0,0,0,0}; //0: empty, 1: muto, 2: atem, 3: kaiba, 4: joey, 5: marik, 6: dartz, 7:bakura, 8: aigami, 9: judai, 10: manjome, 11: kaisa, 12: phoenix, 13: john, 14: yubel, 15: yusei, 16: jack, 17: arki, 18: crow, 19: kiryu, 20: zone, 21: yuma, 22: shark, 23: kaito, 24:iv, 25: DonThousand, 26: yuya, 27: declan, 28: shay, 29: playmaker, 30: soulburner, 31: blueangel, 32: darksiner
    std::vector<std::string> soundcount;
    std::vector<std::string> soundcount2;
	int32_t GetSoundDuration(std::string name);
	int PlayModeSound(bool lock=false);
    void PlayMode(bool lock=false);
	////////kdiy////////
	void SetSoundVolume(double volume);
	void SetMusicVolume(double volume);
	void EnableSounds(bool enable);
	///////kdiy//////////
	void EnableSummonSounds(bool enable);
	void EnableActivateSounds(bool enable);
	void EnableAttackSounds(bool enable);
	///////kdiy//////////
	void EnableMusic(bool enable);
	void StopSounds();
	void StopMusic();
	void PauseMusic(bool pause);
	void Tick();

private:
	std::vector<std::string> BGMList[8];
	////////kdiy////
	//std::string SFXList[SFX::SFX_TOTAL_SIZE];
    std::vector<std::string> SFXList[SFX::SFX_TOTAL_SIZE];
	std::map<uint32_t, std::string> ChantsBGMList;
	std::map<std::pair<CHANT, uint32_t>, std::string> ChantsList[CHARACTER_VOICE + CHARACTER_STORY_ONLY];
	std::vector<std::string> ChantSPList[16][CHARACTER_VOICE + CHARACTER_STORY_ONLY];
	std::string bgm_now = "";
	////////kdiy////
	int bgm_scene = -1;
	RNG::mt19937 rnd;
	std::unique_ptr<SoundBackend> mixer;
	void RefreshSoundsList();
	void RefreshBGMDir(epro::path_stringview path, BGM scene);
	bool soundsEnabled = false;
	bool musicEnabled = false;
	std::string working_dir;
	bool succesfully_initied = false;
};

extern SoundManager* gSoundManager;

}

#endif //SOUNDMANAGER_H
