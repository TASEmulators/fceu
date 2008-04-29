#include "gme/Nes_Apu.h"
#include "gme/Blip_Buffer.h"
#include "gme/Nes_Vrc6_Apu.h"
#include "gme/Nes_Namco_Apu.h"
#include "gme/Nes_Fme7_Apu.h"
#include "BlarggApu.h"

#include <vector>
#include <math.h>

extern "C" {
  namespace FCEU {
    #include "../types.h"
    #include "../x6502.h"
    #include "../fceu.h"
  }
}

static long FinalRate   = 48000;
static long ContempRate = 48000;

static int SimpleDMCreader(void*, nes_addr_t addr)
{
    return FCEU::ARead[addr](addr);
}

static bool VRC6Enabled  = false;
static bool NamcoEnabled = false;
static bool FME7Enabled  = false;

static class gruu
{
public:
    gruu() : Disabled(true)
    {
        ReInitTimers();
    }
    
    void ReInitTimers()
    {
        double gain = 1.4;
        
        apu.reset(FCEU::PAL);

        apu.dmc_reader(SimpleDMCreader, 0);
        apu.output(&buf);
        
        if(FME7Enabled)
        {
            fme7.output(&buf);
            gain *= 0.75;
        }
        if(VRC6Enabled)
        {
            vrc6.output(&buf);
            gain *= 0.75;
        }
        if(NamcoEnabled)
        {
            namco.output(&buf);
            gain *= 0.75;
        }
        
        const double clockrate_fp
            = FCEU::PAL ? 1662607.0
                        : 19687500.0 / 11.0;
        const double fps         
            = FCEU::PAL ?  1662607.0 / 33247.5
                        : 39375000.0 / 655171.0;
        
        this->frame_len = FCEU::PAL ? 33247.5 : 29780.5;
        
        fprintf(stderr, "Set clock rate. %.4f; %.2f ticks per frame. Gain=%g\n",
            clockrate_fp, this->frame_len, gain);
        buf.clock_rate(clockrate_fp);
        
        NewFrame();

        fme7.volume(gain);
        vrc6.volume(gain);
        namco.volume(gain);
        apu.volume(gain);
    }
    
    void SetSampleRate(long rate)
    {
        Disabled=false;
        fprintf(stderr, "Blargg Init with rate %ld\n", rate);
        ReInitTimers();
        buf.sample_rate(rate);
        //buf.enable_nonlinearity(apu);
    }
    
    void WriteReg(unsigned addr, int data)
    {
        if(Disabled) return;
        
        /* TIMER: Number of cycles since the last call of end_frame */
        if(FME7Enabled)
        {
            if(addr == 0xC000) { /*fprintf(stderr,"Gimmick write latch %d\n", data);*/ fme7.write_latch(data); return; }
            if(addr == 0xE000) { /*fprintf(stderr,"Gimmick write data %02X\n", data);*/ fme7.write_data(GetElapsed(), data); return; }
        }
        if(VRC6Enabled)
        {
            if(addr >= 0x9000 && addr <= 0x9002) { vrc6.write_osc(GetElapsed(), 0, addr&3, data); return; }
            if(addr >= 0xA000 && addr <= 0xA002) { vrc6.write_osc(GetElapsed(), 1, addr&3, data); return; }
            if(addr >= 0xB000 && addr <= 0xB002) { vrc6.write_osc(GetElapsed(), 2, addr&3, data); return; }
        }
        if(NamcoEnabled)
        {
            if(addr == Nes_Namco_Apu::data_reg_addr) namco.write_data(GetElapsed(), data);
            if(addr == Nes_Namco_Apu::addr_reg_addr) namco.write_addr(data);
        }
        /* fprintf(stderr, "%d: write[%04X]<-%02X\n", GetElapsed(),addr,data); */
        apu.write_register(GetElapsed(), addr, data);
    }
    int ReadReg()
    {
        if(Disabled) return 0;
        
        if(NamcoEnabled)
            namco.read_data();
        
        return apu.read_status(GetElapsed());
    }
    void NextFrame()
    {
        if(Disabled) return;
        
        frame_len_got += frame_len;
        unsigned frame_length = (unsigned)frame_len_got;
        /*
        fprintf(stderr, "frame_len=%u (got %.2f, added %.2f)\n",
            frame_length, frame_len_got, frame_len);
        */
        apu.end_frame(frame_length);
        if(FME7Enabled) fme7.end_frame(frame_length);
        if(VRC6Enabled) vrc6.end_frame(frame_length);
        if(NamcoEnabled) namco.end_frame(frame_length);

        buf.end_frame(frame_length);
        
        frame_len_got -= frame_length;
        
        NewFrame();
    }
    
    void PrepareForCycles(unsigned num_cycles)
    {
        /* Assume that the time that was last prepared,
         * has been elapsed already
         */
        OffsetSoFar += PreparedFor;
        PreparedFor = num_cycles;
    }
    
    long SamplesAvail() const
    {
    	return buf.samples_avail();
    }

    long ReadSamples( blip_sample_t* p, long s )
    {
    	return buf.read_samples( p, s );
    }
private:
    void NewFrame()
    {
        OffsetSoFar = 0;
        PreparedFor = 0;
        DummyTime = 0;
    }
    int GetElapsed()
    {
        DummyTime += 4;
        return DummyTime;
        /* In practice, it looks like this dummy code above
         * provides better audio quality than the more correct
         * code below. I don't know why! -Bisqwit
         */
    
        int remain  = FCEU::X.count;
        int elapsed = PreparedFor - remain;
        /*
        fprintf(stderr, "Elapsed: prepared for %d, remain %d, offset %d\n",
            PreparedFor, remain, OffsetSoFar);
        */
        return (elapsed + OffsetSoFar) / 48;
    }

private:
    Nes_Apu     apu;
    Blip_Buffer buf;
    
    /* Cycle counters */
    unsigned OffsetSoFar;
    unsigned PreparedFor;
    unsigned DummyTime;
    
    double frame_len;
    double frame_len_got;

private:
    Nes_Fme7_Apu  fme7;
    Nes_Vrc6_Apu  vrc6;
    Nes_Namco_Apu namco;
    
    bool Disabled;
    
} BlarggAPU;


void BlarggSetSampleRate(long rate)
{
    FinalRate   = rate;
    //ContempRate = rate;
    BlarggAPU.SetSampleRate(ContempRate);
    
    fprintf(stderr, "Contemp=%d, Final=%d\n", (int)ContempRate, (int)FinalRate);
}

void BlarggWrite(unsigned addr, int data)
{
    BlarggAPU.WriteReg(addr, data);
}

int BlarggRead()
{
    return BlarggAPU.ReadReg();
}

void BlarggWillExecute(unsigned num_cycles)
{
    BlarggAPU.PrepareForCycles(num_cycles);
}

void BlarggEnableVRC6()
{
    if(VRC6Enabled) return;
    VRC6Enabled = true;
    BlarggSetSampleRate(FinalRate);
    fprintf(stderr, "VRC6 synthesizer enabled\n");
}

void BlarggEnableNamco()
{
    if(NamcoEnabled) return;
    NamcoEnabled = true;
    BlarggSetSampleRate(FinalRate);
    fprintf(stderr, "Namco synthesizer enabled\n");
}

void BlarggEnableFME7()
{
    if(FME7Enabled) return;
    FME7Enabled = true;
    BlarggSetSampleRate(FinalRate);
    fprintf(stderr, "FME7 synthesizer enabled\n");
}

/************* AUDIO RESAMPLE CODE ****************/
static inline double dblmin(const double a, const double b) { return a < b ? a : b; }
static inline double dblmax(const double a, const double b) { return a > b ? a : b; }
static inline double dblabs(const double a) { return a < 0 ? -a : a; }
static inline double Lanczos(double x)
{
  if(x == 0.0) return 1.0;
  if(x <= -3.0 || x >= 3.0)return 0.0;
  double tmp = x * M_PI, tmp2 = tmp / 3.0;
  return sin(tmp) * sin(tmp2) / (tmp * tmp2);
}

typedef short ResampleItem;
struct ContiguousResampleBuf
{ 
public:
    double center;
public:
    ContiguousResampleBuf() : center(0.5), firstpos(0) { }
    
    inline void NotNeedOlderThan(int pos)
    {
        const int n_discard_at_once = 4096;
        int discardable = pos-firstpos;
        if(discardable >= n_discard_at_once)
        {
            int n_discard = (discardable / n_discard_at_once) * n_discard_at_once;
            AudioData.erase(AudioData.begin(), AudioData.begin()+n_discard);
            firstpos += n_discard;
        }
    }
    inline bool AddMul(double& t, int pos, double coeff)
    {
        if(pos >= firstpos)
        {
            if(pos >= (int)(firstpos + AudioData.size()))
                return false;
            t += AudioData[pos - firstpos] * coeff;
        }
        return true;
    }
    inline void Feed(const unsigned char* input, unsigned nbytes)
    {
        AudioData.insert(AudioData.end(),
                         (ResampleItem*)input,
                         (ResampleItem*)(input+nbytes));
    }
    inline int DropExcess()
    {
        //printf("excess=%d, center=%g\n", firstpos, center);
        int res = firstpos;
        firstpos = 0;
        return res;  
    }
private:
    std::vector<ResampleItem> AudioData;
    int firstpos;
};
const std::vector<ResampleItem>
    ContiguousResample(double factor,
                       const unsigned char* input, unsigned nbytes)
{
    static ContiguousResampleBuf status;
    
    if(factor == 1.0)
    {
        return std::vector<ResampleItem>
            ( (ResampleItem*)input,
              (ResampleItem*)(input+nbytes) );
    }
    
    status.Feed(input, nbytes);
    
    const double filter_support = 3.0;
    double scale   = dblmin(factor, 1.0);
    double support = filter_support / scale;
    if(support <= 0.5) { support = 0.5 + 1E-12; scale = 1.0; }
    
    const double factor_rev = 1.0 / factor;
    double contribution[(unsigned)(support*2 + 2)];

    std::vector<ResampleItem> result;
    for(;;)
    {
        const int start = (int)(status.center-support+0.5);
        const int stop  = (int)(status.center+support+0.5);
        double density = 0.0;
        
        unsigned nmax = stop-start;
        
        double s = start - status.center + 0.5;
        for(unsigned n=0; n<nmax; ++n, ++s)
        {
            contribution[n] = Lanczos(s * scale);
            density += contribution[n];
        }
        if(density != 0.0 && density != 1.0)
        {
            for(unsigned n=0; n<nmax; ++n)
                contribution[n] /= density;
        }
        
        double result_t = 0.0;
        for(unsigned n=0; n<nmax; ++n)
        {
            if(!status.AddMul(result_t, start+n, contribution[n]))
            {
                status.NotNeedOlderThan(start);
                goto NoMoreData;
            }
        }
        
        status.center += factor_rev;
        
        int res_t = (int)result_t; if(res_t<-32768)res_t=-32768; if(res_t>32767)res_t=32767;
        result.push_back(res_t);
    }
NoMoreData:
    status.center -= status.DropExcess();
    return result;
}

static class Feeder
{
private:
    std::vector<blip_sample_t> buffer;
public:
    Feeder() : buf(0), bufsize(0)
    {
    }
    void FeedSet()
    {
        if(buf) return;
        
        long avail = BlarggAPU.SamplesAvail();
        if(avail < 0) avail = 0;
        
        /*
        if(avail)
        {
        static double SamplesTotal=0; static unsigned FramesTotal=0;
        SamplesTotal += avail;
        ++FramesTotal;
        static unsigned counter=0;
        if(++counter>=20){counter=0;
        fprintf(stderr, "Average samples per frame: %.4f (this frame: %ld)\n", SamplesTotal/FramesTotal, avail);
        }
        }
        else
        fprintf(stderr, "No samples this frame\n");
        */
        
        if(FinalRate == ContempRate)
        {
            buffer.resize(avail);
            long size = BlarggAPU.ReadSamples(&buffer[0], buffer.size());
            if(size < 0) size = 0;
            buffer.resize(size*2);
            
            for(long s = size; s-- > 0; )
            {
                buffer[s*2+0] = buffer[s];
                buffer[s*2+1] = buffer[s];
            }
            this->buf     = (void*)&buffer[0];
            this->bufsize = buffer.size()/2;
            return;
        }
        
        std::vector<blip_sample_t> tmp(avail);
        long tsize = BlarggAPU.ReadSamples(&tmp[0], tmp.size());
        if(tsize < 0) tsize = 0;
        tmp.resize(tsize);
        
        std::vector<ResampleItem> result =
          ContiguousResample(FinalRate / (double)ContempRate,
                             (const unsigned char*)&tmp[0],
                             tmp.size() * sizeof(tmp[0]));
        
        long size = result.size();
        buffer.resize(size*2);
        for(long s = size; s-- > 0; )
        {
            buffer[s*2+0] = result[s];
            buffer[s*2+1] = result[s];
        }
        this->buf     = (void*)&buffer[0];
        this->bufsize = buffer.size()/2;
    }
    void FeedClear()
    {
        buf=0;
        bufsize=0;
    }
public:
    void* buf;
    long bufsize;
} Feeder;



unsigned BlarggGetSamplesAvail(void)
{
    if(!Feeder.buf) Feeder.FeedSet();
    return Feeder.bufsize;
}

void* BlarggGetSoundBuffer(void)
{
    if(!Feeder.buf) Feeder.FeedSet();
    return Feeder.buf;
}

void BlarggEndFrame()
{
    BlarggAPU.NextFrame();
    Feeder.FeedClear();
}

