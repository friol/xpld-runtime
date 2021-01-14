#ifndef GOORILLA_H
#define GOORILLA_H

#include <iostream>
#include <fstream>

#include "bass/bass.h"

#define numSoundVoices 3
#define samplingFrequency 48000
#define updatesPerSecond 60
//#define sampleBufferLen (samplingFrequency/updatesPerSecond)
#define sampleBufferLen 800

#define tableLength samplingFrequency

#define attackMaxVol 100
#define decayMaxVol 80
#define sustainMaxVol 80

enum voiceWaveforms
{
    VOICE_SQUARE,
    VOICE_TRIANGLE,
    VOICE_SINE,
    VOICE_NOISE
};

typedef struct xpldSoundVoice
{
    int voiceWaveform;
    float voiceFrequency;
    bool playing;
    unsigned int duration;

    unsigned int tick;
    float samplePos;

    unsigned char volume;

    unsigned int attackPercent;
    unsigned int decayPercent;
    unsigned int sustainPercent;
    unsigned int releasePercent;

    float attackSlope;
    float decaySlope;
    float sustainSlope;
    float releaseSlope;

    float envelopeVol;
};

class xpldSoundChip
{
private:

    float angle;
    int sampleBufferPos = 0;
    int demultiplier = 0;
    const int stepsPerCycle = 256; // CPU cycles per soundchip cycle

    short int sampleBuffer[sampleBufferLen*2];

    std::ofstream myfile;

    short int sintable[tableLength];
    short int squaretable[tableLength];
    short int tritable[tableLength];

    xpldSoundVoice voice[numSoundVoices];

    void initVoice(int voiceNum);
    void updateVoice(int voiceNum);
    short int getVoiceSample(int voiceNum);

    void recalcVoiceParams(int voiceNum);

public:

    xpldSoundChip();

    void stepOne();

    void setVoiceStatus(int voiceNum, unsigned char val);
    void setVoiceAttack(int voiceNum, unsigned char val);
    void setVoiceDecay(int voiceNum, unsigned char val);
    void setVoiceSustain(int voiceNum, unsigned char val);
    void setVoiceRelease(int voiceNum, unsigned char val);
    void setVoiceFrequency(int voiceNum, unsigned int freq);
    void setVoiceVolume(int voiceNum, unsigned char val);
    void setVoiceDuration(int voiceNum, unsigned int duration);

    short int* getSampleBuffer();
    bool isSampleBufferFilled();
    void resetPointer();

    ~xpldSoundChip();
};

#endif
