## GPIOs

    /sys/class/gpio

    echo 46 > export
    
    cd gpio46
    echo out > direction

    cat value


|Name|Direction|Description|
|---|---|---|
|GPIO25|Out|IR filter control #1|
|GPIO26|Out|IR filter control #2|
|GPIO46|In|Setup button|
|GPIO49|Out|IR LEDs|


### IR CUT

The IR filter is controlled by a simple coil. The filter can be flipped using an H-bridge connected to GPIOs 25 and 26.

Enabling the IR filter: GPIO25 = 0, GPIO26 = 1 then wait for a little and GPIO26 = 0 (to disable the H-bridge).

Disabling the IR filter: GPIO26 = 0, GPIO25 = 1 then wait for a little and GPIO25 = 0 (to disable the H-bridge).