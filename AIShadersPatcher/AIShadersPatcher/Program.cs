using CATHODE;
using System.CommandLine;
using System.Diagnostics;
using System.Text.RegularExpressions;

namespace AIShadersPatcher
{
    class Program
    {
        static string _pattern1 = @"float4 SourceResolution : packoffset";
        static string _pattern2 = @"r(\d)\.xy = float2\(0\.00100000005,0\.00100000005\) \* v(\d)\.xy;" + "\n" + @"\s+" + @"r(\d)\.zw = v(\d)\.xy \* float2\(0\.00100000005,0\.00100000005\) \+ -v(\d)\.xx;";

        static string _template = @"float2 normalizedCoords = v{2}.xy / SourceResolution.xy;
  r{1}.xy = normalizedCoords * float2(0.001, 0.001);
  r{1}.zw = normalizedCoords * float2(0.001, 0.001) - v{5}.xx;";

        static string _defaultDirectoryPath = Path.Combine(Directory.GetCurrentDirectory(), "Data");
        static string _defaultCompilerPath = "fxc.exe";
        static string _defaultDecompilerPath = "cmd_Decompiler.exe";

        static string _dxbcPath = "temp.dxbc";
        static string _hlslPath = "temp.hlsl";

        static int Main(string[] args)
        {
            //Options
            var patchOption = new Option<bool>("--P", "Apply patch to shaders");
            var restoreOption = new Option<bool>("--R", "Restore original shaders");
            var directoryOption = new Option<string>("--DataDirectory", "Path to AI DATA folder. Defaults to \\DATA");
            var compilerOption = new Option<string>("--Compiler", "Path to shader compiler. Defaults to fxc.exe");
            var decompilerOption = new Option<string>("--Decompiler", "Path to shader decompiler. Defaults to cmd_Decompiler.exe");

            //Root command
            var rootCommand = new RootCommand
        {
            patchOption,
            restoreOption,
            directoryOption,
            compilerOption,
            decompilerOption
        };

            //Set command handler
            rootCommand.SetHandler((bool patch, bool restore, string directoryPath, string compilerPath, string decompilerPath) =>
            {
                if (patch && restore)
                {
                    Console.WriteLine("Cannot combine --P and --R.");
                    Environment.Exit(1);
                }

                if (patch)
                {
                    PatchShaderPAKs(directoryPath ?? _defaultDirectoryPath, decompilerPath ?? _defaultDecompilerPath, compilerPath ?? _defaultCompilerPath);
                }
                if (restore)
                {
                    RestoreShaderPaks(directoryPath ?? _defaultDirectoryPath);
                }
            }, patchOption, restoreOption, directoryOption, compilerOption, decompilerOption);

            //Parse arguments
            if (args.Length == 0)
            {
                return rootCommand.Invoke("-h");
            }
            else
            {
                return rootCommand.InvokeAsync(args).Result;
            }
        }

        static void PatchShaderPAKs(string directoryPath, string decompilerPath, string compilerPath)
        {
            try
            {
                Regex regex1 = new Regex(_pattern1);
                Regex regex2 = new Regex(_pattern2);

                foreach (string file in Directory.GetFiles(directoryPath, "LEVEL_SHADERS_DX11.PAK", SearchOption.AllDirectories))
                {
                    Console.WriteLine("Parsing " + file);

                    //UnPAK Shaders
                    Shaders shaders = new Shaders(file);

                    if (shaders.Entries != null && shaders.Entries.Count > 0)
                    {
                        BackupShaderPAKsIfNotFound(file);

                        //Parse each entry
                        Console.Write("Patching file");
                        int count = 0;
                        int index = 0;
                        foreach (var shader in shaders.Entries)
                        {
                            if (index % 25 == 0) { Console.Write("."); }
                            if (shader.PixelShader == null) { index++; continue; }//We are only interested in pixel shaders

                            //Write binary data to disk
                            File.WriteAllBytes(_dxbcPath, shader.PixelShader);

                            //Decompile
                            RunExecutable(decompilerPath, "-D " + _dxbcPath);

                            //Read Decompiled file and search for relevant pattern
                            string decompiledShader = File.ReadAllText(_hlslPath);
                            Match match1 = regex1.Match(decompiledShader);
                            Match match2 = regex2.Match(decompiledShader);
                            if (match1.Success && match2.Success)
                            {
                                if (match2.Groups.Count == 6
                                    && match2.Groups.Values.ElementAt(1).Value == match2.Groups.Values.ElementAt(3).Value
                                    && match2.Groups.Values.ElementAt(2).Value == match2.Groups.Values.ElementAt(4).Value
                                    )
                                {
                                    //Replace pattern
                                    string replacement = _template.Replace("{1}", match2.Groups.Values.ElementAt(1).Value).Replace("{2}", match2.Groups.Values.ElementAt(2).Value).Replace("{5}", match2.Groups.Values.ElementAt(5).Value);
                                    decompiledShader = decompiledShader.Replace(match2.Groups.Values.ElementAt(0).Value, replacement);

                                    //Write patched shader hlsl to disk
                                    File.WriteAllText(_hlslPath, decompiledShader);

                                    //Recompile shader
                                    RunExecutable(compilerPath, "/T ps_5_0 /E main /O2 /Fo " + _dxbcPath + " " + _hlslPath);

                                    //Read binary data from disk and update existing shader
                                    shader.PixelShader = File.ReadAllBytes(_dxbcPath);

                                    //Update count
                                    count++;
                                }
                            }
                            index++;
                        }
                        Console.WriteLine("\nPatched " + count + " matching shaders of " + shaders.Entries.Count + " shaders total.");

                        //RePAK shaders
                        shaders.Save(file);
                    }
                }

                //Cleanup temp files
                if (File.Exists(_dxbcPath))
                { 
                    File.Delete(_dxbcPath);
                }
                if (File.Exists(_hlslPath))
                {
                    File.Delete(_hlslPath);
                }

                Console.WriteLine("Done patching all found files.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"\nException occurred: \n{ex.Message}");
                Environment.Exit(2);
            }
        }

        static void RestoreShaderPaks(string directoryPath)
        {
            try
            {
                foreach (string file in Directory.GetFiles(directoryPath, "*.PAK.BAK", SearchOption.AllDirectories))
                {
                    Console.WriteLine("Restoring " + file);
                    File.Move(file, Path.GetFullPath(file).Replace(".PAK.BAK", ".PAK"), true);
                }
                Console.WriteLine("Restored all files.");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"\nException occured: \n{ex.Message}");
                Environment.Exit(2);
            }
        }

        static void BackupShaderPAKsIfNotFound(string file)
        {
            try
            {
                string backupPath = Path.GetFullPath(file).Replace(".PAK", ".PAK.BAK");
                if (!File.Exists(backupPath))
                {
                    Console.WriteLine("Backup not found, creating...");
                    File.Move(file, backupPath);

                    string binFile = file.Replace(".PAK", "_BIN.PAK");
                    File.Move(binFile, Path.GetFullPath(binFile).Replace(".PAK", ".PAK.BAK"));

                    string IdxRemapFile = file.Replace(".PAK", "_IDX_REMAP.PAK");
                    File.Move(IdxRemapFile, Path.GetFullPath(IdxRemapFile).Replace(".PAK", ".PAK.BAK"));
                }
            }
            catch (Exception)
            {
                throw;
            }
        }

        static void RunExecutable(string path, string arguments)
        {
            try
            {
                Process process = new Process();
                process.StartInfo.FileName = path;
                process.StartInfo.Arguments = arguments;
                process.StartInfo.UseShellExecute = false;
                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.RedirectStandardError = true;
                process.Start();

                //Read the output
                string output = process.StandardOutput.ReadToEnd();
                string error = process.StandardError.ReadToEnd();

                process.WaitForExit();

                int exitCode = process.ExitCode;
                if (exitCode != 0)//Error occured
                {
                    Console.WriteLine("Process exited with code: " + exitCode);
                    Console.WriteLine("Output: " + output);
                    Console.WriteLine("Error: " + error);
                    throw new Exception("Process execution failed.");
                }
            }
            catch (Exception)
            {
                throw;
            }
        }
    }
}
