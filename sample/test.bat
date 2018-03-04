@echo off
@rem quick & dirty solution to test

@rem Usage:
@rem  test        : run compillations, keeps the results
@rem  test clean  : clean folder, remove results


@rem set bin=..\Debug
set bin=..\bin
@rem set bin=..\Release
set asmb=%bin%\asmb.exe
set simb=%bin%\simb.exe
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
call :compile test

del a.*
@rem if %1.==run. goto :run

goto exit

:compile
set name=%1
echo compile %name%.asm
%asmb% %name%.asm %switch% 1>%name%_grammmar.log 2>%name%_lexer.log
copy /B a.out %name%.out >nul
copy /A a.lst %name%.lst >nul
copy /A a.coe %name%.coe >nul
echo run simulation of %name%.asm
%simb% %name%.out 500 >%name%_sim.log
exit /B


:exit