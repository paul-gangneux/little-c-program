#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gameoflife/cell.h"
#include "gameoflife/text_to_cells.h"

typedef struct point point;
typedef struct char_point_a char_point_a;

struct point {
    int x,y;
    point* next;
};

struct char_point_a {
    char c;
    int width; // thicc
    point* point_list;
    char_point_a* next;
};

// recursive functions

//static char_point_a* last_cpa(char_point_a* cpa);
static void free_all_points(point* p);
static void free_all(char_point_a* cpa);
static char_point_a* getCpaFromChar(char_point_a* cpa, const char c);

static char_point_a* create_cpa(char c) {
    char_point_a* cpa = (char_point_a*) malloc(sizeof(char_point_a));
    cpa->c = c;
    cpa->point_list = NULL;
    cpa->next = NULL;
    cpa->width = 0;
    return cpa;
}

/*
static char_point_a* last_cpa(char_point_a* cpa) {
    if (cpa == NULL) return NULL;
    if (cpa->next==NULL) return cpa;
    return last_cpa(cpa->next);
}
*/

static void addPoint(char_point_a* cpa, int x, int y) {
    point* temp = cpa->point_list;
    cpa->point_list = (point*) malloc(sizeof(point));
    cpa->point_list->x = x;
    cpa->point_list->y = y;
    cpa->point_list->next=temp;
}

static void free_all_points(point* p) {
    if (p==NULL) return;
    free_all_points(p->next);
    free(p);
}

static void free_all(char_point_a* cpa) {
    if (cpa==NULL) return;
    free_all_points(cpa->point_list);
    free_all(cpa->next);
    free(cpa);
}

static char_point_a* getCpaFromChar(char_point_a* cpa, const char c) {
    if (cpa == NULL) return NULL;
    if (cpa->c == c) return cpa;
    return getCpaFromChar(cpa->next,c);
}

static char_point_a* createList() {

    char_point_a* assoc_list = NULL;
    char_point_a* curr_a=NULL;
    char c='0';
    int x=0,y=0;

    FILE* f=fopen("ressource/letters.txt", "r");
    if (f==NULL) {
        printf("Error: unable to load ressource/letters.txt\n");
        exit(1);
    };

    while ((c=fgetc(f))!=EOF) {
        if (assoc_list==NULL) {
            assoc_list = create_cpa(c);
            curr_a = assoc_list;
        } else {
            curr_a->next = create_cpa(c);
            curr_a = curr_a->next;
        }
        
        if (fgetc(f)!='\n' || fgetc(f)!=':' || fgetc(f)!='\n') {
            printf("parsing error 1 %c\n", c);
            exit(1);
        }
        int max=0;
        while ((c=fgetc(f))!=':') {
            switch (c) {
                case ' ': 
                    x++;
                break;
                case 'o':
                    addPoint(curr_a, x, y);
                    x++;
                    max=(max<x)?x:max;
                break;
                case '\n': 
                    y++;
                    x=0;
                break; 
                default :
                    printf("parsing error 2\n");
                    exit(1);
                break;
            }
        }
        if (fgetc(f)!='\n') {
            printf("parsing error 1 %c\n", c);
            exit(1);
        }
        curr_a->width=max;
        x=0; y=0;
    }
    fclose(f);

    return assoc_list;
}

static void createCellsFromPointList(point* p, int x, int y) {
    if (p == NULL) return;
    cell_switchLivingCell(p->x+x, p->y+y);
    createCellsFromPointList(p->next,x,y);
}

void cell_stringToCells(char* s) {

    char_point_a* assoc_list=createList();

    int x=0, y=0; //origin point
    while (*s) {
        if (*s=='\n') {
            y+=6;
            x=0;
        }
        else if (*s==' ') x+=2;
        else {
            char_point_a* cpa = getCpaFromChar(assoc_list, *s);
            if (cpa!=NULL) {
                createCellsFromPointList(cpa->point_list, x, y);
                x += cpa->width+1; // thiccc
            }
        }
        s++;
    }
    
    free_all(assoc_list);

    cell_centerCells();
}

