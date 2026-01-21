#pragma once

#include <irrKlang.h>
#include <string>

class AudioManager {
public:
    static AudioManager& getInstance() {
        static AudioManager instance;
        return instance;
    }

    bool init() {
        engine = irrklang::createIrrKlangDevice();
        return engine != nullptr;
    }

    void shutdown() {
        if (engine) {
            engine->drop();
            engine = nullptr;
        }
    }

    void playSound(const std::string& filename, bool loop = false) {
        if (engine) {
            engine->play2D(filename.c_str(), loop);
        }
    }

    void stopAllSounds() {
        if (engine) {
            engine->stopAllSounds();
        }
    }

private:
    AudioManager() : engine(nullptr) {}
    ~AudioManager() { shutdown(); }

    irrklang::ISoundEngine* engine;
};