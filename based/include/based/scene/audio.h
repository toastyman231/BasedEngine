#pragma once

#include <string>

namespace based::scene
{
	class AudioImpl;

	class Audio
	{
	public:
		static void InitEngine();

		static void ShutdownEngine();

		static void PlayAudio(std::string& path, float volume = 1.f);

		Audio(std::string& path, float volume, bool loops = false);

		~Audio();

		bool IsPlaying() const;

		void SetVolume(float volume);

		void SetLooping(bool loop);

		void ToggleMute();

		void Play();

		void Pause();

		void Stop();
	private:
		based::scene::AudioImpl* pimpl;
		//ma_sound* mSound = nullptr;
		/*float mVolume;
		bool mLoops;
		bool mMuted;*/
	};
}