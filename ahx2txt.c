
/*****************************************************************/
//	ahx2txt [-i][-s] <ahxFile>
//	Dumps the ahx file content into readeable ascii
//	neuroflip/303bcn
//	based on http://xmms-ahx.sourceforge.net/
//	ahx file info: http://lclevy.free.fr/exotica/ahx/ahxformat.txt
/*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "ahx2txt.h"

char *NoteTable[61] = 
{
	"---",
	"C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
	"C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
	"C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
	"C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
	"C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5"
};

int parserPtr = 14;
int songLength = 0;

/*****************************************************************/
/** readAHXHeader: fill the AHX_MODULE.AHX_HEADER information   **/
/*****************************************************************/

void readAHXHeader(unsigned char *songBuffer, struct AHX_MODULE *ahxMod)
{
	printf("·Parsing module header ...\n");

  	if(songBuffer[0]!='T' && songBuffer[1]!='H' && songBuffer[2]!='X')
	{
		printf("IDHEADER: File read error\n");
  	}
	else
	{
		if((songBuffer[0]!=0x54)||(songBuffer[1]!=0x48)||
			(songBuffer[2]!=0x58)||(songBuffer[3]!=0x01)) 
            ahxMod->header.bIs20ModuleFormat=0;
		else if((songBuffer[0]==0x54)&&(songBuffer[1]==0x48)&&
			(songBuffer[2]==0x58)&&(songBuffer[3]==0x01))
			ahxMod->header.bIs20ModuleFormat=1;
	}
	
	//string offset + ahx mod name
    ahxMod->header.namesPtr = (char *) &songBuffer[(songBuffer[4]<<8) | songBuffer[5]];
    ahxMod->header.ahxName = malloc(strlen(ahxMod->header.namesPtr)+1);
    strcpy(ahxMod->header.ahxName, ahxMod->header.namesPtr);
    ahxMod->header.namesPtr += strlen(ahxMod->header.namesPtr)+1;

    ahxMod->header.totalLen = ((songBuffer[6] & 0xF)<<8) | songBuffer[7];
    ahxMod->header.resPoint = (songBuffer[8]<<8) | songBuffer[9];
    ahxMod->header.trackLen = songBuffer[10];
    ahxMod->header.totalTracks = songBuffer[11];
    ahxMod->header.totalSamples = songBuffer[12];
    ahxMod->header.totalSubSongs = songBuffer[13];
    ahxMod->header.bTrack0 = ((songBuffer[6] & 0x80) == 0x80);

/*
    //CHECK VERSION//CIA TIMMING && track 0 usage
	{
		if((btmp & 0xa000)==0xa000) 
		{
			printf("TRACK0: track0 is saved\n");
			ahxMod->header.bTrack0=1;
		}
		else
		{
			printf("TRACK0: track0 is NOT saved\n");
			ahxMod->header.bTrack0=0;
		}
//		btmp=(btmp>>4)%111; //DOC AHX 
//		btmp=((btmp>>5)&3)+1; //xmms-ahx
		btmp=(btmp>>6)+1; //deduced
		printf("CIA TIMING: %dHz\n", 50*btmp);
		ahxMod->header.cia=(int)btmp;
	}
*/
}

/*****************************************************************/
/** readAHXSubSongs: fill the subsong info 						**/
/*****************************************************************/

void readAHXSubSongs(unsigned char *songBuffer, struct AHX_MODULE *ahxMod)
{
	int i;
	unsigned int wtmp;
	
	printf("·Parsing module SubSongs info ... \n");

	if (ahxMod->header.totalSubSongs>0)
	{
		ahxMod->subSongs = malloc(ahxMod->header.totalSubSongs * sizeof(unsigned int));
		for(i=0; i<ahxMod->header.totalSubSongs;i++)
		{
			ahxMod->subSongs[i]=songBuffer[parserPtr];
            if(parserPtr>songLength)
            {
                printf("Error: end of file reached! AHX parse error\n");
                exit(-4);
            }
            parserPtr+=2;
		}
	}
}

/*****************************************************************/
/** readAHXTrackSequence: fill the track sequence info			**/
/*****************************************************************/

void readAHXTrackSequence(unsigned char*songBuffer, struct AHX_MODULE *ahxMod)
{
	int i=0;

	printf("·Parsing sequencer info ...\n");
	if(ahxMod->header.totalLen > 0)
	{
		//allocate the sequence memory
		ahxMod->sequence=malloc(ahxMod->header.totalLen * sizeof(struct AHX_SONGSEQ));
		for(i=0;i<ahxMod->header.totalLen;i++)
		{
           if(parserPtr>songLength)
           {
               printf("Error: end of file reached! AHX parse error\n");
               exit(-4);
            }

			//read the track
			ahxMod->sequence[i].ch1Track = songBuffer[parserPtr];
			ahxMod->sequence[i].ch1Transp = songBuffer[parserPtr+1];

			ahxMod->sequence[i].ch2Track = songBuffer[parserPtr+2];
			ahxMod->sequence[i].ch2Transp = songBuffer[parserPtr+3];

			ahxMod->sequence[i].ch3Track = songBuffer[parserPtr+4];
			ahxMod->sequence[i].ch3Transp = songBuffer[parserPtr+5];

			ahxMod->sequence[i].ch4Track = songBuffer[parserPtr+6];
			ahxMod->sequence[i].ch4Transp = songBuffer[parserPtr+7];

			parserPtr += 8;
		}
	}
	else printf("Sequence: no tracks sequenced\n");
}

/*****************************************************************/
/** readAHXTracks: fill the track patterns 						**/
/*****************************************************************/

void readAHXTracks(unsigned char *songBuffer, struct AHX_MODULE *ahxMod)
{
	int i=0;

	printf("·Parsing tracks (%d tracks)\n", ahxMod->header.totalTracks);

	if(ahxMod->header.totalTracks>0)
	{
		//allocate the sequence memory
		ahxMod->tracks=malloc((ahxMod->header.totalTracks+1) * 
			ahxMod->header.trackLen * sizeof(struct AHX_TRACKSTEP));

		for(i=0;i<((ahxMod->header.totalTracks+1)*ahxMod->header.trackLen);i++)
		{
			if(parserPtr>songLength)
			{
				printf("Error: end of file reached! AHX parse error\n");
				exit(-4);
			}
			if(((i>=0) && (i<ahxMod->header.trackLen)) && (ahxMod->header.bTrack0))
			{
				ahxMod->tracks[i].note = 0x0;
				ahxMod->tracks[i].sample = 0x0;
				ahxMod->tracks[i].command = 0x0;
				ahxMod->tracks[i].param = 0x0;
			}
			else
			{
				//read the track
				ahxMod->tracks[i].note=(songBuffer[parserPtr]>>2) & 0x3F;  //Note
				ahxMod->tracks[i].sample=((songBuffer[parserPtr] & 0x3)<<4) | 
					(songBuffer[parserPtr+1]>>4); //Sample
				ahxMod->tracks[i].command=(songBuffer[parserPtr+1] & 0xF);  //Command
				ahxMod->tracks[i].param=songBuffer[parserPtr+2];     //Command param

				parserPtr +=3;
			}
		}
	}
	else printf("Sequence: no tracks sequenced\n");
}

/*****************************************************************/
/** readAHXSamples: fill the samples structure					**/
/*****************************************************************/

void readAHXSamples(unsigned char *songBuffer, struct AHX_MODULE *ahxMod)
{
	int i, j;
	
	printf("·Parsing samples (%d samples) \n·fileLength:%d\n",
		ahxMod->header.totalSamples, songLength);
	
	if(ahxMod->header.totalSamples>0)
	{
		ahxMod->samples = malloc(sizeof(struct AHX_SAMPLE)*(ahxMod->header.totalSamples+2));

		for(i=1;i<ahxMod->header.totalSamples+1;i++)
		{
		if(parserPtr>songLength)
		{
			printf("Error: end of file reached! AHX parse error\n");
			exit(-4);
		}			
			else
			{
				ahxMod->samples[i].Name = malloc(sizeof(char)*(strlen(ahxMod->header.namesPtr)+1));

				sprintf(ahxMod->samples[i].Name,"%s",ahxMod->header.namesPtr);
				memcpy(ahxMod->samples[i].Name, ahxMod->header.namesPtr,strlen(ahxMod->header.namesPtr));
				strcpy(ahxMod->samples[i].Name,ahxMod->header.namesPtr);

				ahxMod->header.namesPtr += strlen(ahxMod->header.namesPtr)+1;
				ahxMod->samples[i].filterModulationSpeed=songBuffer[parserPtr+1]&0xF8;
				ahxMod->samples[i].waveLen=songBuffer[parserPtr+1]&0x7;
				ahxMod->samples[i].attackLen=songBuffer[parserPtr+2];
				ahxMod->samples[i].attackVol=songBuffer[parserPtr+3];
				ahxMod->samples[i].decayLen=songBuffer[parserPtr+4];
				ahxMod->samples[i].decayVol=songBuffer[parserPtr+5];
				ahxMod->samples[i].sustainLen=songBuffer[parserPtr+6];
				ahxMod->samples[i].releaseLen=songBuffer[parserPtr+7];
				ahxMod->samples[i].releaseVol=songBuffer[parserPtr+8];
				ahxMod->samples[i].filterLowerLimit=songBuffer[parserPtr+12]&0x7f;
				ahxMod->samples[i].vibratoDelay=songBuffer[parserPtr+13];
				ahxMod->samples[i].hardCut=(songBuffer[parserPtr+14]>>4)&7;
				ahxMod->samples[i].bReleaseCut=(songBuffer[parserPtr+14]>>7);
				ahxMod->samples[i].vibratoDepth=(songBuffer[parserPtr+14]&0x0f);
				ahxMod->samples[i].vibratoSpeed=songBuffer[parserPtr+15];
				ahxMod->samples[i].squareModLowerLimit=songBuffer[parserPtr+16];
				ahxMod->samples[i].squareModUpperLimit=songBuffer[parserPtr+17];
				ahxMod->samples[i].squareModSpeed=songBuffer[parserPtr+18];
				ahxMod->samples[i].filterModUpperLimit=songBuffer[parserPtr+19]&0x3f;
				ahxMod->samples[i].speed=songBuffer[parserPtr+20];
				ahxMod->samples[i].len=songBuffer[parserPtr+21];
				parserPtr +=22;

				ahxMod->samples[i].playList = malloc(sizeof(struct AHX_WAVETABLE)*ahxMod->samples[i].len);
				for(j=0;j<ahxMod->samples[i].len;j++)
				{
					if(parserPtr>songLength)
					{
						printf("Error: end of file reached! AHX parse error\n");
						exit(-4);
					}      

					ahxMod->samples[i].playList[j].fx2Command=(songBuffer[parserPtr]>>5)&0x7;
					ahxMod->samples[i].playList[j].fx1Command=((songBuffer[parserPtr]&0x10)>>2)|
						((songBuffer[parserPtr]&0xC)>>2);
					ahxMod->samples[i].playList[j].waveform=((songBuffer[parserPtr] << 1) & 6) | 
						(songBuffer[parserPtr+1] >> 7);
					ahxMod->samples[i].playList[j].bFixNote=(songBuffer[parserPtr+1]>>6)&0x1;
					ahxMod->samples[i].playList[j].noteData=songBuffer[parserPtr+1]&0x3f;
					ahxMod->samples[i].playList[j].fx1Data=songBuffer[parserPtr+2];
					ahxMod->samples[i].playList[j].fx2Data=songBuffer[parserPtr+3];
					
					parserPtr +=4;
				}
			}
		}
	}
	else printf("Samples: no samples included\n");
}

/*****************************************************************/
/** printAHXHeaderInfo: print the header info 					**/
/*****************************************************************/

void printAHXHeaderInfo(struct AHX_MODULE *ahxMod)
{
	printf("\n=================================================\n");
	printf("Module Name: %s\n",ahxMod->header.ahxName);
	printf("=================================================\n");

	if(!ahxMod->header.bIs20ModuleFormat)
		printf("IDHEADER: AXH1.00 or AHX1.27 Module format\n");
	else
		printf("IDHEADER: AXH2.0 Module format\n");
	printf("length = %d\n",ahxMod->header.totalLen);
	printf("reset point = %d\n",ahxMod->header.resPoint);
	printf("track length = %d\n",ahxMod->header.trackLen);
	printf("patterns = %d\n",ahxMod->header.totalTracks);
	printf("samples = %d\n",ahxMod->header.totalSamples);
	printf("subSongs = %d\n",ahxMod->header.totalSubSongs);
	printf("track 0 is saved = %d\n",ahxMod->header.bTrack0);
}

/*****************************************************************/
/** printAHXSamples: print the samples info						**/
/*****************************************************************/

void printWaveLen(value) {
	switch(value) {
		case 0: printf("WaveLen: 04\n"); break;
		case 1: printf("WaveLen: 08\n"); break;
		case 2: printf("WaveLen: 10\n"); break;
		case 3: printf("WaveLen: 20\n"); break;
		case 4: printf("WaveLen: 40\n"); break;
		case 5: printf("WaveLen: 80\n"); break;
	}
}

void printAHXSamples(struct AHX_MODULE *ahxMod)
{
	int i, j;

	printf("\n=================================================\n");
	printf("INSTRUMENt LiST\n");
	printf("=================================================\n");

	for(i=1;i<ahxMod->header.totalSamples+1;i++)
	{
		printf("\n=================================================\n");
		printf("Sample #%d Name:%s\n",i,ahxMod->samples[i].Name);
		printf("=================================================\n");
		printf("Master Vol: %02X\n", ahxMod->samples[i].masterVol);
		printWaveLen(ahxMod->samples[i].waveLen);
		
		printf("Attack Length %02d\n", ahxMod->samples[i].attackLen);
		printf("Attack Volume %02d\n", ahxMod->samples[i].attackVol);
		printf("Decay Length %02d\n", ahxMod->samples[i].decayLen);
		printf("Decay Volume %02d\n", ahxMod->samples[i].decayVol);
		printf("Sustain Length %02d\n", ahxMod->samples[i].sustainLen);
		printf("Release Length %02d\n", ahxMod->samples[i].releaseLen);
		printf("Release Volume %02d\n\n", ahxMod->samples[i].releaseVol);

		printf("Vibrato Delay %02d\n", ahxMod->samples[i].vibratoDelay);
		printf("Vibrato Depth %02d\n", ahxMod->samples[i].vibratoDepth);
		printf("Vibrato Speed %02d\n", ahxMod->samples[i].vibratoSpeed);
		printf("Square Modulation Lower Limit %02d \n",ahxMod->samples[i].squareModLowerLimit);
		printf("Square Modulation Upper Limit %02d \n", ahxMod->samples[i].squareModUpperLimit);
		printf("Square Modulation Speed %02d\n", ahxMod->samples[i].squareModSpeed);
		printf("Filter Modulation Lower Limit %02d\n", ahxMod->samples[i].filterLowerLimit);
		printf("Filter Modulation Upper Limit %02d\n", ahxMod->samples[i].filterModUpperLimit);
		printf("Filter Modulation Speed: %02X\n\n", ahxMod->samples[i].filterModulationSpeed);

		printf("Hard Cut %02d\n", ahxMod->samples[i].hardCut);
		printf("Release Cut %02d\n\n", ahxMod->samples[i].bReleaseCut);

		printf("\n* WAVETABLE: in cmd 6=C and 7=F\n");
		printf("Speed %02d\n",ahxMod->samples[i].speed);
		printf("Length %02d\n",ahxMod->samples[i].len);

		for(j=0;j<ahxMod->samples[i].len;j++)
		{
			printf("%d: %s",j,NoteTable[ahxMod->samples[i].playList[j].noteData]);
			printf("%s",(ahxMod->samples[i].playList[j].bFixNote ? "*" : " "));
			printf(" %d %X%02X %X%02X\n",
				ahxMod->samples[i].playList[j].waveform, 
				ahxMod->samples[i].playList[j].fx1Command, 
				ahxMod->samples[i].playList[j].fx1Data,
				ahxMod->samples[i].playList[j].fx2Command, 
				ahxMod->samples[i].playList[j].fx2Data);
		}
	}
}

/*****************************************************************/
/** printAHXSequence: print the tracks sequenced				**/
/*****************************************************************/

void printAHXSequence(struct AHX_MODULE *ahxMod)
{

	int i,j;

	printf("\n=================================================\n");
	printf("SEQUENCeD TRACKs\n");
	printf("=================================================\n");
	
	for(i=0;i<ahxMod->header.totalLen;i++)
	{
			printf("\n%02d:[%02d-%02d][%02d-%02d][%02d-%02d][%02d-%02d]===================\n\n", i, 
			ahxMod->sequence[i].ch1Track, ahxMod->sequence[i].ch1Transp, ahxMod->sequence[i].ch2Track, 
			ahxMod->sequence[i].ch2Transp, ahxMod->sequence[i].ch3Track, ahxMod->sequence[i].ch3Transp, 
			ahxMod->sequence[i].ch4Track, ahxMod->sequence[i].ch4Transp);

		for(j=0;j<(ahxMod->header.trackLen);j++)
		{
			printf("%02d: %s %02d %X%02X  %s %02d %X%02X  %s %02d %X%02X  %s %02d %X%02X\n", j,
			NoteTable[ahxMod->tracks[(ahxMod->sequence[i].ch1Track*ahxMod->header.trackLen)+j].note],
			ahxMod->tracks[(ahxMod->sequence[i].ch1Track*ahxMod->header.trackLen)+j].sample,
			ahxMod->tracks[(ahxMod->sequence[i].ch1Track*ahxMod->header.trackLen)+j].command,
			ahxMod->tracks[(ahxMod->sequence[i].ch1Track*ahxMod->header.trackLen)+j].param,
			//ahxMod->tracks[(ahxMod->sequence[i].ch2Track*ahxMod->header.trackLen)+j].note,
			NoteTable[ahxMod->tracks[(ahxMod->sequence[i].ch2Track*ahxMod->header.trackLen)+j].note],
			ahxMod->tracks[(ahxMod->sequence[i].ch2Track*ahxMod->header.trackLen)+j].sample,
			ahxMod->tracks[(ahxMod->sequence[i].ch2Track*ahxMod->header.trackLen)+j].command,
			ahxMod->tracks[(ahxMod->sequence[i].ch2Track*ahxMod->header.trackLen)+j].param,

			NoteTable[ahxMod->tracks[(ahxMod->sequence[i].ch3Track*ahxMod->header.trackLen)+j].note],
			ahxMod->tracks[(ahxMod->sequence[i].ch3Track*ahxMod->header.trackLen)+j].sample,
			ahxMod->tracks[(ahxMod->sequence[i].ch3Track*ahxMod->header.trackLen)+j].command,
			ahxMod->tracks[(ahxMod->sequence[i].ch3Track*ahxMod->header.trackLen)+j].param,

			NoteTable[ahxMod->tracks[(ahxMod->sequence[i].ch4Track*ahxMod->header.trackLen)+j].note],
			ahxMod->tracks[(ahxMod->sequence[i].ch4Track*ahxMod->header.trackLen)+j].sample,
			ahxMod->tracks[(ahxMod->sequence[i].ch4Track*ahxMod->header.trackLen)+j].command,
			ahxMod->tracks[(ahxMod->sequence[i].ch4Track*ahxMod->header.trackLen)+j].param);/*,
			(ahxMod->sequence[i].ch3Track*ahxMod->header.trackLen)+j,ahxMod->sequence[i].ch3Track,
			ahxMod->tracks[(ahxMod->sequence[i].ch3Track*ahxMod->header.trackLen)+j].note);*/
		}
    }
}

/*****************************************************************/
/** loadAHX: load the ahx module into memory					**/
/*****************************************************************/

struct AHX_MODULE *loadAHX(char *strFilename)
{
	struct AHX_MODULE *ahxMod;    
	FILE *fp;
	unsigned char songBuffer[65536];

	//Open the file
	if((fp=fopen(strFilename, "r"))==NULL)
	{
		printf("Can't load AHX file\n");
		exit(-2);
	}

	//read the entire file
	fseek(fp,0,SEEK_SET);
	songLength = fread(songBuffer, 1, 65536, fp);
	if((songLength < 14) || (songLength>=65536))
	{
		printf("AHX file corrupted?");
		exit(-3);
	}
	fclose(fp);
	//parse ahx buffer
	ahxMod = malloc(sizeof(struct AHX_MODULE));
	readAHXHeader(songBuffer, ahxMod);
	readAHXSubSongs(songBuffer, ahxMod);
	readAHXTrackSequence(songBuffer, ahxMod);
    readAHXTracks(songBuffer, ahxMod);
	readAHXSamples(songBuffer, ahxMod);

	return ahxMod;
}

void freeAhxModule(struct AHX_MODULE *ahxMod)
{
	int i=0;
	
	free(ahxMod->sequence);
	free(ahxMod->tracks);
	for(i=0;i<sizeof(ahxMod->samples);i++)
	{
		free(ahxMod->samples[i].playList);
		free(ahxMod->samples[i].Name);
	}
	free(ahxMod->samples);
	free(ahxMod);
}

void printHelp()
{
	printf("ahx2txt [options] <filename>\n");
	printf("-i Dump samples\n");
	printf("-s Dump pattern sequence\n");
}

/****************************************************************/
/**														MAiN   **/
/****************************************************************/

int main(int argc, char *argv[])
{
	struct AHX_MODULE *ahxModule;		
	int userWantsSamples=0;
	int userWantsSequence=0;

	printf("\nahx2txt by 303bcn...\n");
	if(argc==2)
	{
		ahxModule=loadAHX(argv[1]);
		userWantsSequence = 1;
		userWantsSamples = 1;
	}
	else if(argc==3)
	{
		if(!strcmp(argv[1],"-i"))
			userWantsSamples = 1;
		else if(!strcmp(argv[1],"-s"))
			userWantsSequence = 1;
		else
		{
			printHelp();
			exit(-1);
		} 
		ahxModule=loadAHX(argv[2]);
	}
	else if(argc==4)
	{
		if(((!strcmp(argv[1],"-i")) && (!strcmp(argv[2],"-s"))) ||
			((!strcmp(argv[1],"-s")) && (!strcmp(argv[2],"-i"))))
		{
			userWantsSequence = 1;
			userWantsSamples = 1;
		}
		else
		{
			printHelp();
			exit(-1);
		} 
		ahxModule=loadAHX(argv[3]);
	}
	else
	{
		printHelp();
		exit(-1);
	}

	printAHXHeaderInfo(ahxModule);
	if(userWantsSequence)
		printAHXSequence(ahxModule);
	if(userWantsSamples)
		printAHXSamples(ahxModule);

	freeAhxModule(ahxModule);
	return 0;
}
