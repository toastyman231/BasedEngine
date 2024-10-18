#include "pch.h"
#include "based/scene/audio.h"

#define MINIAUDIO_IMPLEMENTATION
#include "engine.h"
#include "miniaudio.h"

#include "memory/arena.h"

namespace based::scene
{
	static ma_engine* mEngine;

	class AudioImpl
	{
	private:
		ma_sound mSound;
		float mVolume;
		bool mLoops;
		bool mMuted;

	public:
		static void InitEngine()
		{
			PROFILE_FUNCTION();
			mEngine = static_cast<ma_engine*>(ArenaAlloc(Engine::Instance().GetEngineArena(),sizeof(ma_engine)));
			ma_result result = ma_engine_init(nullptr, mEngine);
			if (result != MA_SUCCESS)
			{
				BASED_WARN("Warning: Audio engine not initialized properly! Error: {}", result);
			}
		}

		static void ShutdownEngine()
		{
			PROFILE_FUNCTION();
			ma_engine_uninit(mEngine);
			ArenaRelease(Engine::Instance().GetEngineArena(), mEngine, sizeof(ma_engine));
		}

		static void PlayAudio(std::string& path, float volume)
		{
			PROFILE_FUNCTION();
			ma_engine_set_volume(mEngine, volume);
			ma_engine_play_sound(mEngine, path.c_str(), nullptr);
		}

		AudioImpl(std::string& path, float volume, bool loops) : mVolume(volume), mLoops(loops), mMuted(false)
		{
			PROFILE_FUNCTION();
			ma_result result = ma_sound_init_from_file(mEngine, path.c_str(), 0, 
				nullptr, nullptr, &mSound);
			if (result != MA_SUCCESS)
			{
				BASED_WARN("Warning: Could not load audio from {}", path);
				return;
			}
			SetVolume(volume);
			SetLooping(loops);
		}

		~AudioImpl()
		{
			if (&mSound)
			{
				ma_sound_uninit(&mSound);
			}
		}

		bool IsPlaying() const
		{
			return ma_sound_is_playing(&mSound);
		}

		void SetVolume(float volume)
		{
			if (volume <= 0.f)
			{
				mMuted = true;
			}

			ma_sound_set_volume(&mSound, volume);
			mVolume = volume;
		}

		void SetLooping(bool loop)
		{
			mLoops = loop;
			ma_sound_set_looping(&mSound, mLoops);
		}

		void ToggleMute()
		{
			mMuted = !mMuted;
			if (mMuted) SetVolume(0.f);
		}

		void Play()
		{
			if (ma_sound_start(&mSound) != MA_SUCCESS)
			{
				BASED_WARN("Warning: Could not play sound!");
			}
		}

		void Pause()
		{
			if (ma_sound_stop(&mSound) != MA_SUCCESS)
			{
				BASED_WARN("Warning: Could not stop playing sound!");
			}
		}

		void Stop()
		{
			if (ma_sound_stop(&mSound) != MA_SUCCESS)
			{
				BASED_WARN("Warning: Could not stop playing sound!");
			}

			if (ma_sound_seek_to_pcm_frame(&mSound, 0) != MA_SUCCESS)
			{
				BASED_WARN("Warning: Could not seek to start of sound!");
			}
		}
	};

	void Audio::InitEngine()
	{
		AudioImpl::InitEngine();
	}

	void Audio::ShutdownEngine()
	{
		AudioImpl::ShutdownEngine();
	}

	void Audio::PlayAudio(std::string& path, float volume)
	{
		AudioImpl::PlayAudio(path, volume);
	}

	Audio::Audio(std::string& path, float volume, bool loops)
	{
		pimpl = new AudioImpl(path, volume, loops);
	}

	Audio::~Audio()
	{
		delete pimpl;
	}

	bool Audio::IsPlaying() const
	{
		return pimpl->IsPlaying();
	}

	void Audio::SetVolume(float volume) const
	{
		pimpl->SetVolume(volume);
	}

	void Audio::SetLooping(bool loop) const
	{
		pimpl->SetLooping(loop);
	}

	void Audio::ToggleMute() const
	{
		pimpl->ToggleMute();
	}

	void Audio::Play() const
	{
		pimpl->Play();
	}

	void Audio::Pause() const
	{
		pimpl->Pause();
	}

	void Audio::Stop() const
	{
		pimpl->Stop();
	}

}
