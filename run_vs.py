import subprocess
import argparse


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', type=str, default="vs22", help='Compiller type: vs22 (default: vs22)')
    parser.add_argument('-a', type=str, default="x64", help='Architecture: x86, x64 (default: x64)')
    parser.add_argument('-p', type=str, default="win", help='Platform: win, uwp, linux , macos, android, ios, web, switch (default: win)')
    parser.add_argument('-e', type=bool, default=True, help='Edittor mode (default: True)')
    args = parser.parse_args()


    # make header only sol
    subprocess.run(['python', './3rd/sol/single/single.py'])

    command = ['cmake ./ -Bbuild']

    if (args.c == 'vs22'):
        command.append('-G "Visual Studio 16 2019"')
    
    if (args.a == 'x64'):
        command.append('-A x64')
    

    subprocess.run(command)


# copy imgui_impl_glfw.h imgui_impl_glfw.cpp
# copy imgui_impl_opengl3.h imgui_impl_opengl3.cpp
# copy imguizmot to imgui
# add /bigobj

# разобраться с шрифтами
# скопировать dll в папку с .exe