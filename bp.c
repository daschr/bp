#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/kd.h>

#define CLOCK_TICK_RATE 1193180.0
#define NOTE_A 440.0
#define CONSOLE "/dev/console"

int console_fd=-1;

void int_handler(int _) {
    if(console_fd != -1) {
        ioctl(console_fd, KIOCSOUND, 0);
        close(console_fd);
    }

    exit(EXIT_SUCCESS);
}

//    http://subsynth.sourceforge.net/midinote2freq.html
void gen_midi_lookup(int table[]) {
    for(int x = 0; x < 127; ++x)
        table[x] = (int) ( CLOCK_TICK_RATE / ( (NOTE_A / 32.0) * pow(2.0, ((x - 9.0) / 12.0)) ) );
}

void play_midi(FILE *f) {
    size_t lsize=0;
    char *line=NULL;
    float c1=0,c2=0,c3=0,dur=0;
    int scfr=0;
    int midi_lookup[127];

    gen_midi_lookup(midi_lookup);

    if((console_fd = open(CONSOLE, O_WRONLY)) == -1) {
        fprintf(stderr, "Could not open " CONSOLE " for writing.\n");
        exit(1);
    }

    while(getline(&line, &lsize, f)>-1) {
        switch(sscanf(line, "%f %f %f", &c1, &c2, &c3)) {
        case 3:
            if(c1<127.0) // nice try
                ioctl(console_fd, KIOCSOUND, midi_lookup[(int)c1]);
#ifdef DEBUG
            printf("Note:  %d Freq: %f dur: %d\n", (int) c1, midi_lookup[(int)c1], (int) ((c3-c2)*1000000));
#endif
            usleep((int) ((c3-c2) * 1000000));
            break;
        case 1:
#ifdef DEBUG
            printf("Quiet dur: %d\n", (int)((c1-c3) *1000000));
#endif
            ioctl(console_fd, KIOCSOUND, 0);
            usleep((int) ((c1-c3) * 1000000));
            break;
        }
    }

    if(line)
        free(line);

    ioctl(console_fd, KIOCSOUND, 0);
    close(console_fd);
}

int main(int ac, char *as[]) {
    signal(SIGINT, &int_handler);
    signal(SIGSEGV, &int_handler);
    signal(SIGTERM, &int_handler);

    if(ac >1) {
        FILE *ff=fopen(as[1], "r");

        if(ff==NULL) {
            fprintf(stderr, "could not open '%s' for reading!\n", as[1]);
            return EXIT_FAILURE;
        }
        play_midi(ff);

        fclose(ff);
    } else {
        play_midi(stdin);
    }
    return EXIT_SUCCESS;
}
