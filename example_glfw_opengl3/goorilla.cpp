
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

    float pi = 3.141592f;
    float sinAngle = 0.0;
    for (int i = 0;i < tableLength;i++)
    {
        float curval = (sin(sinAngle) * 32767.0f);
        sintable[i] = (short int)curval;
        squaretable[i] = 0;
        tritable[i] = 0;

        sinAngle += (2.0f * pi) / ((float)tableLength);
    }

    for (int i = 0;i < sampleBufferLen*2;i++)
    {
        sampleBuffer[i] = 0;
    }
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
        /*voice[voiceNum].samplePos+=voice[voiceNum].voiceFrequency;

        if (voice[voiceNum].samplePos >= tableLength)
        {
            voice[voiceNum].samplePos -= tableLength;
        }*/

        voice[voiceNum].samplePos += 1;

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

            // gah, clipping...
            if (voice[voiceNum].envelopeVol < 0.0) voice[voiceNum].envelopeVol = 0;
            if (voice[voiceNum].envelopeVol > 100.0) voice[voiceNum].envelopeVol = 100.0f;
        }
    }
}

short int xpldSoundChip::getVoiceSample(int voiceNum)
{
    if (voice[voiceNum].playing)
    {
        if (voice[voiceNum].voiceWaveform == VOICE_SINE)
        {
            float side = sin(2.0 * 3.141592 * voice[voiceNum].voiceFrequency * voice[voiceNum].samplePos / 48000.0);
            float sampl = side*32767.0;
            sampl *= voice[voiceNum].envelopeVol;
            sampl *= voice[voiceNum].volume;
            sampl /= 100.0*255.0;
            return (short int)sampl;
        }
        else if (voice[voiceNum].voiceWaveform == VOICE_SQUARE)
        {
            float side = sin(2.0 * 3.141592 * voice[voiceNum].voiceFrequency * voice[voiceNum].samplePos / 48000.0);
            float sign = 1.0;
            if (side < 0) sign = -1.0;
            float sampl = sign * 32767.0;
            sampl *= voice[voiceNum].envelopeVol;
            sampl *= voice[voiceNum].volume;
            sampl /= 100.0 * 255.0;
            return (short int)sampl;
        }
        else if (voice[voiceNum].voiceWaveform == VOICE_TRIANGLE)
        {
            float t = voice[voiceNum].voiceFrequency * voice[voiceNum].samplePos / 48000.0;
            float tri=1.0 - fabs(fmod(t, 2.0) - 1.0);
            float sampl = tri * 32767.0;
            sampl *= voice[voiceNum].envelopeVol;
            sampl *= voice[voiceNum].volume;
            sampl /= 100.0 * 255.0;
            return (short int)sampl;
        }
        else if (voice[voiceNum].voiceWaveform == VOICE_NOISE)
        {
            float sampl = ((float)(rand()%65535))-32767.0;
            sampl *= voice[voiceNum].envelopeVol;
            sampl *= voice[voiceNum].volume;
            sampl /= 100.0 * 255.0;
            return (short int)sampl;
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
    else if (val & 16)
    {
        voice[voiceNum].voiceWaveform = VOICE_NOISE;
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
    demultiplier += 1;
    if (demultiplier < stepsPerCycle) return;
    demultiplier = 0;

    if (sampleBufferPos >= (sampleBufferLen*2)) return;

    int finalSampl = 0;
    for (int v = 0;v < numSoundVoices;v++)
    {
        finalSampl+=getVoiceSample(v);
        updateVoice(v);
    }

    sampleBuffer[sampleBufferPos] = finalSampl;
    sampleBuffer[sampleBufferPos+1] = finalSampl;

    sampleBufferPos+=2;
}

short int* xpldSoundChip::getSampleBuffer()
{
    return sampleBuffer;
}

bool xpldSoundChip::isSampleBufferFilled()
{
    if (sampleBufferPos == (sampleBufferLen*2))
    {
        return true;
    }

    return false;
}

void xpldSoundChip::resetPointer()
{
    sampleBufferPos = 0;
    for (int i = 0;i < sampleBufferLen*2;i++)
    {
        sampleBuffer[i] = 0;
    }
}

xpldSoundChip::~xpldSoundChip()
{
}
