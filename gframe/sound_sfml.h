#ifndef SOUND_SFML_H
#define SOUND_SFML_H
#include "sound_threaded_backend.h"
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <sfAudio/Music.hpp>

namespace sf {
	class Sound;
	class SoundBuffer;
}

class SoundSFMLBase final : public SoundBackend {
public:
	SoundSFMLBase();
	~SoundSFMLBase() override;
	void SetSoundVolume(double volume) override;
	void SetMusicVolume(double volume) override;
	bool PlayMusic(const std::string& name, bool loop) override;
	bool PlaySound(const std::string& name) override;
	void StopSounds() override;
	void StopMusic() override;
	void PauseMusic(bool pause) override;
	bool MusicPlaying() override;
	void Tick() override;
	/////kdiy///////
	bool PlaySound(char* buff, const std::string& filename, long length) override;
	int32_t GetSoundDuration(const std::string& name) override;
	int32_t GetSoundDuration(char* buff, const std::string& filename, long length) override;
	/////kdiy///////
private:
	std::string cur_music;
	sf::Music music;
	std::vector<std::unique_ptr<sf::Sound>> sounds;
	float music_volume, sound_volume;
	std::map<std::string, std::unique_ptr<sf::SoundBuffer>> buffers;
	const sf::SoundBuffer& LookupSound(const std::string& name);
	/////kdiy///////
	const sf::SoundBuffer& LookupSound(char* buff, const std::string& filename, long length);
	/////kdiy///////
};

using SoundSFML = SoundThreadedBackendHelper<SoundSFMLBase>;

#endif //SOUND_SFML_H
