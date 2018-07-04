#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <curses.h>
#include <ncurses.h>
#include <termios.h>


int main(int argc, char *argv[]) {
    //printf("Hello, World!\n");

    #define CHUNK 1024 /* read 1024 bytes at a time*/
    char buf[CHUNK];
    FILE *file;
    size_t nread;
    int numProgs=0;
    char* programs[50];
    char line[50];
    int i =0;
    struct winsize w;
    struct termios info;
    file = fopen(argv[1], "r");
   /* if (file) {
        while ((nread = fread(buf, 1, sizeof buf, file)) > 0) {
            fwrite(buf, 1, nread, stdout);

        }
        if (ferror(file)) {

        }
        fclose(file);
    }*/
    ioctl(0, TIOCGWINSZ, &w);

    //printf ("lines %d\n", w.ws_row);
   // printf ("columns %d\n", w.ws_col);



    while(fgets(line, sizeof line, file)!=NULL) {
        //check to be sure reading correctly
       // printf("%s", line);
        //add each filename into array of programs
        programs[i]=strdup(line);
        i++;
        //count number of programs in file
        numProgs++;
    }

    //check to be sure going into array correctly
    int numberToPrintAtFirst,remainingLines;
    if(w.ws_row <= numProgs+1){
        numberToPrintAtFirst = w.ws_row;
        remainingLines = numProgs - w.ws_row +1;
    }
    else{
        numberToPrintAtFirst = numProgs+1;
    }
    int j;
    for (j=0 ; j< numberToPrintAtFirst/*numProgs+1*/; j++) {
        printf("%s", programs[j]);

    }
    int ch,resetAbleCounter;
    tcgetattr(0, &info);          /* get current terminal attirbutes; 0 is the file descriptor for stdin */
    info.c_lflag &= ~ICANON;      /* disable canonical mode */
    info.c_cc[VMIN] = 1;          /* wait until at least one keystroke available */
    info.c_cc[VTIME] = 0;         /* no timeout */
    tcsetattr(0, TCSANOW, &info);
    while(remainingLines != 0){

        ch = getchar();
        //printf("%d\n",ch);
       // printf("remaining %d\n",remainingLines);
        if(ch  == 32){
            if(w.ws_row <= remainingLines){
                for(resetAbleCounter =0; resetAbleCounter<w.ws_row;resetAbleCounter++){
                    printf("%s", programs[j]);
                    j++;
                }
                remainingLines = remainingLines - w.ws_row;

            }
            else{
                for(resetAbleCounter = 0; resetAbleCounter < remainingLines; resetAbleCounter++){
                    printf("%s", programs[j]);
                    j++;
                }
            }
        }
        else if(ch  == 10){
            printf("%s", programs[j]);
            j++;
            remainingLines--;
        }
    }

    fclose(file);



    return 0;

}