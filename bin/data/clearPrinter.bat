net stop spooler /y
del %systemroot%\System32\spool\printers\* /Q /F /S
net start spooler