#include "fixNES/apu.c"

/*
 * Alternate apuCycle implementation to avoid processing multiple
 * NES frames per URDE frame (costly and jarring to player).
 *
 * This implementation nominally fills 6/10 buffers, allowing
 * emulation to "catch up" by having more buffer headroom available
 * (and also reducing audio latency somewhat).
 *
 * URDE's NesEmuMainLoop uses emuSkipVsync as a signal to proceed
 * with the emulation, allowing audio buffers to be pre-filled with
 * generated tones independent of the emulated CPU. Granted, this
 * compromises accuracy, but doesn't affect NEStroid's behavior and
 * reduces audio discontinuities.
 */

bool apuCycleURDE()
{
    if(curBufPos == apuBufSize)
    {
        int updateRes = audioUpdate();
        if(updateRes == 0)
        {
            emuSkipFrame = false;
            emuSkipVsync = false;
            return false;
        }
        if(updateRes > 6)
        {
            emuSkipVsync = true;
            emuSkipFrame = true;
        }
        else
        {
            emuSkipFrame = false;
            if(updateRes > 4) // 6 buffers filled, stop here
                emuSkipVsync = true;
            else
                emuSkipVsync = false;
        }
        curBufPos = 0;
    }
    uint8_t p1Out = lastP1Out, p2Out = lastP2Out,
        triOut = lastTriOut, noiseOut = lastNoiseOut;
    if(p1LengthCtr && (APU_IO_Reg[0x15] & P1_ENABLE))
    {
        if(p1seq[p1Cycle] && !p1Sweep.mute && freq1 >= 8 && freq1 < 0x7FF)
            lastP1Out = p1Out = (p1Env.constant ? p1Env.vol : p1Env.decay);
        else
            p1Out = 0;
    }
    if(p2LengthCtr && (APU_IO_Reg[0x15] & P2_ENABLE))
    {
        if(p2seq[p2Cycle] && !p2Sweep.mute && freq2 >= 8 && freq2 < 0x7FF)
            lastP2Out = p2Out = (p2Env.constant ? p2Env.vol : p2Env.decay);
        else
            p2Out = 0;
    }
    if(triLengthCtr && triCurLinearCtr && (APU_IO_Reg[0x15] & TRI_ENABLE))
    {
        if(triSeq[triCycle] && triFreq >= 2)
            lastTriOut = triOut = triSeq[triCycle];
        else
            triOut = 0;
    }
    if(noiseLengthCtr && (APU_IO_Reg[0x15] & NOISE_ENABLE))
    {
        if((noiseShiftReg&1) == 0 && noiseFreq > 0)
            lastNoiseOut = noiseOut = (noiseEnv.constant ? noiseEnv.vol : noiseEnv.decay);
        else
            noiseOut = 0;
    }
#if AUDIO_FLOAT
    float curIn = pulseLookupTbl[p1Out + p2Out] + tndLookupTbl[(3*triOut) + (2*noiseOut) + dmcVol];
    //very rough still
    if(vrc6enabled)
    {
        vrc6AudioCycle();
        curIn += ((float)vrc6Out)*0.008f;
        curIn *= 0.6667f;
    }
    if(fdsEnabled)
    {
        fdsAudioCycle();
        curIn += ((float)fdsOut)*0.00617f;
        curIn *= 0.75f;
    }
    if(mmc5enabled)
    {
        mmc5AudioCycle();
        curIn += pulseLookupTbl[mmc5Out]+(mmc5pcm*0.002f);
        curIn *= 0.75f;
    }
    if(vrc7enabled)
    {
        curIn += (((float)(vrc7Out>>7))/32768.f);
        curIn *= 0.75f;
    }
    if(n163enabled)
    {
        curIn += ((float)n163Out)*0.0008f;
        curIn *= 0.6667f;
    }
    if(s5Benabled)
    {
        s5BAudioCycle();
        curIn += ((float)s5BOut)/32768.f;
        curIn *= 0.6667f;
    }
    //amplify input
    curIn *= 3.0f;
    float curLPout = lastLPOut+(lpVal*(curIn-lastLPOut));
    float curHPOut = hpVal*(lastHPOut+lastLPOut-curLPout);
    //set output
    apuOutBuf[curBufPos] = curHPOut;
    lastLPOut = curLPout;
    lastHPOut = curHPOut;
#else
    int32_t curIn = pulseLookupTbl[p1Out + p2Out] + tndLookupTbl[(3*triOut) + (2*noiseOut) + dmcVol];
    //very rough still
    if(vrc6enabled)
    {
        vrc6AudioCycle();
        curIn += ((int32_t)vrc6Out)*262;
        curIn <<= 1; curIn /= 3;
    }
    if(fdsEnabled)
    {
        fdsAudioCycle();
        curIn += ((int32_t)fdsOut)*202;
        curIn *= 3; curIn >>= 2;
    }
    if(mmc5enabled)
    {
        mmc5AudioCycle();
        curIn += pulseLookupTbl[mmc5Out]+(mmc5pcm<<6);
        curIn *= 3; curIn >>= 2;
    }
    if(vrc7enabled)
    {
        curIn += vrc7Out>>7;
        curIn *= 3; curIn >>= 2;
    }
    if(n163enabled)
    {
        curIn += n163Out*26;
        curIn <<= 1; curIn /= 3;
    }
    if(s5Benabled)
    {
        s5BAudioCycle();
        curIn += s5BOut;
        curIn <<= 1; curIn /= 3;
    }
    //amplify input
    curIn *= 3;
    int32_t curOut;
    //gen output
    curOut = lastLPOut+((lpVal*(curIn-lastLPOut))>>15); //Set Lowpass Output
    curIn = (lastHPOut+lastLPOut-curOut); //Set Highpass Input
    curIn += (curIn>>31)&1; //Add Sign Bit for proper Downshift later
    lastLPOut = curOut; //Save Lowpass Output
    curOut = (hpVal*curIn)>>15; //Set Highpass Output
    lastHPOut = curOut; //Save Highpass Output
    //Save Clipped Highpass Output
    apuOutBuf[curBufPos] = (curOut > 32767)?(32767):((curOut < -32768)?(-32768):curOut);
#endif
    apuOutBuf[curBufPos+1] = apuOutBuf[curBufPos];
    curBufPos+=2;
    return true;
}
