"""****************************************************************************
*	Copyright 2016-2017 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
****************************************************************************"""

import os
import sys
import subprocess

class Logging:
    # TODO maybe the right way to do this is to use something like colorama?
    RED = '\033[31m'
    GREEN = '\033[32m'
    YELLOW = '\033[33m'
    MAGENTA = '\033[35m'
    RESET = '\033[0m'

    @staticmethod
    def _print(s, color=None):
        if color and sys.stdout.isatty() and sys.platform != 'win32':
            print(color + s + Logging.RESET)
        else:
            print(s)

    @staticmethod
    def debug(s):
        Logging._print(s, Logging.MAGENTA)

    @staticmethod
    def info(s):
        Logging._print(s, Logging.GREEN)

    @staticmethod
    def warning(s):
        Logging._print(s, Logging.YELLOW)

    @staticmethod
    def error(s):
        Logging._print(s, Logging.RED)

def _check_python_version():
    major_ver = sys.version_info[0]
    if major_ver < 3:
        print ("The python version is %d.%d. But python 3.x is required. (Version 3.6 is well tested)\n"
               "Download it here: https://www.python.org/" % (major_ver, sys.version_info[1]))
        return False

    return True

def check_environment_variable(var):
    ''' Checking the environment variable, if found then return it's value, else raise error
    '''
    try:
        value = os.environ[var]
    except Exception:
        raise Exception('ERROR: Environment varialbe not found = ', var)
    return value

def build(build_path, root_path, args = None):
    try:
        #Check if build path exists
        if not os.path.exists(build_path):
            os.makedirs(build_path)


        #Set ndk and cmake path
        ndk_toolchain_path = os.environ['NDK_ROOT'] + "\\build\\cmake\\android.toolchain.cmake"
        cmake_program_path = os.environ['ANDROID_SDK_ROOT'] + "\\cmake\\3.6.3155560\\bin\\ninja"
        cmake_bin = os.environ['ANDROID_SDK_ROOT'] + '\\cmake\\3.6.3155560\\bin\\cmake'

        shell = False
        if sys.platform.startswith('win'):
            shell = True
        else:
            shell = False


        if args is None:
            cmakeCmd = [cmake_bin,
                    '-GAndroid Gradle - Ninja',
                    '-DANDROID_ABI=arm64-v8a',
                    '-DANDROID_NDK=' + os.environ['NDK_ROOT'],
                    '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + build_path + '\\lib',
                    '-DCMAKE_MAKE_PROGRAM=' + cmake_program_path,
                    '-DCMAKE_TOOLCHAIN_FILE=' + ndk_toolchain_path,
                    '-DANDROID_NATIVE_API_LEVEL = 24',
                    '-DANDROID_TOOLCHAIN = clang ',
                    '-DANDROID_TOOLCHAIN_NAME=arm-linux-androideabi-4.9',
                    '-DCMAKE_BUILD_TYPE=Debug;Release',
                    '-B'+build_path,
                    '-H'+root_path]
        else:
            cmakeCmd = [cmake_bin,
                    '-GAndroid Gradle - Ninja',
                    '-DANDROID_ABI=arm64-v8a',
                    '-DANDROID_NDK=' + os.environ['NDK_ROOT'],
                    '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + build_path + '\\lib',
                    '-DCMAKE_MAKE_PROGRAM=' + cmake_program_path,
                    '-DCMAKE_TOOLCHAIN_FILE=' + ndk_toolchain_path,
                    '-DANDROID_NATIVE_API_LEVEL = 24',
                    '-DANDROID_TOOLCHAIN = clang ',
                    '-DANDROID_TOOLCHAIN_NAME=arm-linux-androideabi-4.9',
                    '-DCMAKE_BUILD_TYPE=Debug;Release',
                    '-B'+build_path,
                    '-H'+root_path,
                    args]

        retCode = subprocess.check_call(cmakeCmd, stderr=subprocess.STDOUT, shell=shell)
    except Exception as e:
        raise e

if __name__ == "__main__":
    if not _check_python_version():
        exit()
    try:
        #Check if environment variable exist
        check_environment_variable('NDK_ROOT')
        check_environment_variable('ANDROID_SDK_ROOT')
        check_environment_variable('ANT_BIN')
        current_path = os.getcwd()

        Logging._print("Building Assimp")
        build('..\\External\\assimp\\build\\android', '..\\External\\assimp', ['-DASSIMP_BUILD_ASSIMP_TOOLS=off', ' -DASSIMP_BUILD_SAMPLES=off', ' -DASSIMP_BUILD_TESTS=off'])

        Logging._print("Building SDL2")
        build('..\\External\\sdl\\build\\android', '..\\External\\sdl')

        Logging._print("Building Freetype")
        build('..\\External\\freetype\\build\\android', '..\\External\\freetype')

    except Exception as e:
        Logging.error(e)