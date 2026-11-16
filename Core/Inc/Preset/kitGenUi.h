/*
 * kitGenUi.h
 *
 * Front panel side of the kit randomizer:
 *   SHIFT+PERF          -> randomizer on / off (off -> plain PERF)
 *   while on            -> PERF blinks in every mode; EDIT/MOD/FX/SAVE work,
 *                          PERF brings back the randomizer page
 *   PERF held           -> recipe per voice on the display
 *   DRUM (on its page)  -> new kit right on the press
 *   hold voice(s) + DRUM-> only the held voices are rolled (DRUM alone: whole kit)
 *   voice buttons       -> trigger the voice (audition, like PERF)
 *   voice LEDs          -> bar over the rolled voices' buttons
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * RXl-20: ported from LXRE_MX v1.1.1 by Roman (RN01MA), renamed from Randomizer*
 * (Golem's parameter randomizer is menu/randomizer.c) and extended to Golem's voice 7.
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef INC_PRESET_KITGENUI_H_
#define INC_PRESET_KITGENUI_H_

#include <stdint.h>

void KitGenUi_setLatched(uint8_t on);			// SHIFT+PERF
uint8_t KitGenUi_isLatched();
void KitGenUi_enter();							// randomizer page shown
void KitGenUi_leave();							// another page / mode selected
void KitGenUi_perfPressed();
void KitGenUi_perfReleased();

void KitGenUi_drumPressed();
void KitGenUi_drumReleased();
void KitGenUi_voicePressed(uint8_t voiceNr);	// 1..7 (7 = open hihat button -> V6)
void KitGenUi_voiceReleased(uint8_t voiceNr);

void KitGenUi_tick();							// main loop: LED bar, result display timeout

// display overlay for the randomizer page; returns 1 if it painted the buffer
uint8_t KitGenUi_paint(char buf[2][17]);

#endif /* INC_PRESET_KITGENUI_H_ */
