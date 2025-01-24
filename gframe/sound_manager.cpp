#define WIN32_LEAN_AND_MEAN
#include "sound_manager.h"
#include "utils.h"
#include "config.h"
#include "fmt.h"
#if defined(YGOPRO_USE_IRRKLANG)
#include "sound_irrklang.h"
#define BACKEND SoundIrrklang
#elif defined(YGOPRO_USE_SDL_MIXER)
#include "sound_sdlmixer.h"
#define BACKEND SoundMixer
#elif defined(YGOPRO_USE_SFML)
#include "sound_sfml.h"
#define BACKEND SoundSFML
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
/////kdiy/////
namespace ygo {
SoundManager::SoundManager(double sounds_volume, double music_volume, bool sounds_enabled, bool music_enabled) {
#ifdef BACKEND
	epro::print("Using: " STR(BACKEND)" for audio playback.\n");
	working_dir = Utils::ToUTF8IfNeeded(Utils::GetWorkingDirectory());
	soundsEnabled = sounds_enabled;
	musicEnabled = music_enabled;
	try {
		mixer = std::make_unique<BACKEND>();
		mixer->SetMusicVolume(music_volume);
		mixer->SetSoundVolume(sounds_volume);
	}
	catch(const std::runtime_error& e) {
		epro::print("Failed to initialize audio backend:\n");
		epro::print(e.what());
		succesfully_initied = soundsEnabled = musicEnabled = false;
		return;
	}
	catch(...) {
		epro::print("Failed to initialize audio backend.\n");
		succesfully_initied = soundsEnabled = musicEnabled = false;
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
	succesfully_initied = true;
#else
	epro::print("No audio backend available.\nAudio will be disabled.\n");
	succesfully_initied = soundsEnabled = musicEnabled = false;
	return;
#endif // BACKEND
}
bool SoundManager::IsUsable() {
	return succesfully_initied;
}
void SoundManager::RefreshBGMList() {
#ifdef BACKEND
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
#endif
}
void SoundManager::RefreshSoundsList() {
#ifdef BACKEND
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
#endif
}
void SoundManager::RefreshBGMDir(epro::path_stringview path, BGM scene) {
#ifdef BACKEND
	for(auto& file : Utils::FindFiles(epro::format(EPRO_TEXT("./sound/BGM/{}"), path), mixer->GetSupportedMusicExtensions())) {
		auto conv = Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), path, file));
		BGMList[BGM::ALL].push_back(conv);
		BGMList[scene].push_back(std::move(conv));
	}
#endif
}
/////kdiy///////
void SoundManager::RefreshZipChants(epro::path_stringview folder, std::vector<std::string>& list, int character) {
#ifdef BACKEND
    for(auto& archive : Utils::archives) {
		if(Utils::ToUTF8IfNeeded({ archive.archive->getArchiveName().c_str(), archive.archive->getArchiveName().size() }).find(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("/sound/character/{}"), textcharacter[character]))) == std::string::npos)
			continue;
        for(auto& file : Utils::FindFileNames(archive.archive, folder, mixer->GetSupportedSoundExtensions())) {
            auto filename = Utils::GetFileName(file);
		    list.push_back(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), folder, filename)));
		}
        break;
    }
#endif
}
void SoundManager::RefreshChants(epro::path_stringview folder, std::vector<std::string>& list) {
#ifdef BACKEND
	for(auto& file : Utils::FindFiles(folder, mixer->GetSupportedMusicExtensions())) {
		auto filename = Utils::GetFileName(file);
        list.push_back(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), folder, filename)));
	}
#endif
}
void SoundManager::RefreshZipCards(epro::path_stringview folder, std::map<std::pair<CHANT, uint32_t>, std::vector<std::string>>& list, CHANT chant, int character) {
#ifdef BACKEND
	for(auto& archive : Utils::archives) {
		if(Utils::ToUTF8IfNeeded({ archive.archive->getArchiveName().c_str(), archive.archive->getArchiveName().size() }).find(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("/sound/character/{}"), textcharacter[character]))) == std::string::npos)
			continue;
		for(auto& file : Utils::FindFileNames(archive.archive, folder, mixer->GetSupportedSoundExtensions())) {
			auto filename = Utils::GetFileName(file);
			size_t pos = filename.find_first_of(EPRO_TEXT("+_"));
			size_t pos_sub = filename.find_first_of(EPRO_TEXT("_"));
			if(pos != std::string::npos) {
			    filename = filename.substr(0, pos);
				size_t pos_sub = filename.find_first_of(EPRO_TEXT("_"));
			    size_t pos_add = filename.find_first_of(EPRO_TEXT("+"));
				if(pos_add == pos && pos_sub == pos_add + 2) {
					auto filename_add = filename.substr(0, pos_sub);
				}
			}
			try {
				uint32_t code = static_cast<uint32_t>(std::stoul(filename));
				auto key = std::make_pair(chant, code);
				if(code && !list.count(key)) {
					list[key].push_back(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), folder, filename)));
				}
			}
			catch(...) {
				continue;
			}
		}
        break;
	}
#endif
}
void SoundManager::RefreshCards(epro::path_stringview folder, std::map<std::pair<CHANT, uint32_t>, std::vector<std::string>>& list, CHANT chant, int character) {
#ifdef BACKEND
    for(auto& file : Utils::FindFiles(folder, mixer->GetSupportedMusicExtensions())) {
		auto filename = Utils::GetFileName(file);
		try {
			uint32_t code = static_cast<uint32_t>(std::stoul(filename));
			auto key = std::make_pair(chant, code);
			if(code && !list.count(key)) {
				list[key].push_back(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), folder, filename)));
			}
		}
		catch(...) {
			continue;
		}
        break;
	}
#endif
}
/////kdiy///////
void SoundManager::RefreshChantsList() {
#ifdef BACKEND
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
		{CHANT::PENDULUM,  EPRO_TEXT("pendulum"sv)},
		{CHANT::PSCALE,  EPRO_TEXT("activate/pendulum"sv)},
		{CHANT::OPPCOUNTER,  EPRO_TEXT("oppcounter"sv)},
		{CHANT::SELFCOUNTER,  EPRO_TEXT("selfcounter"sv)},
		{CHANT::RELEASE,  EPRO_TEXT("release"sv)},
		{CHANT::BATTLEPHASE,  EPRO_TEXT("battlephase"sv)},
		{CHANT::TURNEND,  EPRO_TEXT("turnend"sv)},
		{CHANT::WIN,  EPRO_TEXT("cardwin"sv)},
		{CHANT::LOSE,  EPRO_TEXT("lose"sv)},
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
	Utils::MakeDirectory(EPRO_TEXT("./sound/character"));
	for(uint8_t playno = 0; playno < CHARACTER_VOICE + CHARACTER_STORY_ONLY - 1; playno++)
		Utils::MakeDirectory(epro::format(EPRO_TEXT("./sound/character/{}"), textcharacter[playno]));

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
		std::vector<epro::path_string> searchPath2;
		searchPath2.push_back(epro::format(EPRO_TEXT("./sound/{}"), chantType.second));
		for(uint8_t playno = 0; playno < CHARACTER_VOICE + CHARACTER_STORY_ONLY - 1; playno++) {
			searchPath2.push_back(epro::format(EPRO_TEXT("./sound/character/{}/{}"), textcharacter[playno], chantType.second));
		}
		for(auto path : searchPath2)
			Utils::MakeDirectory(path);

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
		if(i == -1) continue;
		for(int x = 0; x < CHARACTER_VOICE + CHARACTER_STORY_ONLY; x++) {
			if(chantType.first == CHANT::SUMMON) {
				RefreshZipChants(epro::format(EPRO_TEXT("{}/fusion"), chantType.second), Chantaction[i][x][0][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/synchro"), chantType.second), Chantaction[i][x][1][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/xyz"), chantType.second), Chantaction[i][x][2][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/link"), chantType.second), Chantaction[i][x][3][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/ritual"), chantType.second), Chantaction[i][x][4][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/pendulum"), chantType.second), Chantaction[i][x][5][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/summon"), chantType.second), Chantaction[i][x][6][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/spsummon"), chantType.second), Chantaction[i][x][7][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/attack"), chantType.second), Chantaction[i][x][8][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/defense"), chantType.second), Chantaction[i][x][9][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/advance"), chantType.second), Chantaction[i][x][10][0], x);
			} else if(chantType.first == CHANT::ATTACK) {
				RefreshZipChants(epro::format(EPRO_TEXT("{}/attack"), chantType.second), Chantaction[i][x][0][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/monster"), chantType.second), Chantaction[i][x][1][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/directattack"), chantType.second), Chantaction[i][x][2][0], x);
			} else if(chantType.first == CHANT::ACTIVATE) {
				RefreshZipChants(epro::format(EPRO_TEXT("{}/activate"), chantType.second), Chantaction[i][x][0][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/fromhand"), chantType.second), Chantaction[i][x][1][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/monster"), chantType.second), Chantaction[i][x][2][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/normalspell"), chantType.second), Chantaction[i][x][3][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/quickspell"), chantType.second), Chantaction[i][x][4][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/continuousspell"), chantType.second), Chantaction[i][x][5][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/equip"), chantType.second), Chantaction[i][x][6][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/ritual"), chantType.second), Chantaction[i][x][7][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/normaltrap"), chantType.second), Chantaction[i][x][8][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/continuoustrap"), chantType.second), Chantaction[i][x][9][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/countertrap"), chantType.second), Chantaction[i][x][10][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/flip"), chantType.second), Chantaction[i][x][11][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/field"), chantType.second), Chantaction[i][x][12][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/action"), chantType.second), Chantaction[i][x][13][0], x);
			} else if(chantType.first == CHANT::DRAW) {
				RefreshZipChants(epro::format(EPRO_TEXT("{}/advantage"), chantType.second), Chantaction[i][x][0][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/disadvantage"), chantType.second), Chantaction[i][x][1][0], x);
			} else if(chantType.first == CHANT::SET) {
				RefreshZipChants(epro::format(EPRO_TEXT("{}"), chantType.second), Chantaction[i][x][0][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/monster"), chantType.second), Chantaction[i][x][1][0], x);
			} else if(chantType.first == CHANT::DAMAGE) {
				RefreshZipChants(epro::format(EPRO_TEXT("{}"), chantType.second), Chantaction[i][x][0][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/cost"), chantType.second), Chantaction[i][x][1][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/minor"), chantType.second), Chantaction[i][x][2][0], x);
				RefreshZipChants(epro::format(EPRO_TEXT("{}/major"), chantType.second), Chantaction[i][x][3][0], x);
			} else if(chantType.first == CHANT::PENDULUM) {
				RefreshZipChants(epro::format(EPRO_TEXT("{}/activate"), chantType.second), Chantaction[i][x][0][0], x);
			} else if(chantType.first != CHANT::WIN) {
				RefreshZipChants(epro::format(EPRO_TEXT("{}"), chantType.second), Chantaction[i][x][0][0], x);
				for(int playno = 0; playno < CHARACTER_VOICE - 1; playno++)
				    RefreshZipChants(epro::format(EPRO_TEXT("{}/{}"), chantType.second, textcharacter[playno]), Chantaction[i][x][0][playno + 1], x);
			}

			if(chantType.first == CHANT::SUMMON) {
				RefreshChants(epro::format(EPRO_TEXT("{}/summon"), searchPath2[x]), Chantaction2[i][x][0][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/fusion"), searchPath2[x]), Chantaction2[i][x][1][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/synchro"), searchPath2[x]), Chantaction2[i][x][2][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/xyz"), searchPath2[x]), Chantaction2[i][x][3][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/link"), searchPath2[x]), Chantaction2[i][x][4][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/ritual"), searchPath2[x]), Chantaction2[i][x][5][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/pendulum"), searchPath2[x]), Chantaction2[i][x][6][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/spsummon"), searchPath2[x]), Chantaction2[i][x][7][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/attack"), searchPath2[x]), Chantaction2[i][x][8][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/defense"), searchPath2[x]), Chantaction2[i][x][9][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/advance"), searchPath2[x]), Chantaction2[i][x][10][0]);
			} else if(chantType.first == CHANT::ATTACK) {
				RefreshChants(epro::format(EPRO_TEXT("{}/attack"), searchPath2[x]), Chantaction2[i][x][0][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/monster"), searchPath2[x]), Chantaction2[i][x][1][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/directattack"), searchPath2[x]), Chantaction2[i][x][2][0]);
			} else if(chantType.first == CHANT::ACTIVATE) {
				RefreshChants(epro::format(EPRO_TEXT("{}/activate"), searchPath2[x]), Chantaction2[i][x][0][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/fromhand"), searchPath2[x]), Chantaction2[i][x][1][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/monster"), searchPath2[x]), Chantaction2[i][x][2][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/normalspell"), searchPath2[x]), Chantaction2[i][x][3][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/quickspell"), searchPath2[x]), Chantaction2[i][x][4][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/continuousspell"), searchPath2[x]), Chantaction2[i][x][5][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/equip"), searchPath2[x]), Chantaction2[i][x][6][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/ritual"), searchPath2[x]), Chantaction2[i][x][7][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/normaltrap"), searchPath2[x]), Chantaction2[i][x][8][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/continuoustrap"), searchPath2[x]), Chantaction2[i][x][9][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/countertrap"), searchPath2[x]), Chantaction2[i][x][10][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/flip"), searchPath2[x]), Chantaction2[i][x][11][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/field"), searchPath2[x]), Chantaction2[i][x][12][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/action"), searchPath2[x]), Chantaction2[i][x][13][0]);
			} else if(chantType.first == CHANT::DRAW) {
				RefreshChants(epro::format(EPRO_TEXT("{}/advantage"), searchPath2[x]), Chantaction2[i][x][0][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/disadvantage"), searchPath2[x]), Chantaction2[i][x][1][0]);
			} else if(chantType.first == CHANT::SET) {
				RefreshChants(searchPath2[x], Chantaction2[i][x][0][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/monster"), searchPath2[x]), Chantaction2[i][x][1][0]);
			} else if(chantType.first == CHANT::DAMAGE) {
				RefreshChants(searchPath2[x], Chantaction2[i][x][0][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/cost"), searchPath2[x]), Chantaction2[i][x][1][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/minor"), searchPath2[x]), Chantaction2[i][x][2][0]);
				RefreshChants(epro::format(EPRO_TEXT("{}/major"), searchPath2[x]), Chantaction2[i][x][3][0]);
			} else if(chantType.first == CHANT::PENDULUM) {
				RefreshChants(epro::format(EPRO_TEXT("{}/activate"), searchPath2[x]), Chantaction2[i][x][0][0]);
			} else if(chantType.first != CHANT::WIN) {
			    RefreshChants(searchPath2[x], Chantaction2[i][x][0][0]);
				for(int playno = 0; playno < CHARACTER_VOICE - 1; playno++)
				    RefreshChants(epro::format(EPRO_TEXT("{}/{}"), searchPath2[x], textcharacter[playno]), Chantaction2[i][x][0][playno + 1]);
			}

			if(chantType.first == CHANT::SUMMON || chantType.first == CHANT::ATTACK || chantType.first == CHANT::ACTIVATE || chantType.first == CHANT::PENDULUM) {
				RefreshZipCards(epro::format(EPRO_TEXT("{}/card"), chantType.second), Chantcard[x], chantType.first, x);
				RefreshCards(epro::format(EPRO_TEXT("{}/card"), searchPath2[x]), Chantcard[x], chantType.first, x);
			}
			if(chantType.first == CHANT::WIN) {
				RefreshZipCards(epro::format(EPRO_TEXT("{}"), chantType.second), Chantcard[x], chantType.first, x);
				RefreshCards(epro::format(EPRO_TEXT("{}"), chantType.second), Chantcard[x], chantType.first, x);
			}
		}
	}
	/////kdiy///////
#endif	
}
/////kdiy/////
int32_t SoundManager::GetSoundDuration() {
	std::size_t sampleCount = mainGame->soundBuffer.getSampleCount();
    unsigned int sampleRate = mainGame->soundBuffer.getSampleRate();
    float duration = static_cast<float>(sampleCount) / sampleRate;
	return static_cast<int32_t>(duration * 1000);
}
int SoundManager::PlayModeSound(bool lock) {
#ifdef BACKEND
	if(!soundsEnabled) return 0;
	if(!mainGame->mode->isMode) return 0;
    std::string file = epro::format("./mode/story/story{}/soundDialog/{}.mp3", mainGame->mode->chapter, mainGame->mode->plotIndex);
	if(!Utils::FileExists(Utils::ToPathString(file)))
        return 0;
    if(std::find(gSoundManager->soundcount.begin(), gSoundManager->soundcount.end(), file) != gSoundManager->soundcount.end())
        return 2;
    gSoundManager->soundcount.push_back(file);
	gSoundManager->StopSounds();
	mixer->PlaySound(file);
    return 1;
#endif
    return 0;
}
void SoundManager::PlayMode(bool lock) {
    if(!lock) {
        std::unique_lock<epro::mutex> lck(mainGame->gMutex);
        mainGame->cv->wait_for(lck, std::chrono::milliseconds(2500));
        return;
    }
    std::string file = epro::format("./mode/story/story{}/soundDialog/{}.mp3", mainGame->mode->chapter, mainGame->mode->plotIndex);
	if(!Utils::FileExists(Utils::ToPathString(file))) return;
    std::unique_lock<epro::mutex> lck(mainGame->gMutex);
	auto wait = std::chrono::milliseconds(GetSoundDuration());
    mainGame->cv->wait_for(lck, wait);
}
/////kdiy/////
void SoundManager::PlaySoundEffect(SFX sound) {
#ifdef BACKEND
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
#endif
}
void SoundManager::PlayBGM(BGM scene, bool loop) {
#ifdef BACKEND
	if(!musicEnabled)
		return;
	const auto& list = BGMList[scene];
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
		mixer->PlayMusic(BGMName, loop);
	}
#endif
}
///////kdiy//////
bool SoundManager::PlayCardBGM(uint32_t code, uint32_t code2) {
#ifdef BACKEND
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
        for(auto file : gSoundManager->soundcount)
			list.erase(std::remove(list.begin(), list.end(), file), list.end());
		int count = list.size();
		if(count > 0) {
			int soundno = (std::uniform_int_distribution<>(0, count - 1))(rnd);
            if(std::find(gSoundManager->soundcount.begin(), gSoundManager->soundcount.end(), list[soundno]) != gSoundManager->soundcount.end())
                return false;
            gSoundManager->soundcount.push_back(list[soundno]);
			if(mixer->MusicPlaying())
		 	    mixer->StopMusic();
			if(mixer->PlayMusic(list[soundno], gGameConfig->loopMusic))
				return true;
			else return false;
		} else
			return false;
		return true;
	}
#endif
    return false;
}
void SoundManager::PlayCustomMusic(std::string num) {
#ifdef BACKEND
	if(soundsEnabled) {
		const auto extensions = mixer->GetSupportedSoundExtensions();
		for(const auto& ext : extensions) {
			const auto file = epro::format("./sound/custom/{}.{}", num, Utils::ToUTF8IfNeeded(ext));
			if(Utils::FileExists(Utils::ToPathString(file))) {
				if (mainGame->soundBuffer.loadFromFile(file)) {
					mainGame->chantsound.setBuffer(mainGame->soundBuffer);
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
#endif
}
void SoundManager::PlayCustomBGM(std::string num) {
#ifdef BACKEND
	if (musicEnabled) {
		const auto extensions = mixer->GetSupportedSoundExtensions();
		for (const auto& ext : extensions) {
			const auto filename = epro::format("./sound/BGM/custom/{}.{}", num, Utils::ToUTF8IfNeeded(ext));
		 	if (Utils::FileExists(Utils::ToPathString(filename))) {
		 		if(mixer->MusicPlaying())
		 	       mixer->StopMusic();
		 		bgm_now = filename;
		 		if (mixer->PlayMusic(filename, gGameConfig->loopMusic))
					break;
		 	}
		 }
	}
#endif
}
bool SoundManager::PlayFieldSound() {
#ifdef BACKEND
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
#endif
    return false;
}
void SoundManager::AddtoChantSPList(CHANT chant, uint16_t extra, size_t j, std::vector<std::string>& chantlist, std::vector<std::string>& list) {
	for(size_t k = 0; k < chantlist.size(); k++) {
		std::string sound = chantlist[k];
		if(mainGame->dInfo.isInDuel) {
            for(auto file : gSoundManager->soundcount) {
				if(sound == file)
				    continue;
			}
		}
		if(chant == CHANT::SUMMON) {
			if((extra & 0x1) && j == 1)
				list.push_back(sound);
			if((extra & 0x2) && j == 2)
				list.push_back(sound);
			if((extra & 0x4) && j == 3)
				list.push_back(sound);
			if((extra & 0x8) && j == 4)
				list.push_back(sound);
			if((extra & 0x10) && j == 5)
				list.push_back(sound);
			if((extra & 0x20) && j == 6)
				list.push_back(sound);
			if((extra & 0x80) && j == 0)
				list.push_back(sound);
			if((extra & 0x40) && j == 7)
				list.push_back(sound);
			if((extra & 0x100) && j == 8)
				list.push_back(sound);
			if((extra & 0x200) && j == 9)
				list.push_back(sound);
			if((extra & 0x400) && j == 10)
				list.push_back(sound);
            if((extra == 0) && j == 0)
				list.push_back(sound);
		} else if(chant == CHANT::ATTACK) {
			if((extra & 0x1) && j == 0)
				list.push_back(sound);
			if((extra & 0x2) && j == 1)
				list.push_back(sound);
			if((extra & 0x4) && j == 2)
				list.push_back(sound);
		} else if(chant == CHANT::ACTIVATE) {
			if((extra & 0x1) && j == 0)
				list.push_back(sound);
			if((extra & 0x2) && j == 1)
				list.push_back(sound);
			if((extra & 0x800) && j == 2)
				list.push_back(sound);
			if((extra & 0x4) && j == 3)
				list.push_back(sound);
			if((extra & 0x8) && j == 4)
				list.push_back(sound);
			if((extra & 0x10) && j == 5)
				list.push_back(sound);
			if((extra & 0x20) && j == 6)
				list.push_back(sound);
			if((extra & 0x40) && j == 7)
				list.push_back(sound);
			if((extra & 0x80) && j == 8)
				list.push_back(sound);
			if((extra & 0x100) && j == 9)
				list.push_back(sound);
			if((extra & 0x200) && j == 10)
				list.push_back(sound);
			if((extra & 0x400) && j == 11)
				list.push_back(sound);
			if((extra & 0x1000) && j == 12)
				list.push_back(sound);
			if((extra & 0x4000) && j == 13)
				list.push_back(sound);
		} else if(chant == CHANT::DRAW) {
			if((extra & 0x2) && j == 1)
				list.push_back(sound);
			else if(j == 0)
				list.push_back(sound);
		} else if(chant == CHANT::SET) {
			if((extra & 0x1) && j == 1)
				list.push_back(sound);
			else if(j == 0)
				list.push_back(sound);
		} else if(chant == CHANT::DAMAGE) {
			if((extra & 0x1) && j == 1)
				list.push_back(sound);
			else if((extra & 0x2) && j == 2)
				list.push_back(sound);
			else if((extra & 0x4) && j == 3)
				list.push_back(sound);
			else if(j == 0)
				list.push_back(sound);
		} else if(chant == CHANT::PENDULUM) {
			if((extra & 0x1) && j == 0)
				list.push_back(sound);
		} else
			list.push_back(sound);
	}
}
void SoundManager::AddtoZipChantList(std::vector<std::string>& chantlist, int i, std::vector<std::string>& list, std::vector<std::string>& list2, int character) {
#ifdef BACKEND
	for(auto& archive : Utils::archives) {
		if(Utils::ToUTF8IfNeeded({ archive.archive->getArchiveName().c_str(), archive.archive->getArchiveName().size() }).find(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("/sound/character/{}"), textcharacter[character]))) == std::string::npos)
			continue;
		for(auto file : chantlist) {
			for(auto& findfile : Utils::FindFileNames(archive.archive, Utils::ToPathString(file), mixer->GetSupportedSoundExtensions())) {
				auto filename = Utils::GetFileName(findfile);
				if(filename.find(EPRO_TEXT("+")) == std::wstring::npos)
			        list.push_back(file);
			    else
			        list2.push_back(file);
			}
		}
	}
#endif
}
void SoundManager::AddtoChantList(std::vector<std::string>& chantlist, int i, std::vector<std::string>& list, std::vector<std::string>& list2) {
#ifdef BACKEND
    for(auto file : chantlist) {
		if(!Utils::FileExists(Utils::ToPathString(file)))
		    continue;
		auto filename = Utils::GetFileName(file);
		if(filename.find("+") == std::string::npos)
		    list.push_back(file);
		else
			list2.push_back(file);
	}
#endif
}
bool SoundManager::PlayZipChants(CHANT chant, std::string file, std::vector<std::string>& sound, uint8_t player) {
#ifdef BACKEND
	for(auto& archive : Utils::archives) {
		if(Utils::ToUTF8IfNeeded({ archive.archive->getArchiveName().c_str(), archive.archive->getArchiveName().size() }).find(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("/sound/character/{}"), textcharacter[character[player]]))) == std::string::npos)
			continue;
		for(auto& index : Utils::FindFiles(archive.archive, Utils::ToPathString(file), mixer->GetSupportedSoundExtensions(), 1)) {
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
						if(std::find(sound.begin(), sound.end(), file) != sound.end()) {
							reader->drop();
							delete[] buff;
							return false;
						}
						sound.push_back(file);
					}
					StopSounds();
					mainGame->chantsound.setBuffer(mainGame->soundBuffer);
					mainGame->chantsound.play();
					if(chant != CHANT::STARTUP && chant != CHANT::BORED && chant != CHANT::WIN) {
						mainGame->isEvent = true;
						if(mainGame->dInfo.isInDuel && gGameConfig->pauseduel && character[player] > 0) {
							std::unique_lock<epro::mutex> lck(mainGame->gMutex);
							auto wait = std::chrono::milliseconds(GetSoundDuration());
							mainGame->cv->wait_for(lck, wait);
						}
						mainGame->isEvent = false;
					}

					for(int j = 1; j < 6; j++) {
						auto files = Utils::FindFileNames(archive.archive, epro::format(EPRO_TEXT("{}_{}"), Utils::ToPathString(file), j), mixer->GetSupportedSoundExtensions(), 1);
						for(auto& file : files) {
						}
					}
				}
			}
			reader->drop();
			delete[] buff;
            return true;
            break;
		}
	}
#endif
	return false;
}
bool SoundManager::PlayChants(CHANT chant, std::string file, std::vector<std::string>& sound, uint8_t player) {
#ifdef BACKEND
	if(Utils::FileExists(Utils::ToPathString(file))) {
		if (mainGame->soundBuffer.loadFromFile(file)) {
			if(mainGame->dInfo.isInDuel && chant != CHANT::DRAW && chant != CHANT::STARTUP && chant != CHANT::WIN && chant != CHANT::LOSE) {
				if(std::find(sound.begin(), sound.end(), file) != sound.end())
				    return false;
				sound.push_back(file);
		    }
			StopSounds();
			mainGame->chantsound.setBuffer(mainGame->soundBuffer);
			mainGame->chantsound.play();
			if(chant != CHANT::STARTUP && chant != CHANT::BORED && chant != CHANT::WIN) {
				mainGame->isEvent = true;
				if(mainGame->dInfo.isInDuel && gGameConfig->pauseduel && character[player] > 0) {
					std::unique_lock<epro::mutex> lck(mainGame->gMutex);
					auto wait = std::chrono::milliseconds(GetSoundDuration());
					mainGame->cv->wait_for(lck, wait);
				}
				mainGame->isEvent = false;
			}

			const auto filename = Utils::GetFileName(file);
			for(int j = 1; j < 6; j++) {
				for(const auto& ext : mixer->GetSupportedSoundExtensions()) {
					const auto file2 = epro::format("{}+{}.{}", filename, j, Utils::ToUTF8IfNeeded(ext));
					if(Utils::FileExists(Utils::ToPathString(file2))) {
					    if (mainGame->soundBuffer.loadFromFile(file2)) {
							if(mainGame->dInfo.isInDuel && chant != CHANT::DRAW && chant != CHANT::STARTUP && chant != CHANT::WIN && chant != CHANT::LOSE) {
								if(std::find(sound.begin(), sound.end(), file) != sound.end())
								    return false;
								sound.push_back(file2);
							}
							StopSounds();
							mainGame->chantsound.setBuffer(mainGame->soundBuffer);
							mainGame->chantsound.play();
							if(chant != CHANT::STARTUP && chant != CHANT::BORED && chant != CHANT::WIN) {
								mainGame->isEvent = true;
								if(mainGame->dInfo.isInDuel && gGameConfig->pauseduel && character[player] > 0) {
									std::unique_lock<epro::mutex> lck(mainGame->gMutex);
									auto wait = std::chrono::milliseconds(GetSoundDuration());
									mainGame->cv->wait_for(lck, wait);
								}
								mainGame->isEvent = false;
							}
						}
					}
				}
			}
			return true;
		}
	}
#endif
	return false;
}
//bool SoundManager::PlayChant(CHANT chant, uint32_t code) {
bool SoundManager::PlayChant(CHANT chant, uint32_t code, uint32_t code2, uint8_t player, uint16_t extra, uint8_t player2) {
///////kdiy//////
#ifdef BACKEND
	if(!soundsEnabled) return false;
	///////kdiy//////
// 	auto key = std::make_pair(chant, code);
// 	auto chant_it = Chantcard.find(key);
// 	if(chant_it == Chantcard.end())
// 		return false;
// 	return mixer->PlaySound(chant_it->second);
	if(player < 0) return false;
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
	if(i == -1) return false;
	auto key = std::make_pair(chant, code);
	auto key2 = std::make_pair(chant, code2);
	std::vector<std::string> list;
    std::vector<std::string> list2;
	auto chant_it = Chantcard[character[player]].find(key);
	auto chant_it2 = Chantcard[character[player]].find(key2);
	std::vector<std::string> _list;
    std::vector<std::string> _list2;
	auto _chant_it = Chantcard2[character[player]].find(key);
	auto _chant_it2 = Chantcard2[character[player]].find(key2);

	if(chant_it2 == Chantcard[character[player]].end() && _chant_it2 == Chantcard2[character[player]].end()) {
		if(chant_it == Chantcard[character[player]].end() && _chant_it == Chantcard2[character[player]].end()) {
			//not find chant for this code
            for(int j = 0; j < 14; j++) {
				AddtoChantSPList(chant, extra, j, Chantaction[i][character[player]][j][character[player2]], list);
				AddtoChantSPList(chant, extra, j, Chantaction[i][character[player]][j][0], list);
				AddtoChantSPList(chant, extra, j, Chantaction2[i][character[player]][j][character[player2]], _list);
				AddtoChantSPList(chant, extra, j, Chantaction2[i][character[player]][j][0], _list);
			}
		} else {
			//found chant card code
			if(chant_it != Chantcard[character[player]].end())
				AddtoZipChantList(chant_it->second, i, list, list2, character[player]);
			else if(_chant_it != Chantcard2[character[player]].end())
				AddtoChantList(_chant_it->second, i, _list, _list2);
		}
	} else {
		//found chant card alias
		if(chant_it2 != Chantcard[character[player]].end())
			AddtoZipChantList(chant_it2->second, i, list, list2, character[player]);
		else if(_chant_it2 != Chantcard2[character[player]].end())
			AddtoChantList(_chant_it2->second, i, _list, _list2);
	}

	if(mainGame->dInfo.isInDuel) {
        for(auto file : gSoundManager->soundcount) {
            list.erase(std::remove(list.begin(), list.end(), file), list.end());
            _list.erase(std::remove(_list.begin(), _list.end(), file), _list.end());
        }
	}
	int count2 = list.size();
	int _count2 = _list.size();
	if(count2 > 0) {
		int soundno = (std::uniform_int_distribution<>(0, count2 - 1))(rnd);
		if(PlayZipChants(chant, list[soundno], gSoundManager->soundcount, player)) {
			int count22 = list2.size();
			if(count22 > 0) {
				for(int k = 0; k < count22; k++) {
					const auto filename = Utils::GetFileName(list2[k]).substr(0, Utils::GetFileName(list2[k]).size() - 2);
					if(filename == Utils::GetFileName(list[soundno]))
						PlayZipChants(chant, list2[k], gSoundManager->soundcount, player);
				}
			}
			return true;
		}
	} else if(_count2 > 0) {
		int soundno = (std::uniform_int_distribution<>(0, _count2 - 1))(rnd);
		if(PlayChants(chant, _list[soundno], gSoundManager->soundcount, player))
			return true;
	}
	return false;
///////kdiy//////
#else
	return false;
#endif
}
void SoundManager::SetSoundVolume(double volume) {
#ifdef BACKEND
	if(mixer)
		mixer->SetSoundVolume(volume);
#endif
}
void SoundManager::SetMusicVolume(double volume) {
#ifdef BACKEND
	if(mixer)
		mixer->SetMusicVolume(volume);
#endif
}
void SoundManager::EnableSounds(bool enable) {
#ifdef BACKEND
	if(mixer && !(soundsEnabled = enable))
		mixer->StopSounds();
#endif
}
void SoundManager::EnableMusic(bool enable) {
#ifdef BACKEND
	if(mixer && !(musicEnabled = enable))
		mixer->StopMusic();
#endif
}
void SoundManager::StopSounds() {
#ifdef BACKEND
	if(mixer)
		mixer->StopSounds();
#endif
}
void SoundManager::StopMusic() {
#ifdef BACKEND
	if(mixer)
		mixer->StopMusic();
#endif
}
void SoundManager::PauseMusic(bool pause) {
#ifdef BACKEND
	if(mixer)
		mixer->PauseMusic(pause);
#endif
}

void SoundManager::Tick() {
#ifdef BACKEND
	if(mixer)
		mixer->Tick();
#endif
}

} // namespace ygo
