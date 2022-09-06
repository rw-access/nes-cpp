#include <SDL2/SDL.h>
#include <gtest/gtest.h>
#include <math.h>

struct SineWave {
    double wavelength;
    double phase;
    double amplitude;
};

struct SineState {
    std::vector<SineWave> waves;
};

static void playSines(void *userdata, Uint8 *stream, int len) {
    auto sines = static_cast<SineState *>(userdata);


    for (int s = 0; s < len; s += 2) {
        // add each sine wave.
        double amplitude = 0.0;
        for (auto &wl: sines->waves) {
            amplitude += sin(wl.phase) * wl.amplitude;
            wl.phase += (M_PI * 2) / wl.wavelength;
        }

        int16_t signedAmplitude = (amplitude * 256);
        stream[s]               = signedAmplitude;
        stream[s + 1]           = signedAmplitude >> 8;
    }
}

TEST(AudioTest, TwoSecondsSineChord) {
    ASSERT_EQ(0, SDL_Init(SDL_INIT_AUDIO));

    SineState state{
            .waves =
                    {
                            // technically this is just intonation, which sounds pleasing and is simple numbers,
                            // but it will be slightly off to your standard tuner using equal temperament tuning
                            {.wavelength = 44100.0 / 440.0, .phase = 0, .amplitude = 0.8},  // A4  440
                            {.wavelength = 44100.0 / 550.0, .phase = 0, .amplitude = 1.2},  // C#4 550
                            {.wavelength = 44100.0 / 660.0, .phase = 0, .amplitude = 1.3},  // E4  660
                            {.wavelength = 44100.0 / 880.0, .phase = 0, .amplitude = 1.2},  // A5  880
                            {.wavelength = 44100.0 / 1320.0, .phase = 0, .amplitude = 1.4}, // E5  1320
                    },
    };

    SDL_AudioSpec desired{
            .freq     = 44100,
            .format   = AUDIO_S16LSB,
            .channels = 1,    // mono
            .samples  = 1024, // must be power of two
            .size     = 1024,
            .callback = playSines,
            .userdata = &state,
    },
            obtained;
    auto device = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
    ASSERT_NE(0, device);
    // ASSERT_EQ(0, memcmp(&desired, &obtained, sizeof(desired)));

    SDL_PauseAudioDevice(device, 0);
    SDL_Delay(2000);
    SDL_CloseAudioDevice(device);
}