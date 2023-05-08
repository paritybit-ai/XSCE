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

    print(" sodium \n\tlibOTe module.")
    print(" libote \n\tlibOTe module.")
    print(" spdz \n\tMP-SPDZ module.")
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

def check_str_in_file(filename, text):
    
    # 检测文件是否存在
    if os.path.isfile(filename): 
        f = open(filename, "r")
        # 在文件中查找字符串
        if text in f.read():
            return True

    return False

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
    os.system("python3 build.py --setup --boost --relic -D CMAKE_C_FLAGS=-fPIC -D CMAKE_CXX_FLAGS=-fPIC -D ENABLE_SIMPLESTOT=OFF -D ENABLE_SIMPLESTOT_ASM=OFF -D ENABLE_MRR=ON -D ENABLE_SILENTOT=ON -D ENABLE_KKRT=ON")
    os.system("python3 build.py -D ENABLE_RELIC=ON -D ENABLE_SODIUM=OFF -D ENABLE_CIRCUITS=ON -D CMAKE_C_FLAGS=-fPIC -D CMAKE_CXX_FLAGS=-fPIC -D ENABLE_SIMPLESTOT=OFF -D ENABLE_SIMPLESTOT_ASM=OFF -D ENABLE_MRR=ON -D ENABLE_MR=ON -D ENABLE_SILENTOT=ON -D ENABLE_KKRT=ON -D ENABLE_KOS=ON -D ENABLE_IKNP=ON -D ENABLE_DELTA_KOS=ON -D ENABLE_OOS=ON -D ENABLE_RR=ON")
    os.system("python3 build.py --install=../../" + THIRD_LIB)
    os.chdir(wd)

    return THIRD_LIB


def build_sodium(argv):

    sodium_path="third_party/sodium"
    sodium_source = "https://github.com/osu-crypto/libsodium.git"
    sodium_tag = "4e825a68baebdf058543f29762c73c17b1816ec0"

    if "clean" in argv :
        os.system("rm -rf " + sodium_path)

    wd=os.getcwd()

    if(not os.path.isdir(sodium_path)):
        os.mkdir(path=sodium_path)
    os.chdir(sodium_path)

    print("============= XSCE_OSE Building Sodium =============")
    os.system("git clone {0}".format(sodium_source))
    os.chdir("./libsodium")
    os.system("git checkout {0}".format(sodium_tag))
    os.system("./autogen.sh -s")
    os.system("./configure --prefix=${PWD}/../../libOTe_libs")
    os.system("make && make check")
    os.system("make install ")

    os.chdir(wd)

    return True

def build_spdz(argv):

    lib_path="lib"
    if "clean" in argv :
        if (os.path.isdir(lib_path)):
            os.system("rm -f " + lib_path + "/libMP-SPDZ.a")
            os.system("rm -f " + lib_path + "/libmpir.a")
            os.system("rm -f " + lib_path + "/libmpirxx.a")
            os.system("rm -f " + lib_path + "/libsimpleot.a")

    if (not os.path.isdir("lib")):
        os.system("mkdir " + "lib")
    SPDZ="third_party/MP-SPDZ"
    wd=os.getcwd()
    submodule_update(SPDZ)
    os.chdir(SPDZ)
    if "clean" in argv:
        os.system("make clean");
    
    if (False == os.path.isfile("local/lib/libmpir.a")):
        os.system("make mpir")

    if (False == os.path.isfile("local/lib/libmpir.so")):
        os.system("make mpir")

    if (False == os.path.isfile("local/lib/libmpirxx.a")):
        os.system("make mpir")

    if (False == os.path.isfile("local/lib/libmpirxx.so")):
        os.system("make mpir")

    os.system("make linux-machine-setup simde/simde")
    
    if (False == os.path.isfile("SimpleOT/libsimpleot.a")):
        os.system("make SimpleOT/libsimpleot.a")

    # if (False == os.path.isfile("SimplestOT_C/ref10/libSimplestOT.a")):
    #     os.system("make SimplestOT_C/ref10/libSimplestOT.a")
    
    # 检测Makefile释放已调整，如果未调整，则修改Makefile
    makefile_flag = check_str_in_file("Makefile", "libMP-SPDZ.a: $(FHEOBJS) $(COMMONOBJS) $(PROCESSOR) $(GC) $(OT)")
    if (True == makefile_flag):
        makefile_flag = check_str_in_file("Makefile", "		$(AR) -csr $@ $^")

    if (False == makefile_flag):
        os.system("git restore Makefile")
        os.system("echo 'libMP-SPDZ.a: $(FHEOBJS) $(COMMONOBJS) $(PROCESSOR) $(GC) $(OT)' >> Makefile")
        os.system("echo '		$(AR) -csr $@ $^' >> Makefile")

    # 检测CONFIG释放已调整，如果未调整，则修改CONFIG
    config_flag = check_str_in_file("CONFIG", "USE_KOS = 1")
    if (False == config_flag):
        os.system("git restore CONFIG")
        os.system("rm -f CONFIG.old")
        os.system("mv CONFIG CONFIG.old")
        os.system("sed 's/USE_KOS = 1/USE_KOS = 0/' CONFIG.old > CONFIG")

    # 编译
    os.system("make MY_CFLAGS+=\"-Werror=unused-parameter -I../libOTe_libs/include -Ilocal/include -L../libOTe_libs/lib -Llocal/lib\" libMP-SPDZ.a")
    os.chdir(wd)

    # 拷贝编译好的库文件
    os.system("cp " + SPDZ + "/local/lib/*.a ./lib/")
    os.system("cp " + SPDZ + "/libMP-SPDZ.a ./lib/")
    os.system("cp " + SPDZ + "/SimpleOT/libsimpleot.a ./lib/libsimpleot.a")

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
    lib_path="lib"
    if "clean" in argv :
        if (os.path.isdir(build_path)):
            os.system("rm -rf " + build_path)
        if (os.path.isdir(lib_path)):
            os.system("rm -f " + lib_path + "/libarich.a")
            os.system("rm -f " + lib_path + "/libcommon.a")
            os.system("rm -f " + lib_path + "/libPIR.a")
            os.system("rm -f " + lib_path + "/libPSI.a")
            os.system("rm -f " + lib_path + "/libtoolkits.a")

    if (not os.path.isdir(build_path)):
        os.system("mkdir " + build_path)
    if (not os.path.isdir("lib")):
        os.system("mkdir " + "lib")
    
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
    copy_files("./third_party/MP-SPDZ", installpath+"/include/MP-SPDZ", "", "h", sudoflag)
    copy_files("./third_party/MP-SPDZ", installpath+"/include/MP-SPDZ", "", "hpp", sudoflag)
   
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

    THIRD_SPDZ="third_party/MP-SPDZ"
    if "spdz" in argv :
        if "clean" in argv :
            os.system("rm -rf " + THIRD_SPDZ + "/*")
            os.system("rm -rf " + THIRD_SPDZ + "/.git*")
        submodule_update(THIRD_SPDZ)

def main(projectName, argv):

    flag=False
    if "help" in argv:
        help()
        return 

    if "setup" in argv :
        build_setup(argv)
        return 

    THIRD_LIB=""
    if "sodium" in argv :
        flag = True
        build_sodium(argv)

    if "libote" in argv :
        flag = True
        THIRD_LIB = build_libOTe(argv)

    if "spdz" in argv :
        flag = True
        build_spdz(argv)

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
