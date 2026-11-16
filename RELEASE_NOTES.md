# RXl-0 v1.0

Update file: `LXRE_RXl-0_V1.0.img`. Based on Golem-RMX-0 v1.0 and Erica Synths' LXR Eurorack firmware v1.51.
Unofficial: not made, approved or supported by Erica Synths or by the authors of Golem, RMX-0 or LXRE_MX.

## From LXRE_MX v1.1.1 (Roman, RN01MA)
- **Kit generator** (SHIFT + PERF): CLS / EXP / GLT kits with key, LFO routing and spectral separation, extended to
  all 7 voices. Voice 7 is made as an open hi-hat from voice 6's recipes.
- **SHIFT + FX + voice buttons:** toggle a voice between its own output and the FX bus. Works on all 7 voices.
- **Click-free kit morphing:** choice settings change once, half way; the FX bus type and output don't morph.
- **CPU load** in global settings.
- **Fixes:** filter that could fall silent until power-off, wavetable reads past the end, a MIDI CC table overrun,
  CV read timing, accent latching and the open hi-hat trigger timing.
- Not included: LXRE_MX's DJ filter.

## New in RXl-0: Drum Stealer
- SHIFT + voice + DRUM (voice edit mode) opens it. The encoder browses kits 0–127, the blinking voice buttons
  audition compatible voices on your voice (your mix kept, played on St1, everything else silent), a held voice +
  DRUM copies it, SHIFT or PERF cancels. See the README.

## Changed from Golem-RMX-0
- RMX-0's "pan follows sounding notes" is removed: pan is read when a note starts, as in the stock firmware.
- Golem's FX bus B is removed to leave processing time for the global reverb. Voices a kit sent to FX B play dry
  on FX B's output; the FX page cycle is FX → Reverb → Random.
- Voice edit mode: the edited drum's LED is lit (its triggers flash it off) and the open sub-page LED blinks.
- Voice 7's sound engine sits in fast DTCM memory like voices 1–6.
- CPU: the global reverb runs at half the sample rate with a faster inner loop (about half its cost or less; slightly
  less top end in the tail, decay and tone as before). The snare, cymbal and hi-hat engines and the reverb sends
  run from fast ITCM memory; setters that only run when a setting changes moved out to make room.
- A plain FX press cycles the FX pages (as before). SHIFT + FX is now the voice routing shortcut.
- The Performance page is the stock one again (morph, morph target, sample rate). Golem's per-voice morph,
  morph chain (source kit, auto-advance, kit range) and the CV kit selection on global settings sub-page 2 are
  removed. Outputs, choke and voice links still never morph.
- Kit files are unchanged: kits move freely between Golem-RMX-0 and RXl-0.

---

# Golem-RMX-0 v1.0

Update file: `LXRE_GRMX0_V1.0.img`. Based on Erica Synths' LXR Eurorack firmware v1.51.
Unofficial: not made, approved or supported by Erica Synths or by the RMX-0 author.

This release merges **[RMX-0](https://github.com/mechanical-dance/RMX-0) by
[mechanical-dance](https://github.com/mechanical-dance)** into **Golem** (v1.51.5). Thank you to mechanical-dance for
RMX-0, and to **Erica Synths** for opening the LXR firmware.

## From RMX-0
- **LFOs:** eleven new shapes, including smooth random and one-shot envelopes; click-free retriggers; pan that moves
  sounding notes; a restart menu (SHIFT + EDIT + PERF).
- **MIDI:** MIDI input on the OP HH jack, with a channel and note per voice, voice links (unison, chord, poly) for
  drums 1–3, and glide and legato per drum.
- **Samples:** your own WAVs as oscillator waveforms, with start and end trims. Reinstall them after every update.
- **Reverb:** a global stereo reverb with a send per voice.

## Joining the two
- Voice 7 (Golem's separate open hat) gets RMX-0's declick, pan-follow, samples and MIDI, plus its own reverb send.
- The reverb page sits on the FX button cycle: FX A → FX B → Reverb → Random. Its output and return level are on the
  page's 2nd screen, because the Perform page keeps Golem's morph controls.
- The randomizer and morph leave voice links, glide, sample trims and the reverb's bus settings alone.
- Kits from stock 1.51, Golem and RMX-0 all load: RMX-0 kits keep their settings, Golem kits get the reverb sends at 0.

## Everything from Golem
FX bus B, the separate voice 7 with choke switch, the randomizer, per-voice morph, the morph chain with CV kit
selection, the delay CV fix and the mod-target fix.
