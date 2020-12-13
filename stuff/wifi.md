## WiFi

Create a wpa_supplicant config

"/etc/wpa.conf"

    ctrl_interface=/var/run/wpa_supplicant
    ap_scan=1
    network={
            ssid="networkname"
            key_mgmt=WPA-PSK
            pairwise=CCMP TKIP
            group=CCMP TKIP WEP104 WEP40
            psk="networkpassword"
            scan_ssid=1
            priority=2
    }

Start wpa_supplicant

    wpa_supplicant -D nl80211 -i wlan0 -c /tmp/wpa.conf -B

Set a static address or run DHCP:

    udhcpc -i wlan0 -s /system/etc/udhcpc.script -q