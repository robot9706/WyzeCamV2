import time
import machine

def ir_cut(enable):
        pin25 = machine.Pin(25, machine.Pin.OUT)
        pin26 = machine.Pin(26, machine.Pin.OUT)
        pin49 = machine.Pin(49, machine.Pin.OUT)

        if enable:
                pin49.value(1)

                pin26.value(0)
                pin25.value(1)
                time.sleep(1)
                pin25.value(0)
        else:
                pin49.value(0)

                pin25.value(0)
                pin26.value(1)
                time.sleep(1)
                pin26.value(0)

def parse_line(s):
        parts = s.split(':')
        if len(parts) != 2:
                return None
        numPart = parts[1].strip()
        return int(numPart)

ir = False
ir_cut(False)

while True:
        file = open("/proc/jz/isp/isp_info", "r")
        lines = file.readlines()
        file.close()

        exposure = None
        iridix = None
        colortemp = None
        for line in lines:
                if line.startswith("ISP exposure log2 id"):
                        exposure = parse_line(line)
                if line.startswith("ISP Iridix strength"):
                        iridix = parse_line(line)
                if line.startswith("ISP WB Temperature"):
                        colortemp = parse_line(line)

        print(exposure, iridix, colortemp)

        if ir == False and exposure >= 1000000:
                ir = True
                ir_cut(True)
                print("CUT")
        if ir == True and exposure <= 1000000:
                ir = False
                ir_cut(False)
                print("NO CUT")

        time.sleep(1)
