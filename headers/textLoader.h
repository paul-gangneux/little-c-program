#ifndef TEXTLOADER_H
#define TEXTLOADER_H

extern const int TEXT_BLENDED;
extern const int TEXT_SHADED;
extern const int TEXT_SOLID;

void txt_drawText(char* text, int x, int y);
void txt_drawInt(int nb, int x, int y);
void txt_drawLong(long nb, int x, int y);
void txt_drawFloat(double nb, int x, int y);
void txt_initTextLoader(int renderMode);
void txt_drawTextColored(char* text, int x, int y, int R, int G, int B);
void txt_drawAlphabet();

#endif