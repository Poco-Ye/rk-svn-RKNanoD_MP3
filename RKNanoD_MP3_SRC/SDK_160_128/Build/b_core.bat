@echo off
bin_generate.exe  B_CORE\RkNano.bin\BB_SYS_CODE    ..\..\Common\BBSystem\bb_core_code.bin
bin_generate.exe  B_CORE\RkNano.bin\BB_MAIN_STACK  ..\..\Common\BBSystem\bb_core_data.bin
echo ##generate bb_core_code.bin and bb_core_data.bin success!##