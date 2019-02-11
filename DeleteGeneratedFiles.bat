@echo off
if exist Intermediate rmdir Intermediate /S /Q
if exist Binaries rmdir Binaries /S /Q
if exist .vs rmdir .vs /S /Q
if exist Saved rmdir Saved /S /Q

if exist "Plugins/GameSparks/Intermediate" rmdir "Plugins/GameSparks/Intermediate" /S /Q
if exist "Plugins/GameSparks/Binaries" rmdir "Plugins/GameSparks/Binaries" /S /Q

pause