#ifndef CELL_H
#define CELL_H

void cell_init();
void cell_switchLivingCell(int x, int y);
void cell_centerCells();
void cell_freeNonLivingCells();
void cell_freeAllCells();
void cell_nextState();
int cell_countLivingCells();
int cell_countPotentialCells();
int cell_countGarbageCells();
void cell_quit();
int cell_getAllocCounter();
int cell_getFreeCounter();
void cell_drawCells(int size, int R, int G, int B);

#endif