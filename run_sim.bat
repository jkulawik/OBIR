
::Your path to folder with EBSimUnoEth.exe here:
@cd C:\Users\You\Documents\Studia\OBIR\You\EBSimUnoEth\bin-dist\win10\cygwin

@rem DLL preparing
@rem For EXE compiled under cyginw-win10
@rem 
PATH=./dll-win10;%PATH%%

::After first compilation of .ino file (each time after starting a new session) copy .ino.hex path here and correct \\ to \ and / to \
copy C:\Users\You\AppData\Local\Temp\arduino_build_87512\UDPServer.ino.hex %cd% /Y

@rem Exe simulation
@echo "Run simulation"
::IPv4 of your network card here
EBSimUnoEth.exe -ip 192.168.1.27 UDPServer.ino.hex
pause