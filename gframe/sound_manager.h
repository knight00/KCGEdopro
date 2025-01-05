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
	void RefreshZipChants(epro::path_stringview folder, std::vector<std::string> &list);
	void RefreshChants(epro::path_stringview folder, std::vector<std::string> &list);
	void RefreshChantsList();
	void RefreshZipCards(epro::path_stringview folder, std::map<std::pair<CHANT, uint32_t>, std::string>& list, CHANT);
	void PlaySoundEffect(SFX sound);
	void PlayBGM(BGM scene, bool loop = true);
	////////kdiy////////
	bool PlayCardBGM(uint32_t code, uint32_t code2);
	void PlayCustomMusic(std::string num);
	void PlayCustomBGM(std::string num);
    bool PlayFieldSound();
	//bool PlayChant(CHANT chant, uint32_t code);
	void AddtoChantSPList(CHANT chant, uint16_t extra, std::vector<std::string>& chantlist, std::vector<std::string>& list);
	void AddtoZipChantList(std::string file, int i, std::vector<std::string>& list, std::vector<std::string>& list2);
	void AddtoChantList(std::string file, int i, std::vector<std::string>& list, std::vector<std::string>& list2);
	bool PlayZipChants(CHANT chant, std::string file, std::vector<std::string>& sound, uint8_t player);
	bool PlayChants(CHANT chant, std::string file, std::vector<std::string>& sound, uint8_t player);
	bool PlayChant(CHANT chant, uint32_t code, uint32_t code2, uint8_t player, uint16_t extra = 0);
	uint8_t character[6] = {0,0,0,0,0,0}; //0: empty, 1: muto, 2: atem,...
	std::vector<epro::path_stringview> textcharacter = { EPRO_TEXT("muto"),EPRO_TEXT("atem"),EPRO_TEXT("kaiba"),EPRO_TEXT("joey"),EPRO_TEXT("anzu"),EPRO_TEXT("mai"),EPRO_TEXT("pegasus"),EPRO_TEXT("marik"),EPRO_TEXT("dartz"),EPRO_TEXT("bakura"),EPRO_TEXT("aigami"),EPRO_TEXT("honda"),EPRO_TEXT("solomon"),EPRO_TEXT("mokuba"),EPRO_TEXT("ryuji"),EPRO_TEXT("haga"),EPRO_TEXT("lyuzaki"),EPRO_TEXT("ryota"),EPRO_TEXT("keith"),EPRO_TEXT("maze"),EPRO_TEXT("psychic"),EPRO_TEXT("ghost"),EPRO_TEXT("pandora"),EPRO_TEXT("ishizu"),EPRO_TEXT("odion"),EPRO_TEXT("shalla"),EPRO_TEXT("judai"),EPRO_TEXT("manjome"),EPRO_TEXT("asuka"),EPRO_TEXT("kaisa"),EPRO_TEXT("phoenix"),EPRO_TEXT("john"),EPRO_TEXT("cronos"),EPRO_TEXT("sartorius"),EPRO_TEXT("yubel"),EPRO_TEXT("darkness"),EPRO_TEXT("yusei"),EPRO_TEXT("jack"),EPRO_TEXT("arki"),EPRO_TEXT("lua"),EPRO_TEXT("luka"),EPRO_TEXT("crow"),EPRO_TEXT("kiryu"),EPRO_TEXT("carly"),EPRO_TEXT("rex"),EPRO_TEXT("apolia"),EPRO_TEXT("paradox"),EPRO_TEXT("antinomy"),EPRO_TEXT("zone"),EPRO_TEXT("yuma"),EPRO_TEXT("shark"),EPRO_TEXT("kaito"),EPRO_TEXT("iii"),EPRO_TEXT("iv"),EPRO_TEXT("v"),EPRO_TEXT("anna"),EPRO_TEXT("rio"),EPRO_TEXT("DonThousand"),EPRO_TEXT("yuya"),EPRO_TEXT("declan"),EPRO_TEXT("shay"),EPRO_TEXT("sora"),EPRO_TEXT("yuzu"),EPRO_TEXT("playmaker"),EPRO_TEXT("soulburner"),EPRO_TEXT("blueangel"),EPRO_TEXT("gore") };
	std::vector<std::string> soundcount;
	int32_t GetSoundDuration();
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
	//std::map<std::pair<CHANT, uint32_t>, std::string> ChantsList;
    std::vector<std::string> SFXList[SFX::SFX_TOTAL_SIZE];
	std::map<uint32_t, std::string> ChantsBGMList;
	std::map<std::pair<CHANT, uint32_t>, std::string> ChantsList[CHARACTER_VOICE + CHARACTER_STORY_ONLY];
	std::map<std::pair<CHANT, uint32_t>, std::string> ChantsList2[CHARACTER_VOICE + CHARACTER_STORY_ONLY];
	std::vector<std::string> ChantSPList[20][CHARACTER_VOICE + CHARACTER_STORY_ONLY];
	std::vector<std::string> ChantSPList2[20][CHARACTER_VOICE + CHARACTER_STORY_ONLY];
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
