// Metasprite display
#ifndef _METASPRITE_H
#define _METASPRITE_H

extern byte oam_id;

void sprite(word x, byte y, byte ch, byte attr);

void metasprite(byte ch, byte attr, word x, byte y);

#endif