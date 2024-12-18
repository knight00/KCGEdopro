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
	irr::core::rect<irr::s32> head_size[CHARACTER_STORY+1]; //story icon dimension
	irr::core::rect<irr::s32> modehead_size[6];
	irr::core::rect<irr::s32> icon_size[CHARACTER_VOICE];
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
	bool GetTextureCardHD(uint32_t code);
	std::tuple<irr::video::ITexture*, irr::video::SColor> GetTextureCloseup(uint32_t code, uint32_t alias = 0, bool is_closeup=false);
	std::tuple<irr::video::ITexture*, irr::video::SColor> GetTextureCloseupCode(uint32_t code, bool is_closeup=false);
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
#define TEXTURE_COVERS2				3
#define TEXTURE_ATTACK				4
#define TEXTURE_ACTIVATE			5
#define TEXTURE_CHAIN			    6
#define TEXTURE_NEGATED			    7
#define TEXTURE_MASK		        8
#define TEXTURE_EQUIP		        9
#define TEXTURE_TARGET		        10
#define TEXTURE_CHAINTARGET		    11
#define TEXTURE_F1		            12
#define TEXTURE_F2		            13
#define TEXTURE_F3		            14
#define TEXTURE_BACKGROUND		    15
#define TEXTURE_BACKGROUND_MENU		16
#define TEXTURE_BACKGROUND_DECK		17
#define TEXTURE_field2		        18
#define TEXTURE_field_transparent2	19
#define TEXTURE_field3		        20
#define TEXTURE_field_transparent3	21
#define TEXTURE_field		        22
#define TEXTURE_field_transparent	23
#define TEXTURE_field4		        24
#define TEXTURE_field_transparent4	25
#define TEXTURE_field_fieldSP2	    26
#define TEXTURE_field_transparentSP2 27
#define TEXTURE_fieldSP3            28
#define TEXTURE_field_transparentSP3 29
#define TEXTURE_fieldSP             30
#define TEXTURE_field_transparentSP 31
#define TEXTURE_fieldSP4            32
#define TEXTURE_field_transparentSP4 33
#define TEXTURE_UNKNOWN             34
#define TEXTURE_LIM                 35
#define TEXTURE_OT                  36
#define TEXTURE_COVERS3				37
#define TEXTURE_COVERS4				38
#define TEXTURE_SETTING             39

	std::vector<uint8_t> imgcharacter;
	std::unordered_map<uint32_t, irr::video::ITexture*> tCloseup;
    std::unordered_map<uint32_t, irr::video::SColor> tCloseupcolor;
	/////////kdiy////
	irr::IrrlichtDevice* device;
	irr::video::IVideoDriver* driver;
public:
    /////////kdiy////
	// irr::video::ITexture* tCover[2];
	irr::video::ITexture* tCover[4];
    /////////kdiy////
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
	A(tLPFrame_z4)
	A(tLPFrame2_z4)
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
	A(QQ)
	A(modeBody[CHAPTER])
    A(head[CHARACTER_STORY+1]) //story icon
	A(modeHead[6])
	A(tXyz)
	A(tCXyz)
	A(tShield)
	A(tCrack)
	A(tLevel)
	A(tRank)
    A(tLvRank)
	A(tLink)
	A(tMain)
	A(tDeck)
	A(tGrave)
	A(tRemoved)
	A(tOnHand)
	A(tExtra)
	A(tExit)
	A(tRestart)
	A(tButton)
	A(tButtonpress)
	A(tButton2)
	A(tStartReplay)
	A(tPauseReplay)
	A(tNextReplay)
	A(tLastReplay)
	A(tReplaySwap)
	A(tTimer)
	A(tHint)
	A(tTick)
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
