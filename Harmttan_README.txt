Quake III Arena on MeeGo Harmattan


Libraries, scripts and configs are installed in "/usr/lib/ioquake3-touch".

Executable binarys installed in "/usr/bin".
"ioquake3.arm" is client execution file.
"ioq3ded.arm" is server execution file.

Game data should be put in "/home/user/baseq3".
Home path of game is in "/home/user/.q3a".

If you want to set configs for booting Q3A, edit the shell script in "/home/user/baseq3/Quake_III_Arena".
The file is copy from "/usr/lib/ioquake3-touch/Quake_III_Arena" when package is installed.
If this file is not executed, clicking desktop icon will execute "/usr/lib/ioquake3-touch/Quake_III_Arena.default".
Prefer configs, you can see in the shell script "/usr/lib/ioquake3-touch/Quake_III_Arena.default".

Harmattan console command:
* harm_usingVKB: boolean(0 / 1) default is 1 - enable/disable virtual button layer.
* harm_swipeSens: float(greater than 0.0) default is 0.5 - swipe gesture sensitivity on virtual button layer.

