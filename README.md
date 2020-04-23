# bp
pronounced "beep", parses output of aubionotes (https://aubio.org/) to play music over a buzzer

## compile
`apt install make gcc libevent-dev`
`make`

## usage
`aubionotes your_song.mp3 | bp`
or
`bp halloween.txt`

## "It's not working!"

* `modprobe pcspkr`
* do you even have a buzzer?
