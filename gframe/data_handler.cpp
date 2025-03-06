#include "data_handler.h"
#include <irrlicht.h>
#include "config.h"
#include "cli_args.h"
#include "utils_gui.h"
#include "deck_manager.h"
#include "logging.h"
#include "fmt.h"
#include "utils.h"
#include "windbot.h"
#include "windbot_panel.h"
#if IRRLICHT_VERSION_MAJOR==1 && IRRLICHT_VERSION_MINOR==9
#include "IrrlichtCommonIncludes1.9/CFileSystem.h"
#else
#include "IrrlichtCommonIncludes/CFileSystem.h"
#endif
#include "porting.h"
#if EDOPRO_ANDROID
#include "Android/COSAndroidOperator.h"
#endif
#if EDOPRO_IOS
#include "iOS/COSiOSOperator.h"
#endif

namespace ygo {

void DataHandler::LoadDatabases() {
	/////kdiy///////////
	// if(Utils::FileExists(EPRO_TEXT("./cards.cdb"))) {
	// 	if(dataManager->LoadDB(EPRO_TEXT("./cards.cdb")))
	// 		WindBot::AddDatabase(EPRO_TEXT("./cards.cdb"));
	// }
	/////kdiy///////////
	for(auto& file : Utils::FindFiles(EPRO_TEXT("./expansions/"), { EPRO_TEXT("cdb") }, 2)) {
		epro::path_string db = EPRO_TEXT("./expansions/") + file;
		if(dataManager->LoadDB(db))
			WindBot::AddDatabase(db);
	}
	LoadArchivesDB();
}
void DataHandler::LoadArchivesDB() {
	for(auto& archive : Utils::archives) {
		std::lock_guard<epro::mutex> guard(*archive.mutex);
		auto files = Utils::FindFiles(archive.archive, EPRO_TEXT(""), { EPRO_TEXT("cdb") }, 3);
		for(auto& index : files) {
			auto reader = archive.archive->createAndOpenFile(index);
			if(reader == nullptr)
				continue;
			dataManager->LoadDB(reader);
			reader->drop();
		}
	}
}

void DataHandler::LoadPicUrls() {
	for(auto& _config : { &configs->user_configs, &configs->configs }) {
		const auto& config = *_config;
		auto it = config.find("urls");
		if(it != config.end() && it->is_array()) {
			for(const auto& obj : *it) {
				try {
					const auto& type = obj.at("type").get_ref<const std::string&>();
					const auto& url = obj.at("url").get_ref<const std::string&>();
					//kdiy//////
					if(url == "jp") {
#ifdef DEFAULT_JHDPIC_URL
                        imageDownloader->AddDownloadResource({ DEFAULT_JHDPIC_URL, imgType::ART, 1 });
#else
					continue;
#endif							
					} else if(type == "hdpic")
                        imageDownloader->AddDownloadResource({ url, imgType::ART, 1 });
                    else
					//kdiy//////
					if(url == "default") {
						if(type == "pic") {
#ifdef DEFAULT_PIC_URL
							imageDownloader->AddDownloadResource({ DEFAULT_PIC_URL, imgType::ART });
#else
							continue;
#endif
						} else if(type == "field") {
#ifdef DEFAULT_FIELD_URL
							imageDownloader->AddDownloadResource({ DEFAULT_FIELD_URL, imgType::FIELD });
#else
							continue;
#endif
						} else if(type == "cover") {
#ifdef DEFAULT_COVER_URL
							imageDownloader->AddDownloadResource({ DEFAULT_COVER_URL, imgType::COVER });
#else
							continue;
#endif
						}
						//kdiy//////
						else if(type == "closeup") {
#ifdef DEFAULT_CLOSEUP_URL
							imageDownloader->AddDownloadResource({ DEFAULT_CLOSEUP_URL, imgType::CLOSEUP });
#else
							continue;
#endif
						}
						//kdiy//////
					} else {
						imageDownloader->AddDownloadResource({ url, type == "field" ?
																imgType::FIELD : (type == "pic") ?
																//kdiy//////
																//imgType::ART : imgType::COVER });
																imgType::ART : (type == "closeup") ?
																imgType::CLOSEUP :  imgType::COVER});
																//kdiy//////
					}
				}
				catch(const std::exception& e) {
					ErrorLog("Exception occurred: {}", e.what());
				}
			}
		}
	}
}
void DataHandler::LoadZipArchives() {
	irr::io::IFileArchive* tmp_archive = nullptr;
	for(auto& file : Utils::FindFiles(EPRO_TEXT("./expansions/"), { EPRO_TEXT("zip") })) {
		////////kdiy////////
		//filesystem->addFileArchive(epro::format(EPRO_TEXT("./expansions/{}"), file).data(), true, false, irr::io::EFAT_ZIP, "", &tmp_archive);
#if defined(Zip)
		filesystem->addFileArchive(epro::format(EPRO_TEXT("./expansions/{}"), file).data(), false, false, irr::io::EFAT_ZIP, Zip, &tmp_archive);
#else
		filesystem->addFileArchive(epro::format(EPRO_TEXT("./expansions/{}"), file).data(), false, false, irr::io::EFAT_ZIP, "", &tmp_archive);
#endif
		////////kdiy////////
		if(tmp_archive) {
			Utils::archives.emplace_back(tmp_archive);
		}
	}
	for(auto& file : Utils::FindFiles(EPRO_TEXT("./sound/character/"), { EPRO_TEXT("zip") })) {
#if defined(Zip)
		filesystem->addFileArchive(epro::format(EPRO_TEXT("./sound/character/{}"), file).data(), false, false, irr::io::EFAT_ZIP, Zip, &tmp_archive);
#else
		filesystem->addFileArchive(epro::format(EPRO_TEXT("./sound/character/{}"), file).data(), false, false, irr::io::EFAT_ZIP, "", &tmp_archive);
#endif
		if(tmp_archive) {
			Utils::archives.emplace_back(tmp_archive);
		}
	}
}
////////kdiy////////
void DataHandler::LoadKZipArchives() {
	irr::io::IFileArchive* tmp_archive = nullptr;
	for(auto& file : Utils::FindFiles(EPRO_TEXT("./repositories/"), { EPRO_TEXT("zip") })) {
#if defined(Zip)
		filesystem->addFileArchive(epro::format(EPRO_TEXT("./repositories/{}"), file).data(), false, false, irr::io::EFAT_ZIP, Zip, &tmp_archive);
#else
		filesystem->addFileArchive(epro::format(EPRO_TEXT("./repositories/{}"), file).data(), false, false, irr::io::EFAT_ZIP, "", &tmp_archive);
#endif
		if(tmp_archive) {
			Utils::archives.emplace_back(tmp_archive);
		}
	}
}
////////kdiy////////
DataHandler::DataHandler() {
	configs = std::make_unique<GameConfig>();
	gGameConfig = configs.get();
#if !EDOPRO_ANDROID
	tmp_device = GUIUtils::CreateDevice(configs.get());
#endif
#if EDOPRO_IOS
	if(tmp_device->getVideoDriver())
		porting::exposed_data = &tmp_device->getVideoDriver()->getExposedVideoData();
	Utils::OSOperator = new irr::COSiOSOperator();
	configs->ssl_certificate_path = epro::format("{}/cacert.pem", Utils::GetWorkingDirectory());
#elif EDOPRO_ANDROID
	Utils::OSOperator = new irr::COSAndroidOperator();
	configs->ssl_certificate_path = epro::format("{}/cacert.pem", porting::internal_storage);
#else
	Utils::OSOperator = tmp_device->getGUIEnvironment()->getOSOperator();
	Utils::OSOperator->grab();
	if(configs->override_ssl_certificate_path.size()) {
		if(configs->override_ssl_certificate_path != "none" && Utils::FileExists(Utils::ToPathString(configs->override_ssl_certificate_path)))
			configs->ssl_certificate_path = configs->override_ssl_certificate_path;
	} else
		configs->ssl_certificate_path = epro::format("{}/cacert.pem", Utils::ToUTF8IfNeeded(Utils::GetWorkingDirectory()));
#endif
	filesystem = new irr::io::CFileSystem();
	dataManager = std::make_unique<DataManager>();
	auto strings_loaded = dataManager->LoadStrings(EPRO_TEXT("./config/strings.conf"));
	strings_loaded = dataManager->LoadStrings(EPRO_TEXT("./expansions/strings.conf")) || strings_loaded;
	if(!strings_loaded)
		throw std::runtime_error("Failed to load strings!");
	Utils::filesystem = filesystem;
	LoadZipArchives();
	deckManager = std::make_unique<DeckManager>();
	gitManager = std::make_unique<RepoManager>();
	sounds = std::make_unique<SoundManager>(configs->soundVolume / 100.0, configs->musicVolume / 100.0, configs->enablesound, configs->enablemusic);
	gitManager->ToggleReadOnly(cli_args[REPOS_READ_ONLY].enabled);
	////kdiy//////////
	if(Utils::FileExists(EPRO_TEXT("./config/user_configs.json")))
	////kdiy//////////
	gitManager->LoadRepositoriesFromJson(configs->user_configs);
	////kdiy//////////
	else
	////kdiy//////////
	gitManager->LoadRepositoriesFromJson(configs->configs);
	if(gitManager->TerminateIfNothingLoaded())
		deckManager->StopDummyLoading();
	////kdiy//////////
	LoadKZipArchives();
	////kdiy//////////
	imageDownloader = std::make_unique<ImageDownloader>();
	LoadDatabases();
	LoadPicUrls();
	deckManager->LoadLFList();
	dataManager->LoadIdsMapping(EPRO_TEXT("./config/mappings.json"));
	WindBotPanel::absolute_deck_path = Utils::ToUnicodeIfNeeded(Utils::GetAbsolutePath(EPRO_TEXT("./deck")));
}
DataHandler::~DataHandler() {
	if(filesystem)
		filesystem->drop();
	if(Utils::OSOperator)
		Utils::OSOperator->drop();
}

}
