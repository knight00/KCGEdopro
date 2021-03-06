#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <memory>
#include "random_fwd.h"
#include <map>
#include "text_types.h"
#include "sound_backend.h"

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
		//////kdiy/////
		SUMMON,
		ATTACK,
		ACTIVATE
	};
	SoundManager(double sounds_volume, double music_volume, bool sounds_enabled, bool music_enabled, epro::path_stringview working_directory);
	bool IsUsable();
	void RefreshBGMList();
	void RefreshChantsList();
	void PlaySoundEffect(SFX sound);
	void PlayBGM(BGM scene, bool loop = true);
	////////kdiy////////
	void PlayCustomMusic(std::string num);
	void PlayCustomBGM(std::string num);
	//bool PlayChant(CHANT chant, uint32_t code);
	bool PlayChant(CHANT chant, uint32_t code, uint32_t code2, int player);
	uint8_t character[6] = {0,0,0,0,0,0};
	uint8_t totcharacter = 8;
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
	std::map<std::pair<CHANT, uint32_t>, std::string> ChantsList[10];
	std::vector<std::string> ChantSPList[10][8];
	std::string bgm_now = "";
	////////kdiy////	
	int bgm_scene = -1;
	randengine rnd;
	std::unique_ptr<SoundBackend> mixer;
	void RefreshSoundsList();
	void RefreshBGMDir(epro::path_stringview path, BGM scene);
	bool soundsEnabled = false;
	bool musicEnabled = false;
	std::string working_dir = "./";
	bool succesfully_initied = false;
};

extern SoundManager* gSoundManager;

}

#endif //SOUNDMANAGER_H
