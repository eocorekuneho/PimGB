/* MAIN.H                                    *
 * Main file of PMGB.                        *
 * Basic, draft like program for interfacing *
 * with Game Boy via serial link, sending    *
 * MIDI data, to use with ROM mGB            *
 * kuneho 2022                               *
 * https://furryhu.org/                      */
 
#include "pins.h"
#include "midimap.h"
#include "memory.h"

#include <wiringPi.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>	
#include <fcntl.h>	
#include <stdlib.h>
#include <string.h>


// todo: ezt inkább valahogy szebben kéne előszedni /dev/snd/ alól
#define MIDI_DEVICE    "/dev/midi1"
#define GB_MIDI_DELAY  500

#define NEED_CPU_CYCLE_DELAY true
#define CPU_CYCLE_DELAY	1

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

int g_fdMIDI = -1;								// MIDI eszköz
unsigned char g_midiData;						// Bejövő MIDI bájt
unsigned char g_midiStatusChannel;				// Kimenő MIDI csatorna
unsigned char g_midiStatusType;					// Kimenő MIDI parancs
unsigned char g_midiSelectedInstrument = 0;		// Kiválasztott csatorna, amit küldünk ki
bool g_midiAddressMode   = false;				// Kimenő MIDI parancs első argumentumát dolgozzuk fel? (Cím)
bool g_midiValueMode     = false;				// Kimenő MIDI parancs második argumentumát dolgozzuk fel? (Érték)
bool g_midiControlMode   = false;				// Rendszervezérlés történik

// Game Boy -és MIDI csatornaösszerendelés
unsigned char g_midiChannelSettings[5] = {
	0, // PU1 MIDI csatorna (1)
	1, // PU2 MIDI csatorna (2)
	2, // WAV MIDI csatorna (3)
	3, // NOI MIDI csatorna (4)
	4  // POLY MIDI csatorna (5)
};
// A kimenő MIDI üzenet (3 bájt)
unsigned char g_midiDataOut[3] = {
	0, 0, 0
};


int InitGPIO(){
	// GPIO pinek inicializálása
	wiringPiSetup();
	pinMode(GB_SOUT_2, INPUT);
	pinMode(GB_SIN_3,  OUTPUT);
	pinMode(GB_SCK_5,  OUTPUT);
	// Beállítjuk őket ALACSONY állásra
	digitalWrite(GB_SOUT_2, LOW);
	digitalWrite(GB_SIN_3,  LOW);	
	// GB CLK-t magasra állítjuk
    digitalWrite(GB_SCK_5,   HIGH);
}

int InitMemory(){
	/* itt majd lesz egy kis fájl IO, de majd később */
	memcpy(SETTINGS_CURRENT, SETTINGS_DEFAULT, sizeof(SETTINGS_DEFAULT));
printf("OK");	
}

void GB_SET(char SCLK, char SIN, char SOUT){
	digitalWrite(GB_SIN_3,  SIN);
	digitalWrite(GB_SCK_5,  SCLK);
}


void SendByteToGameboy(unsigned char byte_to_send){
	int countTicks;
printf("Sending data to gameboy one-by-one: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(byte_to_send));

	for(countTicks = 0; countTicks != 8; countTicks++){	
		if(byte_to_send & 0x80){
			/* egyest küldünk */
//				   CLK   j			
			GB_SET(LOW,  HIGH, LOW);
			GB_SET(HIGH, HIGH, LOW);
		} else {
			/* nullást küldünk */
//				   CLK   jel			
			GB_SET(LOW,  LOW, LOW);
			GB_SET(HIGH, LOW, LOW);
		}
		
		// lassuljunk le egy kicsit, ha kell
		if(NEED_CPU_CYCLE_DELAY) usleep(CPU_CYCLE_DELAY);
		byte_to_send <<= 1;
	}
}

int main(){
	
	InitGPIO();
	InitMemory();
	
	// Megnyitjuk a MIDI eszközt
	g_fdMIDI = open(MIDI_DEVICE, 'r');
	if (g_fdMIDI == -1) {
		printf("Error: cannot open %s\n", MIDI_DEVICE);
		exit(1);
	}
	
	bool sendByte = false;
	// Fő loop
	while (1) {
		// Olvasunk egyet a MIDI eszközből
		read(g_fdMIDI, &g_midiData, sizeof(g_midiData));
printf("-----\nIncoming byte: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(g_midiData));		
		// Ez elméletileg egy bájt lesz (8 bit)
		// Kell némi vizsgálat, hogy valóban olyan infót olvasunk, ami nekünk szólhat
		if(g_midiData & 0x80){
			// A legmagasabb bit az 1-es
			// Megnézzük a felső 4 bitet, azzal mizu
			unsigned char midiCommand = g_midiData & 0xF0;
			if(midiCommand == MIDI_CMD_SYSTEM ){
				// 1111XXXX
				// MIDI rendszerüzenet.
				g_midiValueMode = false;
			} else {
				sendByte = false;	
				// alsó 4 bit, csatorna 0-127
				g_midiStatusChannel = g_midiData & 0x0F;
				// felső 4 bit, parancs, lsd.: https://www.midi.org/specifications-old/item/table-1-summary-of-midi-message
				g_midiStatusType    = g_midiData & 0xF0;
printf("MIDI command: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(g_midiStatusType));
printf("MIDI channel: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(g_midiStatusChannel));
				
				/*
					és akkor ide kell majd valami logika, hogy egyszerre megszólaltassunk több csatornát is.
					esetleg figyelni NOTE_ON és NOTE_OFF üzeneteket és tologatni a kiküldött csatornát ezeknek megfelelően.
					de ez így önmagában eléggé butuska lenne, szóval ki kell találni, hogy milyen funkciókat lehetne
					beletenni úgy, amit esetleg a MIDI bemeneti eszközről ki is lehet adni
					Pl.:
					- Különböző csatornákat különböző oktávtartományokba helyezni
					- Bevezetni egy vezérlőcsatornát (pl. 10) és definiálni különböző vezérlőüzeneteket
						o Tekerentyűk és billentyű csatorna dinamikus állítása a Pi-ben
							PC0#10	- PU1
							PC1#10	- PU2
							PC2#10	- WAV
							PC3#10	- NOI
							PC4#10	- POLY
							Figyelni kell arra, hogy NEM CSAK a billentyűk csatornáját kell ilyenkor csapkodni
								Csatornánként van, hogy különböző paraméterekre u.a. a CC van definiálva, más értékekkel
					- Szoftveres arpeggiator, akkordépítés...
					- Dobgép
					- Pi-vel esetleg felvenni és lejátszani makrókat, előre rögzített/rögzíthető szekvenciákat
					- MIDI fájlok?
				*/
				
				
				/* Figyeljük a 10-es csatornát. Ott kaphatunk infót, hogy mit kell mikmókolni */
				if(g_midiStatusChannel == SETTINGS_CURRENT[SETTINGINDEX_CHANNEL_CONTROL]){
printf("System Control message");					
					/* Nézzük, milyen utasítást kapunk */
					// PROGRAM CHANGE
					if(g_midiStatusType == MIDI_CMD_PROGRAM_CHANGE){
printf("  Program Change");						
						sendByte = false;
						g_midiControlMode = true;
						g_midiValueMode   = false;
						g_midiAddressMode = false;
					}
				} else if(g_midiStatusChannel == SETTINGS_CURRENT[SETTINGINDEX_CHANNEL_PIANOKEYS]){
printf("MIDI channel is: %d\n", g_midiSelectedInstrument);
					g_midiDataOut[0] = g_midiStatusType + g_midiSelectedInstrument;
					sendByte = true;
				} else {
					g_midiControlMode = false;
					g_midiValueMode   = false;
					g_midiAddressMode = false;
				}
							
printf("OUTPUT byte is set to: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(g_midiDataOut[0]));
printf("Is it Value Mode? %s\n", ((g_midiValueMode) ? "YES" : "NO"));
printf("Is it Address Mode? %s\n", ((g_midiAddressMode) ? "YES" : "NO"));
printf("Are we going to send anything? %s\n", ((sendByte) ? "YES" : "NO"));

				if(sendByte){
					// parancs + csatorna					
					SendByteToGameboy(g_midiDataOut[0]);
					usleep(GB_MIDI_DELAY);
					g_midiValueMode = false;
					g_midiAddressMode = true;
				}
			}
		} else if(g_midiAddressMode){
			g_midiAddressMode = false;
			g_midiValueMode = true;
			g_midiDataOut[1] = g_midiData;
			SendByteToGameboy(g_midiDataOut[1]);
			usleep(GB_MIDI_DELAY);
		} else if(g_midiValueMode){
			g_midiDataOut[2] = g_midiData;
			g_midiAddressMode = true;
			g_midiValueMode = false;
			SendByteToGameboy(g_midiDataOut[2]);
			usleep(GB_MIDI_DELAY);
		} else if(g_midiControlMode){
printf("PC to: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(g_midiData));		
			g_midiSelectedInstrument = g_midiData;
			g_midiControlMode = false;
			g_midiAddressMode = false;
			g_midiValueMode = false;
		}
	}
}