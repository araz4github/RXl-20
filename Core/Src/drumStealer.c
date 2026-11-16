/*
 * drumStealer.c
 *
 * RXl-0 Drum Stealer, see drumStealer.h.
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include "drumStealer.h"
#include "Parameters.h"
#include "menu/menu.h"
#include "frontPanelParser_ui.h"
#include "front.h"
#include "ledHandler.h"
#include "buttonHandler.h"
#include "Preset/PresetManager.h"
#include "Soundengine/DSPAudio/mixer.h"
#include "Soundengine/MIDI/MidiVoiceControl.h"
#include "Soundengine/TriggerProcessor.h"
#include <string.h>

volatile uint8_t drumStealer_active = 0;
volatile uint8_t drumStealer_target = 0;

#define DS_VOICES		7
#define DS_NO_PAR		0xffff

// ---------------------------------------------------------------------------
// The sound of a voice: every parameter that is copied, in the same order for
// voices of the same kind, so role i of one voice matches role i of another.
// Not here (kept from the target kit): volume, pan, output, reverb send, choke,
// LFO, velocity and CV modulation, glide/legato and the voice link.
// ---------------------------------------------------------------------------
#define DS_DRUM_ROLES	26
static const uint16_t dsDrum[3][DS_DRUM_ROLES] =
{
	{
		PAR_COARSE1, PAR_FINE1, PAR_OSC_WAVE_DRUM1, PAR_PWM_1,
		PAR_VELOA1, PAR_VELOD1, PAR_VOL_SLOPE1, PAR_LXR0_TRIM_START1, PAR_LXR0_TRIM_END1,
		PAR_MOD_EG1, PAR_PITCH_SLOPE1, PAR_MODAMNT1, PAR_VOLUME_MOD_ON_OFF1,
		PAR_FMAMNT1, PAR_FM_FREQ1, PAR_MOD_WAVE_DRUM1, PAR_MIX_MOD_1,
		PAR_TRANS1_WAVE, PAR_TRANS1_VOL, PAR_TRANS1_FREQ,
		PAR_FILTER_FREQ_1, PAR_RESO_1, PAR_FILTER_TYPE_1, PAR_FILTER_DRIVE_1,
		PAR_VOICE_DECIMATION1, PAR_DRIVE1
	},
	{
		PAR_COARSE2, PAR_FINE2, PAR_OSC_WAVE_DRUM2, PAR_PWM_2,
		PAR_VELOA2, PAR_VELOD2, PAR_VOL_SLOPE2, PAR_LXR0_TRIM_START2, PAR_LXR0_TRIM_END2,
		PAR_MOD_EG2, PAR_PITCH_SLOPE2, PAR_MODAMNT2, PAR_VOLUME_MOD_ON_OFF2,
		PAR_FMAMNT2, PAR_FM_FREQ2, PAR_MOD_WAVE_DRUM2, PAR_MIX_MOD_2,
		PAR_TRANS2_WAVE, PAR_TRANS2_VOL, PAR_TRANS2_FREQ,
		PAR_FILTER_FREQ_2, PAR_RESO_2, PAR_FILTER_TYPE_2, PAR_FILTER_DRIVE_2,
		PAR_VOICE_DECIMATION2, PAR_DRIVE2
	},
	{
		PAR_COARSE3, PAR_FINE3, PAR_OSC_WAVE_DRUM3, PAR_PWM_3,
		PAR_VELOA3, PAR_VELOD3, PAR_VOL_SLOPE3, PAR_LXR0_TRIM_START3, PAR_LXR0_TRIM_END3,
		PAR_MOD_EG3, PAR_PITCH_SLOPE3, PAR_MODAMNT3, PAR_VOLUME_MOD_ON_OFF3,
		PAR_FMAMNT3, PAR_FM_FREQ3, PAR_MOD_WAVE_DRUM3, PAR_MIX_MOD_3,
		PAR_TRANS3_WAVE, PAR_TRANS3_VOL, PAR_TRANS3_FREQ,
		PAR_FILTER_FREQ_3, PAR_RESO_3, PAR_FILTER_TYPE_3, PAR_FILTER_DRIVE_3,
		PAR_VOICE_DECIMATION3, PAR_DRIVE3
	}
};

#define DS_SNARE_ROLES	25
static const uint16_t dsSnare[DS_SNARE_ROLES] =
{
	PAR_COARSE4, PAR_FINE4, PAR_NOISE_FREQ1, PAR_MIX1, PAR_OSC_WAVE_SNARE, PAR_PWM_4,
	PAR_VELOA4, PAR_VELOD4, PAR_REPEAT4, PAR_VOL_SLOPE4, PAR_LXR0_TRIM_START4, PAR_LXR0_TRIM_END4,
	PAR_MOD_EG4, PAR_PITCH_SLOPE4, PAR_MODAMNT4, PAR_VOLUME_MOD_ON_OFF4,
	PAR_TRANS4_WAVE, PAR_TRANS4_VOL, PAR_TRANS4_FREQ,
	PAR_FILTER_FREQ_4, PAR_RESO_4, PAR_FILTER_TYPE_4, PAR_FILTER_DRIVE_4,
	PAR_VOICE_DECIMATION4, PAR_SNARE_DISTORTION
};

#define DS_CYM_ROLES	26
static const uint16_t dsCym[DS_CYM_ROLES] =
{
	PAR_COARSE5, PAR_FINE5, PAR_WAVE1_CYM, PAR_PWM_5,
	PAR_VELOA5, PAR_VELOD5, PAR_REPEAT5, PAR_VOL_SLOPE5, PAR_LXR0_TRIM_START5, PAR_LXR0_TRIM_END5,
	PAR_VOLUME_MOD_ON_OFF5,
	PAR_MOD_OSC_F1_CYM, PAR_MOD_OSC_F2_CYM, PAR_MOD_OSC_GAIN1_CYM, PAR_MOD_OSC_GAIN2_CYM, PAR_WAVE2_CYM, PAR_WAVE3_CYM,
	PAR_TRANS5_WAVE, PAR_TRANS5_VOL, PAR_TRANS5_FREQ,
	PAR_FILTER_FREQ_5, PAR_RESO_5, PAR_FILTER_TYPE_5, PAR_FILTER_DRIVE_5,
	PAR_VOICE_DECIMATION5, PAR_CYMBAL_DISTORTION
};

// voice 6 (closed hat) and voice 7 (Golem's open hat). Voice 6's closed decay is the
// decay you hear; the two hats share one sample trim (voice 7's page edits voice 6's).
#define DS_HAT_ROLES	25
#define DS_HAT_DECAY	5
static const uint16_t dsHat[2][DS_HAT_ROLES] =
{
	{
		PAR_COARSE6, PAR_FINE6, PAR_WAVE1_HH, PAR_PWM_6,
		PAR_VELOA6, PAR_VELOD6_CLOSED, PAR_VOL_SLOPE6, PAR_VOLUME_MOD_ON_OFF6,
		PAR_MOD_OSC_F1, PAR_MOD_OSC_F2, PAR_MOD_OSC_GAIN1, PAR_MOD_OSC_GAIN2, PAR_WAVE2_HH, PAR_WAVE3_HH,
		PAR_TRANS6_WAVE, PAR_TRANS6_VOL, PAR_TRANS6_FREQ,
		PAR_FILTER_FREQ_6, PAR_RESO_6, PAR_FILTER_TYPE_6, PAR_FILTER_DRIVE_6,
		PAR_VOICE_DECIMATION6, PAR_HAT_DISTORTION,
		PAR_LXR0_TRIM_START6, PAR_LXR0_TRIM_END6
	},
	{
		PAR_HH7_COARSE, PAR_HH7_FINE, PAR_HH7_WAVE1, PAR_HH7_PWM,
		PAR_HH7_ATTACK, PAR_HH7_DECAY, PAR_HH7_SLOPE, PAR_HH7_VOLUME_MOD,
		PAR_HH7_MOD_OSC_F1, PAR_HH7_MOD_OSC_F2, PAR_HH7_MOD_OSC_GAIN1, PAR_HH7_MOD_OSC_GAIN2, PAR_HH7_WAVE2, PAR_HH7_WAVE3,
		PAR_HH7_TRANS_WAVE, PAR_HH7_TRANS_VOL, PAR_HH7_TRANS_FREQ,
		PAR_HH7_FILTER_FREQ, PAR_HH7_RESO, PAR_HH7_FILTER_TYPE, PAR_HH7_FILTER_DRIVE,
		PAR_HH7_DECIMATION, PAR_HH7_DISTORTION,
		PAR_LXR0_TRIM_START6, PAR_LXR0_TRIM_END6
	}
};

_Static_assert(DS_CYM_ROLES == sizeof(dsCym)/sizeof(dsCym[0]), "dsCym");
_Static_assert(DS_SNARE_ROLES == sizeof(dsSnare)/sizeof(dsSnare[0]), "dsSnare");

static const uint16_t* roles(uint8_t v, uint8_t* n)
{
	switch(v)
	{
	case 0: case 1: case 2:	*n = DS_DRUM_ROLES;	return dsDrum[v];
	case 3:					*n = DS_SNARE_ROLES;	return dsSnare;
	case 4:					*n = DS_CYM_ROLES;	return dsCym;
	case 5: case 6:			*n = DS_HAT_ROLES;	return dsHat[v - 5];
	default:				*n = 0;				return 0;
	}
}

// voice buttons that may be the source for a target (bit n = voice n+1)
static uint8_t selectable(uint8_t target)
{
	if(target < 3)	return 0x07;
	if(target == 3)	return 0x08;
	if(target == 4)	return 0x10;
	return 0x60;
}

static uint16_t outPar(uint8_t v)
{
	return v < 6 ? (uint16_t)(PAR_AUDIO_OUT1 + v) : (uint16_t)PAR_HH7_AUDIO_OUT;
}

// ---------------------------------------------------------------------------
static uint8_t ds_backup[END_OF_SOUND_PARAMETERS];	// the target kit when Drum Stealer opened
static uint8_t ds_kit;								// browsed source kit
static uint8_t ds_held;								// selectable voice buttons held (bit n = voice n+1)
static uint8_t ds_heard;							// last auditioned source voice 1..7, 0 = none

// send one sound parameter the way the pots and encoder do
static void sendPar(uint16_t p)
{
	const uint8_t v = activeParameters[p];
	if(p < 128)
		frontPanel_sendData(MIDI_CC, (uint8_t)p, v);
	else if(p < 255)
		frontPanel_sendData(CC_2, (uint8_t)(p - 128), v);
	else if(p < END_OF_SOUND_PARAMETERS)
		frontPanel_sendData(CC3, (uint8_t)(p - 255), v);
}

static void setPar(uint16_t p, uint8_t v)
{
	activeParameters[p] = v;
	sendPar(p);
}

// put the sound of source voice srcVoice (0..6) of kit ds_kit on the target voice
static void applySound(uint8_t srcVoice)
{
	const uint8_t tgt = drumStealer_target;
	const uint8_t* kit = mainParameters[ds_kit];
	uint8_t nSrc, nTgt;
	const uint16_t* src = roles(srcVoice, &nSrc);
	const uint16_t* dst = roles(tgt, &nTgt);
	if(!src || !dst || nSrc != nTgt) return;

	// a kit saved before voice 7 existed: its voice 7 is a copy of its voice 6 (open decay),
	// the same as when Golem loads such a kit
	uint8_t oldV7 = 0;
	if((srcVoice == 6) && (kit[PAR_HH7_VALID] == 0))
	{
		src = dsHat[0];
		oldV7 = 1;
	}

	for(uint8_t i = 0; i < nTgt; i++)
	{
		uint16_t sp = src[i];
		if(oldV7 && (i == DS_HAT_DECAY)) sp = PAR_VELOD6_OPEN;
		setPar(dst[i], kit[sp]);
	}
	if(tgt == 6) setPar(PAR_HH7_VALID, 1);
}

// target voice sound and output back as in the backup
static void restoreTarget(void)
{
	uint8_t n;
	const uint16_t* dst = roles(drumStealer_target, &n);
	for(uint8_t i = 0; i < n; i++)
		setPar(dst[i], ds_backup[dst[i]]);
	if(drumStealer_target == 6) setPar(PAR_HH7_VALID, ds_backup[PAR_HH7_VALID]);
	setPar(outPar(drumStealer_target), ds_backup[outPar(drumStealer_target)]);
}

static void setBlink(uint8_t on)
{
	const uint8_t sel = selectable(drumStealer_target);
	for(uint8_t v = 0; v < DS_VOICES; v++)
		led_setBlinkLed((uint8_t)(LED_2 + v), 0);
	if(!on) return;
	led_clearLeds_1To8();
	for(uint8_t v = 0; v < DS_VOICES; v++)
		if(sel & (1 << v)) led_setBlinkLed((uint8_t)(LED_2 + v), 1);
}

static void leave(void)
{
	drumStealer_active = 0;
	setBlink(0);
	buttonHandler_showVoiceEditLeds();
	menu_repaintAll();
}

// ---------------------------------------------------------------------------
void drumStealer_enter(uint8_t targetVoice)
{
	if(drumStealer_active || targetVoice >= DS_VOICES) return;

	drumStealer_target = targetVoice;
	drumStealer_active = 1;		// from here on, only Drum Stealer auditions sound

	memcpy(ds_backup, activeParameters, sizeof(ds_backup));
	ds_kit = (uint8_t)(menu_loadedPresetNr[CURRENT_KIT] & 0x7f);
	ds_held = 0;
	ds_heard = 0;

	// audition on St1 (put back on leaving)
	setPar(outPar(targetVoice), MIXER_ROUTING_DAC1_STEREO);

	setBlink(1);
	menu_repaintAll();
}

void drumStealer_cancel(void)
{
	if(!drumStealer_active) return;
	restoreTarget();
	leave();
}

void drumStealer_encoder(int8_t inc)
{
	if(!drumStealer_active || inc == 0) return;
	ds_kit = (uint8_t)((ds_kit + inc) & 0x7f);	// 128 kits, wraps
	menu_repaintAll();
}

void drumStealer_voicePressed(uint8_t voiceNr)
{
	if(!drumStealer_active || voiceNr < 1 || voiceNr > DS_VOICES) return;
	const uint8_t v = (uint8_t)(voiceNr - 1);
	if(!(selectable(drumStealer_target) & (1 << v))) return;	// not a compatible source

	ds_held |= (uint8_t)(1 << v);
	ds_heard = voiceNr;
	applySound(v);

	// straight to the target voice: a voice link would play its partners, or nothing on a
	// drum that isn't the link's lead
	voiceControl_queueTrigger(drumStealer_target, SEQ_DEFAULT_NOTE, Trigger_getAccent(drumStealer_target));

	menu_repaintAll();
}

void drumStealer_voiceReleased(uint8_t voiceNr)
{
	if(voiceNr < 1 || voiceNr > DS_VOICES) return;
	ds_held &= (uint8_t)~(1 << (voiceNr - 1));
}

void drumStealer_drumPressed(void)
{
	if(!drumStealer_active || !ds_held) return;

	// the most recently auditioned voice if it is still held, else the lowest held one
	uint8_t v = 0;
	if(ds_heard && (ds_held & (1 << (ds_heard - 1))))
		v = (uint8_t)(ds_heard - 1);
	else
		while(!(ds_held & (1 << v))) v++;

	applySound(v);
	setPar(outPar(drumStealer_target), ds_backup[outPar(drumStealer_target)]);
	preset_touchTimer(PRESET_KIT_TIMER);	// the kit changed, like any edit
	leave();
}

// "STEAL->V3   k012"
// "BigKick src:123 "
uint8_t drumStealer_paint(char buf[2][17])
{
	if(!drumStealer_active) return 0;

	memset(buf, ' ', 2*17);
	memcpy(&buf[0][0], "STEAL->V", 8);
	buf[0][8] = (char)('1' + drumStealer_target);
	buf[0][12] = 'k';
	buf[0][13] = (char)('0' + ds_kit / 100);
	buf[0][14] = (char)('0' + (ds_kit / 10) % 10);
	buf[0][15] = (char)('0' + ds_kit % 10);

	const char* name = savefileNames[ds_kit][KIT_NAMES];
	for(uint8_t i = 0; i < 8; i++)
	{
		const char c = name[i];
		if(c == 0) break;
		buf[1][i] = (c < ' ' || c > '~') ? ' ' : c;
	}
	memcpy(&buf[1][9], "src:", 4);
	uint8_t col = 13;
	const uint8_t sel = selectable(drumStealer_target);
	for(uint8_t v = 0; v < DS_VOICES && col < 16; v++)
		if(sel & (1 << v)) buf[1][col++] = (char)('1' + v);

	buf[0][16] = buf[1][16] = 0;
	return 1;
}
