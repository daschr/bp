# bp
pronounced "beep", parses output of aubionotes (https://aubio.org/) to play music over a buzzer

## compile
`gcc -lm -O3 -o bp bp.c`

## usage
`aubionotes your_song.mp3 | bp`
or
`bp halloween.txt`

## "It's not working!"

* bp currently needs root privileges
* `modprobe pcspkr`
* do you even have a buzzer?
