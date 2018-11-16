setlocal
set files=%1
if "%files%"=="" set files=*.eps
del c:\tmp\*.eps c:\tmp\*.bb
for %%f in (%files%) do findstr /B %%%%BoundingBox %%f > c:\tmp\%%f
ren c:\tmp\*.eps *.bb
copy c:\tmp\*.bb .
del c:\tmp\*.bb
