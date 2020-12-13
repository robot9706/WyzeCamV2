using System;
using System.IO;

namespace FlashImageTool
{
    class Program
    {
        static Tuple<int, int, string>[] parts = new Tuple<int, int, string>[]
        {
            new Tuple<int, int, string>(0x0000000, 0x0040000, "boot"),
            new Tuple<int, int, string>(0x0040000, (0x0240000 - 0x0040000), "kernel"),
            new Tuple<int, int, string>(0x0240000, (0x0590000 - 0x0240000), "root"),
            new Tuple<int, int, string>(0x0590000, (0x0630000 - 0x0590000), "driver"),
            new Tuple<int, int, string>(0x0630000, (0x0ad0000 - 0x0630000), "appfs"),
            new Tuple<int, int, string>(0x0ad0000, (0x0cd0000 - 0x0ad0000), "backupk"),
            new Tuple<int, int, string>(0x0cd0000, (0x0d70000 - 0x0cd0000), "backupd"),
            new Tuple<int, int, string>(0x0d70000, (0x0f70000 - 0x0d70000), "backupa"),
            new Tuple<int, int, string>(0x0f70000, (0x0fb0000 - 0x0f70000), "config"),
            new Tuple<int, int, string>(0x0fb0000, (0x0ff0000 - 0x0fb0000), "para"),
            new Tuple<int, int, string>(0x0ff0000, (0x1000000 - 0x0ff0000), "flag")
        };

        static void Main(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("p[ack] {directory} {output file}");
                Console.WriteLine("u[npack] {directory} {input file}");
                return;
            }

            string directory = args[1];
            string file = args[2];

            switch (args[0])
            {
                case "p":
                case "pack":
                    using (FileStream ofs = new FileStream(file, FileMode.Create))
                    {
                        for (int x = 0; x < parts.Length; x++)
                        {
                            Console.WriteLine("Packing " + parts[x].Item3);

                            using (FileStream partin = new FileStream(Path.Combine(directory, x.ToString() + "-" + parts[x].Item3 + ".bin"), FileMode.Open))
                            {
                                int expectedLength = parts[x].Item2;
                                if (partin.Length > expectedLength)
                                {
                                    Console.WriteLine("Part bigger than space! " + partin.Length.ToString() + " > " + expectedLength.ToString());
                                    return;
                                }

                                partin.CopyTo(ofs);

                                if (partin.Length < expectedLength)
                                {
                                    int fillSize = expectedLength - (int)partin.Length;
                                    Console.WriteLine("\t Filling " + fillSize.ToString() + " bytes");
                                    byte[] filler = new byte[fillSize];
                                    ofs.Write(filler, 0, filler.Length);
                                }

                                if (ofs.Position != parts[x].Item1 + parts[x].Item2)
                                {
                                    Console.WriteLine("Unexpected file offset!");
                                    return;
                                }
                            }
                        }
                    }
                    break;

                case "u":
                case "unpack":
                    using (FileStream ifs = new FileStream(file, FileMode.Open))
                    {
                        if (!Directory.Exists(directory))
                        {
                            Directory.CreateDirectory(directory);
                        }

                        for (int x = 0; x < parts.Length; x++)
                        {
                            Console.WriteLine("Unpacking " + parts[x].Item3);

                            using (FileStream partout = new FileStream(Path.Combine(directory, x.ToString() + "-" + parts[x].Item3 + ".bin"), FileMode.Create))
                            {
                                ifs.Position = parts[x].Item1;

                                int partLength = parts[x].Item2;
                                byte[] buff = new byte[partLength];
                                int readCount = ifs.Read(buff, 0, partLength);
                                if (readCount != partLength)
                                {
                                    Console.WriteLine("Unable to read data!");
                                    return;
                                }

                                partout.Write(buff, 0, partLength);
                            }
                        }
                    }
                    break;
            }

            Console.WriteLine("Done");
        }
    }
}
