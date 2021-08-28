PS2 Doom
========

PlayStation 2 Doom port - More information available at [website](http://lukasz.dk/2008/02/11/doom-playstation-2-port/)

Requires PS2SDK, gsKit and SDL for PS2 to compile.

Build with: make clean all

Some very hackish stuff in w_wad.c / W_ReadLump function, to speed up loading,
which will probably break loading from multiple files.

	Controls:

	Left Analog Stick  : Move (up, down, left, right)
	Right Analog Stick : Strafe (comma, period)
	Start              : Enter
	Select             : Map (tab)
	Square             : Previous Weapon (o)
	Circle             : Nex Weapon (p)
	Triangle           : Escape
	L1                 : n
	L2                 : y
	R1                 : Fire (ctrl)
	R2                 : Use/Open Doors (space)

	L1 and L2 are for entering savegame names.  
