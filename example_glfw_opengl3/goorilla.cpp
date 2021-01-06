
/* growl. */

#include <math.h>
#include "bass/bass.h"

#include "goorilla.h"

xpldSoundChip::xpldSoundChip()
{
    for (int i = 0;i < numSoundVoices;i++)
    {
        initVoice(i);
    }

    // init tables

    float pi = 3.14159265358979323846f;

    float sinAngle = 0.0;
    for (int i = 0;i < tableLength;i++)
    {
        sintable[i] = (int)(sin(sinAngle) * 32767.0f);
        sinAngle += (2.0f * pi) / (float)(tableLength);
    }

    angle = 0.0;
}

void xpldSoundChip::initVoice(int voiceNum)
{
    voice[voiceNum].playing = false;
    voice[voiceNum].duration = 0;
    voice[voiceNum].voiceFrequency = 0;
    voice[voiceNum].voiceWaveform = VOICE_SINE;
    voice[voiceNum].samplePos = 0;
    voice[voiceNum].volume = 0;
    voice[voiceNum].tick = 0;
    voice[voiceNum].attackPercent = 25;
    voice[voiceNum].decayPercent = 25;
    voice[voiceNum].sustainPercent = 25;
    voice[voiceNum].releasePercent = 25;
    voice[voiceNum].envelopeVol = 0.0f;
}

void xpldSoundChip::updateVoice(int voiceNum)
{
    if (voice[voiceNum].playing)
    {
        voice[voiceNum].samplePos+=voice[voiceNum].voiceFrequency;
        voice[voiceNum].tick++;

        if (voice[voiceNum].tick >= voice[voiceNum].duration)
        {
            voice[voiceNum].playing = false;
        }
        else
        {
            int attackLen = (voice[voiceNum].duration * voice[voiceNum].attackPercent) / 100;
            if (attackLen == 0) attackLen = 1;
            int decayLen = (voice[voiceNum].duration * voice[voiceNum].decayPercent) / 100;
            if (decayLen == 0) decayLen = 1;
            int sustainLen = (voice[voiceNum].duration * voice[voiceNum].sustainPercent) / 100;
            if (sustainLen == 0) sustainLen = 1;
            int releaseLen = (voice[voiceNum].duration * voice[voiceNum].releasePercent) / 100;
            if (releaseLen == 0) releaseLen = 1;

            int voiceTick = voice[voiceNum].tick;
            if (voiceTick < attackLen) voice[voiceNum].envelopeVol += voice[voiceNum].attackSlope;
            else if (voiceTick<(attackLen+decayLen)) voice[voiceNum].envelopeVol -= voice[voiceNum].decaySlope;
            else if (voiceTick<(attackLen+decayLen+sustainLen)) voice[voiceNum].envelopeVol += voice[voiceNum].sustainSlope;
            else voice[voiceNum].envelopeVol -= voice[voiceNum].releaseSlope;
        }
    }
}

short int xpldSoundChip::getVoiceSample(int voiceNum)
{
    if (voice[voiceNum].playing)
    {
        short int sampl;

        if (voice[voiceNum].voiceWaveform == VOICE_SINE)
        {
            sampl = sintable[voice[voiceNum].samplePos % tableLength];
            float fsamp = (((float)sampl) * voice[voiceNum].envelopeVol)/100.0;
            return (short)fsamp;
        }

        return 0;
    }

    return 0;
}

void xpldSoundChip::recalcVoiceParams(int voiceNum)
{
    int attackLen = (voice[voiceNum].duration * voice[voiceNum].attackPercent) / 100;
    if (attackLen == 0) attackLen = 1;
    voice[voiceNum].attackSlope = (attackMaxVol) / (float)attackLen;

    int decayLen= (voice[voiceNum].duration * voice[voiceNum].decayPercent) / 100;
    if (decayLen == 0) decayLen = 1;
    voice[voiceNum].decaySlope = (attackMaxVol-decayMaxVol) / (float)decayLen;

    voice[voiceNum].sustainSlope = 0;

    int releaseLen = (voice[voiceNum].duration * voice[voiceNum].releasePercent) / 100;
    if (releaseLen == 0) releaseLen = 1;
    voice[voiceNum].releaseSlope = (decayMaxVol) / (float)releaseLen;
}

void xpldSoundChip::setVoiceStatus(int voiceNum, unsigned char val)
{
    if ((val & 1) == 0)
    {
        voice[voiceNum].playing = false;
        voice[voiceNum].samplePos = 0;
        voice[voiceNum].tick = 0;
    }
    else
    {
        voice[voiceNum].playing = true;
        voice[voiceNum].samplePos = 0;
        voice[voiceNum].tick = 0;
        recalcVoiceParams(voiceNum);
    }

    if (val & 2)
    {
        voice[voiceNum].voiceWaveform = VOICE_SQUARE;
    }
    else if (val & 4)
    {
        voice[voiceNum].voiceWaveform = VOICE_TRIANGLE;
    }
    else if (val & 8)
    {
        voice[voiceNum].voiceWaveform = VOICE_SINE;
    }
}

void xpldSoundChip::setVoiceAttack(int voiceNum, unsigned char val)
{
    voice[voiceNum].attackPercent = val;
}

void xpldSoundChip::setVoiceDecay(int voiceNum, unsigned char val)
{
    voice[voiceNum].decayPercent = val;
}

void xpldSoundChip::setVoiceSustain(int voiceNum, unsigned char val)
{
    voice[voiceNum].sustainPercent = val;
}

void xpldSoundChip::setVoiceRelease(int voiceNum, unsigned char val)
{
    voice[voiceNum].releasePercent = val;
}

void xpldSoundChip::setVoiceFrequency(int voiceNum, unsigned int freq)
{
    voice[voiceNum].voiceFrequency = freq;
}

void xpldSoundChip::setVoiceVolume(int voiceNum, unsigned char val)
{
    voice[voiceNum].volume = val;
}

void xpldSoundChip::setVoiceDuration(int voiceNum, unsigned int duration)
{
    voice[voiceNum].duration = duration;
}

void xpldSoundChip::stepOne()
{
    if (sampleBufferPos == sampleBufferLen) return;

    demultiplier += 1;
    if (demultiplier < stepsPerCycle) return;
    demultiplier = 0;

    short int finalSampl = 0;
    for (int v = 0;v < numSoundVoices;v++)
    {
        finalSampl += getVoiceSample(v);
        updateVoice(v);
    }

    sampleBuffer[sampleBufferPos] = finalSampl;
    sampleBufferPos++;
}

/*DWORD xpldSoundChip::writeToOutputBuffer(HSTREAM handle, short* buffer, DWORD length, void* user)
{
    return 0;
}*/

short int* xpldSoundChip::getSampleBuffer()
{
    return sampleBuffer;
}

bool xpldSoundChip::isSampleBufferFilled()
{
    if (sampleBufferPos == sampleBufferLen)
    {
        return true;
    }

    return false;
}

void xpldSoundChip::resetPointer()
{
    sampleBufferPos = 0;
}
