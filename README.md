------------------------------------------------------------------------
                  AGL-Advanced-Audio-Agent 
------------------------------------------------------------------------

# Cloning Audio-Binding from Git
-------------------------------------------------------

```
# Initial clone with submodules
git clone --recurse-submodules https://github.com/iotbzh/audio-bindings
cd  audio-binding

# Do not forget submodules with pulling
git pull --recurse-submodules https://github.com/iotbzh/audio-bindings

```

# AFB-daemon dependencies
-------------------------------------------------------

    OpenSuse >=42.2, Fedora>=25, Ubuntu>=16.4 Binary packages from  https://en.opensuse.org/LinuxAutomotive

    For other distro see # Building AFB-daemon from source on Standard Linux Distribution

 
# Specific Dependencies 

 * alsa-devel >= 1.1.2 Warning some distro like Fedora-25 still ship version 1.1.1 as default
 * lua >= 5.3  Most distribution only ship version 5.2 but binary package should be easy to find


```
  OpenSuse
     - LUA-5.3-devel  https://software.opensuse.org//download.html?project=devel%3Alanguages%3Alua&package=lua53
     - Alsa-devel zypper --install alsa-devel # 42.3 is shipped default with 1.1.4 
```


# Compile AGL-Advanced-Audio-Agent
--------------------------------------

* Edit your ~/.config/app-templates/cmake.d/00-common-userconf.cmake to reflect your local configuration

```
    message(STATUS "*** Fulup Local Config For Native Linux ***")
    add_compile_options(-DNATIVE_LINUX)

    set (RSYNC_TARGET "10.20.101.198")
    set (RSYNC_PREFIX "opt")

    set(CMAKE_INSTALL_PREFIX $ENV{HOME}/opt)
    set(BINDINGS_INSTALL_PREFIX $ENV{HOME}/opt)

    set(CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX}/lib64/pkgconfig ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
    set(LD_LIBRARY_PATH ${CMAKE_INSTALL_PREFIX}/lib64 ${CMAKE_INSTALL_PREFIX}/lib)

```


```
    mkdir -p build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX ..
    make

    afb-daemon --workdir=. --ldpaths=. --port=1234  --roothttp=../htdocs --token="" --verbose

    Warning: See below net on GDB requiring (--workdir=.)
```

# Note: GDB Source Debug limits/features

Warning: technically AGL bindings are shared libraries loaded thought 'dlopen'. GDB supports source debug of dynamically
loaded libraries, but user should be warn that the actual path to sharelib symbols is directly inherited from DLOPEN.
As a result if you change your directory after binder start with --workdir=xxx then GDB will stop working.

Conclusion: double-check that --workdir=. and run debug session from ./build directory. Any IDEs like Netbeans or VisualCode should work out of the box.


```
    Examples:

    # WORK when running in direct
    afb-daemon --workdir=.. --ldpaths=build --port=1234 --roothttp=./htdocs

    # FAIL when using GDB with warning: Could not load shared library ....
    gdb -args afb-daemon --workdir=.. --ldpaths=build --port=1234 --roothttp=./htdocs
    ...
    warning: Could not load shared library symbols for ./build/ucs2-afb/afb-ucs2.so.
    Do you need "set solib-search-path" or "set sysroot"?
    ...
```

To debug sharelib symbol path: start your binder under GDB. Then break your session after the binder has
loaded its bindings. Finally use "info sharedlibrary" and check 'SymsRead'. If equal to 'No' then either you start GDB
from the wrong relative directory, either you have to use 'set solib-search-path' to force the path.

```
    gdb -args afb-daemon --workdir=.. --ldpaths=build --port=1234 --roothttp=./htdocs
    run
        ...
        NOTICE: API UNICENS added
        NOTICE: Waiting port=1234 rootdir=.
        NOTICE: Browser URL= http://localhost:1234
    (hit Ctrl-C to break the execution)
    info sharedlibrary afb-*
```

# ToBeBone (WorkInProgess: This list is getting longer every day)
-------------------------------------------------------------------

* Support response from LUA with nested table (currently fail miserably)
* Add timer base callback from Lua
* Enable export of LUA commands directly from Plugin

# Running an debugging on a target
-------------------------------------------------------


```
export RSYNC_TARGET=root@xx.xx.xx.xx
export RSYNC_PREFIX=/opt    # WARNING: installation directory should exist

mkdir $RSYNC_TARGET
cd    $RSYNC_TARGET

cmake -DRSYNC_TARGET=$RSYNC_TARGET -DRSYNC_PREFIX=$RSYNC_PREFIX ..
make remote-target-populate

    ./build/target/start-${RSYNC_TARGET}.sh
    firefox http://localhost:1234    # WARNING: do not forget firewall if any

    ssh -tt ${RSYNC_TARGET} speaker-test -twav -D hw:ep01 -c 2
```

Note: remote-target-populate will
 - create a script to remotely start the binder on the target in ./build/target/start-on-target-name.sh
 - create a gdbinit file to transparently debug remotely in source code with gdb -x ./build/target/gdb-on-target-name.ini
 - to run and debug directly from your IDE just configure the run and debug properties with the corresponding filename

Note that Netbeans impose to set debug directory to ./build/pkgout or it won't find binding symbols for source debugging


# Building AFB-daemon from source on Standard Linux Distribution
-------------------------------------------------------

   # handle dependencies > (OpenSuse-42.2, Fedora-25, Ubuntu 16.04.2LTS)
    gcc > 4.8
    systemd-devel (libsystemd-dev>=222)
    libuuid-devel
    file-devel(OpenSuSe) or libmagic-dev(Ubuntu)
    libjson-c-devel
    ElectricFence (BUG should not be mandatory)
    libopenssl-devel libgcrypt-devel libgnutls-devel (optional but requested by libmicrohttpd for https)

    OpenSuse >=42.2
      zypper in gcc5 gdb gcc5-c++ git cmake make ElectricFence systemd-devel libopenssl-devel  libuuid-devel alsa-devel libgcrypt-devel libgnutls-devel libjson-c-devel file-devel mxml-devel

    Ubuntu >= 16.4 libuuid-devel
      apt-get install cmake git electric-fence libsystemd-dev libssl-dev uuid-dev libasound2-dev libgcrypt20-dev libgnutls-dev libgnutls-dev libjson-c-dev libmagic-dev  libmxml-dev

    libmicrohttpd>=0.9.55 (as today OpenSuse-42.2 or Ubuntu-.16.4 ship older versions)
    afb-daemon from AGL Gerrit git clone https://gerrit.automotivelinux.org/gerrit/src/app-framework-binder

```
    # Might want to add following variables into ~/.bashrc
    echo "#----------  AGL options Start ---------" >>~/.bashrc
    echo "# Object: AGL cmake option for  binder/bindings" >>~/.bashrc
    echo "# Date: `date`" >>~/.bashrc
    echo 'export CC=gcc-5; export CXX=g++-5' >>~/.bashrc   # if using gcc5
    echo 'export INSTALL_PREFIX=$HOME/opt' >>~/.bashrc
    echo 'export LD_LIBRARY_PATH=$INSTALL_PREFIX/lib64:$INSTALL_PREFIX/lib' >>~/.bashrc
    echo 'export LIBRARY_PATH=$INSTALL_PREFIX/lib64:$INSTALL_PREFIX/lib' >>~/.bashrc
    echo 'export PKG_CONFIG_PATH=$INSTALL_PREFIX/lib64/pkgconfig:$INSTALL_PREFIX/lib/pkgconfig' >>~/.bashrc
    echo 'export PATH=$INSTALL_PREFIX/bin:$PATH' >>~/.bashrc
    echo 'export RSYNC_TARGET=MY_TARGET_HOSTNAME' >>~/.bashrc
    echo 'export RSYNC_PREFIX=./opt' >>~/.bashrc

    echo "#----------  AGL options End ---------" >>~/.bashrc
    source ~/.bashrc

    # install LibMicroHttpd
    LIB_MH_VERSION=0.9.55
    wget https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-${LIB_MH_VERSION}.tar.gz
    tar -xzf libmicrohttpd-${LIB_MH_VERSION}.tar.gz
    cd libmicrohttpd-${LIB_MH_VERSION}
    ./configure --prefix=${INSTALL_PREFIX}
    make
    sudo make install-strip

    # retrieve last AFB_daemon from AGL
    git clone https://gerrit.automotivelinux.org/gerrit/src/app-framework-binder

    # Warning: previous GCC options should be set before initial cmake (clean Build/*)
    cd app-framework-binder; mkdir -p build; cd build
    cmake -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX ..
    make
    make install
```
