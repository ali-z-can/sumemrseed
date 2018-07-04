/*
  SOLVE.C
  =======
  (c) Copyright Paul Griffiths 2002
  Email: mail@paulgriffiths.net

  Implementation of maze solving operation
*/


#include "solve.h"


/*  Recursive function to find a path through a maze  */

int look(struct maze * maze, struct pos pos) {
    int i, n;


    /*  Set new position based on direction  */

    switch ( pos.dir ) {
    case MOVE_UP:
        pos.y -= 1;
        break;
        
    case MOVE_DOWN:
        pos.y += 1;
        break;

    case MOVE_LEFT:
        pos.x -= 1;
        break;

    case MOVE_RIGHT:
        pos.x += 1;
        break;

    default:
        break;
    }


    /*  Check new position  */

    if ( pos.y < 0 || pos.y >= maze->numrows )
        return MAZE_NOWAY;
    else if ( pos.x < 0 || pos.x >= strlen(maze->map[pos.y]) )
        return MAZE_NOWAY;
    else if ( maze->map[pos.y][pos.x] == MAZE_WALL )
        return MAZE_NOWAY;
    else if ( maze->map[pos.y][pos.x] == MAZE_EXIT )
        return MAZE_FOUNDEXIT;
    else if ( maze->map[pos.y][pos.x] == MAZE_ENTRANCE )
        return MAZE_NOEXIT;


    /*  Try all the three directions other than backwards  */

    pos.dir -= 1;
    if ( pos.dir < 0 )
        pos.dir += 4;

    for ( i = 0; i < 3; ++i ) {
        maze->map[pos.y][pos.x] = MAZE_TRAIL;          /*  Leave trail through maze  */

        n = look(maze, pos);
        if ( n ) {
            if ( n == MAZE_NOEXIT )
                maze->map[pos.y][pos.x] = MAZE_PATH;   /*  No exit, so clear trail   */
            return n;
        }

        pos.dir += 1;
        if ( pos.dir > 3 )
            pos.dir -= 4;
    }
    

    /*  Dead end, so clear trail and return  */

    maze->map[pos.y][pos.x] = MAZE_PATH;

    return 0;
}


/*  Function to find a path through a maze  */
    
int solve(struct maze * maze) {
    struct pos pos;

    pos.x = maze->startx;
    pos.y = maze->starty;
    pos.dir = maze->initdir;

    return look(maze, pos);
}    


