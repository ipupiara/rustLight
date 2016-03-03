echo off
echo .
echo .
echo .clean up all temporary and and incremental buildup-files"
echo .
erase /F /S /Q simulation\*.*
rmdir /S /Q simulation
erase /F /S /Q incremental_db\*.*
rmdir /S /Q incremental_db
erase /F /S /Q db\*.*
rmdir /S /Q db
erase /F /S /Q .qsys_edit\*.*
rmdir /S /Q .qsys_edit
erase /F /S /Q output_files\*.*
rmdir /S /Q output_files
echo .
echo .
echo .end of job
echo .
echo on