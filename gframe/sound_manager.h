#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <array>
#include <memory>
#include "RNG/mt19937.h"
#include <map>
#include "compiler_features.h"
#include "fmt.h"
#include "text_types.h"
#include "sound_backend.h"
///kdiy////////
#include "common.h"
///kdiy////////

namespace ygo {

class SoundManager {
public:
	enum BACKEND {
		DEFAULT,
		NONE,
		IRRKLANG,
		SDL,
		SFML,
		MINIAUDIO,
	};
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
		CUTIN_chain,
		CUTIN_damage,
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
		TAGSWITCH,
		REINCARNATE,
		//////kdiy/////
		SUMMON,
		ATTACK,
		ACTIVATE
	};
	SoundManager(double sounds_volume, double music_volume, bool sounds_enabled, bool music_enabled, BACKEND backend);
	bool IsUsable();
	void RefreshBGMList();
	void RefreshChantsList();
	void PlaySoundEffect(SFX sound);
	void PlayBGM(BGM scene, bool loop = true);
	////////kdiy////////
	//bool PlayChant(CHANT chant, uint32_t code);
	bool PlayCardBGM(uint32_t code, uint32_t code2);
	void PlayCustomMusic(std::string num);
	void PlayCustomBGM(std::string num);
    bool PlayFieldSound();
	void RefreshZipChants(epro::path_stringview folder, std::vector<std::string> &list, int character);
	void RefreshChants(epro::path_stringview folder, std::vector<std::string> &list);
	void RefreshZipCards(epro::path_stringview folder, std::map<std::pair<CHANT, uint32_t>, std::vector<std::string>>& list, CHANT chant, int character);
	void RefreshCards(epro::path_stringview folder, std::map<std::pair<CHANT, uint32_t>, std::vector<std::string>>& list, CHANT chant);
	void AddtoZipChantSPList(CHANT chant, uint16_t extra, size_t j, std::vector<std::string>& chantlist, std::vector<std::string>& list, std::vector<std::string>& list2, std::vector<std::string>& list3, int character);
	void AddtoChantSPList(CHANT chant, uint16_t extra, size_t j, std::vector<std::string>& chantlist, std::vector<std::string>& list, std::vector<std::string>& list2, std::vector<std::string>& list3);
	bool AddtoZipChantList(std::vector<std::string>& chantlist, std::vector<std::string>& list, std::vector<std::string>& list2, std::vector<std::string>& list3, int character);
	bool AddtoChantList(std::vector<std::string>& chantlist, std::vector<std::string>& list, std::vector<std::string>& list2, std::vector<std::string>& list3);
	bool PlayZipChants(CHANT chant, std::string file, const uint8_t side, uint8_t player);
	bool PlayChants(CHANT chant, std::string file, const uint8_t side, uint8_t player);
	bool PlayChant(CHANT chant, uint32_t code, uint32_t code2, const uint8_t side, uint8_t player, uint16_t extra = 0, uint16_t card_extra = 0, uint8_t card_extra2 = 0, uint8_t player2 = 0);
	void PlayStartupChant(uint8_t player, std::vector<uint8_t> team);
	uint8_t character[6] = {0,0,0,0,0,0}; //0: empty, 1: muto, 2: atem,...
	int subcharacter[CHARACTER_VOICE] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	std::vector<epro::path_string> textcharacter[CHARACTER_VOICE-1+CHARACTER_STORY_ONLY] = {
		{EPRO_TEXT("muto"),EPRO_TEXT("muto_dm"),EPRO_TEXT("muto_dsod")},
		{EPRO_TEXT("atem")},
		{EPRO_TEXT("kaiba"),EPRO_TEXT("kaiba_dm"),EPRO_TEXT("kaiba_dsod")},
		{EPRO_TEXT("joey"),EPRO_TEXT("joey_dm"),EPRO_TEXT("joey_controlled"),EPRO_TEXT("joey_dsod")},
		{EPRO_TEXT("anzu"),EPRO_TEXT("anzu_dm"),EPRO_TEXT("anzu_dsod")},
		{EPRO_TEXT("mai")},
		{EPRO_TEXT("honda")},
		{EPRO_TEXT("solomon")},
		{EPRO_TEXT("mokuba"),EPRO_TEXT("mokuba_dm"),EPRO_TEXT("mokuba_dsod")},
		{EPRO_TEXT("ryuji")},
		{EPRO_TEXT("haga")},
		{EPRO_TEXT("lyuzaki")},
		{EPRO_TEXT("ryota")},
		{EPRO_TEXT("keith")},
		{EPRO_TEXT("maze"),EPRO_TEXT("maze_1"),EPRO_TEXT("maze_2"),EPRO_TEXT("maze_1+2")},
		{EPRO_TEXT("psychic")},
		{EPRO_TEXT("ghost")},
		{EPRO_TEXT("mask"),EPRO_TEXT("mask_light"),EPRO_TEXT("mask_dark"),EPRO_TEXT("mask_light+dark")},
		{EPRO_TEXT("pandora")},
		{EPRO_TEXT("ishizu")},
		{EPRO_TEXT("odion")},
		{EPRO_TEXT("shalla"),EPRO_TEXT("shalla_human"),EPRO_TEXT("shalla_masked")},
		{EPRO_TEXT("pegasus")},
		{EPRO_TEXT("marik")},
		{EPRO_TEXT("dartz")},
		{EPRO_TEXT("bakura"),EPRO_TEXT("bakura_dm"),EPRO_TEXT("bakura_dsod")},
		{EPRO_TEXT("aigami")}, //27

		{EPRO_TEXT("judai"),EPRO_TEXT("judai_1"),EPRO_TEXT("judai_2"),EPRO_TEXT("judai_supreme")},
		{EPRO_TEXT("manjome")},
		{EPRO_TEXT("asuka")},
		{EPRO_TEXT("daichi")},
		{EPRO_TEXT("kaisa")},
		{EPRO_TEXT("phoenix")},
		{EPRO_TEXT("john")},
		{EPRO_TEXT("sho")},
		{EPRO_TEXT("kenzan")},
		{EPRO_TEXT("lei")},
		{EPRO_TEXT("jim")},
		{EPRO_TEXT("obrian")},
		{EPRO_TEXT("cronos")},
		{EPRO_TEXT("sartorius")},
		{EPRO_TEXT("yubel")},
		{EPRO_TEXT("darkness")}, //43
		
		{EPRO_TEXT("yusei")},
		{EPRO_TEXT("jack")},
		{EPRO_TEXT("arki")},
		{EPRO_TEXT("lua")},
		{EPRO_TEXT("luka")},
		{EPRO_TEXT("crow")},
		{EPRO_TEXT("ushio")},
		{EPRO_TEXT("kiryu"),EPRO_TEXT("kiryu_darksiner"),EPRO_TEXT("kiryu_death")},
		{EPRO_TEXT("carly"),EPRO_TEXT("carly_human"),EPRO_TEXT("carly_darksiner")},
		{EPRO_TEXT("sherry")},
		{EPRO_TEXT("rex")},
		{EPRO_TEXT("meklord"),EPRO_TEXT("placido"),EPRO_TEXT("apolia")},
		{EPRO_TEXT("paradox")},
		{EPRO_TEXT("antinomy")},
		{EPRO_TEXT("zone")}, //58
		
		{EPRO_TEXT("yuma"),EPRO_TEXT("yuma_human"),EPRO_TEXT("ZEXAL")},
		{EPRO_TEXT("shark")},
		{EPRO_TEXT("kaito")},
		{EPRO_TEXT("tori")},
		{EPRO_TEXT("tetsuo")},
		{EPRO_TEXT("iii")},
		{EPRO_TEXT("iv")},
		{EPRO_TEXT("v")},
		{EPRO_TEXT("anna")},
		{EPRO_TEXT("dorube"),EPRO_TEXT("dorube_human"),EPRO_TEXT("dorube_seventh")},
		{EPRO_TEXT("rio")},
		{EPRO_TEXT("alit"),EPRO_TEXT("alit_human"),EPRO_TEXT("alit_seventh")},
		{EPRO_TEXT("girag"),EPRO_TEXT("girag_human"),EPRO_TEXT("girag_seventh")},
		{EPRO_TEXT("mizarel"),EPRO_TEXT("mizarel_human"),EPRO_TEXT("mizarel_seventh")},
		{EPRO_TEXT("DonThousand")}, //73
		
		{EPRO_TEXT("yuya"),EPRO_TEXT("yuya_main"),EPRO_TEXT("yugo"),EPRO_TEXT("yuto")},
		{EPRO_TEXT("declan")},
		{EPRO_TEXT("shay")},
		{EPRO_TEXT("sora")},
		{EPRO_TEXT("yuzu"),EPRO_TEXT("yuzu_main"),EPRO_TEXT("selina"),EPRO_TEXT("ruri"),EPRO_TEXT("rin")},
		{EPRO_TEXT("gongenzaka")},
		{EPRO_TEXT("sawatari")},
		{EPRO_TEXT("dennis")}, //81
		
		{EPRO_TEXT("playmaker")},
		{EPRO_TEXT("revolver")},
		{EPRO_TEXT("soulburner")},
		{EPRO_TEXT("blueangel")},
		{EPRO_TEXT("gore")},
		{EPRO_TEXT("ghostgirl")},
		{EPRO_TEXT("spectre")},
		{EPRO_TEXT("bravemax")},
		{EPRO_TEXT("hanoi")}, //90

		{EPRO_TEXT("yuuga")},
		{EPRO_TEXT("luke")},
		{EPRO_TEXT("romin")},
		{EPRO_TEXT("gakuto")},
		{EPRO_TEXT("mimi")}, //95
		
		{EPRO_TEXT("guider")},
		
		{EPRO_TEXT("darksiner")} }; //total 97
	std::vector<std::string> soundcount;
	int32_t GetSoundDuration();
	void PlayModeSound(int i, uint32_t code, bool music = false);
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

	static constexpr auto GetSupportedBackends() {
		return std::array{
			DEFAULT,
#if defined(YGOPRO_USE_MINIAUDIO)
			MINIAUDIO,
#endif
#if defined(YGOPRO_USE_SFML)
			SFML,
#endif
#if defined(YGOPRO_USE_IRRKLANG)
			IRRKLANG,
#endif
#if defined(YGOPRO_USE_SDL_MIXER)
			SDL,
#endif
			NONE,
		};
	}

	static constexpr auto GetDefaultBackend() {
		return GetSupportedBackends()[1];
	}

	static constexpr bool HasMultipleBackends() {
		return GetSupportedBackends().size() > 3;
	}

	template<typename T = char>
	static constexpr auto GetBackendName(BACKEND backend) {
		switch(backend) {
			case IRRKLANG:
				return CHAR_T_STRINGVIEW(T, "Irrklang");
			case SDL:
				return CHAR_T_STRINGVIEW(T, "SDL");
			case SFML:
				return CHAR_T_STRINGVIEW(T, "SFML");
			case MINIAUDIO:
				return CHAR_T_STRINGVIEW(T, "miniaudio");
			case NONE:
				return CHAR_T_STRINGVIEW(T, "none");
			case DEFAULT:
				return CHAR_T_STRINGVIEW(T, "default");
			default:
				unreachable();
		}
	}

private:
	std::vector<std::string> BGMList[8];
	////////kdiy////
	//std::string SFXList[SFX::SFX_TOTAL_SIZE];
	//std::map<std::pair<CHANT, uint32_t>, std::string> ChantsList;
    std::vector<std::string> SFXList[SFX::SFX_TOTAL_SIZE];
	std::map<uint32_t, std::string> ChantBGM;
	//card chant, Chantcard2 for zipped
	std::map<std::pair<CHANT, uint32_t>, std::vector<std::string>> Chantcard[CHARACTER_VOICE + CHARACTER_STORY_ONLY][11][8];
	std::map<std::pair<CHANT, uint32_t>, std::vector<std::string>> Chantcard2[CHARACTER_VOICE + CHARACTER_STORY_ONLY][11][8];
	//action chant, Chantaction2 for zipped
	std::vector<std::string> Chantaction[21][CHARACTER_VOICE + CHARACTER_STORY_ONLY][14][CHARACTER_VOICE + CHARACTER_STORY_ONLY]; //1st: action no., 2nd: character(0=no character), 3rd: subaction, 4th: opponent character
	std::vector<std::string> Chantaction2[21][CHARACTER_VOICE + CHARACTER_STORY_ONLY][14][CHARACTER_VOICE + CHARACTER_STORY_ONLY];
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
	bool currentlyLooping{ false };
};

extern SoundManager* gSoundManager;

}

template<typename CharT>
struct fmt::formatter<ygo::SoundManager::BACKEND, CharT> {
	template<typename ParseContext>
	constexpr auto parse(ParseContext& ctx) const { return ctx.begin(); }

	template <typename FormatContext>
	constexpr auto format(ygo::SoundManager::BACKEND value, FormatContext& ctx) const {
		return format_to(ctx.out(), CHAR_T_STRINGVIEW(CharT, "{}"), ygo::SoundManager::GetBackendName<CharT>(value));
	}
};

#endif //SOUNDMANAGER_H
