#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <unordered_map>
#include <cstdint>
#include <memory>
#include "common.h"
#include "text_types.h"
#include "utils.h"
/////kdiy/////
#include "client_card.h"
/////kdiy/////

struct sqlite3;
struct sqlite3_stmt;
struct sqlite3_vfs;

namespace irr {
namespace io {
class IReadFile;
}
}

#define SCOPE_OCG        0x1
#define SCOPE_TCG        0x2
#define SCOPE_ANIME      0x4
#define SCOPE_ILLEGAL    0x8
#define SCOPE_VIDEO_GAME 0x10
#define SCOPE_CUSTOM     0x20
#define SCOPE_SPEED      0x40
#define SCOPE_PRERELEASE 0x100
#define SCOPE_RUSH       0x200
#define SCOPE_LEGEND     0x400
#define SCOPE_HIDDEN     0x1000
/////zdiy/////
#define SCOPE_ZCG        0x2000
/////zdiy/////

#define SCOPE_OCG_TCG    (SCOPE_OCG | SCOPE_TCG)
#define SCOPE_OFFICIAL   (SCOPE_OCG | SCOPE_TCG | SCOPE_PRERELEASE)

#define TYPE_SKILL       0x8000000
#define TYPE_ACTION      0x10000000

struct OCG_CardData;

namespace ygo {

struct CardData {
	uint32_t code;
	uint32_t alias;
	uint16_t* setcodes;
    uint32_t type;
	//////kdiy///////
	//uint32_t level;
	int32_t level;
	//////kdiy///////
	uint32_t attribute;
	uint64_t race;
	int32_t attack;
	int32_t defense;
	uint32_t lscale;
	uint32_t rscale;
	uint32_t link_marker;
	//////kdiy///////
	uint32_t ot;
	//////kdiy///////
};
struct CardDataC {
	uint32_t code;
	uint32_t alias;
	uint16_t* setcodes_p;
    uint32_t type;
	//////kdiy///////
	//uint32_t level;
	int32_t level;
	//////kdiy///////
	uint32_t attribute;
	uint64_t race;
	int32_t attack;
	int32_t defense;
	uint32_t lscale;
	uint32_t rscale;
	uint32_t link_marker;
	uint32_t ot;
	uint32_t category;
	std::vector<uint16_t> setcodes;

	static constexpr auto CARD_ARTWORK_VERSIONS_OFFSET = 10;

	bool IsInArtworkOffsetRange() const {
		return IsInArtworkOffsetRange(this);
	}

	template<typename T>
	static bool IsInArtworkOffsetRange(const T* pcard) {
		if(pcard->alias == 0)
			return false;
		return (pcard->alias - pcard->code < CARD_ARTWORK_VERSIONS_OFFSET || pcard->code - pcard->alias < CARD_ARTWORK_VERSIONS_OFFSET);
	}

	uint32_t getRealCode() const {
		// dummy entries have a code of 0 with the alias corresponding to the actual code
		return code ? code : alias;
	}

	bool isRitualMonster() const {
		return (type & (TYPE_MONSTER | TYPE_RITUAL)) == (TYPE_MONSTER | TYPE_RITUAL);
	}

	bool isRush() const {
		return ot & SCOPE_RUSH;
	}
};
struct CardString {
	std::wstring name;
	std::wstring text;
	std::wstring uppercase_name;
	std::wstring uppercase_text;
	std::wstring desc[16];
};

class CardDataM {
public:
	CardDataC _data{};
	const CardString& GetStrings() const {
		if(_locale_strings)
			return *_locale_strings;
		return _strings;
	}
	CardDataM(){}
	CardDataM(CardDataC&& data, CardString&& strings, CardString* locale_strings = nullptr):
		_data(std::move(data)), _strings(std::move(strings)), _locale_strings(locale_strings){}
	CardString _strings{};
	CardString* _locale_strings = nullptr;
};

class DataManager {
public:
	DataManager();
	~DataManager();
	void ClearLocaleTexts();
	inline bool LoadLocaleDB(const epro::path_string& file) {
		return ParseLocaleDB(OpenDb(file));
	}
	inline bool LoadDB(epro::path_stringview file) {
		return ParseDB(OpenDb(file));
	}
	inline bool LoadDB(irr::io::IReadFile* reader) {
		return ParseDB(OpenDb(reader));
	}
	bool LoadStrings(const epro::path_string& file);
	bool LoadLocaleStrings(const epro::path_string& file);
	bool LoadIdsMapping(const epro::path_string& file);
	void ClearLocaleStrings();
	const CardDataC* GetCardData(uint32_t code) const;
	const CardDataC* GetMappedCardData(uint32_t code) const;
	/////kdiy/////
	//epro::wstringview GetName(uint32_t code) const;
	std::wstring GetOriginalName(uint32_t code, bool hidenametag=false) const;
	std::wstring GetName(uint32_t code, bool hidenametag=false) const;
	std::wstring GetName(ClientCard* pcard, bool hidenametag=false) const;
	std::wstring GetName(ClientCard* pcard, uint32_t code, bool hidenametag=false) const;
	/////kdiy/////
	epro::wstringview GetText(uint32_t code) const;
	epro::wstringview GetUppercaseName(uint32_t code) const;
	epro::wstringview GetUppercaseText(uint32_t code) const;
	epro::wstringview GetDesc(uint64_t strCode, bool compat) const;
	inline epro::wstringview GetSysString(uint32_t code)  const {
		return _sysStrings.GetLocale(code);
	}
	inline bool HasSysString(uint32_t code)  const {
		return _sysStrings.HasLocale(code);
	}
	inline epro::wstringview GetVictoryString(uint32_t code)  const {
		return _victoryStrings.GetLocale(code);
	}
	inline epro::wstringview GetCounterName(uint32_t code)  const {
		return _counterStrings.GetLocale(code);
	}
	inline epro::wstringview GetSetName(uint32_t code)  const {
		return _setnameStrings.GetLocale(code, L"");
	}
	inline uint32_t GetRaceStringIndex(uint32_t race_idx)  const {
		if(race_idx < 30)
			return 1020 + race_idx;
		//strings 1050 above are already used, read the rest from this other range
		return (2500 - 30) + race_idx;
	}
	std::vector<uint16_t> GetSetCode(const std::vector<epro::wstringview>& setname) const;
	std::wstring GetNumString(size_t num, bool bracket = false) const;
	epro::wstringview FormatLocation(uint32_t location, int sequence) const;
	std::wstring FormatAttribute(uint32_t attribute) const;
	std::wstring FormatRace(uint64_t race, bool isSkill = false) const;
	std::wstring FormatType(uint32_t type) const;
	std::wstring FormatScope(uint32_t scope, bool hideOCGTCG = false) const;
	std::wstring FormatSetName(const std::vector<uint16_t>& setcodes) const;
	std::wstring FormatLinkMarker(uint32_t link_marker) const;
	std::unordered_map<uint32_t, CardDataM> cards;

	static constexpr auto unknown_string = L"???"sv;
	static void CardReader(void* payload, uint32_t code, OCG_CardData* data);
	static bool deck_sort_lv(const CardDataC* l1, const CardDataC* l2);
	static bool deck_sort_atk(const CardDataC* l1, const CardDataC* l2);
	static bool deck_sort_def(const CardDataC* l1, const CardDataC* l2);
	static bool deck_sort_name(const CardDataC* l1, const CardDataC* l2);
private:
	std::unique_ptr<sqlite3_vfs> irrvfs;
	template<typename T1, typename T2 = T1>
	using indexed_map = std::map<uint32_t, std::pair<T1, T2>>;

	class LocaleStringHelper {
	public:
		indexed_map<std::wstring> map{};
		epro::wstringview GetLocale(uint32_t code, epro::wstringview ret = DataManager::unknown_string) const {
			auto search = map.find(code);
			if(search == map.end() || search->second.first.empty())
				return ret;
			return search->second.second.size() ? search->second.second : search->second.first;
		}
		bool HasLocale(uint32_t code) const {
			auto search = map.find(code);
			return search != map.end() && !search->second.first.empty();
			if(search == map.end() || search->second.first.empty())
				return false;
			return true;
		}
		void ClearLocales() {
			for(auto& elem : map)
				elem.second.second.clear();
		}
		void SetMain(uint32_t code, std::wstring&& val) {
			map[code].first = std::move(val);
		}
		void SetLocale(uint32_t code, std::wstring&& val) {
			map[code].second = std::move(val);
		}
	};
	sqlite3* OpenDb(epro::path_stringview file);
	sqlite3* OpenDb(irr::io::IReadFile* reader);
	bool ParseDB(sqlite3* pDB);
	bool ParseLocaleDB(sqlite3* pDB);
	bool Error(sqlite3* pDB, sqlite3_stmt* pStmt = nullptr) const;
	std::unordered_map<uint32_t, CardString> locales;
	indexed_map<CardDataM*, CardString*> indexes;
	LocaleStringHelper _counterStrings;
	LocaleStringHelper _victoryStrings;
	LocaleStringHelper _setnameStrings;
	LocaleStringHelper _sysStrings;
	std::string cur_database;
	std::map<uint32_t, uint32_t> mapped_ids;
};

extern DataManager* gDataManager;

}

#endif // DATAMANAGER_H
