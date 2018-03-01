@echo off
@rem quick & dirty solution to test

@rem Usage:
@rem  test        : run compillations, keeps the results
@rem  test clean  : clean folder, remove results


@rem set asmb=..\Debug\asmb.exe
set asmb=..\bin\asmb.exe
set switch=
echo **TEST**
echo clean previous compile output
del /Q *.out 2>nul
del /Q *.lst 2>nul
del /Q *.log 2>nul

if %1.==clean. goto exit

call :compile test1
call :compile test2
call :compile test3

del a.*
goto exit

:compile
set name=%1
echo compile %name%.asm
%asmb% %name%.asm %switch% 1>%name%_grammmar.log 2>%name%_lexer.log
copy /B a.out %name%.out >nul
copy /A a.lst %name%.lst >nul
exit /B

:exit