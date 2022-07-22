@echo off

:: MongoDB
rmdir /s /q mongodb
powershell $ProgressPreference = 'SilentlyContinue'; Invoke-WebRequest -Uri https://fastdl.mongodb.org/windows/mongodb-windows-x86_64-5.0.9.zip -OutFile mongodb-windows-x86_64-5.0.9.zip
powershell $ProgressPreference = 'SilentlyContinue'; Expand-Archive -LiteralPath mongodb-windows-x86_64-5.0.9.zip -DestinationPath .
del mongodb-windows-x86_64-5.0.9.zip
ren mongodb-win32-x86_64-windows-5.0.9 mongodb
mkdir mongodb\data