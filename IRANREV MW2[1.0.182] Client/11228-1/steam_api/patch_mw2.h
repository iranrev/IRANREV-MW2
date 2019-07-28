#pragma once
#include "stdafx.h"

// inline cmd functions
extern DWORD* cmd_id;
extern DWORD* cmd_argc;
extern DWORD** cmd_argv;

inline int	Cmd_Argc( void )
{
	return cmd_argc[*cmd_id];
}

inline char *Cmd_Argv( int arg )
{
	if ( (unsigned)arg >= cmd_argc[*cmd_id] ) {
		return "";
	}
	return (char*)(cmd_argv[*cmd_id][arg]);	
}

