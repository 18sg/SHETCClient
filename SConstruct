import glob
import os.path

env = Environment()

env.ParseConfig("pkg-config json-c --cflags --libs")
env.ParseConfig("pkg-config shet --cflags --libs")

env.Append(CCFLAGS = ['-g', '-Wall', '-O3',
                      '--std=c99',
                      '-D_POSIX_C_SOURCE=200112L'])

shet = env.Program(target="shet", source="src/shet.c")
shetls = env.Program(target="shetls", source="src/shetls.c")

install_path = ARGUMENTS.get("installdir", "/usr/")

inst_bin = env.Install(os.path.join(install_path, "bin"), [shet, shetls, "src/shet_complete"])

env.Alias("install", inst_bin)


