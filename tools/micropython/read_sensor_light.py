import time

def parse_line(s):
        parts = s.split(':')
        if len(parts) != 2:
                return None
        numPart = parts[1].strip()
        return int(numPart)

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

        time.sleep(1)
