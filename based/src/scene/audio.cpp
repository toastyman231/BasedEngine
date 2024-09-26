#include "based/scene/audio.h"
#include "log.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "based/core/profiler.h"

namespace based::scene
{
	static ma_engine* mEngine = (ma_engine*) malloc(sizeof(*mEngine));

	class AudioImpl
	{
	private:
		ma_sound mSound;
		float mVolume;
		bool mLoops;
		bool mMuted;

	public:
		static void AudioImpl::InitEngine()
		{
			PROFILE_FUNCTION();
			ma_result result = ma_engine_init(NULL, mEngine);
			if (result != MA_SUCCESS)
			{
				BASED_WARN("Warning: Audio engine not initialized properly! Error: {}", result);
			}
		}

		static void AudioImpl::ShutdownEngine()
		{
			PROFILE_FUNCTION();
			ma_engine_uninit(mEngine);
			free(mEngine);
		}

		static void AudioImpl::PlayAudio(std::string& path, float volume)
		{
			PROFILE_FUNCTION();
			ma_engine_set_volume(mEngine, volume);
			ma_engine_play_sound(mEngine, path.c_str(), NULL);
		}

		AudioImpl(std::string& path, float volume, bool loops) : mVolume(volume), mLoops(loops), mMuted(false)
		{
			PROFILE_FUNCTION();
			ma_result result = ma_sound_init_from_file(mEngine, path.c_str(), 0, NULL, NULL, &mSound);
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

		bool AudioImpl::IsPlaying() const
		{
			return ma_sound_is_playing(&mSound);
		}

		void AudioImpl::SetVolume(float volume)
		{
			if (volume <= 0.f)
			{
				mMuted = true;
			}

			ma_sound_set_volume(&mSound, volume);
			mVolume = volume;
		}

		void AudioImpl::SetLooping(bool loop)
		{
			mLoops = loop;
			ma_sound_set_looping(&mSound, mLoops);
			//ma_data_source_set_looping(&mDecoder, mLoops);
		}

		void AudioImpl::ToggleMute()
		{
			mMuted = !mMuted;
			if (mMuted) SetVolume(0.f);
		}

		void AudioImpl::Play()
		{
			if (ma_sound_start(&mSound) != MA_SUCCESS)
			{
				BASED_WARN("Warning: Could not play sound!");
			}
		}

		void AudioImpl::Pause()
		{
			if (ma_sound_stop(&mSound) != MA_SUCCESS)
			{
				BASED_WARN("Warning: Could not stop playing sound!");
			}
		}

		void AudioImpl::Stop()
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

	void Audio::SetVolume(float volume)
	{
		pimpl->SetVolume(volume);
	}

	void Audio::SetLooping(bool loop)
	{
		pimpl->SetLooping(loop);
	}

	void Audio::ToggleMute()
	{
		pimpl->ToggleMute();
	}

	void Audio::Play()
	{
		pimpl->Play();
	}

	void Audio::Pause()
	{
		pimpl->Pause();
	}

	void Audio::Stop()
	{
		pimpl->Stop();
	}

}