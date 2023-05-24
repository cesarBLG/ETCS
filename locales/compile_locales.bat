@echo off
setlocal EnableDelayedExpansion

for /R %%f in (*.po) do (
	set current_file=%%f
	msgfmt -o !current_file:~0,-3!.mo !current_file!
)