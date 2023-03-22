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
		EQUIP,
		DESTROY,
		BANISH,
		DRAW,
		DAMAGE,
		RECOVER,
		NEXTTURN,
		STARTUP,
		BORED,
		PENDULUM,
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
	void PlayCustomMusic(std::string num);
	void PlayCustomBGM(std::string num);
	//bool PlayChant(CHANT chant, uint32_t code);
	bool PlayChant(CHANT chant, uint32_t code, uint32_t code2, int player, uint8_t extra = 0);
	uint8_t character[6] = {0,0,0,0,0,0}; //0: empty, 1: muto, 2: atem, 3: kaiba, 4: joey, 5: marik, 6: dartz, 7:bakura, 8: aigami, 9: judai, 10: manjome, 11: kaisa, 12: phoenix, 13: john, 14: yubel, 15: yusei, 16: jack, 17: arki, 18: yuma, 19: shark, 20: kaito, 21: DonThousand, 22: yuya, 23: declan, 24: playmaker, 25: soulburner, 26: blueangel, 27: darksiner
    std::vector<std::string> soundcount;
	int32_t GetSoundDuration(std::string name);
	void PlayModeSound(uint8_t index, bool lock=false);
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
	std::string SFXList[SFX::SFX_TOTAL_SIZE];
	////////kdiy////
	std::map<std::pair<CHANT, uint32_t>, std::string> ChantsList[CHARACTER_VOICE + CHARACTER_STORY_ONLY];
	std::vector<std::string> ChantSPList[11][CHARACTER_VOICE + CHARACTER_STORY_ONLY];
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
