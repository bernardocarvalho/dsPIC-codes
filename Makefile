MCU        = 30F4011

TGT        = main

PORT       = /dev/ttyUSB0
BAUD       = 115200

WFLAGS     = -std=c11 -pedantic -Werror -Wall -W -Wmissing-prototypes -Wstrict-prototypes\
	     -Wconversion -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings\
	     -fshort-enums -fno-common -Wnested-externs -Wno-error=main

CFLAGS     =  -mcpu=${MCU} -omf=elf \
	      -O0 -msmart-io=1 -Wall -msfr-warn=off -g -fomit-frame-pointer
LFLAGS     = -mcpu=${MCU} -omf=elf \
	     -Wl,,,--defsym=__MPLAB_BUILD=1,,--script=p${MCU}.gld,--stack=16, \
	     --check-sections,--data-init,--pack-data,--handles,--isr,\
	     --no-gc-sections, --fill-upper=0,--stackguard=16,--no-force-link,--smart-io,\
	     --report-mem -g
HFLAGS     = 

UFLAGS     = -TPPK3 -P${MCU} -M -OL
MCHIP_HOME = /home/microchip
CC = ${MCHIP_HOME}/xc16/v1.41/bin/xc16-gcc 
HH = ${MCHIP_HOME}/xc16/v1.41/bin/xc16-bin2hex
UU = ${MCHIP_HOME}/mplabx/v5.30/mplab_ipe/ipecmd.sh 
RM = /bin/rm

# Peripheral Lib
PLIB = ${MCHIP_HOME}/xc16/v1.41/lib/dsPIC30F/libp${MCU}-elf.a

CFILES := $(wildcard *.c)
ASMFILES := $(wildcard *.S)
ASMOBJ := $(patsubst %.S, %.o, ${ASMFILES})
OBJECT := $(patsubst %.c, %.o, ${CFILES})

all: comp link 

comp: $(OBJECT) $(ASMOBJ)

link: $(TGT).elf

hex: $(TGT).hex

up: $(TGT).hex comp link
		$(UU) $(UFLAGS) -F./$<

%.o: %.c 
		$(CC) $(CFLAGS) -c $< 

%.o: %.S
		$(CC) $(CFLAGS) -c $< 

$(TGT).elf: $(OBJECT) $(ASMOBJ)
		$(CC) $(LFLAGS) -o $@ $^ ${PLIB}

$(TGT).hex: $(TGT).elf
		$(HH) $(HFLAGS) $^ 

clean:
		$(RM) -f $(OBJECT) $(TGT).hex $(ASMOBJ) $(TGT).elf log* MPLABXLog*

# You may need to install picocom and/or st commands (in Debian/Ubunto do 'apt install picocom stterm')
serial:
		nohup st -f inconsolata-12 -e picocom $(PORT) -b $(BAUD) &>/dev/null &

edit:
		vim Makefile

count:
		wc -l *.c

