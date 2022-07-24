#!/usr/bin/python3

import os
import sys

#   submodule update
def submodule_update(submoddir) :
    if (False == os.path.isfile(submoddir + "/.git")):
        if (os.path.isdir(submoddir)) :
            os.system("rm -rf " + submoddir)
        os.system("git submodule update --init --recursive " + submoddir)
    if (not os.path.isdir(submoddir)):
        print("Error: No " + submoddir + " submodule")
        exit(1)

def help():

    print(" libote \n\tlibOTe module.")
    print(" xsce  \n\txsce module.")
    print(" debug  \n\tdebug build.")
    print(" release  \n\trelease build, default.")
    print(" clean  \n\tclean build.")
    print(" verbose \n\tshow the build details.")
    print(" install \n\tinstall the exe and library to the default location.")
    print(" install=prefix  \n\tinstall exe and library to the provided predix.")
    print(" setup  \n\tsetup the source code of module in third_party.")
    print(" docker=images  \n\tbuild the docker.")
    print(" sodu \n\tprivilege escalation.")

def copy_files(srcpath, dstpath, subdir, tail2, sudoflag):

    for i in os.listdir(srcpath):
        sub_path = os.path.join(srcpath, i)
        if os.path.isdir(sub_path):
            local_dir=""
            if subdir=="":
                local_dir=i
            else :
                local_dir=subdir+"/"+i
            copy_files(sub_path, dstpath, local_dir, tail2, sudoflag)
        elif os.path.isfile(sub_path):
            tail1 = i.split('.')[-1]
            if tail1 == tail2:
                local_path=dstpath+"/"+subdir
                SUDO=""
                if sudoflag :
                    SUDO="sudo "
                os.system(SUDO + "mkdir -p " + local_path)
                os.system(SUDO + "cp " + sub_path + " " + local_path)

def build_libOTe(argv):

    THIRD_LIB="third_party/libOTe_libs"
    LIBOTE="third_party/libOTe"

    if "clean" in argv :
        os.system("rm -rf " + THIRD_LIB)
        os.system("rm -rf " + LIBOTE + "/out")

    wd=os.getcwd()
    submodule_update(LIBOTE)
    os.chdir(LIBOTE)
    submodule_update("cryptoTools")
    os.system("python3 build.py --setup --boost --relic --sodium")
    os.system("python3 build.py -D ENABLE_RELIC=ON -D ENABLE_ALL_OT=ON -D ENABLE_SODIUM=ON -D ENABLE_CIRCUITS=ON")
    os.system("python3 build.py --install=../../" + THIRD_LIB)
    os.chdir(wd)

    return THIRD_LIB

def build_clean(argv) :

    build_path="build"
    lib_path="lib"
    if "clean" in argv :
        if (os.path.isdir(build_path)):
            os.system("rm -rf " + build_path)
        if (os.path.isdir(lib_path)):
            os.system("rm -rf " + lib_path)

def build_xsce(argv, THIRD_LIB):
    
    build_path="build"
    if (not os.path.isdir(build_path)):
        os.system("mkdir " + build_path)
    
    #   设置cmake编译选项
    cmake_compile_arg = "cmake -S . -B " + build_path
    if "verbose" in argv :
        cmake_compile_arg += " -DCMAKE_VERBOSE_MAKEFILE=ON "

    if "debug" in argv :
        cmake_compile_arg += " -DCMAKE_BUILD_TYPE=Debug "
    else :
        cmake_compile_arg += " -DCMAKE_BUILD_TYPE=Release "

    if THIRD_LIB == "" :
        cmake_compile_arg += ""
    else :
        cmake_compile_arg += " -DTHIRD_LIB=" + THIRD_LIB

    print(cmake_compile_arg)
    os.system(cmake_compile_arg)
    os.system("make -j5 -C " + build_path)

def parseArgs(arg, args, default_value):
    doarg = False
    valuearg = default_value
    for x in args:
        if x.startswith(arg+"="):
            valuearg = x.split("=",1)[1]
            doarg = True
        if x == arg:
            doarg = True

    return (valuearg, doarg)

def parseInstallArgs(args):
    installpath = "/usr/local"
    doInstall = False
    for x in args:
        if x.startswith("install="):
            installpath = x.split("=",1)[1]
            doInstall = True
        if x == "install":
            doInstall = True

    return (installpath, doInstall)

def build_install(installpath, argv):

    if installpath == "" :
        installpath = "/usr/local"

    installpath_include = installpath + "/include/xsce"
    installpath_lib = installpath + "/lib"

    sudoflag = "sudo" in argv
    SUDO=""
    if sudoflag :
       SUDO = "sudo "

    if (not os.path.isdir(installpath_include)):
        os.system(SUDO + "mkdir -p " + installpath_include)
    if (not os.path.isdir(installpath_lib)):
        os.system(SUDO + "mkdir -p " + installpath_lib)
    
    os.system(SUDO + "cp -rf third_party/libOTe_libs/* " + installpath)
    os.system(SUDO + "cp lib/* " + installpath_lib)
    copy_files("./src", installpath_include, "", "h", sudoflag)
    copy_files("./src", installpath_include, "", "hpp", sudoflag)
   
def build_docker(images):
    os.system("docker build -t " + images + " . ")

def build_setup(argv):

    THIRD_LIB="third_party/libOTe_libs"
    LIBOTE="third_party/libOTe"
    if "libote" in argv :
        if "clean" in argv :
            os.system("rm -rf " + THIRD_LIB)
            os.system("rm -rf " + LIBOTE)
        submodule_update(LIBOTE)

def main(projectName, argv):

    flag=False
    if "help" in argv:
        help()
        return 

    if "clean" in argv :
        flag = True
        build_clean(argv)

    if "setup" in argv :
        build_setup(argv)
        return 

    THIRD_LIB=""
    if "libote" in argv :
        flag = True
        THIRD_LIB = build_libOTe(argv)

    if "xsce" in argv :
        flag = True
        build_xsce(argv, THIRD_LIB)

    installpath, install = parseInstallArgs(argv)
    if install :
        flag = True
        build_install(installpath, argv)

    images, dockerflag = parseArgs("docker", argv, "")
    if dockerflag :
        if images=="" :
            help()
            return 
        flag = True
        build_docker(images)

    if False == flag :
        help()

if __name__ == "__main__":
    main("xsce", sys.argv[1:])
