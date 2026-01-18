#define WIN32_LEAN_AND_MEAN
#include "sound_manager.h"
#include "utils.h"
#include "config.h"
#include "fmt.h"
#if defined(YGOPRO_USE_IRRKLANG)
#include "SoundBackends/irrklang/sound_irrklang.h"
#endif
#if defined(YGOPRO_USE_SDL_MIXER)
#include "SoundBackends/sdlmixer/sound_sdlmixer.h"
#endif
#if defined(YGOPRO_USE_SDL_MIXER3)
#include "SoundBackends/sdlmixer3/sound_sdlmixer3.h"
#endif
#if defined(YGOPRO_USE_SFML)
#include "SoundBackends/sfml/sound_sfml.h"
#endif
#if defined(YGOPRO_USE_MINIAUDIO)
#include "SoundBackends/miniaudio/sound_miniaudio.h"
#endif
/////kdiy/////////
#include "game_config.h"
#include "game.h"
#include "windbot_panel.h"
#if IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9
#include "IrrlichtCommonIncludes1.9/CFileSystem.h"
#else
#include "IrrlichtCommonIncludes/CFileSystem.h"
#endif
#include <irrlicht.h>
/////kdiy/////
namespace ygo {
namespace {
std::unique_ptr<SoundBackend> make_backend(SoundManager::BACKEND backend) {
	switch(backend) {
#ifdef YGOPRO_USE_IRRKLANG
		case SoundManager::IRRKLANG:
			return std::make_unique<SoundIrrklang>();
#endif
#ifdef YGOPRO_USE_SDL_MIXER
		case SoundManager::SDL:
			return std::make_unique<SoundMixer>();
#endif
#ifdef YGOPRO_USE_SDL_MIXER3
		case SoundManager::SDL3:
			return std::make_unique<SoundMixer3>();
#endif
#ifdef YGOPRO_USE_SFML
		case SoundManager::SFML:
			return std::make_unique<SoundSFML>();
#endif
#ifdef YGOPRO_USE_MINIAUDIO
		case SoundManager::MINIAUDIO:
			return std::make_unique<SoundMiniaudio>();
#endif
		default:
			epro::print("{} sound backend not compiled in.\n", backend);
			[[fallthrough]];
		case SoundManager::NONE:
			return nullptr;
	}
}
}

SoundManager::SoundManager(double sounds_volume, double music_volume, bool sounds_enabled, bool music_enabled, BACKEND backend) {
	if(backend == DEFAULT) {
		backend = GetDefaultBackend();
	}
	epro::print("Using: {} for audio playback.\n", backend);
	if(backend == NONE) {
		soundsEnabled = musicEnabled = false;
		return;
	}
	working_dir = Utils::ToUTF8IfNeeded(Utils::GetWorkingDirectory());
	soundsEnabled = sounds_enabled;
	musicEnabled = music_enabled;
	try {
		auto tmp_mixer = make_backend(backend);
		if(!tmp_mixer) {
			epro::print("Failed to initialize audio backend:\n");
			soundsEnabled = musicEnabled = false;
			return;
		}
		tmp_mixer->SetMusicVolume(music_volume);
		tmp_mixer->SetSoundVolume(sounds_volume);
		mixer = std::move(tmp_mixer);
	}
	catch(const std::runtime_error& e) {
		epro::print("Failed to initialize audio backend:\n");
		epro::print(e.what());
		soundsEnabled = musicEnabled = false;
		return;
	}
	catch(...) {
		epro::print("Failed to initialize audio backend.\n");
		soundsEnabled = musicEnabled = false;
		return;
	}
	rnd.seed(static_cast<uint32_t>(time(0)));
	////////kdiy////
	std::string bgm_now = "";
	////////kdiy////
	bgm_scene = -1;
	RefreshBGMList();
	RefreshSoundsList();
	RefreshChantsList();
}
bool SoundManager::IsUsable() {
	return mixer != nullptr;
}
void SoundManager::RefreshBGMList() {
	if(!IsUsable())
		return;
	Utils::MakeDirectory(EPRO_TEXT("./sound/BGM/"));
	Utils::MakeDirectory(EPRO_TEXT("./sound/BGM/duel"));
	Utils::MakeDirectory(EPRO_TEXT("./sound/BGM/menu"));
	Utils::MakeDirectory(EPRO_TEXT("./sound/BGM/deck"));
	Utils::MakeDirectory(EPRO_TEXT("./sound/BGM/advantage"));
	Utils::MakeDirectory(EPRO_TEXT("./sound/BGM/disadvantage"));
	Utils::MakeDirectory(EPRO_TEXT("./sound/BGM/win"));
	Utils::MakeDirectory(EPRO_TEXT("./sound/BGM/lose"));
	////////kdiy////
	Utils::MakeDirectory(EPRO_TEXT("./sound/BGM/card/"));
	////////kdiy////
	for (auto& list : BGMList)
		list.clear();
	RefreshBGMDir(EPRO_TEXT(""), BGM::DUEL);
	RefreshBGMDir(EPRO_TEXT("duel"), BGM::DUEL);
	RefreshBGMDir(EPRO_TEXT("menu"), BGM::MENU);
	RefreshBGMDir(EPRO_TEXT("deck"), BGM::DECK);
	RefreshBGMDir(EPRO_TEXT("advantage"), BGM::ADVANTAGE);
	RefreshBGMDir(EPRO_TEXT("disadvantage"), BGM::DISADVANTAGE);
	RefreshBGMDir(EPRO_TEXT("win"), BGM::WIN);
	RefreshBGMDir(EPRO_TEXT("lose"), BGM::LOSE);
}
void SoundManager::RefreshSoundsList() {
	if(!IsUsable())
		return;
	static constexpr std::pair<SFX, epro::path_stringview> fx[]{
        /////kdiy///////
		// {SUMMON, EPRO_TEXT("./sound/summon.{}"sv)},
		// {SPECIAL_SUMMON, EPRO_TEXT("./sound/specialsummon.{}"sv)},
		// {ACTIVATE, EPRO_TEXT("./sound/activate.{}"sv)},
		// {SET, EPRO_TEXT("./sound/set.{}"sv)},
		// {FLIP, EPRO_TEXT("./sound/flip.{}"sv)},
		// {REVEAL, EPRO_TEXT("./sound/reveal.{}"sv)},
		// {EQUIP, EPRO_TEXT("./sound/equip.{}"sv)},
		// {DESTROYED, EPRO_TEXT("./sound/destroyed.{}"sv)},
		// {TOKEN, EPRO_TEXT("./sound/token.{}"sv)},
		// {ATTACK, EPRO_TEXT("./sound/attack.{}"sv)},
		// {DIRECT_ATTACK, EPRO_TEXT("./sound/directattack.{}"sv)},
		// {DRAW, EPRO_TEXT("./sound/draw.{}"sv)},
		// {SHUFFLE, EPRO_TEXT("./sound/shuffle.{}"sv)},
		// {DAMAGE, EPRO_TEXT("./sound/damage.{}"sv)},
		// {RECOVER, EPRO_TEXT("./sound/gainlp.{}"sv)},
		// {COUNTER_ADD, EPRO_TEXT("./sound/addcounter.{}"sv)},
		// {COUNTER_REMOVE, EPRO_TEXT("./sound/removecounter.{}"sv)},
		// {COIN, EPRO_TEXT("./sound/coinflip.{}"sv)},
		// {DICE, EPRO_TEXT("./sound/diceroll.{}"sv)},
		// {NEXT_TURN, EPRO_TEXT("./sound/nextturn.{}"sv)},
		// {PHASE, EPRO_TEXT("./sound/phase.{}"sv)},
		// {PLAYER_ENTER, EPRO_TEXT("./sound/playerenter.{}"sv)},
		// {CHAT, EPRO_TEXT("./sound/chatmessage.{}"sv)}
		{SUMMON, EPRO_TEXT("./sound/summon"sv)},
        {SUMMON_DARK, EPRO_TEXT("./sound/summon/ATTRIBUTE_DARK"sv)},
        {SUMMON_DIVINE, EPRO_TEXT("./sound/summon/ATTRIBUTE_DIVINE"sv)},
        {SUMMON_EARTH, EPRO_TEXT("./sound/summon/ATTRIBUTE_EARTH"sv)},
        {SUMMON_FIRE, EPRO_TEXT("./sound/summon/ATTRIBUTE_FIRE"sv)},
        {SUMMON_LIGHT, EPRO_TEXT("./sound/summon/ATTRIBUTE_LIGHT"sv)},
        {SUMMON_WATER, EPRO_TEXT("./sound/summon/ATTRIBUTE_WATER"sv)},
        {SUMMON_WIND, EPRO_TEXT("./sound/summon/ATTRIBUTE_WIND"sv)},
		{SPECIAL_SUMMON, EPRO_TEXT("./sound/specialsummon"sv)},
        {SPECIAL_SUMMON_DARK, EPRO_TEXT("./sound/specialsummon/ATTRIBUTE_DARK"sv)},
        {SPECIAL_SUMMON_DIVINE, EPRO_TEXT("./sound/specialsummon/ATTRIBUTE_DIVINE"sv)},
        {SPECIAL_SUMMON_EARTH, EPRO_TEXT("./sound/specialsummon/ATTRIBUTE_EARTH"sv)},
        {SPECIAL_SUMMON_FIRE, EPRO_TEXT("./sound/specialsummon/ATTRIBUTE_FIRE"sv)},
        {SPECIAL_SUMMON_LIGHT, EPRO_TEXT("./sound/specialsummon/ATTRIBUTE_LIGHT"sv)},
        {SPECIAL_SUMMON_WATER, EPRO_TEXT("./sound/specialsummon/ATTRIBUTE_WATER"sv)},
        {SPECIAL_SUMMON_WIND, EPRO_TEXT("./sound/specialsummon/ATTRIBUTE_WIND"sv)},
		{FUSION_SUMMON, EPRO_TEXT("./sound/specialsummon/fusion"sv)},
		{SYNCHRO_SUMMON, EPRO_TEXT("./sound/specialsummon/synchro"sv)},
		{XYZ_SUMMON, EPRO_TEXT("./sound/specialsummon/xyz"sv)},
		{PENDULUM_SUMMON, EPRO_TEXT("./sound/specialsummon/pendulum"sv)},
		{LINK_SUMMON, EPRO_TEXT("./sound/specialsummon/link"sv)},
		{RITUAL_SUMMON, EPRO_TEXT("./sound/specialsummon/ritual"sv)},
		{NEGATE, EPRO_TEXT("./sound/negate"sv)},
        {OVERLAY, EPRO_TEXT("./sound/overlay"sv)},
		{ACTIVATE, EPRO_TEXT("./sound/activate"sv)},
		{SET, EPRO_TEXT("./sound/set"sv)},
		{FLIP, EPRO_TEXT("./sound/flip"sv)},
		{REVEAL, EPRO_TEXT("./sound/reveal"sv)},
		{EQUIP, EPRO_TEXT("./sound/equip.{}"sv)},
		{DESTROYED, EPRO_TEXT("./sound/destroyed"sv)},
		{TOKEN, EPRO_TEXT("./sound/token"sv)},
		{ATTACK, EPRO_TEXT("./sound/attack"sv)},
		{DIRECT_ATTACK, EPRO_TEXT("./sound/directattack"sv)},
		{ATTACK_DISABLED, EPRO_TEXT("./sound/attackdisabled"sv)},
		{POS_CHANGE, EPRO_TEXT("./sound/poschange"sv)},
		{DRAW, EPRO_TEXT("./sound/draw"sv)},
		{SHUFFLE, EPRO_TEXT("./sound/shuffle"sv)},
		{DAMAGE, EPRO_TEXT("./sound/damage"sv)},
		{RECOVER, EPRO_TEXT("./sound/gainlp"sv)},
		{COUNTER_ADD, EPRO_TEXT("./sound/addcounter"sv)},
		{COUNTER_REMOVE, EPRO_TEXT("./sound/removecounter"sv)},
		{COIN, EPRO_TEXT("./sound/coinflip"sv)},
		{DICE, EPRO_TEXT("./sound/diceroll"sv)},
		{NEXT_TURN, EPRO_TEXT("./sound/nextturn"sv)},
		{PHASE, EPRO_TEXT("./sound/phase"sv)},
		{PLAYER_ENTER, EPRO_TEXT("./sound/playerenter"sv)},
		{CUTIN_chain, EPRO_TEXT("./sound/cutin_chain"sv)},
		{CUTIN_damage, EPRO_TEXT("./sound/cutin_damage"sv)},
		{CHAT, EPRO_TEXT("./sound/chatmessage"sv)}
        /////kdiy///////
	};
	const auto extensions = mixer->GetSupportedSoundExtensions();
	for(const auto& sound : fx) {
        /////kdiy///////
		// for(const auto& ext : extensions) {
		// 	const auto filename = epro::format(sound.second, ext);
		// 	if(Utils::FileExists(filename)) {
		// 		SFXList[sound.first] = Utils::ToUTF8IfNeeded(filename);
		// 		break;
		// 	}
		// }
        for (auto& file : Utils::FindFiles(sound.second, mixer->GetSupportedSoundExtensions())) {
			auto files = Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}/{}"), Utils::ToPathString(working_dir), sound.second, file));
			SFXList[sound.first].push_back(files);
		}
        /////kdiy///////
	}
}
void SoundManager::RefreshBGMDir(epro::path_stringview path, BGM scene) {
	if(!IsUsable())
		return;
	for(auto& file : Utils::FindFiles(epro::format(EPRO_TEXT("./sound/BGM/{}"), path), mixer->GetSupportedMusicExtensions())) {
		auto conv = Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), path, file));
		BGMList[BGM::ALL].push_back(conv);
		BGMList[scene].push_back(std::move(conv));
	}
}
/////kdiy///////
void SoundManager::RefreshZipChants(epro::path_stringview folder, std::vector<std::string>& list, int character) {
    if(character < 1) return;
    for(auto& archive : Utils::archives) {
		if(Utils::ToUTF8IfNeeded({ archive.archive->getArchiveName().c_str(), archive.archive->getArchiveName().size() }).find(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("/sound/character/{}.zip"), textcharacter[character - 1][0]))) != std::string::npos) {
        	for(auto& file : Utils::FindFileNames(archive.archive, folder, mixer->GetSupportedSoundExtensions(), 1)) {
            	auto filename = Utils::GetFileName(file);
		    	list.push_back(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), folder, filename)));
			}
			break;
		}
    }
}
void SoundManager::RefreshZipCards(epro::path_stringview folder, std::map<std::pair<CHANT, uint32_t>, std::vector<std::string>>& list, CHANT chant, int character) {
    if(character < 1) return;
    for(auto& archive : Utils::archives) {
		if(Utils::ToUTF8IfNeeded({ archive.archive->getArchiveName().c_str(), archive.archive->getArchiveName().size() }).find(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("/sound/character/{}.zip"), textcharacter[character - 1][0]))) != std::string::npos) {
			for(auto& file : Utils::FindFileNames(archive.archive, folder, mixer->GetSupportedSoundExtensions(), 1)) {
				auto filename = Utils::GetFileName(file);
				auto filenamecode = filename;
				size_t pos = filename.find_first_of(EPRO_TEXT("+_"));
				size_t pos_sub = filename.find_first_of(EPRO_TEXT("_"));
				if(pos != std::string::npos) {
					size_t pos_add = filename.find_first_of(EPRO_TEXT("+"));
					if(pos_sub == pos) {
						if(pos_add != std::string::npos && pos_add != pos_sub + 2)
							continue;
						filenamecode = filename.substr(0, pos);
					}
				}
				try {
					uint32_t code = static_cast<uint32_t>(std::stoul(filenamecode));
					auto key = std::make_pair(chant, code);
					if(code)
						list[key].push_back(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), folder, filename)));
				}
				catch(...) {
					continue;
				}
			}
        	break;
		}
	}
}
void SoundManager::RefreshChants(epro::path_stringview folder, std::vector<std::string>& list) {
	for(auto& file : Utils::FindFiles(epro::format(EPRO_TEXT("./sound/character/{}"), folder), mixer->GetSupportedMusicExtensions())) {
		auto filename = Utils::GetFileName(file);
        list.push_back(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), folder, filename)));
	}
}
void SoundManager::RefreshCards(epro::path_stringview folder, std::map<std::pair<CHANT, uint32_t>, std::vector<std::string>>& list, CHANT chant) {
    for(auto& file : Utils::FindFiles(epro::format(EPRO_TEXT("./sound/character/{}"), folder), mixer->GetSupportedMusicExtensions())) {
		auto filename = Utils::GetFileName(file);
		auto filenamecode = filename;
		size_t pos = filename.find_first_of(EPRO_TEXT("+_"));
		size_t pos_sub = filename.find_first_of(EPRO_TEXT("_"));
		if(pos != std::string::npos) {
			size_t pos_add = filename.find_first_of(EPRO_TEXT("+"));
			if(pos_sub == pos) {
				if(pos_add != std::string::npos && pos_add != pos_sub + 2)
					continue;
				filenamecode = filename.substr(0, pos);
			}
		}
		try {
			uint32_t code = static_cast<uint32_t>(std::stoul(filenamecode));
			auto key = std::make_pair(chant, code);
			if(code && code > 0)
				list[key].push_back(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), folder, filename)));
		}
		catch(...) {
			continue;
		}
	}
}
/////kdiy///////
void SoundManager::RefreshChantsList() {
	if(!IsUsable())
		return;
	static constexpr std::pair<CHANT, epro::path_stringview> types[]{
		/////kdiy///////
		{CHANT::SET,       EPRO_TEXT("set"sv)},
		{CHANT::DESTROY,   EPRO_TEXT("destroyed"sv)},
		{CHANT::DRAW,      EPRO_TEXT("draw"sv)},
		{CHANT::DAMAGE,    EPRO_TEXT("damage"sv)},
		{CHANT::RECOVER,   EPRO_TEXT("gainlp"sv)},
		{CHANT::NEXTTURN,  EPRO_TEXT("nextturn"sv)},
		{CHANT::STARTUP,  EPRO_TEXT("startup"sv)},
		{CHANT::BORED,  EPRO_TEXT("bored"sv)},
		{CHANT::TAGSWITCH,  EPRO_TEXT("tagswitch"sv)},
		{CHANT::PENDULUM,  EPRO_TEXT("pendulum"sv)},
		{CHANT::PSCALE,  EPRO_TEXT("activate/pendulum"sv)},
		{CHANT::OPPCOUNTER,  EPRO_TEXT("oppcounter"sv)},
		{CHANT::SELFCOUNTER,  EPRO_TEXT("selfcounter"sv)},
		{CHANT::RELEASE,  EPRO_TEXT("release"sv)},
		{CHANT::BATTLEPHASE,  EPRO_TEXT("battlephase"sv)},
		{CHANT::TURNEND,  EPRO_TEXT("turnend"sv)},
		{CHANT::WIN,  EPRO_TEXT("cardwin"sv)},
		{CHANT::LOSE,  EPRO_TEXT("lose"sv)},
		{CHANT::REINCARNATE,    EPRO_TEXT("summon/reincarnate"sv)},
		/////kdiy///////
		{CHANT::SUMMON,    EPRO_TEXT("summon"sv)},
		{CHANT::ATTACK,    EPRO_TEXT("attack"sv)},
		{CHANT::ACTIVATE,  EPRO_TEXT("activate"sv)}
	};
	/////kdiy//////
	//for (const auto& chantType : types) {
		// const epro::path_string searchPath = epro::format(EPRO_TEXT("./sound/{}"), chantType.second);
		// Utils::MakeDirectory(searchPath);
		//for (auto& file : Utils::FindFiles(searchPath, mixer->GetSupportedSoundExtensions())) {
			// auto scode = Utils::GetFileName(file);
            // try {
            //     uint32_t code = static_cast<uint32_t>(std::stoul(scode));
            //     auto key = std::make_pair(chantType.first, code);
				// if (code && !Chantcard.count(key))	
				// 	Chantcard[key] = epro::format("{}/{}", working_dir, Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), searchPath, file)));
        //     }
        //     catch (...) {
        //         continue;
        //     }
        // }

	for (auto& file : Utils::FindFiles(EPRO_TEXT("./sound/BGM/card"), mixer->GetSupportedSoundExtensions())) {
		auto scode = Utils::GetFileName(file);
		try {
			uint32_t code = static_cast<uint32_t>(std::stoul(scode));
			if (code && !ChantBGM.count(code)) {
				ChantBGM[code] = Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("./sound/BGM/card/{}"), scode));
			}
		}
		catch (...) {
			continue;
		}
	}

	for(const auto& chantType : types) {
		int i = -1;
		if(chantType.first == CHANT::SET) i = 0;
		if(chantType.first == CHANT::DESTROY) i = 1;
		if(chantType.first == CHANT::DRAW) i = 2;
		if(chantType.first == CHANT::DAMAGE) i = 3;
		if(chantType.first == CHANT::RECOVER) i = 4;
		if(chantType.first == CHANT::NEXTTURN) i = 5;
		if(chantType.first == CHANT::STARTUP) i = 6;
		if(chantType.first == CHANT::BORED) i = 7;
		if(chantType.first == CHANT::SUMMON) i = 8;
		if(chantType.first == CHANT::ATTACK) i = 9;
		if(chantType.first == CHANT::ACTIVATE) i = 10;
		if(chantType.first == CHANT::PENDULUM) i = 11;
		if(chantType.first == CHANT::PSCALE) i = 12;
		if(chantType.first == CHANT::OPPCOUNTER) i = 13;
		if(chantType.first == CHANT::SELFCOUNTER) i = 14;
		if(chantType.first == CHANT::RELEASE) i = 15;
		if(chantType.first == CHANT::BATTLEPHASE) i = 16;
		if(chantType.first == CHANT::TURNEND) i = 17;
		if(chantType.first == CHANT::WIN) i = 18;
		if(chantType.first == CHANT::LOSE) i = 19;
		if(chantType.first == CHANT::TAGSWITCH) i = 20;
		if(chantType.first == CHANT::REINCARNATE) i = 21;
		if(i == -1) continue;
		for(int x = 1; x < CHARACTER_VOICE + CHARACTER_STORY_ONLY; x++) {
			int size = textcharacter[x-1].size();
			for(int j = 0; j < size; j++) {
				if(size > 1 && j == 0) continue;
				auto character_name = textcharacter[x-1][j];
				if(chantType.first == CHANT::SUMMON) {
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/summon"), character_name, chantType.second), Chantaction[i][x][0][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/fusion"), character_name, chantType.second), Chantaction[i][x][1][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/synchro"), character_name, chantType.second), Chantaction[i][x][2][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/xyz"), character_name, chantType.second), Chantaction[i][x][3][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/link"), character_name, chantType.second), Chantaction[i][x][4][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/ritual"), character_name, chantType.second), Chantaction[i][x][5][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/pendulum"), character_name, chantType.second), Chantaction[i][x][6][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/spsummon"), character_name, chantType.second), Chantaction[i][x][7][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/attack"), character_name, chantType.second), Chantaction[i][x][8][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/defense"), character_name, chantType.second), Chantaction[i][x][9][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/advance"), character_name, chantType.second), Chantaction[i][x][10][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/max"), character_name, chantType.second), Chantaction[i][x][11][0], x);
				} else if(chantType.first == CHANT::REINCARNATE)
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}"), character_name, chantType.second), Chantaction[8][x][12][0], x);
				else if(chantType.first == CHANT::ATTACK) {
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/attack"), character_name, chantType.second), Chantaction[i][x][0][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/monster"), character_name, chantType.second), Chantaction[i][x][1][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/directattack"), character_name, chantType.second), Chantaction[i][x][2][0], x);
				} else if(chantType.first == CHANT::ACTIVATE) {
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/activate"), character_name, chantType.second), Chantaction[i][x][0][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/fromhand"), character_name, chantType.second), Chantaction[i][x][1][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/monster"), character_name, chantType.second), Chantaction[i][x][2][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/normalspell"), character_name, chantType.second), Chantaction[i][x][3][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/quickspell"), character_name, chantType.second), Chantaction[i][x][4][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/continuousspell"), character_name, chantType.second), Chantaction[i][x][5][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/equip"), character_name, chantType.second), Chantaction[i][x][6][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/ritual"), character_name, chantType.second), Chantaction[i][x][7][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/normaltrap"), character_name, chantType.second), Chantaction[i][x][8][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/continuoustrap"), character_name, chantType.second), Chantaction[i][x][9][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/countertrap"), character_name, chantType.second), Chantaction[i][x][10][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/flip"), character_name, chantType.second), Chantaction[i][x][11][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/field"), character_name, chantType.second), Chantaction[i][x][12][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/action"), character_name, chantType.second), Chantaction[i][x][13][0], x);
				} else if(chantType.first == CHANT::DRAW) {
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/advantage"), character_name, chantType.second), Chantaction[i][x][0][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/disadvantage"), character_name, chantType.second), Chantaction[i][x][1][0], x);
				} else if(chantType.first == CHANT::SET) {
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}"), character_name, chantType.second), Chantaction[i][x][0][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/monster"), character_name, chantType.second), Chantaction[i][x][1][0], x);
				} else if(chantType.first == CHANT::DAMAGE) {
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}"), character_name, chantType.second), Chantaction[i][x][0][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/cost"), character_name, chantType.second), Chantaction[i][x][1][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/minor"), character_name, chantType.second), Chantaction[i][x][2][0], x);
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/major"), character_name, chantType.second), Chantaction[i][x][3][0], x);
				} else if(chantType.first == CHANT::PENDULUM) {
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/activate"), character_name, chantType.second), Chantaction[i][x][0][0], x);
				} else if(chantType.first != CHANT::WIN) {
					RefreshZipChants(epro::format(EPRO_TEXT("{}/{}"), character_name, chantType.second), Chantaction[i][x][0][0], x);
					for(int playno = 0; playno < CHARACTER_VOICE - 1; playno++)
				    	RefreshZipChants(epro::format(EPRO_TEXT("{}/{}/{}"), character_name, chantType.second, textcharacter[playno][0]), Chantaction[i][x][0][playno + 1], x);
				}
				if(chantType.first == CHANT::SUMMON || chantType.first == CHANT::ATTACK || chantType.first == CHANT::ACTIVATE || chantType.first == CHANT::PENDULUM || chantType.first == CHANT::REINCARNATE)
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card"), character_name, chantType.second), Chantcard[x][0][0], chantType.first, x);
				if(chantType.first == CHANT::ACTIVATE) {
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/activate"), character_name, chantType.second), Chantcard[x][0][0], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/flip"), character_name, chantType.second), Chantcard[x][1][0], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/recover"), character_name, chantType.second), Chantcard[x][2][0], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/damage"), character_name, chantType.second), Chantcard[x][3][0], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/chain"), character_name, chantType.second), Chantcard[x][4][0], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/fussummon"), character_name, chantType.second), Chantcard[x][5][0], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/synsummon"), character_name, chantType.second), Chantcard[x][5][1], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/xyzsummon"), character_name, chantType.second), Chantcard[x][5][2], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/linksummon"), character_name, chantType.second), Chantcard[x][5][3], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/ritsummon"), character_name, chantType.second), Chantcard[x][5][4], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/spsummon"), character_name, chantType.second), Chantcard[x][5][5], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/summon"), character_name, chantType.second), Chantcard[x][5][6], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/otherssummon"), character_name, chantType.second), Chantcard[x][5][7], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/equip"), character_name, chantType.second), Chantcard[x][6][0], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/othersequip"), character_name, chantType.second), Chantcard[x][6][1], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/atkannounce"), character_name, chantType.second), Chantcard[x][7][0], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/battled"), character_name, chantType.second), Chantcard[x][7][1], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/battle_noxyz"), character_name, chantType.second), Chantcard[x][7][2], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/battle"), character_name, chantType.second), Chantcard[x][7][3], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/standby"), character_name, chantType.second), Chantcard[x][7][4], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/end"), character_name, chantType.second), Chantcard[x][7][5], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/othersdestroy"), character_name, chantType.second), Chantcard[x][8][0], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/othersremove"), character_name, chantType.second), Chantcard[x][9][0], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/cost"), character_name, chantType.second), Chantcard[x][10][0], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/xyzmaterial"), character_name, chantType.second), Chantcard[x][11][0], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/nonfield"), character_name, chantType.second), Chantcard[x][12][0], chantType.first, x);
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}/card/main"), character_name, chantType.second), Chantcard[x][13][0], chantType.first, x);
				}
				if(chantType.first == CHANT::WIN)
					RefreshZipCards(epro::format(EPRO_TEXT("{}/{}"), character_name, chantType.second), Chantcard[x][0][0], chantType.first, x);

				if(size > 1) character_name = epro::format(EPRO_TEXT("{}/{}"), textcharacter[x-1][0], textcharacter[x-1][j]);
				if(chantType.first == CHANT::SUMMON) {
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/summon"), character_name, chantType.second), Chantaction2[i][x][0][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/fusion"), character_name, chantType.second), Chantaction2[i][x][1][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/synchro"), character_name, chantType.second), Chantaction2[i][x][2][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/xyz"), character_name, chantType.second), Chantaction2[i][x][3][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/link"), character_name, chantType.second), Chantaction2[i][x][4][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/ritual"), character_name, chantType.second), Chantaction2[i][x][5][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/pendulum"), character_name, chantType.second), Chantaction2[i][x][6][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/spsummon"), character_name, chantType.second), Chantaction2[i][x][7][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/attack"), character_name, chantType.second), Chantaction2[i][x][8][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/defense"), character_name, chantType.second), Chantaction2[i][x][9][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/advance"), character_name, chantType.second), Chantaction2[i][x][10][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/max"), character_name, chantType.second), Chantaction2[i][x][11][0]);
				} else if(chantType.first == CHANT::REINCARNATE)
					RefreshChants(epro::format(EPRO_TEXT("{}/{}"), character_name, chantType.second), Chantaction2[8][x][12][0]);
				else if(chantType.first == CHANT::ATTACK) {
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/attack"), character_name, chantType.second), Chantaction2[i][x][0][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/monster"), character_name, chantType.second), Chantaction2[i][x][1][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/directattack"), character_name, chantType.second), Chantaction2[i][x][2][0]);
				} else if(chantType.first == CHANT::ACTIVATE) {
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/activate"), character_name, chantType.second), Chantaction2[i][x][0][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/fromhand"), character_name, chantType.second), Chantaction2[i][x][1][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/monster"), character_name, chantType.second), Chantaction2[i][x][2][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/normalspell"), character_name, chantType.second), Chantaction2[i][x][3][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/quickspell"), character_name, chantType.second), Chantaction2[i][x][4][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/continuousspell"), character_name, chantType.second), Chantaction2[i][x][5][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/equip"), character_name, chantType.second), Chantaction2[i][x][6][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/ritual"), character_name, chantType.second), Chantaction2[i][x][7][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/normaltrap"), character_name, chantType.second), Chantaction2[i][x][8][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/continuoustrap"), character_name, chantType.second), Chantaction2[i][x][9][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/countertrap"), character_name, chantType.second), Chantaction2[i][x][10][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/flip"), character_name, chantType.second), Chantaction2[i][x][11][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/field"), character_name, chantType.second), Chantaction2[i][x][12][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/action"), character_name, chantType.second), Chantaction2[i][x][13][0]);
				} else if(chantType.first == CHANT::DRAW) {
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/advantage"), character_name, chantType.second), Chantaction2[i][x][0][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/disadvantage"), character_name, chantType.second), Chantaction2[i][x][1][0]);
				} else if(chantType.first == CHANT::SET) {
					RefreshChants(epro::format(EPRO_TEXT("{}/{}"), character_name, chantType.second), Chantaction2[i][x][0][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/monster"), character_name, chantType.second), Chantaction2[i][x][1][0]);
				} else if(chantType.first == CHANT::DAMAGE) {
					RefreshChants(epro::format(EPRO_TEXT("{}/{}"), character_name, chantType.second), Chantaction2[i][x][0][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/cost"), character_name, chantType.second), Chantaction2[i][x][1][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/minor"), character_name, chantType.second), Chantaction2[i][x][2][0]);
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/major"), character_name, chantType.second), Chantaction2[i][x][3][0]);
				} else if(chantType.first == CHANT::PENDULUM) {
					RefreshChants(epro::format(EPRO_TEXT("{}/{}/activate"), character_name, chantType.second), Chantaction2[i][x][0][0]);
				} else if(chantType.first != CHANT::WIN) {
			    	RefreshChants(epro::format(EPRO_TEXT("{}/{}"), character_name, chantType.second), Chantaction2[i][x][0][0]);
					for(int playno = 0; playno < CHARACTER_VOICE - 1; playno++)
				    	RefreshChants(epro::format(EPRO_TEXT("{}/{}/{}"), character_name, chantType.second, textcharacter[playno][0]), Chantaction2[i][x][0][playno + 1]);
				}

				if(chantType.first == CHANT::SUMMON || chantType.first == CHANT::ATTACK || chantType.first == CHANT::ACTIVATE || chantType.first == CHANT::PENDULUM || chantType.first == CHANT::REINCARNATE)
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card"), character_name, chantType.second), Chantcard2[x][0][0], chantType.first);
				if(chantType.first == CHANT::ACTIVATE) {
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/activate"), character_name, chantType.second), Chantcard2[x][0][0], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/flip"), character_name, chantType.second), Chantcard2[x][1][0], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/recover"), character_name, chantType.second), Chantcard2[x][2][0], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/damage"), character_name, chantType.second), Chantcard2[x][3][0], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/chain"), character_name, chantType.second), Chantcard2[x][4][0], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/fussummon"), character_name, chantType.second), Chantcard2[x][5][0], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/synsummon"), character_name, chantType.second), Chantcard2[x][5][1], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/xyzsummon"), character_name, chantType.second), Chantcard2[x][5][2], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/linksummon"), character_name, chantType.second), Chantcard2[x][5][3], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/ritsummon"), character_name, chantType.second), Chantcard2[x][5][4], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/spsummon"), character_name, chantType.second), Chantcard2[x][5][5], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/summon"), character_name, chantType.second), Chantcard2[x][5][6], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/otherssummon"), character_name, chantType.second), Chantcard2[x][5][7], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/equip"), character_name, chantType.second), Chantcard2[x][6][0], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/othersequip"), character_name, chantType.second), Chantcard2[x][6][1], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/atkannounce"), character_name, chantType.second), Chantcard2[x][7][0], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/battled"), character_name, chantType.second), Chantcard2[x][7][1], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/battle_noxyz"), character_name, chantType.second), Chantcard2[x][7][2], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/battle"), character_name, chantType.second), Chantcard2[x][7][3], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/standby"), character_name, chantType.second), Chantcard2[x][7][4], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/end"), character_name, chantType.second), Chantcard2[x][7][5], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/othersdestroy"), character_name, chantType.second), Chantcard2[x][8][0], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/othersremove"), character_name, chantType.second), Chantcard2[x][9][0], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/cost"), character_name, chantType.second), Chantcard2[x][10][0], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/xyzmaterial"), character_name, chantType.second), Chantcard2[x][11][0], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/nonfield"), character_name, chantType.second), Chantcard2[x][12][0], chantType.first);
					RefreshCards(epro::format(EPRO_TEXT("{}/{}/card/main"), character_name, chantType.second), Chantcard2[x][13][0], chantType.first);
				}
				if(chantType.first == CHANT::WIN)
					RefreshCards(epro::format(EPRO_TEXT("{}/{}"), character_name, chantType.second), Chantcard2[x][0][0], chantType.first);
			}
		}
	}
	/////kdiy///////
}
/////kdiy/////
int32_t SoundManager::GetSoundDuration() {
	std::size_t sampleCount = mainGame->soundBuffer.getSampleCount();
    unsigned int sampleRate = mainGame->soundBuffer.getSampleRate();
    float duration = static_cast<float>(sampleCount) / sampleRate;
	return static_cast<int32_t>(duration * 1000);
}
void SoundManager::PlayModeSound(int i, uint32_t code, bool music) {
	if(!mainGame->mode->isMode) return;
	bool lock = false;
    std::string file = epro::format("./mode/story/story{}/soundDialog/{}.mp3", mainGame->mode->chapter, mainGame->mode->plotIndex);
    if(std::find(soundcount.begin(), soundcount.end(), file) != soundcount.end())
        return;
	if(soundsEnabled && Utils::FileExists(Utils::ToPathString(file)) && mainGame->soundBuffer.loadFromFile(file)) {
    	soundcount.push_back(file);
		if(music) PauseMusic(true);
		StopSounds();
		mainGame->chantsound.setBuffer(mainGame->soundBuffer);
		mainGame->chantsound.setVolume(gGameConfig->soundVolume);
		mainGame->chantsound.play();
    	lock = true;
	}
	mainGame->ShowElement(mainGame->wChPloatBody[i]);
    mainGame->stChPloatInfo[i]->setText(mainGame->mode->GetPloat(code).data());
	if(mainGame->dInfo.isStarted) {
    	if(!lock) {
        	std::unique_lock<epro::mutex> lck(mainGame->gMutex);
        	mainGame->cv->wait_for(lck, std::chrono::milliseconds(2500));
        	return;
		}
		mainGame->isEvent = true;
    	std::unique_lock<epro::mutex> lck(mainGame->gMutex);
		auto wait = std::chrono::milliseconds(GetSoundDuration());
    	mainGame->cv->wait_for(lck, wait);
		mainGame->isEvent = false;
	}
	if(music) PauseMusic(false);
}
/////kdiy/////
void SoundManager::PlaySoundEffect(SFX sound) {
	if(!IsUsable())
		return;
	if(!soundsEnabled) return;
	if(sound >= SFX::SFX_TOTAL_SIZE) return;
    /////kdiy/////
	// const auto& soundfile = SFXList[sound];
	// if(soundfile.empty()) return;
    //mixer->PlaySound(soundfile);
    int count = SFXList[sound].size();
	if(count > 0) {
		int soundno = (std::uniform_int_distribution<>(0, count - 1))(rnd);
		mixer->PlaySound(SFXList[sound][soundno]);
    } else if(sound == SFX::SUMMON_DARK || sound == SFX::SUMMON_DIVINE || sound == SFX::SUMMON_EARTH || sound == SFX::SUMMON_FIRE || sound == SFX::SUMMON_LIGHT || sound == SFX::SUMMON_WATER || sound == SFX::SUMMON_WIND) {
        int count = SFXList[SFX::SUMMON].size();
        if(count > 0) {
            int soundno = (std::uniform_int_distribution<>(0, count - 1))(rnd);
            mixer->PlaySound(SFXList[SFX::SUMMON][soundno]);
        }
    } else if(sound == SFX::SPECIAL_SUMMON_DARK || sound == SFX::SPECIAL_SUMMON_DIVINE || sound == SFX::SPECIAL_SUMMON_EARTH || sound == SFX::SPECIAL_SUMMON_FIRE || sound == SFX::SPECIAL_SUMMON_LIGHT || sound == SFX::SPECIAL_SUMMON_WATER || sound == SFX::SPECIAL_SUMMON_WIND) {
        int count = SFXList[SFX::SPECIAL_SUMMON].size();
        if(count > 0) {
            int soundno = (std::uniform_int_distribution<>(0, count - 1))(rnd);
            mixer->PlaySound(SFXList[SFX::SPECIAL_SUMMON][soundno]);
        }
    }
    /////kdiy/////
}
void SoundManager::PlayBGM(BGM scene, bool loop) {
	if(!IsUsable())
		return;
	if(!musicEnabled)
		return;
	auto& list = BGMList[scene];
	auto count = static_cast<int>(list.size());
	if(count == 0)
		return;
	if(scene != bgm_scene || !mixer->MusicPlaying()) {
		bgm_scene = scene;
		auto bgm = (std::uniform_int_distribution<>(0, count - 1))(rnd);
		const std::string BGMName = epro::format("{}/./sound/BGM/{}", working_dir, list[bgm]);
		/////kdiy/////
		std::string bgm_custom = "BGM/custom/";
		std::string bgm_menu = "BGM/menu/";
		std::string bgm_deck = "BGM/deck/";
		std::string bgm_win = "BGM/win/";
		std::string bgm_lose = "BGM/lose/";
		if(BGMName.find(bgm_menu) != std::string::npos && BGMName.find(bgm_deck) != std::string::npos && BGMName.find(bgm_win) != std::string::npos && BGMName.find(bgm_lose) != std::string::npos && bgm_now.find(bgm_custom) != std::string::npos) return;
		bgm_now = BGMName;
		/////kdiy/////
		if(!mixer->PlayMusic(BGMName, loop)) {
			// music failed to load, directly remove it from the list
			currentlyLooping = loop;
			list.erase(std::next(list.begin(), bgm));
		}
	} else if(loop != currentlyLooping) {
		currentlyLooping = loop;
		mixer->LoopMusic(loop);
	}
}
///////kdiy//////
bool SoundManager::PlayCardBGM(uint32_t code, uint32_t code2) {
	if (musicEnabled) {
		std::vector<std::string> list;
		auto chant_it = ChantBGM.find(code);
		auto chant_it2 = ChantBGM.find(code2);
		if(chant_it2 == ChantBGM.end()) {
			if(chant_it == ChantBGM.end())
				return false;
			else {
				const auto extensions = mixer->GetSupportedSoundExtensions();
				for(const auto& ext : extensions) {
					const auto filename = epro::format("{}.{}", chant_it->second, Utils::ToUTF8IfNeeded(ext));
					if(Utils::FileExists(Utils::ToPathString(filename)))
						list.push_back(filename);
					for(int i = 1; i < 9; i++) {
						const auto filename2 = epro::format("{}_{}.{}", chant_it->second, i, Utils::ToUTF8IfNeeded(ext));
						if (Utils::FileExists(Utils::ToPathString(filename2)))
							list.push_back(filename2);
					}
				}
			}
		} else {
			const auto extensions = mixer->GetSupportedSoundExtensions();
			for(const auto& ext : extensions) {
				const auto filename = epro::format("{}.{}", chant_it2->second, Utils::ToUTF8IfNeeded(ext));
				if(Utils::FileExists(Utils::ToPathString(filename)))
					list.push_back(filename);
				for(int i = 1; i < 9; i++) {
					const auto filename2 = epro::format("{}_{}.{}", chant_it2->second, i, Utils::ToUTF8IfNeeded(ext));
					if(Utils::FileExists(Utils::ToPathString(filename2)))
						list.push_back(filename2);
				}
			}
		}
		for(auto playedfile : soundcount) {
			for(auto it = list.begin(); it != list.end(); /* NOTHING */) {
				if(playedfile == *it)
					it = list.erase(it);
				else
					++it;
			}
		}
        for(auto playedfile : soundcount)
			list.erase(std::remove(list.begin(), list.end(), playedfile), list.end());
		int count = list.size();
		if(count > 0) {
			int soundno = (std::uniform_int_distribution<>(0, count - 1))(rnd);
            if(std::find(soundcount.begin(), soundcount.end(), list[soundno]) != soundcount.end())
                return false;
            soundcount.push_back(list[soundno]);
			if(mixer->MusicPlaying())
		 	    mixer->StopMusic();
			if(mixer->PlayMusic(list[soundno], false)) {
				return true;
			} else return false;
		} else
			return false;
		return true;
	}
    return false;
}
void SoundManager::PlayCustomMusic(std::string num) {
	if(soundsEnabled) {
		const auto extensions = mixer->GetSupportedSoundExtensions();
		for(const auto& ext : extensions) {
			const auto file = epro::format("./sound/custom/{}.{}", num, Utils::ToUTF8IfNeeded(ext));
			if(Utils::FileExists(Utils::ToPathString(file))) {
				if (mainGame->soundBuffer.loadFromFile(file)) {
					mainGame->chantsound.setBuffer(mainGame->soundBuffer);
					mainGame->chantsound.setVolume(gGameConfig->soundVolume);
					mainGame->chantsound.play();
					mainGame->isEvent = true;
					if(mainGame->dInfo.isInDuel && gGameConfig->pauseduel) {
						std::unique_lock<epro::mutex> lck(mainGame->gMutex);
						auto wait = std::chrono::milliseconds(GetSoundDuration());
						mainGame->cv->wait_for(lck, wait);
					}
					mainGame->isEvent = false;
				    break;
				}
			}
		}
	}
}
void SoundManager::PlayCustomBGM(std::string num) {
	if (musicEnabled) {
		const auto extensions = mixer->GetSupportedSoundExtensions();
		for (const auto& ext : extensions) {
			const auto filename = epro::format("./sound/BGM/custom/{}.{}", num, Utils::ToUTF8IfNeeded(ext));
		 	if (Utils::FileExists(Utils::ToPathString(filename))) {
		 		if(mixer->MusicPlaying())
		 	       mixer->StopMusic();
		 		bgm_now = filename;
		 		if(mixer->PlayMusic(filename, false))
					break;
		 	}
		 }
	}
}
bool SoundManager::PlayFieldSound() {
	if (soundsEnabled) {
        std::vector<std::string> list;
        for (auto& file : Utils::FindFiles(epro::format(EPRO_TEXT("{}/sound/field"), Utils::ToPathString(working_dir)), mixer->GetSupportedSoundExtensions()))
			list.push_back(Utils::ToUTF8IfNeeded(file));
		int count = list.size();
		if(count > 0) {
			int soundno = (std::uniform_int_distribution<>(0, count - 1))(rnd);
			if(mixer->PlaySound(epro::format("{}/sound/field/{}", working_dir, list[soundno])))
				return true;
			else return false;
        }
    }
    return false;
}
void SoundManager::AddtoZipChantSPList(CHANT chant, uint16_t extra, size_t j, std::vector<std::string>& chantlist, std::vector<std::string>& list, std::vector<std::string>& list2, std::vector<std::string>& list3, int character) {
    if(character < 1) return;
    for(auto& archive : Utils::archives) {
		if(Utils::ToUTF8IfNeeded({ archive.archive->getArchiveName().c_str(), archive.archive->getArchiveName().size() }).find(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("/sound/character/{}.zip"), textcharacter[character - 1][0]))) != std::string::npos) {
			for(size_t k = 0; k < chantlist.size(); k++) {
				std::string sound = chantlist[k];
				for(auto& findfile : Utils::FindFileNames(archive.archive, Utils::ToPathString(sound), mixer->GetSupportedSoundExtensions())) {
					auto file = Utils::ToUTF8IfNeeded(findfile);
					if(mainGame->dInfo.isInDuel) {
						if(std::find(soundcount.begin(), soundcount.end(), file) != soundcount.end())
		        			continue;
			    	}
					if(chant == CHANT::SUMMON) {
						if((extra & 0x1) && j == 1) {
					    	if(sound.find("+") == std::string::npos)
		                    	list.push_back(file);
		                	else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
				    	}
						if((extra & 0x2) && j == 2) {
							if(sound.find("+") == std::string::npos)
		                    	list.push_back(file);
		                	else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x4) && j == 3) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x8) && j == 4) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x10) && j == 5) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x20) && j == 6) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x40) && j == 7) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x80) && j == 0) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x100) && j == 8) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x200) && j == 9) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x400) && j == 10) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x800) && j == 11) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
                		if((extra == 0) && j == 0) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
					} else if(chant == CHANT::REINCARNATE) {
			    		if((extra & 0x1000) && j == 12) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
		    		} else if(chant == CHANT::ATTACK) {
			    		if((extra & 0x1) && j == 0) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x2) && j == 1) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x4) && j == 2) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
		    		} else if(chant == CHANT::ACTIVATE) {
			    		if((extra & 0x1) && j == 0) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x2) && j == 1) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x4) && j == 3) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x8) && j == 4) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x10) && j == 5) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x20) && j == 6) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x40) && j == 7) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x80) && j == 8) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x100) && j == 9) {
				   			if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x200) && j == 10) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x400) && j == 11) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x800) && j == 2) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x1000) && j == 12) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		if((extra & 0x4000) && j == 13) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
		    		} else if(chant == CHANT::DRAW) {
			    		if((extra & 0x2) && j == 1) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		else if(j == 0) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
		    		} else if(chant == CHANT::SET) {
			    		if((extra & 0x1) && j == 1) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		else if(j == 0) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
		    		} else if(chant == CHANT::DAMAGE) {
			    		if((extra & 0x1) && j == 1) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		else if((extra & 0x2) && j == 2) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		else if((extra & 0x4) && j == 3) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
			    		else if(j == 0) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
		   	 		} else if(chant == CHANT::PENDULUM) {
			    		if((extra & 0x1) && j == 0) {
				    		if(sound.find("+") == std::string::npos)
		                		list.push_back(file);
		            		else if(sound.find("+2") == std::string::npos)
			    				list2.push_back(file);
		    				else
			    				list3.push_back(file);
						}
		    		} else {
						if(sound.find("+") == std::string::npos)
		            		list.push_back(file);
		        		else if(sound.find("+2") == std::string::npos)
			    			list2.push_back(file);
		    			else
			    			list3.push_back(file);
					}
				}
			}
			break;
		}
	}
}
void SoundManager::AddtoChantSPList(CHANT chant, uint16_t extra, size_t j, std::vector<std::string>& chantlist, std::vector<std::string>& list, std::vector<std::string>& list2, std::vector<std::string>& list3) {
	for(size_t k = 0; k < chantlist.size(); k++) {
		std::string sound = chantlist[k];
		for(const auto& ext : mixer->GetSupportedSoundExtensions()) {
			const auto file = epro::format("{}.{}", sound, Utils::ToUTF8IfNeeded(ext));
			if(!Utils::FileExists(Utils::ToPathString(epro::format("./sound/character/{}", file))))
		        continue;
			if(mainGame->dInfo.isInDuel) {
				if(std::find(soundcount.begin(), soundcount.end(), file) != soundcount.end())
		        	continue;
			}
		    if(chant == CHANT::SUMMON) {
				if((extra & 0x1) && j == 1) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x2) && j == 2) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x4) && j == 3) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x8) && j == 4) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x10) && j == 5) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x20) && j == 6) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x40) && j == 7) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x80) && j == 0) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x100) && j == 8) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x200) && j == 9) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x400) && j == 10) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x800) && j == 11) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
                if((extra == 0) && j == 0) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			} else if(chant == CHANT::REINCARNATE) {
			    if((extra & 0x1000) && j == 12) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
		    } else if(chant == CHANT::ATTACK) {
			    if((extra & 0x1) && j == 0) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x2) && j == 1) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x4) && j == 2) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
		    } else if(chant == CHANT::ACTIVATE) {
			    if((extra & 0x1) && j == 0) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x2) && j == 1) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x4) && j == 3) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x8) && j == 4) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x10) && j == 5) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x20) && j == 6) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x40) && j == 7) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x80) && j == 8) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x100) && j == 9) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x200) && j == 10) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x400) && j == 11) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x800) && j == 2) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x1000) && j == 12) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    if((extra & 0x4000) && j == 13) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
		    } else if(chant == CHANT::DRAW) {
			    if((extra & 0x2) && j == 1) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    else if(j == 0) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
		    } else if(chant == CHANT::SET) {
			    if((extra & 0x1) && j == 1) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    else if(j == 0) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
		    } else if(chant == CHANT::DAMAGE) {
			    if((extra & 0x1) && j == 1) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    else if((extra & 0x2) && j == 2) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    else if((extra & 0x4) && j == 3) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			    else if(j == 0) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
		    } else if(chant == CHANT::PENDULUM) {
			    if((extra & 0x1) && j == 0) {
				    if(sound.find("+") == std::string::npos)
		                list.push_back(file);
		            else if(sound.find("+2") == std::string::npos)
			    	list2.push_back(file);
		    	else
			    	list3.push_back(file);
				}
		    } else {
				if(sound.find("+") == std::string::npos)
		            list.push_back(file);
		        else if(sound.find("+2") == std::string::npos)
			    	list2.push_back(file);
		    	else
			    	list3.push_back(file);
			}
		}
	}
}
bool SoundManager::AddtoZipChantList(std::vector<std::string>& chantlist, std::vector<std::string>& list, std::vector<std::string>& list2, std::vector<std::string>& list3, int character) {
	bool haschant = false;
    if(character < 1) return haschant;
    for(auto& archive : Utils::archives) {
		if(Utils::ToUTF8IfNeeded({ archive.archive->getArchiveName().c_str(), archive.archive->getArchiveName().size() }).find(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("/sound/character/{}.zip"), textcharacter[character - 1][0]))) != std::string::npos) {
			for(size_t k = 0; k < chantlist.size(); k++) {
				std::string sound = chantlist[k];
				for(auto& findfile : Utils::FindFileNames(archive.archive, Utils::ToPathString(sound), mixer->GetSupportedSoundExtensions())) {
					haschant = true;
					auto file = Utils::ToUTF8IfNeeded(findfile);
					if(std::find(soundcount.begin(), soundcount.end(), file) != soundcount.end())
		        		continue;
					if(sound.find("+") == std::string::npos)
		            	list.push_back(file);
		        	else if(sound.find("+2") == std::string::npos)
			    		list2.push_back(file);
		    		else
			    		list3.push_back(file);
				}
			}
			break;
		}
	}
	return haschant;
}
bool SoundManager::AddtoChantList(std::vector<std::string>& chantlist, std::vector<std::string>& list, std::vector<std::string>& list2, std::vector<std::string>& list3) {
	bool haschant = false;
	for(size_t k = 0; k < chantlist.size(); k++) {
		std::string sound = chantlist[k];
		for(const auto& ext : mixer->GetSupportedSoundExtensions()) {
			const auto file = epro::format("{}.{}", sound, Utils::ToUTF8IfNeeded(ext));
			if(!Utils::FileExists(Utils::ToPathString(epro::format("./sound/character/{}", file))))
		        continue;
			haschant = true;
			if(std::find(soundcount.begin(), soundcount.end(), file) != soundcount.end())
		        continue;
			if(sound.find("+") == std::string::npos)
		        list.push_back(file);
		    else if(sound.find("+2") == std::string::npos)
			    list2.push_back(file);
		    else
			    list3.push_back(file);
		}
	}
	return haschant;
}
bool SoundManager::PlayZipChants(CHANT chant, std::string file, const uint8_t side, uint8_t player) {
    if(character[player] < 1) return false;
    for(auto& archive : Utils::archives) {
		if(Utils::ToUTF8IfNeeded({ archive.archive->getArchiveName().c_str(), archive.archive->getArchiveName().size() }).find(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("/sound/character/{}.zip"), textcharacter[character[player] - 1][0]))) != std::string::npos) {
			for(auto& index : Utils::FindFiles(archive.archive, Utils::ToPathString(file), mixer->GetSupportedSoundExtensions())) {
				auto reader = archive.archive->createAndOpenFile(index);
				if(reader == nullptr)
					continue;
				long length = reader->getSize();
				if(length == 0) {
					reader->drop();
					continue;
				}
				char* buff = new char[length + 1];
				reader->read(buff, length);
				const auto& name = reader->getFileName();
				const std::string filename = Utils::ToUTF8IfNeeded({ name.c_str(), name.size() });
				if(filename == file) {
					if (mainGame->soundBuffer.loadFromMemory(buff, length)) {
						//record this chant as played
						if(mainGame->dInfo.isInDuel && chant != CHANT::DRAW && chant != CHANT::STARTUP && chant != CHANT::WIN && chant != CHANT::LOSE) {
							soundcount.push_back(file);
						}
						StopSounds();
						mainGame->chantsound.setBuffer(mainGame->soundBuffer);
						mainGame->chantsound.setVolume(gGameConfig->soundVolume);
						mainGame->chantsound.play();
						if((mainGame->dInfo.isInDuel && chant == CHANT::STARTUP) || (chant != CHANT::STARTUP && chant != CHANT::BORED && chant != CHANT::WIN)) {
							if(mainGame->dInfo.isInDuel && gGameConfig->pauseduel) {
								mainGame->isEvent = true;
								auto t = mainGame->Ploats[character[player]-1].find(file);
								if(t != mainGame->Ploats[character[player]-1].end()) {
									mainGame->ShowElement(mainGame->wChPloatBody[side]);
        							mainGame->stChPloatInfo[side]->setText(mainGame->Ploats[character[player]-1][file].data());
								}
								std::unique_lock<epro::mutex> lck(mainGame->gMutex);
								auto wait = std::chrono::milliseconds(GetSoundDuration());
								mainGame->cv->wait_for(lck, wait);
								if(mainGame->wChPloatBody[side]->isVisible())
									mainGame->HideElement(mainGame->wChPloatBody[side]);
								mainGame->isEvent = false;
							}
						}
					}
				}
				reader->drop();
				delete[] buff;
            	return true;
            	break;
			}
			break;
		}
	}
	return false;
}
bool SoundManager::PlayChants(CHANT chant, std::string file, const uint8_t side, uint8_t player) {
	auto filepath = epro::format("./sound/character/{}", file);
	if(Utils::FileExists(Utils::ToPathString(filepath))) {
		if (mainGame->soundBuffer.loadFromFile(filepath)) {
			if(mainGame->dInfo.isInDuel && chant != CHANT::DRAW && chant != CHANT::STARTUP && chant != CHANT::WIN && chant != CHANT::LOSE) {
				soundcount.push_back(file);
		    }
			StopSounds();
			mainGame->chantsound.setBuffer(mainGame->soundBuffer);
			mainGame->chantsound.setVolume(gGameConfig->soundVolume);
			mainGame->chantsound.play();
			if((mainGame->dInfo.isInDuel && chant == CHANT::STARTUP) || (chant != CHANT::STARTUP && chant != CHANT::BORED && chant != CHANT::WIN)) {
				if(mainGame->dInfo.isInDuel && gGameConfig->pauseduel) {
					mainGame->isEvent = true;
					int size = gSoundManager->textcharacter[character[player]-1].size();
					auto unzipfile = size == 1 ? file : file.substr(gSoundManager->textcharacter[character[player]-1][0].length()+1);
					auto t = mainGame->Ploats[character[player]-1].find(unzipfile);
					if(t != mainGame->Ploats[character[player]-1].end()) {
						mainGame->ShowElement(mainGame->wChPloatBody[side]);
        				mainGame->stChPloatInfo[side]->setText(mainGame->Ploats[character[player]-1][unzipfile].data());
					}
					std::unique_lock<epro::mutex> lck(mainGame->gMutex);
					auto wait = std::chrono::milliseconds(GetSoundDuration());
					mainGame->cv->wait_for(lck, wait);
					if(mainGame->wChPloatBody[side]->isVisible())
						mainGame->HideElement(mainGame->wChPloatBody[side]);
					mainGame->isEvent = false;
				}
			}
			return true;
		}
	}
	return false;
}
void SoundManager::PlayStartupChant(uint8_t player, std::vector<uint8_t> team) {
	int playerno = (std::uniform_int_distribution<>(0, team.size() - 1))(rnd);
	PlayChant(CHANT::STARTUP, 0, 0, 0, player, 0, 0, 0, team[playerno]);
}
//bool SoundManager::PlayChant(CHANT chant, uint32_t code) {
bool SoundManager::PlayChant(CHANT chant, uint32_t code, uint32_t code2, const uint8_t side, uint8_t player, uint16_t extra, uint16_t card_extra, uint8_t card_extra2, uint8_t player2) {
///////kdiy//////
	if(!IsUsable())
		return false;
	if(!soundsEnabled) return false;
	///////kdiy//////
// 	auto key = std::make_pair(chant, code);
// 	auto chant_it = ChantsList.find(key);
// 	if(chant_it == ChantsList.end())
// 		return false;
// 	return mixer->PlaySound(chant_it->second);
	if(player < 0) return false;
	if(character[player] < 1) return false;
	int i = -1;
	if(chant == CHANT::SET) i = 0;
	if(chant == CHANT::DESTROY) i = 1;
	if(chant == CHANT::DRAW) i = 2;
	if(chant == CHANT::DAMAGE) i = 3;
	if(chant == CHANT::RECOVER) i = 4;
	if(chant == CHANT::NEXTTURN) i = 5;
	if(chant == CHANT::STARTUP) i = 6;
	if(chant == CHANT::BORED) i = 7;
	if(chant == CHANT::SUMMON) i = 8;
	if(chant == CHANT::ATTACK) i = 9;
	if(chant == CHANT::ACTIVATE) i = 10;
	if(chant == CHANT::PENDULUM) i = 11;
	if(chant == CHANT::PSCALE) i = 12;
	if(chant == CHANT::OPPCOUNTER) i = 13;
	if(chant == CHANT::SELFCOUNTER) i = 14;
	if(chant == CHANT::RELEASE) i = 15;
	if(chant == CHANT::BATTLEPHASE) i = 16;
	if(chant == CHANT::TURNEND) i = 17;
	if(chant == CHANT::WIN) i = 18;
	if(chant == CHANT::LOSE) i = 19;
	if(chant == CHANT::TAGSWITCH) i = 20;
	if(chant == CHANT::REINCARNATE) i = 21;
	if(i == -1) return false;
	auto key = std::make_pair(chant, code);
	auto key2 = std::make_pair(chant, code2);
	std::vector<std::string> list; //store zip chant
    std::vector<std::string> list2; //store zip chant part 2
    std::vector<std::string> list3; //store zip chant part 2
	std::vector<std::string> _list; //store chant
    std::vector<std::string> _list2; //store chant part 2
    std::vector<std::string> _list3; //store chant part 2

	//for chant card extra>0
	if(card_extra > 0) {
		bool haschant = false; //find chant card extra exists
		for(int k = 1; k < 14; k++) {
			if(!(card_extra & (0x1<<(k-1)))) continue;
			for(int k2 = 0; k2 < 8; k2++) {
				if((k != 5 && k != 6 && k != 7) && k2 > 0) break;
				if(k == 6 && k2 > 1) break;
				if(k == 7 && k2 > 5) break;
				auto chant_it = Chantcard[character[player]][k][k2].find(key); //code zip chant
				auto _chant_it = Chantcard2[character[player]][k][k2].find(key); //code chant
				auto chant_it2 = Chantcard[character[player]][k][k2].find(key2); //alias zip chant
				auto _chant_it2 = Chantcard2[character[player]][k][k2].find(key2); //alias chant
				//found chant card code
				if(chant_it != Chantcard[character[player]][k][k2].end())
					haschant = AddtoZipChantList(chant_it->second, list, list2, list3, character[player]); //list can be empty even if chant card file exists due to chant already played, use haschant to check if this file exists
				else if(_chant_it != Chantcard2[character[player]][k][k2].end())
					haschant = AddtoChantList(_chant_it->second, _list, _list2, _list3);

				if(list.size() < 1 && _list.size() < 1) {
				//found chant card alias
					if(chant_it2 != Chantcard[character[player]][k][k2].end())
						haschant = AddtoZipChantList(chant_it2->second, list, list2, list3, character[player]);
					else if(_chant_it2 != Chantcard2[character[player]][k][k2].end())
						haschant = AddtoChantList(_chant_it2->second, _list, _list2, _list3);
				}
				//found 1 extra chant card, no need add other extra
				if(haschant)
					break;
			}
			if(haschant)
				break;
		}
	}
	//for chant card extra=0
	if(list.size() < 1 && _list.size() < 1) {
		auto chant_it = Chantcard[character[player]][0][0].find(key); //code zip chant
		auto _chant_it = Chantcard2[character[player]][0][0].find(key); //code chant
		auto chant_it2 = Chantcard[character[player]][0][0].find(key2); //alias zip chant
		auto _chant_it2 = Chantcard2[character[player]][0][0].find(key2); //alias chant
		if(chant_it != Chantcard[character[player]][0][0].end())
			AddtoZipChantList(chant_it->second, list, list2, list3, character[player]);
		else if(_chant_it != Chantcard2[character[player]][0][0].end())
			AddtoChantList(_chant_it->second, _list, _list2, _list3);

		if(list.size() < 1 && _list.size() < 1) {
			if(chant_it2 != Chantcard[character[player]][0][0].end())
				AddtoZipChantList(chant_it2->second, list, list2, list3, character[player]);
			else if(_chant_it2 != Chantcard2[character[player]][0][0].end())
				AddtoChantList(_chant_it2->second, _list, _list2, _list3);
		}
	}
	
	if(list.size() < 1 && _list.size() < 1) {
		//not find chant for code & alias
        for(int j = 0; j < 14; j++) {
			if(character[player2] > 0) {
				AddtoZipChantSPList(chant, extra, j, Chantaction[i][character[player]][j][character[player2]], list, list2, list3, character[player]);
				AddtoChantSPList(chant, extra, j, Chantaction2[i][character[player]][j][character[player2]], _list, _list2, _list3);
			}
			if(list.size() < 1 && _list.size() < 1) {
				AddtoZipChantSPList(chant, extra, j, Chantaction[i][character[player]][j][0], list, list2, list3, character[player]);
				AddtoChantSPList(chant, extra, j, Chantaction2[i][character[player]][j][0], _list, _list2, _list3);
			}
		}
	}
	
	int count = list.size();
	int _count = _list.size();
	if(count > 0) {
		int soundno = (std::uniform_int_distribution<>(0, count - 1))(rnd);
		if(PlayZipChants(chant, list[soundno], side, player)) {
			auto filename = Utils::ToUTF8IfNeeded(Utils::GetFileName(list[soundno]));
			int count2 = list2.size();
			if(count2 > 0) {
				for(auto it = list2.begin(); it != list2.end(); /* NOTHING */) {
					auto filename2 = Utils::ToUTF8IfNeeded(Utils::GetFileName(*it));
					if(filename != filename2.substr(0, filename2.size() - 2))
						it = list2.erase(it);
					else
						++it;
				}
				count2 = list2.size();
				if(count2 > 0) {
					int soundno2 = (std::uniform_int_distribution<>(0, count2 - 1))(rnd);
					if(PlayZipChants(chant, list2[soundno2], side, player)) {
						int count3 = list3.size();
						if(count3 > 0) {
							for(auto it = list3.begin(); it != list3.end(); /* NOTHING */) {
								auto filename2 = Utils::ToUTF8IfNeeded(Utils::GetFileName(*it));
								if(filename != filename2.substr(0, filename2.size() - 2))
									it = list3.erase(it);
								else
									++it;
							}
							count3 = list3.size();
							if(count3 > 0) {
								int soundno2 = (std::uniform_int_distribution<>(0, count3 - 1))(rnd);
								PlayZipChants(chant, list3[soundno2], side, player);
							}
						}
					}
				}
			}
			return true;
		}
	} else if(_count > 0) {
		int soundno = (std::uniform_int_distribution<>(0, _count - 1))(rnd);
		if(PlayChants(chant, _list[soundno], side, player)) {
			const auto filename = Utils::GetFileName(_list[soundno]);
			int _count2 = _list2.size();
			if(_count2 > 0) {
				for(auto it = _list2.begin(); it != _list2.end(); /* NOTHING */) {
					auto filename2 = Utils::ToUTF8IfNeeded(Utils::GetFileName(*it));
					if(filename != filename2.substr(0, filename2.size() - 2))
						it = _list2.erase(it);
					else
						++it;
				}
				_count2 = _list2.size();
				if(_count2 > 0) {
					int soundno2 = (std::uniform_int_distribution<>(0, _count2 - 1))(rnd);
					if(PlayChants(chant, _list2[soundno2], side, player)) {
						int _count3 = _list3.size();
						if(_count3 > 0) {
							for(auto it = _list3.begin(); it != _list3.end(); /* NOTHING */) {
								auto filename2 = Utils::ToUTF8IfNeeded(Utils::GetFileName(*it));
								if(filename != filename2.substr(0, filename2.size() - 2))
									it = _list3.erase(it);
								else
									++it;
							}
							_count3 = _list3.size();
							if(_count3 > 0) {
								int soundno2 = (std::uniform_int_distribution<>(0, _count3 - 1))(rnd);
								PlayChants(chant, _list3[soundno2], side, player);
							}
						}
					}
				}
			}
			return true;
		}
	}
	return false;
///////kdiy//////
}
void SoundManager::SetSoundVolume(double volume) {
	if(!IsUsable())
		return;
	mixer->SetSoundVolume(volume);
}
void SoundManager::SetMusicVolume(double volume) {
	if(!IsUsable())
		return;
	mixer->SetMusicVolume(volume);
}
void SoundManager::EnableSounds(bool enable) {
	if(!IsUsable())
		return;
	if(!(soundsEnabled = enable))
		mixer->StopSounds();
}
void SoundManager::EnableMusic(bool enable) {
	if(!IsUsable())
		return;
	if(!(musicEnabled = enable))
		mixer->StopMusic();
}
void SoundManager::StopSounds() {
	if(!IsUsable())
		return;
	mixer->StopSounds();
}
void SoundManager::StopMusic() {
	if(!IsUsable())
		return;
	mixer->StopMusic();
}
void SoundManager::PauseMusic(bool pause) {
	if(!IsUsable())
		return;
	mixer->PauseMusic(pause);
}

void SoundManager::Tick() {
	if(!IsUsable())
		return;
	mixer->Tick();
}

} // namespace ygo
