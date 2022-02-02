/* MIDIMAP.H                               *
 * Some globals for translating MIDI data  *
 * kuneho 2022                             *
 * https://furryhu.org/                    */

#define MIDI_CMD_NOTE_OFF              0x80		// Note OFF						 1000 0000
#define MIDI_CMD_NOTE_ON               0x90		// Note ON						 1001 0000
#define MIDI_CMD_AFTERTOUCH            0xA0		// Aftertouch					 1010 0000
#define MIDI_CMD_CONTINUOUS_CONTROLLER 0xB0		// CC							 1011 0000
#define MIDI_CMD_PROGRAM_CHANGE 	   0xC0		// PC							 1100 0000
#define MIDI_CMD_CHANNEL_PRESSURE      0xD0		// Channel pressure				 1101 0000
#define MIDI_CMD_PITCH_BEND 		   0xE0		// Pitch bend					 1110 0000
#define MIDI_CMD_SYSTEM 			   0xF0		// Reserved for system messages	 1111 0000