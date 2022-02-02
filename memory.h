/* MEMORY.H                                *
 * Settings                                *
 * kuneho 2022                             *
 * https://furryhu.org/                    */
 
#define MEM_MAX 65
#define SETTINGINDEX_CHANNEL_PIANOKEYS	8
#define SETTINGINDEX_CHANNEL_CONTROL	9
#define SETTINGINDEX_PC_PU1				10
#define SETTINGINDEX_PC_PU2				11
#define SETTINGINDEX_PC_WAV				12
#define SETTINGINDEX_PC_NOI				13
#define SETTINGINDEX_PC_POLY			14
unsigned char SETTINGS_DEFAULT[MEM_MAX] = {
	'k', 'u', 'n', 'e', 'm', 'i', 'd', 'i',			// memory check	0-7
	0,												// PianoKeys channel
	9,												// MIDI channel used for control messages
	//  PU1 PU2 WAV NOI POLY						   PC# to change mapped channel to PianoKeys
		0,  1,  2,  3,  4
	
};
unsigned char SETTINGS_CURRENT[MEM_MAX];