## Boot log:

    U-Boot SPL 2013.07 (Jul 05 2018 - 13:33:27)
    pll_init:365
    l2cache_clk = 375000000
    pll_cfg.pdiv = 8, pll_cfg.h2div = 4, pll_cfg.h0div = 4, pll_cfg.cdiv = 1, pll_cfg.l2div = 3
    nf=36 nr = 1 od0 = 1 od1 = 1
    cppcr is 02404900
    CPM_CPAPCR 0470890d
    nf=42 nr = 1 od0 = 1 od1 = 1
    cppcr is 02a04900
    CPM_CPMPCR 07d0c90d
    nf=50 nr = 1 od0 = 1 od1 = 1
    cppcr is 03204900
    CPM_CPVPCR 0320490d
    cppcr 0x9a794410
    apll_freq 860160000
    mpll_freq 1000000000
    vpll_freq = 1200000000
    ddr sel mpll, cpu sel apll
    ddrfreq 500000000
    cclk  860160000
    l2clk 286720000
    h0clk 250000000
    h2clk 250000000
    pclk  125000000
    DDRC_DLP:0000f003


    U-Boot 2013.07 (Jul 05 2018 - 13:33:27)

    Board: ISVP (Ingenic XBurst T20 SoC)
    DRAM:  128 MiB
    Top of RAM usable for U-Boot at: 84000000
    Reserving 399k for U-Boot at: 83f9c000
    Reserving 32784k for malloc() at: 81f98000
    Reserving 32 Bytes for Board Info at: 81f97fe0
    Reserving 124 Bytes for Global Data at: 81f97f64
    Reserving 128k for boot params() at: 81f77f64
    Stack Pointer at: 81f77f48
    Now running in RAM - U-Boot at: 83f9c000
    MMC:   msc: 0
    the manufacturer c8
    SF: Detected GD25Q128

    *** Warning - bad CRC, using default environment

    In:    serial
    Out:   serial
    Err:   serial
    misc_init_r before change the wifi_enable_gpio
    gpio_request lable = wifi_enable_gpio gpio = 62
    misc_init_r after gpio_request the wifi_enable_gpio ret is 62
    misc_init_r after change the wifi_enable_gpio ret is 0
    misc_init_r before change the yellow_gpio
    gpio_request lable = yellow_gpio gpio = 38
    misc_init_r after gpio_request the yellow_gpio ret is 38
    misc_init_r after change the yellow_gpio ret is 0
    misc_init_r before change the blue_gpio
    gpio_request lable = blue_gpio gpio = 39
    misc_init_r after gpio_request the blue_gpio ret is 39
    misc_init_r after change the blue_gpio ret is 1
    gpio_request lable = night_gpio gpio = 81
    misc_init_r after gpio_request the night_gpio ret is 81
    misc_init_r after change the night_gpio ret is 0
    gpio_request lable = night_gpio gpio = 25
    misc_init_r after gpio_request the night_gpio ret is 25
    misc_init_r after change the night_gpio ret is 0
    gpio_request lable = night_gpio gpio = 49
    misc_init_r after gpio_request the night_gpio ret is 49
    misc_init_r after change the night_gpio ret is 0
    gpio_request lable = USB_able_gpio gpio = 47
    misc_init_r after gpio_request the USB_able_gpio ret is 47
    misc_init_r after change the USB_able_gpio ret is 0
    gpio_request lable = TF_able_gpio gpio = 43
    misc_init_r after gpio_request the TF_able_gpio ret is 43
    misc_init_r after change the TF_able_gpio ret is 1
    gpio_request lable = SPK_able_gpio gpio = 63
    misc_init_r after gpio_request the SPK_able_gpio ret is 63
    misc_init_r after change the SPK_able_gpio ret is 0
    gpio_request lable = SD_able_gpio gpio = 48
    misc_init_r after gpio_request the SD_able_gpio ret is 48
    misc_init_r after change the SD_able_gpio ret is 0
    misc_init_r before change the wifi_enable_gpio
    gpio_request lable = wifi_enable_gpio gpio = 62
    misc_init_r after gpio_request the wifi_enable_gpio ret is 62
    misc_init_r after change the wifi_enable_gpio ret is 1
    Hit any key to stop autoboot:  0


## Available commands:

    isvp_t20# help
    ?       - alias for 'help'
    base    - print or set address offset
    boot    - boot default, i.e., run 'bootcmd'
    boota   - boot android system
    bootd   - boot default, i.e., run 'bootcmd'
    bootm   - boot application image from memory
    chpart  - change active partition
    cmp     - memory compare
    coninfo - print console devices and information
    cp      - memory copy
    crc32   - checksum calculation
    echo    - echo args to console
    env     - environment handling commands
    fatinfo - print information about filesystem
    fatload - load binary file from a dos filesystem
    fatls   - list files in a directory (default /)
    gettime - get timer val elapsed,

    go      - start application at address 'addr'
    help    - print command description/usage
    loadb   - load binary file over serial line (kermit mode)
    loads   - load S-Record file over serial line
    loady   - load binary file over serial line (ymodem mode)
    loop    - infinite loop on address range
    md      - memory display
    mm      - memory modify (auto-incrementing address)
    mmc     - MMC sub system
    mmcinfo - display MMC info
    mtdparts- define flash/nand partitions
    mw      - memory write (fill)
    nm      - memory modify (constant address)
    printenv- print environment variables
    reset   - Perform RESET of the CPU
    run     - run commands in an environment variable
    saveenv - save environment variables to persistent storage
    sdupdate- auto upgrade file from mmc to flash
    setenv  - set environment variables
    sf      - SPI flash sub-system
    sleep   - delay execution for some time
    source  - run script from memory
    version - print monitor, compiler and linker version

## Environment:

    isvp_t20# env print
    baudrate=115200
    bootargs=console=ttyS1,115200n8 mem=104M@0x0 ispmem=8M@0x6800000 rmem=16M@0x7000000 init=/linuxrc rootfstype=squashfs root=/dev/mtdblock2 rw mtdparts=jz_sfc:256k(boot),2048k(kernel),3392k(root),640k(driver),4736k(appfs),2048k(backupk),640k(backupd),2048k(backupa),256k(config),256k(para),-(flag)
    bootcmd=sdupdate;sf probe;sf read 0x80600000 0x40000 0x280000; bootm 0x80600000
    bootdelay=1
    ethaddr=00:11:22:33:44:55
    gatewayip=193.169.4.1
    ipaddr=193.169.4.81
    loads_echo=1
    netmask=255.255.255.0
    serverip=193.169.4.2
    stderr=serial
    stdin=serial
    stdout=serial

    Environment size: 596/16380 bytes

## Dumping the SPI flash to an SD card:

The flash is 16MB or 16 777 216 bytes so the address space is: 0x0 - 0xFFFFFF.

1) Probe the flash

        isvp_t20# sf probe
        the manufacturer c8
        SF: Detected GD25Q128

        --->probe spend 4 ms

2) Read flash contents into RAM

        isvp_t20# sf read 0x80600000 0x0 0x1000000
        SF: 16777216 bytes @ 0x0 Read: OK
        --->read spend 2419 ms

3) Check SD

        isvp_t20# mmc rescan

        isvp_t20# mmcinfo
        Device: msc
        Manufacturer ID: 27
        OEM: 5048
        Name: SD16G
        Tran Speed: 50000000
        Rd Block Len: 512
        SD version 3.0
        High Capacity: Yes
        Capacity: 14.4 GiB
        Bus Width: 4-bit

4) Write RAM contents to SD

        isvp_t20# mmc write 0x80600000 0 0x8000

        MMC write: dev # 0, block # 0, count 32768 ... 32768 blocks write: OK

## Boot without starting any services

1) Change the "bootargs" so a bash shell is started after boot

    Basically just swap `init=/linuxrc` with `init=/bin/sh`.

       setenv bootargs console=ttyS1,115200n8 mem=104M@0x0 ispmem=8M@0x6800000 rmem=16M@0x7000000 init=/bin/sh rootfstype=squashfs root=/dev/mtdblock2 rw mtdparts=jz_sfc:256k(boot),2048k(kernel),3392k(root),640k(driver),4736k(appfs),2048k(backupk),640k(backupd),2048k(backupa),256k(config),256k(para),-(flag)

2) Boot into Linux

        isvp_t20# run bootcmd

## Root access

    Username: "root"
    Password: "ismart12"