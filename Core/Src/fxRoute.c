/*
 * fxRoute.c
 *
 * Hot key "voices -> FX bus", see fxRoute.h.
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * RXl-0: ported from LXRE_MX v1.1.1 by Roman (RN01MA). Each press toggles a voice between its own
 * output and the FX bus; voice 7 (Golem's open hi-hat) routes its own output.
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#include "fxRoute.h"
#include "Parameters.h"
#include "menu/menu.h"
#include "frontPanelParser_ui.h"
#include "front.h"
#include "Preset/PresetManager.h"
#include "Soundengine/DSPAudio/mixer.h"
#include <string.h>

#define FXR_VOICES	7
#define NO_PREV		0xff

static uint8_t fxr_active;
static uint8_t fxr_prev[FXR_VOICES] = { NO_PREV, NO_PREV, NO_PREV, NO_PREV, NO_PREV, NO_PREV, NO_PREV };

// output parameter of a voice (voice 7 has its own, above 255)
static uint16_t outPar(uint8_t v)
{
	return v < 6 ? (uint16_t)(PAR_AUDIO_OUT1 + v) : (uint16_t)PAR_HH7_AUDIO_OUT;
}

// set a voice's audio out the way the MIX page encoder does: parameter,
// engine (CC2 / CC3 -> mixer routing), kit autosave
static void setAudioOut(uint8_t v, uint8_t out)
{
	const uint16_t par = outPar(v);
	activeParameters[par] = out;
	if(par <= 255)
		frontPanel_sendData(CC_2, (uint8_t)(par - 128), out);
	else
		frontPanel_sendData(CC3, (uint8_t)(par - 255), out);
	preset_touchTimer(PRESET_KIT_TIMER);
}

void fxRoute_fxPressed(uint8_t shiftHeld)
{
	fxr_active = shiftHeld ? 1 : 0;
	if(fxr_active) menu_repaintAll();
}

void fxRoute_fxReleased()
{
	if(fxr_active)
	{
		fxr_active = 0;
		menu_repaintAll();
	}
}

uint8_t fxRoute_isActive()
{
	return fxr_active;
}

void fxRoute_voicePressed(uint8_t voiceNr)
{
	if(voiceNr < 1 || voiceNr > FXR_VOICES) return;
	const uint8_t v = (uint8_t)(voiceNr - 1);
	const uint8_t cur = activeParameters[outPar(v)];

	if(cur != MIXER_ROUTING_FX_BUS)
	{
		fxr_prev[v] = cur;								// remember where it came from
		setAudioOut(v, MIXER_ROUTING_FX_BUS);
	}
	else
	{
		// back where it came from (unknown -> St1)
		const uint8_t back = (fxr_prev[v] != NO_PREV && fxr_prev[v] < MIXER_ROUTING_FX_BUS) ? fxr_prev[v] : MIXER_ROUTING_DAC1_STEREO;
		setAudioOut(v, back);
	}
	menu_repaintAll();
}

uint8_t fxRoute_paint(char buf[2][17])
{
	if(!fxr_active) return 0;

	// "FX route 1234567"
	// "         F--F--F"
	memset(buf, ' ', 2*17);
	memcpy(&buf[0][0], "FX route 1234567", 16);
	for(uint8_t v=0; v<FXR_VOICES; v++)
	{
		const uint8_t out = activeParameters[outPar(v)];
		buf[1][9 + v] = (out == MIXER_ROUTING_FX_BUS) ? 'F' : '-';
	}
	buf[0][16] = buf[1][16] = 0;
	return 1;
}
