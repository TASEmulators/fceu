#ifdef __cplusplus
extern "C" {
#endif
void BlarggSetSampleRate(long rate);
void BlarggWrite(unsigned addr, int data);
int BlarggRead(void);
void BlarggEndFrame(void);

void BlarggEnableVRC6();
void BlarggEnableNamco();
void BlarggEnableFME7();

void BlarggWillExecute(unsigned num_cycles);

unsigned BlarggGetSamplesAvail(void);
void* BlarggGetSoundBuffer(void);

#ifdef __cplusplus
}
#endif
