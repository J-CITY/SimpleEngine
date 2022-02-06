# Common Tecmake definitions for Lua 5.4

SFX = 54
PROJNAME = lua$(SFX)
OPT = Yes           # Turn optimization on
#USE_NODEPEND = Yes  # Disable dependencies
NO_SCRIPTS = Yes    # Disable scripts and batch files creation
NO_DYNAMIC = no     # Build also dynamic libraries with gcc in Windows

#######################################################################

DEFINES = LUA_COMPAT_5_3
                       
# Windows DLL Only
ifneq ($(findstring dll, $(TEC_UNAME)), )
  DEFINES += LUA_BUILD_AS_DLL
endif
                       
# Linux Only
ifneq ($(findstring Linux, $(TEC_UNAME)), )
  DEFINES += LUA_USE_LINUX LUA_USE_READLINE
  FLAGS = -std=gnu99
  ifneq ($(findstring CentOS5, $(TEC_DIST)), )
    DEFINES += LUA_C89_NUMBERS
    FLAGS =
  endif
endif

# FreeBSD Only
ifneq ($(findstring FreeBSD, $(TEC_UNAME)), )
  DEFINES += LUA_USE_LINUX LUA_USE_READLINE
endif

# MacOS Only
ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  DEFINES += LUA_USE_MACOSX LUA_USE_READLINE
  
  # Since 10.4 must build only dylib instead of bundle
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB = Yes
  endif
endif

# AIX Only
ifneq ($(findstring AIX, $(TEC_UNAME)), )
  # CC="xlc"
  DEFINES += LUA_USE_POSIX LUA_USE_DLOPEN
endif

# SunOS Only
ifneq ($(findstring SunOS, $(TEC_UNAME)), )
  DEFINES += LUA_USE_POSIX LUA_USE_DLOPEN _REENTRANT
  ifneq ($(findstring x86, $(TEC_UNAME)), )
    FLAGS = -std=gnu99
  endif
endif

# Cygwin Only
ifneq ($(findstring cygw, $(TEC_UNAME)), )
  DEFINES += LUA_USE_LINUX LUA_USE_READLINE
endif

#######################################################################

# For Executables only
ifdef APPNAME
  # Disable strip
  STRIP = 
  
  APPTYPE = console

  ifneq ($(findstring Win, $(TEC_SYSNAME)), )
    # In Windows, use the Dynamic Libraries
    USE_DLL = Yes
    LIBS += lua$(SFX)
    
    ifneq ($(findstring ming, $(TEC_UNAME)), )
      LDIR_SUB := $(subst ming,dll,$(TEC_UNAME))
      LDIR += ../lib/$(LDIR_SUB)
    else 
      ifneq ($(findstring vc, $(TEC_UNAME)), )
        SLIB += setargv.obj
        LDIR_SUB := $(subst vc,dll,$(TEC_UNAME))
        LDIR += ../lib/$(LDIR_SUB)
      endif
    endif
  else
    #UNIX Use Static Libraries
    SLIB += ../lib/$(TEC_UNAME)/liblua$(SFX).a
  endif
  
  ifneq ($(findstring Linux, $(TEC_UNAME)), )
    LFLAGS = -Wl,-E 
    LIBS += dl readline 
#    LIBS += ncurses
  endif
  
  ifneq ($(findstring FreeBSD, $(TEC_UNAME)), )
    LFLAGS = -Wl,-E
    LIBS += readline
  endif
  
  ifneq ($(findstring MacOS, $(TEC_UNAME)), )
    LIBS += readline
  endif
  
  ifneq ($(findstring AIX, $(TEC_UNAME)), )
    LFLAGS = -brtl -bexpall
    LIBS += dl
  endif
  
  ifneq ($(findstring SunOS, $(TEC_UNAME)), )
    LIBS += dl 
  endif
  
  ifneq ($(findstring cygw, $(TEC_UNAME)), )
    LIBS += lua$(SFX) readline
    SLIB =
    LDIR += ../lib/$(TEC_UNAME)   
  endif
endif
