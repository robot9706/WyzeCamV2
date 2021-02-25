using System;
using System.Linq;
using System.IO.Ports;
using System.Threading;
using System.Collections.Generic;
using System.Text;

namespace UBootTool
{
    class Program
    {
        // Flash partitions
        [Flags]
        enum PartitionWarn : byte
        {
            None = 0,
            Warning = 1,
            Advanced = 2
        }

        // From, Length, Name
        static Tuple<int, int, string, PartitionWarn>[] flashPartitions = new Tuple<int, int, string, PartitionWarn>[]
        {
            new Tuple<int, int, string, PartitionWarn>(0x0000000, 0x0040000, "boot", PartitionWarn.Advanced),
            new Tuple<int, int, string, PartitionWarn>(0x0040000, (0x0240000 - 0x0040000), "kernel", PartitionWarn.Warning),
            new Tuple<int, int, string, PartitionWarn>(0x0240000, (0x0590000 - 0x0240000), "root", PartitionWarn.None),
            new Tuple<int, int, string, PartitionWarn>(0x0590000, (0x0630000 - 0x0590000), "driver", PartitionWarn.None),
            new Tuple<int, int, string, PartitionWarn>(0x0630000, (0x0ad0000 - 0x0630000), "appfs", PartitionWarn.None),
            new Tuple<int, int, string, PartitionWarn>(0x0ad0000, (0x0cd0000 - 0x0ad0000), "backupk", PartitionWarn.None),
            new Tuple<int, int, string, PartitionWarn>(0x0cd0000, (0x0d70000 - 0x0cd0000), "backupd", PartitionWarn.None),
            new Tuple<int, int, string, PartitionWarn>(0x0d70000, (0x0f70000 - 0x0d70000), "backupa", PartitionWarn.None),
            new Tuple<int, int, string, PartitionWarn>(0x0f70000, (0x0fb0000 - 0x0f70000), "config", PartitionWarn.None),
            new Tuple<int, int, string, PartitionWarn>(0x0fb0000, (0x0ff0000 - 0x0fb0000), "para", PartitionWarn.None),
            new Tuple<int, int, string, PartitionWarn>(0x0ff0000, (0x1000000 - 0x0ff0000), "flag", PartitionWarn.None)
        };

        // UBoot and Hardware stuff
        static string UBOOT_PROMPT = "isvp_t20#";

        static ulong RAM_ADDRESS = 0x80600000; // RAM base address
        static ulong FLASH_SIZE = 0x1000000; // Size of flash in bytes
        static ulong SD_BLOCKS = 0x8000; // Size of flash in SD sectors

        // Serial port
        static SerialPort serial;

        static string FormatPartitionFlags(PartitionWarn warn)
        {
            if (warn == PartitionWarn.None)
                return string.Empty;

            return $"({warn.ToString()})";
        }

        static void EnterUBoot()
        {
            while (true)
            {
                // Spam enters
                serial.Write(new byte[] { 0x0D }, 0, 1);

                if (serial.BytesToRead > 0)
                {
                    string line = serial.ReadLine();
                    if (line.StartsWith(UBOOT_PROMPT))
                    {
                        break;
                    }
                }

                Thread.Sleep(10);
            }
        }

        static string[] WaitForPrompt()
        {
            StringBuilder builder = new StringBuilder();

            while (true)
            {
                string data = serial.ReadExisting();
                builder.Append(data);

                if (builder.ToString().Contains(UBOOT_PROMPT))
                {
                    break;
                }
            }

            return builder.ToString().Split('\r', '\n');
        }

        static int Menu(string title, params string[] options)
        {
            Console.WriteLine(title);
            for (int x = 0; x < options.Length; x++)
            {
                Console.WriteLine((x + 1).ToString() + ") " + options[x]);
            }

            string input;
            int selected;
            do
            {
                Console.Write("#? ");
                input = Console.ReadLine();
            }
            while (!Int32.TryParse(input, out selected) && selected < 1 || selected > options.Length);

            return selected - 1;
        }

        static int[] Options(string title, params string[] options)
        {
            Console.WriteLine(title);
            for (int x = 0; x < options.Length; x++)
            {
                Console.WriteLine((x + 1).ToString() + ") " + options[x]);
            }

            while (true)
            {
                Console.Write("Enter multiple options: ");
                string input = Console.ReadLine();
                string[] parts = input.Split(' ');

                int temp;
                if (parts.Any(x => !Int32.TryParse(x, out temp)))
                {
                    continue;
                }

                return parts.Select(x => Int32.Parse(x) - 1).ToArray();
            }
        }

        static string[] RunUBootCommand(string command)
        {
            serial.DiscardInBuffer();

            serial.WriteLine(command);

            return WaitForPrompt();
        }

        static void DumpFlashToSD()
        {
            // Probe flash
            Console.Write("Probing flash... ");

            string[] result = RunUBootCommand("sf probe");
            if (!result.Any(x => x.StartsWith("SF: Detected")))
            {
                Console.WriteLine("Flash probe failed!");
                return;
            }

            Console.WriteLine("OK");

            // Probe MMC (SD)
            Console.Write("Probing SD... ");

            result = RunUBootCommand("mmc rescan");
            if (result.Any(x => x.StartsWith("Card did not respond to voltage select!")))
            {
                Console.WriteLine("No SD found");
                return;
            }

            Console.WriteLine("OK");

            // Copy flash to RAM
            Console.Write("Reading flash to RAM (this might take a few seconds) ... ");

            result = RunUBootCommand($"sf read 0x{RAM_ADDRESS.ToString("X2")} 0x0 0x{FLASH_SIZE.ToString("X2")}"); // sf read [to RAM address] [flash offset] [number of bytes to read]

            string readLine = result.SingleOrDefault(x => x.StartsWith("SF: "));
            if (string.IsNullOrEmpty(readLine))
            {
                Console.WriteLine("Failed");
                return;
            }

            string[] readParts = readLine.Split(' '); // SF: [num bytes] bytes @ 0x0 Read: [OK]
            if (readParts.Last() != "OK")
            {
                Console.WriteLine($"Failed {readParts.Last()}");
                return;
            }

            ulong read = Convert.ToUInt64(readParts[1]);
            if (read != FLASH_SIZE)
            {
                Console.WriteLine("Unexpeced length!");
                return;
            }

            Console.WriteLine("OK");

            // Write RAM to SD
            Console.Write("Writing flash (from RAM) to SD (this might take a while) ... ");

            result = RunUBootCommand($"mmc write 0x{RAM_ADDRESS.ToString("X2")} 0 0x{SD_BLOCKS.ToString("X2")}"); // mmc write [from RAM address] [offset sector in SD] [number of sectors to write]

            readLine = result.SingleOrDefault(x => x.StartsWith("MMC write: dev #"));
            if (string.IsNullOrEmpty(readLine))
            {
                Console.WriteLine("Failed");
                return;
            }

            readParts = readLine.Split(' ');
            if (readParts.Last() != "OK")
            {
                Console.WriteLine($"Failed {readParts.Last()}");
                return;
            }

            int index = Array.FindIndex(readParts, x => x.StartsWith("blocks"));
            if (index < 0)
            {
                Console.WriteLine("Failed");
                return;
            }

            int writeBlocks = Convert.ToInt32(readParts[index - 1]);
            if ((ulong)writeBlocks != SD_BLOCKS)
            {
                Console.WriteLine("Unexpeced block count!");
                return;
            }

            Console.WriteLine("OK");

            // Done
            Console.WriteLine("DONE!");
        }

        static void WriteSDToFlash()
        {
            string[] selection = flashPartitions.Select(x => $"{x.Item3} {FormatPartitionFlags(x.Item4)}").ToArray();
            int[] options = Options("Select partitions to flash:", selection);

            if (options.Length == 0)
            {
                Console.WriteLine("No selection");
                return;
            }

            Console.WriteLine("Paritions to write: " + String.Join(", ", options.Select(x => flashPartitions[x].Item3).ToArray()));

            // Advanced
            if (options.Any(x => flashPartitions[x].Item4.HasFlag(PartitionWarn.Advanced)))
            {
                Console.WriteLine("You selected an ADVANCED partition to flash, only continue if you know what you are doing! You can brick your device!");
                Console.WriteLine("Are you sure want to continue? [Yes/no]");

                string read = Console.ReadLine();
                if (read != "Yes")
                {
                    return;
                }
            }

            // Warning
            if (options.Any(x => flashPartitions[x].Item4.HasFlag(PartitionWarn.Warning)))
            {
                Console.WriteLine("You selected a special partition to flash!");
                Console.WriteLine("Are you sure want to continue? [Yes/no]");

                string read = Console.ReadLine();
                if (read != "Yes")
                {
                    return;
                }
            }

            // Probe flash
            Console.Write("Probing flash... ");

            string[] result = RunUBootCommand("sf probe");
            if (!result.Any(x => x.StartsWith("SF: Detected")))
            {
                Console.WriteLine("Flash probe failed!");
                return;
            }

            Console.WriteLine("OK");

            // Probe MMC (SD)
            Console.Write("Probing SD... ");

            result = RunUBootCommand("mmc rescan");
            if (result.Any(x => x.StartsWith("Card did not respond to voltage select!")))
            {
                Console.WriteLine("No SD found");
                return;
            }

            Console.WriteLine("OK");

            // Read MMC to RAM
            Console.Write("Reading SD into RAM (this might take a while) ... ");

            result = RunUBootCommand($"mmc read 0x{RAM_ADDRESS.ToString("X2")} 0 0x{SD_BLOCKS.ToString("X2")}"); // mmc read [from RAM address] [offset sector in SD] [number of sectors to write]

            string readLine = result.SingleOrDefault(x => x.StartsWith("MMC read: dev #"));
            if (string.IsNullOrEmpty(readLine))
            {
                Console.WriteLine("Failed");
                return;
            }

            string[] readParts = readLine.Split(' ');
            if (readParts.Last() != "OK")
            {
                Console.WriteLine($"Failed {readParts.Last()}");
                return;
            }

            int index = Array.FindIndex(readParts, x => x.StartsWith("blocks"));
            if (index < 0)
            {
                Console.WriteLine("Failed");
                return;
            }

            int writeBlocks = Convert.ToInt32(readParts[index - 1]);
            if ((ulong)writeBlocks != SD_BLOCKS)
            {
                Console.WriteLine("Unexpeced block count!");
                return;
            }

            Console.WriteLine("OK");

            // Write partitions
            for (int optX = 0; optX < options.Length; optX++)
            {
                int optionIndex = options[optX];
                Tuple<int, int, string, PartitionWarn> part = flashPartitions[optionIndex];

                Console.WriteLine($"Writing \"{part.Item3}\"...");

                // Erase partition in flash
                Console.Write($"Erasing {part.Item3} in flash (0x{part.Item1.ToString("X2")} -> 0x{(part.Item1 + part.Item2).ToString("X2")}), this might take a while... ");

                result = RunUBootCommand($"sf erase 0x{part.Item1.ToString("X2")} 0x{part.Item2.ToString("X2")}"); // sf erase [offset] [length]

                readLine = result.SingleOrDefault(x => x.StartsWith("SF:"));
                if (string.IsNullOrEmpty(readLine))
                {
                    Console.WriteLine("Failed");
                    return;
                }

                readParts = readLine.Split(' ');
                if (readParts.Last() != "OK")
                {
                    Console.WriteLine($"Failed {readParts.Last()}");
                    return;
                }

                int numBytesErased = Convert.ToInt32(readParts[1]);
                if (numBytesErased != part.Item2)
                {
                    Console.WriteLine("Unexpected erased bytes!");
                    return;
                }

                Console.WriteLine("OK");

                // Write partition from RAM to flash
                ulong partitionInRamAddress = (RAM_ADDRESS + (ulong)part.Item1);

                Console.Write($"Writing partition from RAM (0x{partitionInRamAddress.ToString("X2")}) to flash... ");

                result = RunUBootCommand($"sf write 0x{partitionInRamAddress.ToString("X2")} 0x{part.Item1.ToString("X2")} 0x{part.Item2.ToString("X2")}"); // sf write [source ram address] [target flash offset] [num bytes]

                readLine = result.SingleOrDefault(x => x.StartsWith("SF:"));
                if (string.IsNullOrEmpty(readLine))
                {
                    Console.WriteLine("Failed");
                    return;
                }

                readParts = readLine.Split(' ');
                if (readParts.Last() != "OK")
                {
                    Console.WriteLine($"Failed {readParts.Last()}");
                    return;
                }

                int numBytesWritten = Convert.ToInt32(readParts[1]);
                if (numBytesWritten != part.Item2)
                {
                    Console.WriteLine("Unexpected written bytes!");
                    return;
                }

                Console.WriteLine("OK");
            }

            // Done
            Console.WriteLine("DONE!");
        }

        static void Main(string[] args)
        {
            Console.WriteLine("Make sure your WyzeCam is turned off.");

            // Get serial port name
            string port;
            do
            {
                Console.Write("Port? ");
            }
            while (string.IsNullOrEmpty(port = Console.ReadLine())) ;

            // Get serial port baud
            int baud;
            string read;
            do
            {
                Console.Write("Baud? (115200) ");

                read = Console.ReadLine();
                if (string.IsNullOrEmpty(read))
                {
                    baud = 115200;
                    break;
                }
            }
            while (!Int32.TryParse(read, out baud));

            // Open port
            try
            {
                serial = new SerialPort(port, baud);
                serial.Open();

                serial.DiscardInBuffer();
                serial.DiscardOutBuffer();
            }
            catch (Exception)
            {
                Console.WriteLine("Failed to open port.");
                return;
            }

            // Wait for UBoot prompt
            Console.WriteLine("Turn on your WyzeCam (V2) now");
            Console.WriteLine("Waiting for UBoot prompt...");
            EnterUBoot();

            serial.DiscardInBuffer();

            // Present main menu
            while (true)
            {
                bool exit = false;
                switch (Menu("Select an option:", "Dump flash to SD", "Write SD to flash (more options)", "Quit"))
                {
                    case 0:
                        DumpFlashToSD();
                        break;
                    case 1:
                        WriteSDToFlash();
                        break;

                    case 2:
                        exit = true;
                        break;
                }

                if (exit)
                {
                    break;
                }
            }

            // Exit
            serial.Close();
        }
    }
}
