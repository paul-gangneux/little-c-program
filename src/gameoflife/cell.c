#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include "gameoflife/cell.h"
#include "windowManager.h"

// for debugging purposes
int alloc_counter=0;
int free_counter=0;

int pair=0;

typedef struct cell cell;

// binary search tree
// sorted by x, then y (except for garbageCells)
struct cell {
    int x,y;
    int willChange;
    cell* right;
    cell* left;
};

typedef struct tree {
    cell* root;
} tree;

cell* END_OF_TREE; // don't trust NULL
tree livingCells; // sorted
tree potentialCells; // sorted
tree garbageCells; // not sorted

// -- functions related to cells :

// allocate memory, use with caution !
static cell* createCell(int x, int y, cell* left, cell* right) {
    cell* c = (cell*) malloc(sizeof(cell));
    c->x = x; c->y = y; 
    c->left = left; c->right = right;
    c->willChange = 0;
    alloc_counter++;
    return c;
}

static void freeCells_rec(cell *cp) {
    if (cp==END_OF_TREE) return;
    freeCells_rec(cp->left);
    freeCells_rec(cp->right);
    free(cp);
    alloc_counter--;
}

static void freeCells(cell **cp) {
    freeCells_rec(*cp);
    *cp=END_OF_TREE;
}

static int compareCells(cell* c1, cell* c2) {
    if (c2==END_OF_TREE) return -42;
    if (c1->x != c2->x) return c1->x - c2->x;
    if (c1->y != c2->y) return c1->y - c2->y;
    return 0;
}

static int compareCoordsToCell(int x, int y, cell* c) {
    if (c==END_OF_TREE) return -42;
    if (x != c->x) return x - c->x;
    if (y != c->y) return y - c->y;
    return 0;
}

/*
static int areNeighbors(cell* c1, cell* c2) {
    int d1=c1->x - c2->x;
    int d2=c1->y - c2->y;
    if (d1==0 && d2==0) return 0; // case where c1==c2
    return (d1>=-1 && d1<=1 && d2>=-1 && d2<=1);
}
*/

static int isInList(int x, int y, cell* cp) {
    if (cp==END_OF_TREE) return 0;
    if (cp->x==x && cp->y==y) return 1;
    if (compareCoordsToCell(x,y,cp)<0) return isInList(x,y,cp->left);
    if (compareCoordsToCell(x,y,cp)>0) return isInList(x,y,cp->right);
    return 0;
}

static int countCells(cell* cp) {
    if (cp==END_OF_TREE) return 0;
    return 1 + countCells(cp->left) + countCells(cp->right);
}

// create the cell at pos x,y if it doesn't exist
// keeps the tree sorted
static void addNewCell(int x, int y, cell **firstCell) {
    if (*firstCell==END_OF_TREE) {
        *firstCell=createCell(x, y, END_OF_TREE, END_OF_TREE);
    } else {
        if (compareCoordsToCell(x,y,*firstCell)<0) 
            addNewCell(x,y,&(*firstCell)->left);
        else if (compareCoordsToCell(x,y,*firstCell)>0)
            addNewCell(x,y,&(*firstCell)->right);
    }
}

// add the cell pointed by cp to the list pointed by firstCell
// keeps the tree sorted
static void addExistingCell(cell* cp, cell** firstCell) {
    if (*firstCell==END_OF_TREE) {
        cp->right = END_OF_TREE;
        cp->left = END_OF_TREE;
        cp->willChange=0;
        *firstCell=cp;
    }
    else if (compareCells(cp, *firstCell)<0) {
        addExistingCell(cp, &(*firstCell)->left);
    }
    else if (compareCells(cp, *firstCell)>0) {
        addExistingCell(cp, &(*firstCell)->right);
    }
}

// same as above, but does not keep the chain sorted
// only use with the garbage cells
static void addCellAtBegining(cell* cp, cell** firstCell) {
    cp->right = *firstCell;
    cp->left = END_OF_TREE;
    *firstCell = cp;
}

static cell* popMinRec(cell* root) {
    if (root->left->left==END_OF_TREE) {
        cell* min = root->left;
        root->left = root->left->right;
        return min;
    }
    return popMinRec(root->left);
}

static cell* popMin(cell** root) {
    if (*root==END_OF_TREE) return END_OF_TREE;
    if ((*root)->left==END_OF_TREE) {
        cell* min = *root;
        *root = (*root)->right;
        return min;
    }
    return popMinRec(*root);
}

static cell* popMaxRec(cell* root) {
    if (root->right->right==END_OF_TREE) {
        cell* min = root->right;
        root->right = root->right->left;
        return min;
    }
    return popMaxRec(root->right);
}

static cell* popMax(cell** root) {
    if (*root==END_OF_TREE) return END_OF_TREE;
    if ((*root)->right==END_OF_TREE) {
        cell* min = *root;
        *root = (*root)->left;
        return min;
    }
    return popMaxRec(*root);
}

// remove living cells at the begining of the chain that should die
static void removeFirstCellsIfNeeded(cell** root) {
    if (*root==END_OF_TREE) return;
    while (*root != END_OF_TREE && (*root)->willChange) {
        cell* new;
        cell* old=*root;
        if ((*root)->left == END_OF_TREE) {
            new = popMin(&(*root)->right);
        } else if ((*root)->right == END_OF_TREE) {
            new = popMax(&(*root)->left);
        }
        else if (pair) {
            new = popMin(&(*root)->right);
        } else {
            new = popMax(&(*root)->left);
        }
        pair=!pair;
        new->right = (*root)->right;
        new->left = (*root)->left;
        *root = new;
       
        addCellAtBegining(old, &garbageCells.root);
    }
    removeFirstCellsIfNeeded(&(*root)->left);
    removeFirstCellsIfNeeded(&(*root)->right);
}

// remove living cells that will change 
static void removeDeadCells() {
    removeFirstCellsIfNeeded(&livingCells.root);
}

static void addNewLivingCells_rec(cell **root) {
    if (*root==END_OF_TREE) return;
    while (*root != END_OF_TREE && (*root)->willChange) {
        cell* new;
        cell* old=*root;
        if ((*root)->left == END_OF_TREE) {
            new = popMin(&(*root)->right);
        } else if ((*root)->right == END_OF_TREE) {
            new = popMax(&(*root)->left);
        }
        else if (pair) {
            new = popMin(&(*root)->right);
        } else {
            new = popMax(&(*root)->left);
        }
        pair=!pair;
        new->right = (*root)->right;
        new->left = (*root)->left;
        *root = new;

        addExistingCell(old, &livingCells.root);
    }
    addNewLivingCells_rec(&(*root)->left);
    addNewLivingCells_rec(&(*root)->right);
}



// move the potential cells that need to change to the living cell list
static void addNewLivingCells() {
    addNewLivingCells_rec(&potentialCells.root);
}

// check if a living cell should die, or a potential cell should
// turn into a new cell. change the value of willChange accordingly
static void updateWillChangeForOneCell(cell* cp, int isAlive) {
    if (cp==END_OF_TREE) return;
    int n=0;
    for (int i=-1; i<=1; i++) {
        for (int j=-1; j<=1; j++) {
            if ((j!=0 || i!=0) && isInList(cp->x+i, cp->y+j, livingCells.root)) n++;
            if (n>=5) goto done;
        }
    }
    done:
    if (isAlive) cp->willChange = (n<2 || n>3);
    else cp->willChange = (n==3);
}

static void updateWillChangeForAll(cell* firstCell, int isAlive) {
    if (firstCell==END_OF_TREE) return;
    updateWillChangeForOneCell(firstCell, isAlive);
    updateWillChangeForAll(firstCell->left, isAlive);
    updateWillChangeForAll(firstCell->right, isAlive);
}

static void trashPotentialCell(cell* cp) {
    if (cp==END_OF_TREE) return;
    trashPotentialCell(cp->left);
    trashPotentialCell(cp->right);
    addCellAtBegining(cp, &garbageCells.root);
}

static void addPotencialCells_rec(cell* cp) {
    if (cp==END_OF_TREE) return;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (
                (i!=0 || j!=0) && 
                !isInList(cp->x+i, cp->y+j, livingCells.root) &&
                !isInList(cp->x+i, cp->y+j, potentialCells.root)
            ) {
                if (garbageCells.root!=END_OF_TREE) {
                    cell* tmp = garbageCells.root->right;
                    garbageCells.root->x=cp->x+i;
                    garbageCells.root->y=cp->y+j;
                    garbageCells.root->willChange=0;
                    addExistingCell(garbageCells.root, &potentialCells.root);
                    garbageCells.root=tmp;
                }
                else {
                    addNewCell(cp->x+i, cp->y+j, &potentialCells.root);
                }
            }
        }
    }
    addPotencialCells_rec(cp->left);
    addPotencialCells_rec(cp->right);
}

// put every cell from the potentialCells list in the garbage list, and re-build it
// by adding a potential cell for every empty neighbors of all living cells.
static void addPotencialCells() {
    trashPotentialCell(potentialCells.root);
    potentialCells.root=END_OF_TREE;
    addPotencialCells_rec(livingCells.root);
}

static cell* findCell(int x, int y, cell* cp) {
    if (cp==END_OF_TREE) return NULL;
    if (x==cp->x && y==cp->y) return cp;
    if (compareCoordsToCell(x,y,cp)<0) return findCell(x, y, cp->left);
    return findCell(x, y, cp->right);
}

// if exists, delete cell, else create it
// only use during initialisation phase
// not efficient at all but good enough since there's no need for efficiency during initialisation
void cell_switchLivingCell(int x, int y) {

    if (!isInList(x, y, livingCells.root)) {
        addNewCell(x, y, &livingCells.root);
    } else {
        cell* cp = findCell(x, y, livingCells.root);
        cp->willChange=1;
        removeDeadCells();
    }
}

// only use these for the text to cells thing

static void shiftCells(cell* c, int x, int y) {
    if (c==END_OF_TREE) return;
    c->x+=x; 
    c->y+=y;
    shiftCells(c->left,x,y);
    shiftCells(c->right,x,y);
}

static void shiftAllCells(int x, int y) {
    shiftCells(livingCells.root,x,y);
    shiftCells(potentialCells.root,x,y);
}

void cell_centerCells() {
    cell* min = popMin(&livingCells.root);
    cell* max = popMax(&livingCells.root);
    int min_x = min->x;
    int max_x = max->x;
    int min_y = min->y;
    int max_y = max->y;
    addExistingCell(min, &livingCells.root);
    addExistingCell(max, &livingCells.root);

    shiftAllCells((min_x-max_x)/2, (min_y-max_y)/2);
}

void cell_freeNonLivingCells() {
    freeCells(&garbageCells.root);
    freeCells(&potentialCells.root);
}

void cell_freeAllCells() {
    freeCells(&livingCells.root);
    freeCells(&garbageCells.root);
    freeCells(&potentialCells.root);
}

void cell_nextState() {
    addPotencialCells();        
    updateWillChangeForAll(livingCells.root, 1);     
    updateWillChangeForAll(potentialCells.root, 0);
    removeDeadCells();   
    addNewLivingCells(); 
}

int cell_countLivingCells() {
    return countCells(livingCells.root);
}

int cell_countPotentialCells() {
    return countCells(potentialCells.root);
}

int cell_countGarbageCells() {
    return countCells(garbageCells.root);
}

void cell_init() {
    END_OF_TREE=createCell(0,0,NULL, NULL);
    END_OF_TREE->right=END_OF_TREE;
    END_OF_TREE->left=END_OF_TREE;
    livingCells.root=END_OF_TREE;
    potentialCells.root=END_OF_TREE;
    garbageCells.root=END_OF_TREE;
}

void cell_quit() {
    cell_freeAllCells();
    free(END_OF_TREE);
    alloc_counter--;
}

int cell_getAllocCounter() {
    return alloc_counter;
}

int cell_getFreeCounter() {
    return free_counter;
}

// ------------------- drawing functions --------------------- //

static void drawCells_rec(cell* cp, int size, int d) {
    if (cp==END_OF_TREE) return;
    SDL_Rect r;
    r.w=size;
    r.h=size;
    
    r.x=cp->x*(size+d)+screen_w/2-cameraX*(size+d);
    r.y=cp->y*(size+d)+screen_h/2+cameraY*(size+d);

    SDL_RenderFillRect(renderer, &r);
    
    drawCells_rec(cp->left, size, d);
    drawCells_rec(cp->right, size, d);
}

void cell_drawCells(int size, int R, int G, int B) {
    
    SDL_SetRenderDrawColor(renderer, R, G, B, 255);
    int d = size/5;
    drawCells_rec(livingCells.root, size, d);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}