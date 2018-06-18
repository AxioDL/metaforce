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
    if(apu.curBufPos == apu.BufSize)
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
        apu.curBufPos = 0;
    }

    uint8_t aExp = audioExpansion;
    if(!(apu.apuClock&7))
    {
        if(apu.p1LengthCtr && (apu.reg[0x15] & P1_ENABLE))
        {
            if(!apu.p1Sweep.mute && apu.freq1 >= 8 && apu.freq1 < 0x7FF)
                apu.p1Out = apu.p1seq[apu.p1Cycle] ? (apu.p1Env.constant ? apu.p1Env.vol : apu.p1Env.decay) : 0;
        }
        if(apu.p2LengthCtr && (apu.reg[0x15] & P2_ENABLE))
        {
            if(!apu.p2Sweep.mute && apu.freq2 >= 8 && apu.freq2 < 0x7FF)
                apu.p2Out = apu.p2seq[apu.p2Cycle] ? (apu.p2Env.constant ? apu.p2Env.vol : apu.p2Env.decay) : 0;
        }
        if(apu.triLengthCtr && apu.triCurLinearCtr && (apu.reg[0x15] & TRI_ENABLE))
        {
            if(apu.triFreq >= 2)
                apu.triOut = apu.triSeq[apu.triCycle];
        }
        if(apu.noiseLengthCtr && (apu.reg[0x15] & NOISE_ENABLE))
        {
            if(apu.noiseFreq > 0)
                apu.noiseOut = (apu.noiseShiftReg&1) == 0 ? (apu.noiseEnv.constant ? apu.noiseEnv.vol : apu.noiseEnv.decay) : 0;
        }
#if AUDIO_FLOAT
        float curIn = apu.pulseLookupTbl[apu.p1Out + apu.p2Out] + apu.tndLookupTbl[(3*apu.triOut) + (2*apu.noiseOut) + apu.dmcVol];
		uint8_t ampVolPos = 0;
		//very rough still
		if(aExp & EXP_VRC6)
		{
			vrc6AudioCycle();
			curIn += ((float)vrc6Out)*0.008f;
			ampVolPos++;
		}
		if(aExp & EXP_FDS)
		{
			fdsAudioCycle();
			curIn += ((float)fdsOut)*0.00617f;
			ampVolPos++;
		}
		if(aExp & EXP_MMC5)
		{
			mmc5AudioCycle();
			curIn += apu.pulseLookupTbl[mmc5Out]+(mmc5pcm*0.002f);
			ampVolPos++;
		}
		if(aExp & EXP_VRC7)
		{
			curIn += (((float)(vrc7Out>>7))/32768.f);
			ampVolPos++;
		}
		if(aExp & EXP_N163)
		{
			curIn += ((float)n163Out)*0.0008f;
			ampVolPos++;
		}
		if(aExp & EXP_S5B)
		{
			s5BAudioCycle();
			curIn += ((float)s5BOut)/32768.f;
			ampVolPos++;
		}
		//amplify input
		curIn *= ampVol[ampVolPos];
		float curLPout = apu.lastLPOut+(apu.lpVal*(curIn-apu.lastLPOut));
		float curHPOut = apu.hpVal*(apu.lastHPOut+apu.lastLPOut-curLPout);
		//set output
		apu.OutBuf[apu.curBufPos] = curHPOut;
		apu.lastLPOut = curLPout;
		apu.lastHPOut = curHPOut;
#else
        int32_t curIn = apu.pulseLookupTbl[apu.p1Out + apu.p2Out] + apu.tndLookupTbl[(3*apu.triOut) + (2*apu.noiseOut) + apu.dmcVol];
        uint8_t ampVolPos = 0;
        //very rough still
        if(aExp & EXP_VRC6)
        {
            vrc6AudioCycle();
            curIn += ((int32_t)vrc6Out)*262;
            ampVolPos++;
        }
        if(aExp & EXP_FDS)
        {
            fdsAudioCycle();
            curIn += ((int32_t)fdsOut)*202;
            ampVolPos++;
        }
        if(aExp & EXP_MMC5)
        {
            mmc5AudioCycle();
            curIn += apu.pulseLookupTbl[mmc5Out]+(mmc5pcm<<6);
            ampVolPos++;
        }
        if(aExp & EXP_VRC7)
        {
            curIn += vrc7Out>>7;
            ampVolPos++;
        }
        if(aExp & EXP_N163)
        {
            curIn += n163Out*26;
            ampVolPos++;
        }
        if(aExp & EXP_S5B)
        {
            s5BAudioCycle();
            curIn += s5BOut;
            ampVolPos++;
        }
        //amplify input
        curIn *= apu.ampVol[ampVolPos];
        int32_t curOut;
        //gen output
        curOut = apu.lastLPOut+((apu.lpVal*((curIn>>6)-apu.lastLPOut))>>15); //Set Lowpass Output
        curIn = (apu.lastHPOut+apu.lastLPOut-curOut); //Set Highpass Input
        curIn += (curIn>>31)&1; //Add Sign Bit for proper Downshift later
        apu.lastLPOut = curOut; //Save Lowpass Output
        curOut = (apu.hpVal*curIn)>>15; //Set Highpass Output
        apu.lastHPOut = curOut; //Save Highpass Output
        //Save Clipped Highpass Output
        apu.OutBuf[apu.curBufPos] = (curOut > 32767)?(32767):((curOut < -32768)?(-32768):curOut);
#endif
        apu.OutBuf[apu.curBufPos+1] = apu.OutBuf[apu.curBufPos];
        apu.curBufPos+=2;
    }
    apu.apuClock++;

    if(apu.p1freqCtr == 0)
    {
        apu.p1freqCtr = (apu.freq1<<1)+1;
        apu.p1Cycle = (apu.p1Cycle+1)&7;
    }
    else
        apu.p1freqCtr--;

    if(apu.p2freqCtr == 0)
    {
        apu.p2freqCtr = (apu.freq2<<1)+1;
        apu.p2Cycle = (apu.p2Cycle+1)&7;
    }
    else
        apu.p2freqCtr--;

    if(apu.triFreqCtr == 0)
    {
        apu.triFreqCtr = apu.triFreq;
        apu.triCycle = (apu.triCycle+1)&31;
    }
    else
        apu.triFreqCtr--;

    if(apu.noiseFreqCtr == 0)
    {
        apu.noiseFreqCtr = apu.noiseFreq;
        uint8_t cmpBit = apu.noiseMode1 ? (apu.noiseShiftReg>>6)&1 : (apu.noiseShiftReg>>1)&1;
        uint8_t cmpRes = (apu.noiseShiftReg&1)^cmpBit;
        apu.noiseShiftReg >>= 1;
        apu.noiseShiftReg |= cmpRes<<14;
    }
    else
        apu.noiseFreqCtr--;

    if(apu.dmcFreqCtr == 0)
    {
        apu.dmcFreqCtr = apu.dmcFreq;
        if(apu.dmcenabled)
        {
            if(apu.dmcSampleBuf&1)
            {
                if(apu.dmcVol <= 125)
                    apu.dmcVol += 2;
            }
            else if(apu.dmcVol >= 2)
                apu.dmcVol -= 2;
            apu.dmcSampleBuf>>=1;
        }
        if(apu.dmcSampleRemain == 0)
        {
            if(apu.dmcready)
            {
                apu.dmcSampleBuf = apu.dmcCpuBuf;
                apu.dmcenabled = true;
                apu.dmcready = false;
            }
            else
                apu.dmcenabled = false;
            apu.dmcSampleRemain = 7;
        }
        else
            apu.dmcSampleRemain--;
    }
    else
        apu.dmcFreqCtr--;
    if(!apu.dmcready && !cpuInDMC_DMA() && apu.dmcCurLen)
    {
        cpuDoDMC_DMA(apu.dmcCurAddr);
        apu.dmcCurLen--;
    }

    if(aExp&EXP_VRC7)
    {
        if(apu.vrc7Clock == vrc7CycleTimer)
        {
            vrc7AudioCycle();
            apu.vrc7Clock = 1;
        }
        else
            apu.vrc7Clock++;
    }
    if(aExp&EXP_FDS)
        fdsAudioMasterUpdate();
    if(aExp&EXP_MMC5)
        mmc5AudioLenCycle();

    if(apu.mode_change)
        apuChangeMode();

    if(apu.mode5 == false)
    {
        if(apu.modeCurCtr == 0)
        {
            if(apu.modePos == 5)
                apu.modePos = 0;
            else
                apu.modePos++;
            apu.modeCurCtr = apu.mode4Ctr[apu.modePos]-1;
            if(apu.modePos == 3 || apu.modePos == 5)
            {
                if(apu.enable_irq)
                    apu.irq = 1;
            }
            else
            {
                if(apu.modePos == 1)
                    apuClockA();
                else if(apu.modePos == 4)
                {
                    apuClockA();
                    if(apu.enable_irq)
                    {
                        apu.irq = 1;
                        //actually set for cpu
                        interrupt |= APU_IRQ;
                    }
                }
                apuClockB();
            }
        }
        else
            apu.modeCurCtr--;
    }
    else
    {
        if(apu.modeCurCtr == 0)
        {
            if(apu.modePos == 5)
                apu.modePos = 0;
            else
                apu.modePos++;
            apu.modeCurCtr = apu.mode5Ctr[apu.modePos]-1;
            if(apu.modePos != 1 && apu.modePos != 5)
            {
                if(apu.modePos == 0 || apu.modePos == 3)
                    apuClockA();
                apuClockB();
            }
        }
        else
            apu.modeCurCtr--;
    }

    return true;
}

uint32_t apuGetMaxBufSize()
{
    return apu.BufSizeBytes;
}
