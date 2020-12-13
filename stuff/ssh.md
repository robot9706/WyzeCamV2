## SSH

1) Move the `bin/dropbearmulti` to an SD card or a USB stick.

2) Plug the SD card or USB stick into the WyzeCam and mount it:

        mkdir /tmp/usb

        mount /dev/sda1 /tmp/usb

3) Copy the ssh tool to `system/bin`:

        cp /tmp/usb/dropbearmulti /system/bin

    This is needed because /system/bin is in an executable partition, or mount the drive as an executable partition.

4) Generate a key:

        dropbearmulti dropbearkey -t ecdsa -f /tmp/key.ecdsa

5) Start the SSH server:

        dropbearmulti dropbear -E -r /tmp/key.ecdsa



### Dropbear compilation:

https://github.com/Dafang-Hacks/Main

Patch: Goto `Modules/dropbear/compile.sh` and change `hg clone https://secure.ucc.asn.au/hg/dropbear` (line 8) to

        git clone https://github.com/mkj/dropbear.git

At the time of writing it'd look like this:

        #!/usr/bin/env bash
        export CFLAGS="-muclibc -O3 -DFAKE_ROOT "
        export CPPFLAGS="-muclibc -O3"
        export LDFLAGS="-muclibc -O3"
        . ../../setCompilePath.sh
        if [ ! -d dropbear ]
        then
            git clone https://github.com/mkj/dropbear.git
        fi
        cp *.h dropbear
        cd dropbear/
        echo '#define DEFAULT_PATH "/usr/bin:/bin:/system/bin:/system/sdcard/bin"' > localoptions.h

        autoconf; autoheader
        make clean
        ./configure --host=mips-linux --disable-zlib
        make PROGRAMS="dropbear dbclient scp dropbearkey dropbearconvert" MULTI=1 SCPPROGRESS=1

        cp dropbearmulti ${INSTALL}/bin