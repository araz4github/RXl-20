/*
 * fxRoute.h
 *
 * Hot key "voices -> FX bus": hold SHIFT+FX and press voice buttons.
 * Each press toggles the voice between its own output and the FX bus.
 *
 * Copyright (C) 2012-2026 Erica Synths
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * RXl-20: ported from LXRE_MX v1.1.1 by Roman (RN01MA), extended to Golem's voice 7.
 *
 * This file is part of the LXR firmware. It is licensed under the GNU General
 * Public License version 3, with the additional permission and terms in
 * NOTICE.md. See LICENSE and NOTICE.md in the root of the repository.
 */

#ifndef INC_FXROUTE_H_
#define INC_FXROUTE_H_

#include <stdint.h>

void fxRoute_fxPressed(uint8_t shiftHeld);	// the combo is active while FX stays held after SHIFT+FX
void fxRoute_fxReleased();
uint8_t fxRoute_isActive();
void fxRoute_voicePressed(uint8_t voiceNr);	// 1..7 (7 = voice 7, Golem's open hi-hat)

// display while the combo is held; returns 1 if it painted the buffer
uint8_t fxRoute_paint(char buf[2][17]);

#endif /* INC_FXROUTE_H_ */
