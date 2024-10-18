#pragma once

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

		void SetVolume(float volume) const;

		void SetLooping(bool loop) const;

		void ToggleMute() const;

		void Play() const;

		void Pause() const;

		void Stop() const;
	private:
		based::scene::AudioImpl* pimpl;
	};
}