#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include "config.h"
#include <path.h>
#include <rect.h>
#include <unordered_map>
#include <map>
#include <atomic>
#include <queue>
#include "epro_mutex.h"
#include "epro_condition_variable.h"
#include "epro_thread.h"
///kdiy////////
#include "common.h"
///kdiy////////

namespace irr {
class IrrlichtDevice;
namespace io {
class IReadFile;
}
namespace video {
class IImage;
class ITexture;
class IVideoDriver;
class SColor;
}
}

namespace ygo {

#ifndef IMGTYPE
#define IMGTYPE
enum imgType {
	ART,
	FIELD,
	COVER,
	///kdiy///////////
	CLOSEUP,
	////kdiy///////////
	THUMB
};
#endif

class ImageManager {
private:
	using chrono_time = uint64_t;
	enum class preloadStatus {
		NONE,
		LOADING,
		LOADED,
		WAIT_DOWNLOAD,
	};
	struct texture_map_entry {
		preloadStatus preload_status;
		irr::video::ITexture* texture;
	};
	using texture_map = std::unordered_map<uint32_t, texture_map_entry>;
	struct load_parameter {
		uint32_t code;
		imgType type;
		size_t index;
		const std::atomic<irr::s32>& reference_width;
		const std::atomic<irr::s32>& reference_height;
		chrono_time timestamp;
		const std::atomic<chrono_time>& reference_timestamp;
		load_parameter(uint32_t code_, imgType type_, size_t index_, const std::atomic<irr::s32>& reference_width_,
					   const std::atomic<irr::s32>& reference_height_, chrono_time timestamp_, const std::atomic<chrono_time>& reference_timestamp_) :
			code(code_), type(type_), index(index_), reference_width(reference_width_),
			reference_height(reference_height_), timestamp(timestamp_),
			reference_timestamp(reference_timestamp_) {
		}
	};
	enum class loadStatus {
		LOAD_OK,
		LOAD_FAIL,
		WAIT_DOWNLOAD,
	};
	struct load_return {
		loadStatus status;
		uint32_t code;
		irr::video::IImage* texture;
		epro::path_string path;
	};
public:
	ImageManager();
	~ImageManager();
	bool Initial();
	/////kdiy/////
    void SetAvatar(int player, const wchar_t *avatar);
	std::vector<epro::path_string> ImageList[40+CHARACTER_VOICE+CHARACTER_VOICE-2];
	int saved_image_id[40+CHARACTER_VOICE+CHARACTER_VOICE-2];
	//random image
	void GetRandomImage(irr::video::ITexture*& src, int image_type, bool force_random=false);
	void GetRandomImage(irr::video::ITexture*& src, int image_type, int width, int height, bool force_random = false);
	void GetRandomImagef(int width, int height);
	void RefreshRandomImageList();
	void RefreshImageDir(epro::path_string path, int image_type);
	void RefreshImageDirf();
    void RefreshKCGImage();
	/////kdiy/////

	void ChangeTextures(epro::path_stringview path);
	void ResetTextures();
	void SetDevice(irr::IrrlichtDevice* dev);
	void ClearTexture(bool resize = false);
	void RefreshCachedTextures();
	void ClearCachedTextures();
	static bool imageScaleNNAA(irr::video::IImage* src, irr::video::IImage* dest, chrono_time timestamp_id, const std::atomic<chrono_time>& source_timestamp_id);
	irr::video::IImage* GetScaledImage(irr::video::IImage* srcimg, int width, int height, chrono_time timestamp_id, const std::atomic<chrono_time>& source_timestamp_id);
	irr::video::IImage* GetScaledImageFromFile(const irr::io::path& file, int width, int height);
	irr::video::ITexture* GetTextureFromFile(const irr::io::path& file, int width, int height);
	irr::video::ITexture* GetTextureCard(uint32_t code, imgType type, bool wait = false, bool fit = false, int* chk = nullptr);
	irr::video::ITexture* GetTextureField(uint32_t code);
	////////kdiy////
	irr::video::ITexture* GetTextureCloseup(uint32_t code);
	////////kdiy////
	irr::video::ITexture* GetCheckboxScaledTexture(float scale);
	irr::video::ITexture* guiScalingResizeCached(irr::video::ITexture* src, const irr::core::rect<irr::s32>& srcrect,
												 const irr::core::rect<irr::s32> &destrect);
	void draw2DImageFilterScaled(irr::video::ITexture* txr,
								 const irr::core::rect<irr::s32>& destrect, const irr::core::rect<irr::s32>& srcrect,
								 const irr::core::rect<irr::s32>* cliprect = nullptr, const irr::video::SColor* const colors = nullptr,
								 bool usealpha = false);
private:
	texture_map tMap[2];
	texture_map tThumb;
	std::unordered_map<uint32_t, irr::video::ITexture*> tFields;
	texture_map tCovers;
	/////////kdiy////
#define TEXTURE_DECK				0
#define TEXTURE_MENU				1
#define TEXTURE_COVERS				2
#define TEXTURE_COVERO				3
#define TEXTURE_ATTACK				4
#define TEXTURE_ACTIVATE			5
#define TEXTURE_CHAIN			    6
#define TEXTURE_NEGATED			    7
#define TEXTURE_LP		            8
#define TEXTURE_LPf		            9
#define TEXTURE_MASK		        10
#define TEXTURE_EQUIP		        11
#define TEXTURE_TARGET		        12
#define TEXTURE_CHAINTARGET		    13
#define TEXTURE_F1		            14
#define TEXTURE_F2		            15
#define TEXTURE_F3		            16
#define TEXTURE_BACKGROUND		    17
#define TEXTURE_BACKGROUND_MENU		18
#define TEXTURE_BACKGROUND_DECK		19
#define TEXTURE_field2		        20
#define TEXTURE_field_transparent2	21
#define TEXTURE_field3		        22
#define TEXTURE_field_transparent3	23
#define TEXTURE_field		        24
#define TEXTURE_field_transparent	25
#define TEXTURE_field4		        26
#define TEXTURE_field_transparent4	27
#define TEXTURE_field_fieldSP2	    28
#define TEXTURE_field_transparentSP2 29
#define TEXTURE_fieldSP3            30
#define TEXTURE_field_transparentSP3 31
#define TEXTURE_fieldSP             32
#define TEXTURE_field_transparentSP 33
#define TEXTURE_fieldSP4            34
#define TEXTURE_field_transparentSP4 35
#define TEXTURE_UNKNOWN             36
#define TEXTURE_LIM                 37
#define TEXTURE_OT                  38
#define TEXTURE_SETTING             39

#define TEXTURE_MUTO                40
#define TEXTURE_ATEM                41
#define TEXTURE_KAIBA               42
#define TEXTURE_JOEY                43
#define TEXTURE_MARIK               44
#define TEXTURE_DARTZ               45
#define TEXTURE_BAKURA              46
#define TEXTURE_AIGAMI              47
#define TEXTURE_JUDAI               48
#define TEXTURE_MANJOME             49
#define TEXTURE_KAISA               50
#define TEXTURE_PHORNIX             51
#define TEXTURE_JOHN                52
#define TEXTURE_YUBEL               53
#define TEXTURE_YUSEI               54
#define TEXTURE_JACK                55
#define TEXTURE_ARKI                56
#define TEXTURE_CROW                57
#define TEXTURE_KIRYU               58
#define TEXTURE_PARADOX             59
#define TEXTURE_ZONE                60
#define TEXTURE_YUMA                61
#define TEXTURE_SHARK               62
#define TEXTURE_KAITO               63
#define TEXTURE_IV                  64
#define TEXTURE_DONTHOUSAND         65
#define TEXTURE_YUYA                66
#define TEXTURE_DECLAN              67
#define TEXTURE_SHAY                68
#define TEXTURE_PLAYMAKER           69
#define TEXTURE_SOULBURNER          70
#define TEXTURE_BLUEANGEL           71
	std::unordered_map<uint32_t, irr::video::ITexture*> tCloseup;
	int imgcharacter[CHARACTER_VOICE-1] = {TEXTURE_MUTO,TEXTURE_ATEM,TEXTURE_KAIBA,TEXTURE_JOEY,TEXTURE_MARIK,TEXTURE_DARTZ,TEXTURE_BAKURA,TEXTURE_AIGAMI,TEXTURE_JUDAI,TEXTURE_MANJOME,TEXTURE_KAISA,TEXTURE_PHORNIX,TEXTURE_JOHN,TEXTURE_YUBEL,TEXTURE_YUSEI,TEXTURE_JACK,TEXTURE_ARKI,TEXTURE_CROW,TEXTURE_KIRYU,TEXTURE_PARADOX,TEXTURE_ZONE,TEXTURE_YUMA,TEXTURE_SHARK,TEXTURE_KAITO,TEXTURE_IV,TEXTURE_DONTHOUSAND,TEXTURE_YUYA,TEXTURE_DECLAN,TEXTURE_SHAY,TEXTURE_PLAYMAKER,TEXTURE_SOULBURNER,TEXTURE_BLUEANGEL};
	/////////kdiy////
	irr::IrrlichtDevice* device;
	irr::video::IVideoDriver* driver;
public:
	irr::video::ITexture* tCover[2];
	irr::video::ITexture* tUnknown;
#define A(what) \
		public: \
		irr::video::ITexture* what;\
		private: \
		irr::video::ITexture* def_##what;
	A(tAct)
	A(tAttack)
	A(tNegated)
	A(tChain)
	A(tNumber)
	A(tLPFrame)
	A(tLPBar)
	A(tMask)
	A(tEquip)
	A(tTarget)
	A(tChainTarget)
	A(tLim)
	A(tOT)
	A(tHand[3])
	A(tBackGround)
	A(tBackGround_menu)
	A(tBackGround_deck)
	A(tBackGround_duel_topdown)
	A(tField[2][4])
	A(tFieldTransparent[2][4])
	/////////kdiy////
	A(tRScale[14])
	A(tLScale[14])
	A(icon[CHARACTER_VOICE])
	A(character[CHARACTER_VOICE])
	A(characterd[CHARACTER_VOICE])
	A(cardchant0)
	A(cardchant1)
	A(cardchant2)
	A(cardchant00)
	A(cardchant01)
	A(cardchant02)
	A(tcharacterselect)
	A(tcharacterselect2)
	A(scharacter[6][2])
	A(avcharacter[2])
	A(QQ)
	A(modeBody[CHAPTER])
    A(head[CHARACTER_STORY])
	A(modeHead[6])
	/////kdiy/////
	A(tSettings)
	A(tCheckBox[3])
#undef A
private:
	void ClearFutureObjects();
	void RefreshCovers();
	void LoadPic();
	irr::video::ITexture* loadTextureFixedSize(epro::path_stringview texture_name, int width, int height);
	irr::video::ITexture* loadTextureAnySize(epro::path_stringview texture_name);
	void replaceTextureLoadingFixedSize(irr::video::ITexture*& texture, irr::video::ITexture* fallback, epro::path_stringview texture_name, int width, int height);
	void replaceTextureLoadingAnySize(irr::video::ITexture*& texture, irr::video::ITexture* fallback, epro::path_stringview texture_name);
	load_return LoadCardTexture(uint32_t code, imgType type, const std::atomic<irr::s32>& width, const std::atomic<irr::s32>& height, chrono_time timestamp_id, const std::atomic<chrono_time>& source_timestamp_id);
	epro::path_string textures_path;
	std::pair<std::atomic<irr::s32>, std::atomic<irr::s32>> sizes[3];
	std::atomic<chrono_time> timestamp_id;
	std::map<epro::path_string, irr::video::ITexture*> g_txrCache;
	std::map<irr::io::path, irr::video::IImage*> g_imgCache; //ITexture->getName returns a io::path
	epro::mutex obj_clear_lock;
	epro::thread obj_clear_thread;
	epro::condition_variable cv_clear;
	std::deque<load_return> to_clear;
	std::atomic<bool> stop_threads;
	epro::condition_variable cv_load;
	std::deque<load_parameter> to_load;
	std::deque<load_return> loaded_pics[4];
	epro::mutex pic_load;
	//bool stop_threads;
	std::vector<epro::thread> load_threads;
};

#define CARD_IMG_WIDTH		177
#define CARD_IMG_HEIGHT		254
#define CARD_IMG_WIDTH_F	177.0f
#define CARD_IMG_HEIGHT_F	254.0f
#define CARD_THUMB_WIDTH	44
#define CARD_THUMB_HEIGHT	64

}

#endif // IMAGEMANAGER_H
