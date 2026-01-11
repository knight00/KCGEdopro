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
	epro::path_string ImageFolder[20];
	std::vector<epro::path_string> ImageList[36+CHARACTER_VOICE+CHARACTER_VOICE-2];
	int saved_image_id[36+CHARACTER_VOICE+CHARACTER_VOICE-2];
	//random image
	void GetRandomImage(irr::video::ITexture*& src, int image_type, bool force_random=false);
	void GetRandomImage(irr::video::ITexture*& src, int image_type, int width, int height, bool force_random=false);
	void GetRandomImagef(int width, int height);
	void GetRandomCharacter(irr::video::ITexture*& src, std::vector<epro::path_string>& list);
	void GetRandomVWallpaper();
	void LoadCharacter(int player, int subcharacter);
	void RefreshRandomImageList();
	void RefreshImageDir(epro::path_string path, int image_type);
	void RefreshImageDirf();
	irr::video::ITexture* UpdatetTexture(int i, std::wstring filepath);
	irr::core::rect<irr::s32> head_size[CHARACTER_STORY+1]; //story icon
	irr::core::rect<irr::s32> modehead_size[6];
	irr::core::rect<irr::s32> cutincharacter_size[CHARACTER_VOICE][3];
	std::vector<epro::path_string> cutincharacter[CHARACTER_VOICE][6][3]; //cutin path, 1st: character(0=no character), 2nd: subcharacter, 3rd: emotion(0=damage, 1=advan, 2=surprise)
	std::vector<epro::path_string> imgcharacter[CHARACTER_VOICE][6][3]; //bodycharacter path, 1st: character(0=no character), 2nd: subcharacter, 3rd: emotion(0=normal, 1=damage, 2=advan)
	bool GetTextureCardHD(uint32_t code);
	std::tuple<irr::video::ITexture*, irr::video::SColor> GetTextureCloseup(uint32_t code, uint32_t alias = 0, bool is_closeup=false);
	std::tuple<irr::video::ITexture*, irr::video::SColor> GetTextureCloseupCode(uint32_t code, bool is_closeup=false);
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
#define TEXTURE_MENU				0
#define TEXTURE_BACKGROUND		    1
#define TEXTURE_DECK				2
#define TEXTURE_COVERS				3
#define TEXTURE_COVERS2				4
#define TEXTURE_COVERS3				5
#define TEXTURE_COVERS4				6
#define TEXTURE_UNKNOWN             7
#define TEXTURE_ATTACK				8
#define TEXTURE_ACTIVATE			9
#define TEXTURE_CHAIN			    10
#define TEXTURE_NEGATED			    11
#define TEXTURE_MASK		        12
#define TEXTURE_EQUIP		        13
#define TEXTURE_TARGET		        14
#define TEXTURE_CHAINTARGET		    15
#define TEXTURE_LIM                 16
#define TEXTURE_OT                  17
#define TEXTURE_SETTING             18
#define TEXTURE_F1		            19
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
	A(tLPBar)
	A(tLPFrame)
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
	A(tLPFrame_dm)
	A(tLPFrame_dsod)
	A(tLPFrame_gx)
	A(tLPFrame_5ds)
	A(tLPFrame_z4)
	A(tLPFrame2_z4)
	A(tLPFrame_a5)
	A(tLPFrame2_a5)
	A(tLPFrame_v6)
	A(tRScale[14])
	A(tLScale[14])
	A(icon[CHARACTER_VOICE])
	A(vs[CHARACTER_VOICE])
	A(name[CHARACTER_VOICE])
	A(lpicon[CHARACTER_VOICE])
	A(bodycharacter[CHARACTER_VOICE][3])
	A(cutin[CHARACTER_VOICE][3])
	A(cardchant0)
	A(cardchant1)
	A(cardchant2)
	A(cardchant00)
	A(cardchant01)
	A(cardchant02)
	A(tcharacterselect)
	A(tcharacterselect2)
	A(tsubcharacterselect)
	A(tsubcharacterselect2)
	A(tsubcharacterselect3)
	A(tsubcharacterselect4)
	A(tsubcharacterselect5)
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
