# File Name: Makefile_LIB
TARGET  = AS350Plus_API_test
# TARGET  = DSS_main
SRC_DIR = ./
INC_DIR = ./
BUILD_DIR = ./build/
CC = arm-linux-gnueabihf-gcc 
CFLAGS += -I$(INC_DIR)
CFLAGS += -I.
#CFLAGS += -funwind-tables -mapcs-frame -rdynamic  
SRC     = $(wildcard $(SRC_DIR)*.c)
OBJ     = $(SRC:$(SRC_DIR)%.c=$(BUILD_DIR)%.o) #$(patsubst %.c, %.o, $(SRC))
DEP     = $(SRC:$(SRC_DIR)%.c=$(BUILD_DIR)%.d) #$(patsubst %.o, %.d, $(OBJ))

STATIC_LIB_NAME = POSAPI



#LIB_DIR = ./lib/

#LIBS = $(wildcard $(LIB_DIR)*.a)


#OBJ     = $(SRC:%.c=%.o) #$(patsubst %.c, %.o, $(SRC))
#DEP     = $(SRC:%.c=%.d) #$(patsubst %.o, %.d, $(OBJ))


# srcDirs = .
# srcDirs += ./UnitTest
# srcDirs += ./Test
# srcDirs += ./POSAPI
# srcDirs += ./others
# srcDirs += ./openct
# srcDirs += ./ctls/LV1
# srcDirs += ./ctls/LV2
# srcDirs += ./APP
# srcDirs += ./APP/IPASS
# srcDirs += ./APP/EASYCARD

#buildDirs = $(srcDirs:./%=./build/%)


CFLAGS += -ITest/include
# SRC  +=  $(wildcard ./TEST/*.c)
CFLAGS += -IUnitTest/include
# SRC +=   $(wildcard ./UnitTest/*.c)
CFLAGS += -Ictls/LV1/include
SRC  +=  $(wildcard ./ctls/LV1/*.c)
CFLAGS += -Ictls/LV2/include
SRC  +=  $(wildcard ./ctls/LV2/*.c)
SRC  +=  $(wildcard ./ctls/LV2/Dummy_Function.c)
CFLAGS += -Iothers/include
# SRC +=   $(wildcard ./others/*.c)
CFLAGS += -Iopenct/include
SRC +=   $(wildcard ./openct/*.c)
CFLAGS += -IDSS/include
# SRC +=   $(wildcard ./DSS/*.c)
CFLAGS += -IPOSAPI/include
SRC +=   $(wildcard ./POSAPI/*.c)

 CFLAGS += -IGzip/APP
 CFLAGS += -IGzip/BSP_Include
 SRC +=   $(wildcard ./Gzip/APP/*.c)

CFLAGS += -IEMVK
SRC += 	$(wildcard ./EMVK/*.c)

CFLAGS += -IPEDSAPI
SRC += $(wildcard ./PEDSAPI/*.c)

#CFLAGS += -Iftp
# SRC +=  $(wildcard ./ftp/*.c)
#===========bluetooth stuff===========
CFLAGS += -Ibluez
CFLAGS += -Ibluez/include
CFLAGS += -Ibluez/client
CFLAGS += -Ibluez/ssp
CFLAGS += -I/usr/arm-linux-gnueabihf/include/dbus-1.0 -I/usr/arm-linux-gnueabihf/lib/dbus-1.0/include -I/usr/arm-linux-gnueabihf/lib/dbus-1.0/dbus
CFLAGS += -I/usr/arm-linux-gnueabihf/include/glib-2.0 -I/usr/arm-linux-gnueabihf/lib/glib-2.0/include -I/usr/arm-linux-gnueabihf/include/glib-2.0 -I/usr/arm-linux-gnueabihf/lib/glib-2.0/include
CFLAGS += -I/usr/arm-linux-gnueabihf/include/gio-unix-2.0/gio -I/usr/arm-linux-gnueabihf/include/gio-unix-2.0
SRC +=   $(wildcard ./bluez/client/display.c)
SRC +=   $(wildcard ./bluez/client/agent.c)
SRC +=   $(wildcard ./bluez/client/gatt.c)
SRC +=   $(wildcard ./bluez/client/advertising.c)
SRC +=   $(wildcard ./bluez/lib/uuid.c)
SRC +=   $(wildcard ./bluez/gdbus/*.c)
SRC +=   $(wildcard ./bluez/src/shared/*.c)
SRC +=   $(wildcard ./bluez/ssp/profile1-iface.c)
#===========bluetooth stuff===========


#CFLAGS += -I../../../../usr/include
CFLAGS += -IINC/EMV
CFLAGS += -IINC/FS
CFLAGS += -IINC/ICERInc_3003G
CFLAGS += -IINC/IPASS_v0008
CFLAGS += -IINC/openssl
CFLAGS += -IINC/POSAPI
CFLAGS += -IINC
CFLAGS += -IAPP/EASYCARD
CFLAGS += -IAPP/IPASS
CFLAGS += -IAPP
#CFLAGS += -Wall -g

# use in easy card corporation (ECC) libiary  
CFLAGS += -D READER_MANUFACTURERS=NE_SYMLINK_API -D LIB_DEFINE
# use in gzip
CFLAGS += -D BSP_OS_IS_NO_OS
# use in OS_FLASH
CFLAGS += -D _FLASH_32MB_
# define if  resistive touch screen or not 
# CFLAGS += -D TSC_RESIST
# use in API_PED
# CFLAGS += -D _SRAM_PAGE_ 
# CFLAGS += -D EDU_MODE

#for SDK printer example
# CFLAGS += -D _MOUNT_PRINTER_ 
CFLAGS += -fstack-protector
CFLAGS += -D _PLATFORM_AS350 
CFLAGS += -D _SCREEN_SIZE_240x320 
CFLAGS += -D _SCREEN_SIZE_320x240
CFLAGS += -D_GNU_SOURCE #for gatt.c ,definition O_DIRECT 
CFLAGS += -D AS350PLUS_COUNTERTOP
# CFLAGS += -D AS350PLUS_WIRELESS

# SRC +=   $(wildcard ./APP/*.c)
# SRC :=   $(filter-out ./APP/Main.c ,$(SRC))
# SRC +=   $(wildcard ./APP/EMVtrans.c)
# SRC +=   $(wildcard ./APP/Disp.c)
# SRC +=   $(wildcard ./APP/Beep.c)
# SRC +=   $(wildcard ./APP/Keyin.c)
# SRC +=   $(wildcard ./APP/Clear.c)
# SRC +=   $(wildcard ./APP/AUXsend.c)
# SRC +=   $(wildcard ./APP/Tool.c)
# SRC +=   $(wildcard ./APP/Get.c)
# SRC +=   $(wildcard ./APP/EASYCARD/*.c)
#SRC +=   $(wildcard ./APP/EASYCARD/ICERInc_3003G/*.c)
# SRC +=   $(wildcard ./APP/IPASS/*.c)

#CFLAGS +=-L./lib -lcrypto -lssl

#SDK version
# LINKFLAG= -L./lib -lssl -lcrypto -lAS350PlusPOSAPI
#before BLE_API version
# LINKFLAG=  -Bstatic -L./lib -lssl -lcrypto   -lgdbus-internal -lrt 
# DLINKFLAG= -Bdynamic -L/usr/arm-linux-gnueabihf/lib    -lgthread-2.0 -pthread -ldl -lglib-2.0 -ldbus-1 -lreadline
#for BLE_API version  
DLINKFLAG=-Bstatic -L/usr/arm-linux-gnueabihf/lib -lssl -lcrypto -lbluetooth -lffi -lresolv -lrt\
-Bdynamic -L/usr/arm-linux-gnueabihf/lib -lgthread-2.0 -pthread -ldl -lglib-2.0  -ldbus-1 -lreadline -lz -lgmodule-2.0 -lgobject-2.0 -lgio-2.0


#$(shell mkdir -p $(buildDirs))


.PHONY: clean

.PHONY: LIB





#@echo 'create .a file'
#ar rcs lib$(STATIC_LIB_NAME).a $^

$(TARGET) : $(OBJ)
	@echo 'link finial file'
	$(CC) --sysroot=/usr/arm-linux-gnueabihf/include  $(CFLAGS) $^ -o $@ $(LINKFLAG)  $(DLINKFLAG) 
# Include all .d files
-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered by calling `-include $(DEP)`.
${BUILD_DIR}%.o : %.c
	@echo $(dir $@)
	@if [ ! -d "$(dir $@)" ]; then mkdir -p $(dir $@); fi
	$(CC) --sysroot=/usr/arm-linux-gnueabihf/include $(CFLAGS) -MMD -c $< -o $@ $(LINKFLAG)  $(DLINKFLAG) 

# Build target for every single object file.
# The potential dependency on header files is covered by calling `-include $(DEP)`.
%.o : %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@ $(LINKFLAG)  $(DLINKFLAG) 

lib$(STATIC_LIB_NAME).a : $(OBJ)
	ar rcs lib$(STATIC_LIB_NAME).a $^

# create static libiart
LIB : lib$(STATIC_LIB_NAME).a

## clean
clean:
	rm -f $(TARGET) $(OBJ) $(DEP) lib$(STATIC_LIB_NAME).a
	rm -r $(BUILD_DIR)
all :
	$(TARGET)