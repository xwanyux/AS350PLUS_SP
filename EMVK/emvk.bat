@echo off
REM ** To change the cpu designator, change the following line:
REM ** Set this according to the Core Module being used

SET CPU=ARM920T
SET BSP_OS=BSP_OS_IS_NOS
SET OS_SRVC_PATH=OS_Services\NOS

SET LIB_PATH=..\LIB
SET COMPILER=%GCC_BIN_PATH%\gcc
SET ASSEMBLER=%GCC_BIN_PATH%\gcc
SET ARCHIVER=%GCC_BIN_PATH%\ar
SET TARGET_LIB=libemvk_350sx_sw_201d.a
SET ARCHIVER_FLAGS=rcs
SET CFLAG=-mcpu=arm9tdmi -g -O1 -mlittle-endian -c -mthumb-interwork -W -I %GCC_INC_PATH% -I ..\INC\BSP -I ..\NOS_xx6.S1 -I ..\INC\POSAPI -D ARM_GCC -D %BSP_OS%
SET AS_FLAGS=-c -mapcs -mthumb-interwork -mcpu=arm9tdmi -mlittle-endian -g
SET OUTPUT_DIR=OBJ

:BEGIN

REM @echo on

%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\UI.o		UI.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\TOOL.o		TOOL.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\EMVCONST.o	EMVCONST.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\ISO88591.o	ISO88591.c
@if errorlevel 1 goto ERROR

%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APDU_REC.o	APDU_REC.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APDU_SEL.o	APDU_SEL.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APDU_GDT.o	APDU_GDT.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APDU_IAU.o	APDU_IAU.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APDU_GCH.o	APDU_GCH.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APDU_SAM.o	APDU_SAM.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APDU_GPO.o	APDU_GPO.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APDU_VER.o	APDU_VER.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APDU_GAC.o	APDU_GAC.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APDU_XAU.o	APDU_XAU.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APDU_ISC.o	APDU_ISC.c
@if errorlevel 1 goto ERROR

%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APK_SRAM.o	APK_SRAM.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APK_FTAG.o	APK_FTAG.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APK_MAPP.o	APK_MAPP.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APK_RREC.o	APK_RREC.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APK_SPSE.o	APK_SPSE.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APK_OFDA.o	APK_OFDA.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APK_INIT.o	APK_INIT.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APK_PRES.o	APK_PRES.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APK_CVML.o	APK_CVML.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APK_TRMS.o	APK_TRMS.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APK_GACC.o	APK_GACC.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\APK_PFCI.o	APK_PFCI.c
@if errorlevel 1 goto ERROR

%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\L2API_00.o	L2API_00.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\L2API_01.o	L2API_01.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\L2API_02.o	L2API_02.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\L2API_03.o	L2API_03.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\L2API_04.o	L2API_04.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\L2API_05.o	L2API_05.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\L2API_06.o	L2API_06.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\L2API_07.o	L2API_07.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\L2API_08.o	L2API_08.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\L2API_09.o	L2API_09.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\L2API_10.o	L2API_10.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\L2API_11.o	L2API_11.c
@if errorlevel 1 goto ERROR

%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\PBOC_01.o		PBOC_01.c
@if errorlevel 1 goto ERROR
%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\PBOC_02.o		PBOC_02.c
@if errorlevel 1 goto ERROR

%COMPILER% %CFLAG% -I .. -I includes -o  %OUTPUT_DIR%\PINPAD.o		PINPAD.c
@if errorlevel 1 goto ERROR

@DEL %LIB_PATH%\%TARGET_LIB%

@echo on

%ARCHIVER% %ARCHIVER_FLAGS% %LIB_PATH%\%TARGET_LIB% %OUTPUT_DIR%\UI.o %OUTPUT_DIR%\TOOL.o %OUTPUT_DIR%\EMVCONST.o %OUTPUT_DIR%\ISO88591.o %OUTPUT_DIR%\APDU_REC.o %OUTPUT_DIR%\APDU_SEL.o %OUTPUT_DIR%\APDU_GDT.o %OUTPUT_DIR%\APDU_IAU.o %OUTPUT_DIR%\APDU_GCH.o %OUTPUT_DIR%\APDU_SAM.o %OUTPUT_DIR%\APDU_GPO.o %OUTPUT_DIR%\APDU_VER.o %OUTPUT_DIR%\APDU_GAC.o %OUTPUT_DIR%\APDU_XAU.o %OUTPUT_DIR%\APDU_ISC.o %OUTPUT_DIR%\APK_SRAM.o %OUTPUT_DIR%\APK_FTAG.o %OUTPUT_DIR%\APK_MAPP.o %OUTPUT_DIR%\APK_RREC.o %OUTPUT_DIR%\APK_SPSE.o %OUTPUT_DIR%\APK_OFDA.o %OUTPUT_DIR%\APK_INIT.o %OUTPUT_DIR%\APK_PRES.o %OUTPUT_DIR%\APK_CVML.o %OUTPUT_DIR%\APK_TRMS.o %OUTPUT_DIR%\APK_GACC.o %OUTPUT_DIR%\APK_PFCI.o %OUTPUT_DIR%\L2API_00.o %OUTPUT_DIR%\L2API_01.o %OUTPUT_DIR%\L2API_02.o %OUTPUT_DIR%\L2API_03.o %OUTPUT_DIR%\L2API_04.o %OUTPUT_DIR%\L2API_05.o %OUTPUT_DIR%\L2API_06.o %OUTPUT_DIR%\L2API_07.o %OUTPUT_DIR%\L2API_08.o %OUTPUT_DIR%\L2API_09.o %OUTPUT_DIR%\L2API_10.o %OUTPUT_DIR%\L2API_11.o %OUTPUT_DIR%\PBOC_01.o %OUTPUT_DIR%\PBOC_02.o %OUTPUT_DIR%\PINPAD.o

@goto OK

:ERROR
@echo *** ERROR ***
@goto EXIT

:OK
@echo *** OK ***

:EXIT