#!python
import os, shutil, subprocess

opts = Variables([], ARGUMENTS)

# Gets the standard flags CC, CCX, etc.
env = DefaultEnvironment()

bin_dir = 'demo/addons/Sunvox/bin/'

if not os.path.isdir(bin_dir):
    os.mkdir(bin_dir)

# Define our options
opts.Add(EnumVariable('target', "Compilation target", 'debug', ['d', 'debug', 'r', 'release']))
opts.Add(EnumVariable('platform', "Compilation platform", '', ['', 'windows', 'x11', 'linux', 'osx']))
opts.Add(EnumVariable('p', "Compilation target, alias for 'platform'", '', ['', 'windows', 'x11', 'linux', 'osx']))
opts.Add(BoolVariable('use_llvm', "Use the LLVM / Clang compiler", 'no'))
opts.Add(PathVariable('target_path', 'The path where the lib is installed.', bin_dir))
opts.Add(PathVariable('target_name', 'Godot Sunvox.', 'libgodotsunvox', PathVariable.PathAccept))

# Local dependency paths, adapt them to your setup
godot_headers_path = "godot_headers/"

# only support 64 at this time..
bits = 64

# Updates the environment with the option variables.
opts.Update(env)

# Process some arguments
if env['use_llvm']:
    env['CC'] = 'clang'
    env['CXX'] = 'clang++'

if env['p'] != '':
    env['platform'] = env['p']

if env['platform'] == '':
    print("No valid target platform selected.")
    quit();

# Check our platform specifics
if env['platform'] == "osx":
    env['target_path'] += 'osx/'
    if env['target'] in ('debug', 'd'):
        env.Append(CCFLAGS = ['-g','-O2', '-arch', 'x86_64'])
        env.Append(LINKFLAGS = ['-arch', 'x86_64'])
    else:
        env.Append(CCFLAGS = ['-g','-O3', '-arch', 'x86_64'])
        env.Append(LINKFLAGS = ['-arch', 'x86_64'])

elif env['platform'] in ('x11', 'linux'):
    env['target_path'] += 'linux_x86_64/'
    if env['target'] in ('debug', 'd'):
        env.Append(CCFLAGS = ['-fPIC', '-g3','-Og'])
    else:
        env.Append(CCFLAGS = ['-fPIC', '-g','-O3'])

elif env['platform'] == "windows":
    env['target_path'] += 'win64/'
    # This makes sure to keep the session environment variables on windows,
    # that way you can run scons in a vs 2017 prompt and it will find all the required tools
    env.Append(ENV = os.environ)

    env.Append(CCFLAGS = ['-DWIN32', '-D_WIN32', '-D_WINDOWS', '-W3', '-GR', '-D_CRT_SECURE_NO_WARNINGS'])
    if env['target'] in ('debug', 'd'):
        env.Append(CCFLAGS = ['-EHsc', '-D_DEBUG', '-MDd'])
    else:
        env.Append(CCFLAGS = ['-O2', '-EHsc', '-DNDEBUG', '-MD'])

env.Append(LINKFLAGS=[
    '-Wl,-rpath,\'$$ORIGIN\''
])

# make sure our binding library is properly includes
env.Append(CPPPATH=['.', godot_headers_path])

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=['src/'])
sources = Glob('src/*.c')

library = env.SharedLibrary(target=env['target_path'] + env['target_name'] , source=sources)

Default(library)

if not os.path.isdir(env['target_path']):
    os.mkdir(env['target_path'])

if env['platform'] == "osx":
    shutil.copyfile('sunvox_lib/macos/lib_x86_64/sunvox.dylib', env['target_path'] + 'sunvox.dylib')
elif env['platform'] in ('x11', 'linux'):
    shutil.copyfile('sunvox_lib/linux/lib_x86_64/sunvox.so', env['target_path'] + 'sunvox.so')
elif env['platform'] == "windows":
    shutil.copyfile('sunvox_lib/windows/lib_x86_64/sunvox.dll', env['target_path'] + 'sunvox.dll')

if os.path.isdir('demo/addons/SunvoxCallback/bin'):
    shutil.rmtree('demo/addons/SunvoxCallback/bin')

shutil.copytree('demo/addons/Sunvox/bin', 'demo/addons/SunvoxCallback/bin')

# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))
