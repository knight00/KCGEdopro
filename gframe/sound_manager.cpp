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
void SoundManager::RefreshZipChants(epro::path_stringview folder, std::vector<std::string>& list) {
#ifdef BACKEND
    for(auto& archive : Utils::archives) {
		if(Utils::ToUTF8IfNeeded({ archive.archive->getArchiveName().c_str(), archive.archive->getArchiveName().size() }).find("/expansions/kcgchant.zip") == std::string::npos)
			continue;
        for(auto& file : Utils::FindFileNames(archive.archive, folder, mixer->GetSupportedSoundExtensions(), 1))
		    list.push_back(Utils::ToUTF8IfNeeded(file));
        break;
    }
#endif
}
void SoundManager::RefreshChants(epro::path_stringview folder, std::vector<std::string>& list) {
#ifdef BACKEND
	for(auto& file : Utils::FindFiles(folder, mixer->GetSupportedMusicExtensions()))
        list.push_back(Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), folder, file)));
#endif
}
void SoundManager::RefreshZipCards(epro::path_stringview folder, std::map<std::pair<CHANT, uint32_t>, std::string>& list, CHANT chant) {
#ifdef BACKEND
	for(auto& archive : Utils::archives) {
		if(Utils::ToUTF8IfNeeded({ archive.archive->getArchiveName().c_str(), archive.archive->getArchiveName().size() }).find("/expansions/kcgchant.zip") == std::string::npos)
			continue;
		for(auto& file : Utils::FindFileNames(archive.archive, folder, mixer->GetSupportedSoundExtensions(), 1)) {
			auto scode = Utils::GetFileName(file);
            auto filename = Utils::ToUTF8IfNeeded(scode);
			if(filename.find("+") != std::string::npos || filename.find("-") != std::string::npos || filename.find("_") != std::string::npos || filename.find(".") != std::string::npos)
				continue;
			try {
				uint32_t code = static_cast<uint32_t>(std::stoul(scode));
				auto key = std::make_pair(chant, code);
				if(code && !list.count(key)) {
					list[key] = Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), folder, scode));
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
				// if (code && !ChantsList.count(key))	
				// 	ChantsList[key] = epro::format("{}/{}", working_dir, Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), searchPath, file)));
        //     }
        //     catch (...) {
        //         continue;
        //     }
        // }
	Utils::MakeDirectory(EPRO_TEXT("./sound/character"));
	for(uint8_t playno = 0; playno < CHARACTER_VOICE - 1; playno++)
		Utils::MakeDirectory(epro::format(EPRO_TEXT("./sound/character/{}"), textcharacter[playno]));
	Utils::MakeDirectory(EPRO_TEXT("./sound/character/darksiner"));
	ChantsBGMList.clear();
	for(auto list : ChantsList)
		list.clear();
    for(auto list : ChantsList2)
		list.clear();
	for(int i = 0; i < 20; i++) {
		for(uint8_t playno = 0; playno < CHARACTER_VOICE + CHARACTER_STORY_ONLY; playno++) {
			ChantSPList[i][playno].clear();
			ChantSPList2[i][playno].clear();
		}
	}

	for (auto& file : Utils::FindFiles(EPRO_TEXT("./sound/BGM/card"), mixer->GetSupportedSoundExtensions())) {
		auto scode = Utils::GetFileName(file);
		try {
			uint32_t code = static_cast<uint32_t>(std::stoul(scode));
			if (code && !ChantsBGMList.count(code)) {
				ChantsBGMList[code] = Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("./sound/BGM/card/{}"), scode));
			}
		}
		catch (...) {
			continue;
		}
	}

	for(const auto& chantType : types) {
		std::vector<epro::path_string> searchPath;
		searchPath.push_back(epro::format(EPRO_TEXT("{}"), chantType.second));
		std::vector<epro::path_string> searchPath2;
		searchPath2.push_back(epro::format(EPRO_TEXT("./sound/{}"), chantType.second));
		for(uint8_t playno = 0; playno < CHARACTER_VOICE - 1; playno++) {
			searchPath.push_back(epro::format(EPRO_TEXT("character/{}/{}"), textcharacter[playno], chantType.second));
			searchPath2.push_back(epro::format(EPRO_TEXT("./sound/character/{}/{}"), textcharacter[playno], chantType.second));
		}
		searchPath.push_back(epro::format(EPRO_TEXT("character/darksiner/{}"), chantType.second));
		searchPath2.push_back(epro::format(EPRO_TEXT("./sound/character/darksiner/{}"), chantType.second));
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
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/fusion"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/synchro"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/xyz"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/link"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/ritual"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/pendulum"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/summon"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/spsummon"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/attack"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/defense"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/advance"), ChantSPList[i][x]);
			} else if(chantType.first == CHANT::ATTACK) {
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/attack"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/monster"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/directattack"), ChantSPList[i][x]);
			} else if(chantType.first == CHANT::ACTIVATE) {
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/activate"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/fromhand"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/monster"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/quickspell"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/continuousspell"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/equip"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/ritual"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/normaltrap"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/continuoustrap"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/countertrap"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/flip"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/field"), ChantSPList[i][x]);
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/action"), ChantSPList[i][x]);
			} else if(chantType.first == CHANT::PENDULUM) {
				RefreshZipChants(searchPath[x] + EPRO_TEXT("/activate"), ChantSPList[i][x]);
			} else if(chantType.first != CHANT::WIN)
				RefreshZipChants(searchPath[x], ChantSPList[i][x]);

			if(chantType.first == CHANT::SUMMON || chantType.first == CHANT::ATTACK || chantType.first == CHANT::ACTIVATE || chantType.first == CHANT::PENDULUM)
				RefreshZipCards(epro::format(EPRO_TEXT("{}/card"), searchPath[x]), ChantsList[x], chantType.first);
			if(chantType.first == CHANT::WIN)
				RefreshZipCards(epro::format(EPRO_TEXT("{}"), searchPath[x]), ChantsList[x], chantType.first);

			if(chantType.first == CHANT::SUMMON) {
				RefreshChants(searchPath2[x] + EPRO_TEXT("/fusion"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/synchro"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/xyz"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/link"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/ritual"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/pendulum"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/summon"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/spsummon"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/attack"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/defense"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/advance"), ChantSPList2[i][x]);
			} else if(chantType.first == CHANT::ATTACK) {
				RefreshChants(searchPath2[x] + EPRO_TEXT("/attack"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/monster"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/directattack"), ChantSPList2[i][x]);
			} else if(chantType.first == CHANT::ACTIVATE) {
				RefreshChants(searchPath2[x] + EPRO_TEXT("/activate"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/fromhand"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/monster"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/quickspell"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/continuousspell"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/equip"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/ritual"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/normaltrap"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/continuoustrap"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/countertrap"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/flip"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/field"), ChantSPList2[i][x]);
				RefreshChants(searchPath2[x] + EPRO_TEXT("/action"), ChantSPList2[i][x]);
			} else if(chantType.first == CHANT::PENDULUM) {
				RefreshChants(searchPath2[x] + EPRO_TEXT("/activate"), ChantSPList2[i][x]);
			} else if(chantType.first != CHANT::WIN)
				RefreshChants(searchPath2[x], ChantSPList2[i][x]);

			if(chantType.first == CHANT::SUMMON || chantType.first == CHANT::ATTACK || chantType.first == CHANT::ACTIVATE || chantType.first == CHANT::PENDULUM) {
				for(auto& file : Utils::FindFiles(searchPath2[x] + EPRO_TEXT("/card"), mixer->GetSupportedSoundExtensions())) {
					auto scode = Utils::GetFileName(file);
					auto filename = Utils::ToUTF8IfNeeded(scode);
                    if(filename.find("+") != std::string::npos || filename.find("-") != std::string::npos || filename.find("_") != std::string::npos || filename.find(".") != std::string::npos)
						continue;
					try {
						uint32_t code = static_cast<uint32_t>(std::stoul(scode));
						auto key = std::make_pair(chantType.first, code);
						if(code && !ChantsList2[x].count(key))
							ChantsList2[x][key] = Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/card/{}"), searchPath2[x], scode));
					}
					catch(...) {
						continue;
					}
				}
			}
			if(chantType.first == CHANT::WIN) {
				for(auto& file : Utils::FindFiles(searchPath2[x], mixer->GetSupportedSoundExtensions())) {
					auto scode = Utils::GetFileName(file);
					auto filename = Utils::ToUTF8IfNeeded(scode);
                    if(filename.find("+") != std::string::npos || filename.find("-") != std::string::npos || filename.find("_") != std::string::npos || filename.find(".") != std::string::npos)
						continue;
					try {
						uint32_t code = static_cast<uint32_t>(std::stoul(scode));
						auto key = std::make_pair(chantType.first, code);
						if(code && !ChantsList2[x].count(key))
							ChantsList2[x][key] = Utils::ToUTF8IfNeeded(epro::format(EPRO_TEXT("{}/{}"), searchPath2[x], scode));
					}
					catch(...) {
						continue;
					}
				}
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
		auto chant_it = ChantsBGMList.find(code);
		auto chant_it2 = ChantsBGMList.find(code2);
		if(chant_it2 == ChantsBGMList.end()) {
			if(chant_it == ChantsBGMList.end())
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
void SoundManager::AddtoChantSPList(CHANT chant, uint16_t extra, std::vector<std::string>& chantlist, std::vector<std::string>& list) {
	for(size_t j = 0; j < chantlist.size(); j++) {
		std::string sound = chantlist[j];
		if(chant == CHANT::SUMMON) {
			if((extra & 0x1) && sound.find("summon/fusion/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x2) && sound.find("summon/synchro/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x4) && sound.find("summon/xyz/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x8) && sound.find("summon/link/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x10) && sound.find("summon/ritual/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x20) && sound.find("summon/pendulum/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x80) && sound.find("summon/summon/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x40) && sound.find("summon/spsummon/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x100) && sound.find("summon/attack/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x200) && sound.find("summon/defense/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x400) && sound.find("summon/advance/") != std::string::npos)
				list.push_back(sound);
            if((extra == 0) && sound.find("summon/summon/") != std::string::npos)
				list.push_back(sound);
		} else if(chant == CHANT::ATTACK) {
			if((extra & 0x1) && sound.find("attack/attack/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x2) && sound.find("attack/monster/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x4) && sound.find("attack/directattack/") != std::string::npos)
				list.push_back(sound);
		} else if(chant == CHANT::ACTIVATE) {
			if((extra & 0x1) && sound.find("activate/activate/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x2) && sound.find("activate/fromhand/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x4) && sound.find("activate/normalspell/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x8) && sound.find("activate/quickspell/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x10) && sound.find("activate/continuousspell/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x20) && sound.find("activate/equip/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x40) && sound.find("activate/ritual/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x80) && sound.find("activate/normaltrap/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x100) && sound.find("activate/continuoustrap/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x200) && sound.find("activate/countertrap/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x400) && sound.find("activate/flip/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x800) && sound.find("activate/monster/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x1000) && sound.find("activate/field/") != std::string::npos)
				list.push_back(sound);
			if((extra & 0x4000) && sound.find("activate/action/") != std::string::npos)
				list.push_back(sound);
		} else if(chant == CHANT::PENDULUM) {
			if((extra & 0x1) && sound.find("pendulum/activate/") != std::string::npos)
				list.push_back(sound);
		} else if(chant == CHANT::DRAW) {
			if((extra & 0x1) && sound.find("draw/disadvantage/") != std::string::npos)
				list.push_back(sound);
			else if((extra & 0x2) && sound.find("draw/advantage/") != std::string::npos)
				list.push_back(sound);
			else if(sound.find("draw/") != std::string::npos && sound.find("draw/disadvantage/") == std::string::npos && sound.find("draw/advantage/") == std::string::npos)
				list.push_back(sound);
		} else if(chant == CHANT::SET) {
			if((extra & 0x1) && sound.find("set/monster/") != std::string::npos)
				list.push_back(sound);
			else if(sound.find("set/") != std::string::npos && sound.find("set/monster/") == std::string::npos)
				list.push_back(sound);
		} else if(chant == CHANT::DAMAGE) {
			if((extra & 0x1) && sound.find("damage/cost/") != std::string::npos)
				list.push_back(sound);
			else if((extra & 0x2) && sound.find("damage/minor/") != std::string::npos)
				list.push_back(sound);
			else if((extra & 0x4) && sound.find("damage/major/") != std::string::npos)
				list.push_back(sound);
			else if(sound.find("damage/") != std::string::npos && sound.find("damage/cost/") == std::string::npos && sound.find("damage/minor/") == std::string::npos && sound.find("damage/major/") == std::string::npos)
				list.push_back(sound);
		}
	}
}
void SoundManager::AddtoZipChantList(std::string file, int i, std::vector<std::string>& list, std::vector<std::string>& list2) {
#ifdef BACKEND
	for(auto& archive : Utils::archives) {
		if(Utils::ToUTF8IfNeeded({ archive.archive->getArchiveName().c_str(), archive.archive->getArchiveName().size() }).find("/expansions/kcgchant.zip") == std::string::npos)
			continue;
		for(auto& file : Utils::FindFileNames(archive.archive, Utils::ToPathString(file), mixer->GetSupportedSoundExtensions(), 1)) {
			auto scode = Utils::GetFileName(file);
			auto filename = Utils::ToUTF8IfNeeded(scode);
			if(!(filename.find("+") != std::string::npos || filename.find("-") != std::string::npos || filename.find("_") != std::string::npos || filename.find(".") != std::string::npos))
				list.push_back(Utils::ToUTF8IfNeeded(file));
			else if(!(filename.find("-") != std::string::npos || filename.find("_") != std::string::npos || filename.find(".") != std::string::npos))
				list2.push_back(Utils::ToUTF8IfNeeded(file));
		}
		for(int j = 1; j < 9; j++) {
			auto files = Utils::FindFileNames(archive.archive, epro::format(EPRO_TEXT("{}_{}"), Utils::ToPathString(file), j), mixer->GetSupportedSoundExtensions(), 1);
			bool chkexist = false;
			for(auto& file : files) {
				auto scode = Utils::GetFileName(file);
				auto filename = Utils::ToUTF8IfNeeded(scode);
                if(!(filename.find("+") != std::string::npos || filename.find("-") != std::string::npos || filename.find(".") != std::string::npos)) {
					chkexist = true;
					list.push_back(Utils::ToUTF8IfNeeded(file));
				}
				if(chkexist) {
					for(int k = 1; k < 9; k++) {
						if(filename.find(epro::format("_{}+{}", j, k)) != std::string::npos && filename.find("-") == std::string::npos && filename.find(".") == std::string::npos)
							list2.push_back(Utils::ToUTF8IfNeeded(file));
					}
				}
			}
		}
	}
#endif
}
void SoundManager::AddtoChantList(std::string file, int i, std::vector<std::string>& list, std::vector<std::string>& list2) {
#ifdef BACKEND
	for(const auto& ext : mixer->GetSupportedSoundExtensions()) {
		const auto filename = epro::format("{}.{}", file, Utils::ToUTF8IfNeeded(ext));
		if(Utils::FileExists(Utils::ToPathString(filename))) {
			list.push_back(filename);
			for(int j = 1; j < 6; j++) {
				const auto filename2 = epro::format("{}+{}.{}", file, j, Utils::ToUTF8IfNeeded(ext));
				if(Utils::FileExists(Utils::ToPathString(filename2)))
					list2.push_back(filename2);
			}
		}
		for(int j = 1; j < 9; j++) {
			const auto filename = epro::format("{}_{}.{}", file, i, Utils::ToUTF8IfNeeded(ext));
			if(Utils::FileExists(Utils::ToPathString(filename))) {
				list.push_back(filename);
				for(int k = 1; k < 9; k++) {
					const auto filename2 = epro::format("{}_{}+{}.{}", file, j, k, Utils::ToUTF8IfNeeded(ext));
					if(Utils::FileExists(Utils::ToPathString(filename2)))
						list2.push_back(filename2);
				}
			}
		}
	}
#endif
}
bool SoundManager::PlayZipChants(CHANT chant, std::string file, std::vector<std::string>& sound, uint8_t player) {
#ifdef BACKEND
	for(auto& archive : Utils::archives) {
		if(Utils::ToUTF8IfNeeded({ archive.archive->getArchiveName().c_str(), archive.archive->getArchiveName().size() }).find("/expansions/kcgchant.zip") == std::string::npos)
			continue;
		//std::lock_guard<epro::mutex> guard(*archive.mutex);
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
				StopSounds();
				//record this chant as played
				if(mainGame->dInfo.isInDuel && chant != CHANT::DRAW && chant != CHANT::STARTUP && chant != CHANT::WIN && chant != CHANT::LOSE) {
					if(std::find(sound.begin(), sound.end(), file) != sound.end()) {
						reader->drop();
						delete[] buff;
						return false;
					}
					sound.push_back(file);
				}
				if (mainGame->soundBuffer.loadFromMemory(buff, length)) {
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
		if(mainGame->dInfo.isInDuel && chant != CHANT::DRAW && chant != CHANT::STARTUP && chant != CHANT::WIN && chant != CHANT::LOSE) {
			if(std::find(sound.begin(), sound.end(), file) != sound.end())
				return false;
			sound.push_back(file);
		}
		StopSounds();
		if (mainGame->soundBuffer.loadFromFile(file)) {
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
			return true;
		}
	}
#endif
	return false;
}
//bool SoundManager::PlayChant(CHANT chant, uint32_t code) {
bool SoundManager::PlayChant(CHANT chant, uint32_t code, uint32_t code2, uint8_t player, uint16_t extra) {
///////kdiy//////
#ifdef BACKEND
	if(!soundsEnabled) return false;
	///////kdiy//////
// 	auto key = std::make_pair(chant, code);
// 	auto chant_it = ChantsList.find(key);
// 	if(chant_it == ChantsList.end())
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
	if(code == 0) {
        std::vector<std::string> chantlist = ChantSPList[i][character[player]];
        std::vector<std::string> chantlist2 = ChantSPList2[i][character[player]];
        //not play again for same chant
        if(mainGame->dInfo.isInDuel && chant != CHANT::DRAW && chant != CHANT::STARTUP && chant != CHANT::WIN && chant != CHANT::LOSE) {
            for(auto file : gSoundManager->soundcount) {
                chantlist.erase(std::remove(chantlist.begin(), chantlist.end(), file), chantlist.end());
                chantlist2.erase(std::remove(chantlist2.begin(), chantlist2.end(), file), chantlist2.end());
            }
        }
		int count = (int)chantlist.size();
		int _count = (int)chantlist2.size();
		if(count > 0) {
			int chantno = (std::uniform_int_distribution<>(0, count - 1))(rnd);
			return PlayZipChants(chant, chantlist[chantno], gSoundManager->soundcount, player);
		} else if(_count > 0) {
			int chantno = (std::uniform_int_distribution<>(0, _count - 1))(rnd);
			return PlayChants(chant, chantlist2[chantno], gSoundManager->soundcount, player);
		}
	} else {
		auto key = std::make_pair(chant, code);
		auto key2 = std::make_pair(chant, code2);
        std::vector<std::string> chantlist = ChantSPList[i][character[player]];
        std::vector<std::string> chantlist2 = ChantSPList2[i][character[player]];
		std::vector<std::string> list;
        std::vector<std::string> list2;
		auto chant_it = ChantsList[character[player]].find(key);
		auto chant_it2 = ChantsList[character[player]].find(key2);
		std::vector<std::string> _list;
        std::vector<std::string> _list2;
		auto _chant_it = ChantsList2[character[player]].find(key);
		auto _chant_it2 = ChantsList2[character[player]].find(key2);

		if(chant_it2 == ChantsList[character[player]].end() && _chant_it2 == ChantsList2[character[player]].end()) {
			if(chant_it == ChantsList[character[player]].end() && _chant_it == ChantsList2[character[player]].end()) {
                if(mainGame->dInfo.isInDuel) {
                    for(auto file : gSoundManager->soundcount) {
                        chantlist.erase(std::remove(chantlist.begin(), chantlist.end(), file), chantlist.end());
                        chantlist2.erase(std::remove(chantlist2.begin(), chantlist2.end(), file), chantlist2.end());
                    }
                }
                int count = chantlist.size();
                int _count = chantlist2.size();
				if(count > 0)
					AddtoChantSPList(chant, extra, chantlist, list);
				else if(_count > 0)
					AddtoChantSPList(chant, extra, chantlist2, _list);
			} else {
				if(chant_it != ChantsList[character[player]].end())
					AddtoZipChantList(chant_it->second, i, list, list2);
				else if(_chant_it != ChantsList2[character[player]].end())
					AddtoChantList(_chant_it->second, i, _list, _list2);
			}
		} else {
			if(chant_it2 != ChantsList[character[player]].end())
				AddtoZipChantList(chant_it2->second, i, list, list2);
			else if(_chant_it2 != ChantsList2[character[player]].end())
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
			if(PlayChants(chant, _list[soundno], gSoundManager->soundcount, player)) {
				int _count22 = _list2.size();
				if(_count22 > 0) {
					for(int k = 0; k < count2; k++) {
						const auto filename = Utils::GetFileName(_list2[k]).substr(0, Utils::GetFileName(_list2[k]).size() - 2);
						if(filename == Utils::GetFileName(_list[soundno]))
							PlayChants(chant, _list2[k], gSoundManager->soundcount, player);
					}
				}
				return true;
			}
		}
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
