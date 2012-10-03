struct AHX_HEADER
{
	char *ahxName;
	int cia;
    char *namesPtr;
	unsigned char totalLen;
	unsigned int resPoint;
	unsigned char trackLen;
	int bTrack0;
	int bIs20ModuleFormat;
	unsigned char totalTracks;
	unsigned char totalSamples;
	unsigned char totalSubSongs;
	
};

struct AHX_SONGSEQ
{
	unsigned char ch1Track;
	unsigned char ch2Track;
	unsigned char ch3Track;
	unsigned char ch4Track;
	unsigned char ch1Transp;
	unsigned char ch2Transp;
	unsigned char ch3Transp;
	unsigned char ch4Transp;
};

struct AHX_TRACKSTEP
{
	unsigned char note;
	unsigned char sample;
	unsigned char command;
	unsigned char param;
};

struct AHX_SAMPLE
{
	char *Name;
	unsigned char masterVol;
	unsigned char filterModulationSpeed;
	unsigned char waveLen;
	unsigned char attackLen;
	unsigned char attackVol;
	unsigned char decayLen;
	unsigned char decayVol;
	unsigned char sustainLen;
	unsigned char releaseLen;
	unsigned char releaseVol;
	unsigned char filterLowerLimit;
	unsigned char vibratoDelay;
	unsigned char vibratoDepth;
	unsigned char vibratoSpeed;
	unsigned char hardCut;
	int bReleaseCut;
	unsigned char squareModLowerLimit;
	unsigned char squareModUpperLimit;
	unsigned char squareModSpeed;
	unsigned char filterModUpperLimit;
	unsigned char speed;
	unsigned char len;
	struct AHX_WAVETABLE *playList;
};

struct AHX_WAVETABLE
{
	unsigned char waveform;
	int bFixNote;
	unsigned char fx1Command; 
	unsigned char fx2Command;
	unsigned char noteData;
	unsigned char fx1Data;
	unsigned char fx2Data;
};

struct AHX_MODULE
{
	struct AHX_HEADER header;
	struct AHX_SONGSEQ *sequence;
	unsigned int *subSongs;
	struct AHX_TRACKSTEP *tracks;
	struct AHX_SAMPLE *samples;
};

void readAHXHeader(unsigned char *songBuffer, struct AHX_MODULE *ahxMod);
void readAHXSubSongs(unsigned char *songBuffer, struct AHX_MODULE *ahxMod);
void readAHXTrackSequence(unsigned char*songBuffer, struct AHX_MODULE *ahxMod);
void readAHXTracks(unsigned char *songBuffer, struct AHX_MODULE *ahxMod);
void readAHXSamples(unsigned char *songBuffer, struct AHX_MODULE *ahxMod);
void printAHXHeaderInfo(struct AHX_MODULE *ahxMod);
void printAHXSamples(struct AHX_MODULE *ahxMod);
void printAHXSequence(struct AHX_MODULE *ahxMod);
struct AHX_MODULE *loadAHX(char *strFilename);
void freeAhxModule(struct AHX_MODULE *ahxMod);