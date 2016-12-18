
if not defined VisualStudioVersion (
    call "%VSINSTALLDIR%\VC\vcvarsall.bat" %1
)

MSBuild gv.vcxproj /t:%3 /p:Platform=%1 /p:Configuration=%2
