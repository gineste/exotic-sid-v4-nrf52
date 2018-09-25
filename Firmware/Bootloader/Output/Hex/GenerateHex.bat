@ECHO OFF
CLS

ECHO.
ECHO /*********************************
ECHO  * CREATING PACKAGE FILE FOR DFU *
ECHO  *********************************/
ECHO.

REM ---------------------------------------------------
REM --         SD, BL and SD_BL Zip Generation       --
REM ---------------------------------------------------
REM Combine the softdevice with the Bootloader
mergehex --merge s132_nrf52_5.0.0_softdevice.hex  Bootloader.hex --output SD_BL.hex
