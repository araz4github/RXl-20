/*
 * drumStealer.h
 *
 * RXl-0 Drum Stealer: borrow one drum sound from another kit.
 *
 * In voice edit mode hold SHIFT, hold the target voice button, press DRUM.
 * - the encoder browses the stored kits 0..127 (read from RAM, never from the SD card)
 * - the blinking voice buttons are the compatible source voices (1-3 / 4 / 5 / 6-7)
 * - press one: its sound is put on the target voice (your kit's mix is kept) and played
 * - hold one + DRUM: copy it into the target voice and leave
 * - SHIFT or PERF: leave and put the target voice back as it was
 * While it is open only the target voice sounds (all other triggers are ignored), the
 * target plays on St1, the pots do nothing and kit autosave waits.
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef INC_DRUMSTEALER_H_
#define INC_DRUMSTEALER_H_

#include <stdint.h>

// read by the trigger path (also from interrupts): keep it to plain variables
extern volatile uint8_t drumStealer_active;
extern volatile uint8_t drumStealer_target;	// 0..6

// 1 = this trigger must be dropped (auditions bypass voiceControl_noteOn)
static inline uint8_t drumStealer_blocks(uint8_t voice)
{
	(void)voice;
	return drumStealer_active;
}

static inline uint8_t drumStealer_isActive(void) { return drumStealer_active; }

void drumStealer_enter(uint8_t targetVoice);		// 0..6
void drumStealer_cancel(void);
void drumStealer_encoder(int8_t inc);
void drumStealer_voicePressed(uint8_t voiceNr);		// 1..7
void drumStealer_voiceReleased(uint8_t voiceNr);	// 1..7
void drumStealer_drumPressed(void);

// display; returns 1 if it painted the buffer
uint8_t drumStealer_paint(char buf[2][17]);

#endif /* INC_DRUMSTEALER_H_ */
