/*
  MAZE.C
  ======
  (c) Copyright Paul Griffiths 2002
  Email: mail@paulgriffiths.net

  Implementation of maze functions
*/


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "maze.h"


/*  Creates a maze from a file  */

void GetMazeFromFile(char * filename, struct maze * maze) {
    char buffer[BUFFERSIZE];
    FILE * fp;
    char ** map;
    int n = 0, foundentrance = 0, foundexit = 0;


    /*  Open file  */

    if ( !(fp = fopen(filename, "r")) ) {
        sprintf(buffer, "Couldn't open file %s", filename);
        perror(buffer);
        exit(EXIT_FAILURE);
    }


    /*  Determine number of rows in maze  */

    while ( fgets(buffer, BUFFERSIZE, fp) )
        ++n;


    /*  Allocate correct number of rows  */

    if ( !(map = malloc(n * sizeof *map)) ) {
        fputs("Couldn't allocate memory for map\n", stderr);
        exit(EXIT_FAILURE);
    }


    /*  Reset file  */

    rewind(fp);
    n = 0;


    /*  Store each row  */

    while ( fgets(buffer, BUFFERSIZE, fp) ) {
        int i;

        if ( !(map[n] = malloc((strlen(buffer)+1) * sizeof map[n])) ) {
            fputs("Couldn't allocate memory for map\n", stderr);

            for ( i = 0; i < n; ++i )
                free(map[i]);

            free(map);

            exit(EXIT_FAILURE);
        }

        strcpy(map[n], buffer);
        

        /*  Remove trailing whitespace  */

        for ( i = strlen(map[n]) - 1; isspace(map[n][i]); --i )
            map[n][i] = 0;


        /*  Check for entrance and store position if found  */

        if ( !foundentrance ) {
            i = 0;
            while ( map[n][i] != 'I' && map[n][i++] );
            if ( map[n][i] == MAZE_ENTRANCE ) {
                maze->startx = i;
                maze->starty = n;
                foundentrance = 1;
            }
        }

        /*  Check for exit  */

        if ( !foundexit ) {
            if ( strchr(map[n], MAZE_EXIT) )
                foundexit = 1;
        }
        ++n;
    }


    /*  Fill in maze structure  */

    maze->map = map;
    maze->numrows = n;


    /*  Exit if there is no entrance or no exit */

    if ( !foundentrance ) {
        fputs("Maze has no entrance!\n", stderr);
        FreeMaze(maze);
        exit(EXIT_FAILURE);
    }

    if ( !foundexit ) {
        fputs("Maze has no exit!\n", stderr);
        FreeMaze(maze);
        exit(EXIT_FAILURE);
    }


    /*  Check for initial direction into the maze  */

    if ( maze->startx < strlen(maze->map[maze->starty]) - 1 &&
         maze->map[maze->starty][maze->startx+1] == MAZE_PATH )
        maze->initdir = MOVE_RIGHT;
    else if ( maze->startx > 0 &&
              maze->map[maze->starty][maze->startx-1] == MAZE_PATH )
        maze->initdir = MOVE_LEFT;
    else if ( maze->starty > 0 &&
         maze->map[maze->starty-1][maze->startx] == MAZE_PATH )
        maze->initdir = MOVE_UP;
    else if ( maze->starty < (maze->numrows-1) &&
              maze->map[maze->starty+1][maze->startx] == MAZE_PATH )
        maze->initdir = MOVE_DOWN;


    /*  Close file and return  */

    if ( fclose(fp) ) {
        perror("Couldn't close file");
        FreeMaze(maze);
        exit(EXIT_FAILURE);
    }
}


/*  Frees memory used by a maze  */

void FreeMaze(struct maze * maze) {
    int n;

    for ( n = 0; n < maze->numrows; ++n )
        free(maze->map[n]);

    free(maze->map);
}


/*  Outputs a maze  */

void PrintMaze(struct maze * maze) {
    int n;

    for ( n = 0; n < maze->numrows; ++n )
        puts(maze->map[n]);
}
