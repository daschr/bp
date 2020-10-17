#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <getopt.h>

#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <linux/kd.h>
#include <linux/input.h>
#include <event.h>

#define CLOCK_TICK_RATE 1193180.0
#define NOTE_A 440.0
#define CONSOLE "/dev/console"
#define PLATFORM_SPEAKER "/dev/input/by-path/platform-pcspkr-event-spkr"


int is_ev=0;
int console_fd=-1;
struct input_event ev_inp;
FILE *ff=NULL;
char *line=NULL;

void play_note(int);

void int_handler(int _) {
    if(console_fd != -1) {
        play_note(0);
        close(console_fd);
    }

    if(ff != NULL)
        fclose(ff);
    free(line);

    exit(EXIT_SUCCESS);
}

//    http://subsynth.sourceforge.net/midinote2freq.html
void gen_midi_lookup(int table[]) {
    for(int x = 0; x < 128; ++x)
        table[x] = (int) ( ( (NOTE_A / 32.0) * pow(2.0, ((x - 9.0) / 12.0)) ) );
}

void play_note(int n) {
    if(is_ev) {
        ev_inp.value=n;
        write(console_fd, &ev_inp, sizeof(ev_inp));
    } else
        ioctl(console_fd, KIOCSOUND, (int) (CLOCK_TICK_RATE/n) );
}

int play_midi(FILE *f, int *midi_lookup) {
    size_t lsize=0;
    double c1=0,c2=0,c3=0;

    if((console_fd = open(PLATFORM_SPEAKER, O_WRONLY)) != -1 &&
            ioctl(console_fd, EVIOCGSND(0)) != -1) {
        is_ev=1;
    }

    if(!is_ev && (console_fd = open(CONSOLE, O_WRONLY)) == -1) {
        fprintf(stderr, "Could not open " PLATFORM_SPEAKER " (permissions?)\nor " CONSOLE " for writing.\n");
        return 0;
    }

    while(getline(&line, &lsize, f)>-1) {
        switch(sscanf(line, "%lf %lf %lf", &c1, &c2, &c3)) {
        case 3:
            if(c1<127.0) // nice try
                play_note(midi_lookup[(int)c1]);
            usleep((int) ((c3-c2) * 1000000));
            break;
        case 1:
            play_note(0);
            usleep((int) ((c1-c3) * 1000000));
            break;
        }
    }

    if(line) {
        free(line);
        line=NULL;
    }

    play_note(0);
    close(console_fd);
    console_fd=-1;

    return 1;
}

int main(int ac, char *as[]) {
    signal(SIGINT, &int_handler);
    signal(SIGSEGV, &int_handler);
    signal(SIGTERM, &int_handler);

    memset(&ev_inp, 0, sizeof(ev_inp));
    ev_inp.type = EV_SND;
    ev_inp.code = SND_TONE;

    int midi_lookup[128];
    gen_midi_lookup(midi_lookup);

    int ec=EXIT_SUCCESS;

    if(ac >1) {
        int loop= ac >2 ? strcmp(as[2], "loop") : -1;

        ff=fopen(as[1], "r");

        if(ff==NULL) {
            fprintf(stderr, "could not open '%s' for reading!\n", as[1]);
            return EXIT_FAILURE;
        }

        do {
            if(!play_midi(ff, midi_lookup)) {
                ec=EXIT_FAILURE;
                break;
            }
            fseek(ff, 0, SEEK_SET);
        } while(loop == 0);

        fclose(ff);
    } else {
        ec=play_midi(stdin, midi_lookup) ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    return ec;
}
