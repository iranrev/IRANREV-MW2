#pragma once

// not complete, but enough for reading
typedef struct cvar_t
{
	char* name;
	int unknown;
	int flags;
	int type;
	union cvar_value_t {
		float value;
		int integer;
		char* string;
		bool boolean;
	} value;
} cvar_t;

typedef struct cmd_t
{
	char pad[24];
} cmd_t;

// netadr_t
typedef enum {
	NA_BOT,
	NA_BAD,					// an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
} netadrtype_t;

typedef enum {
	NS_CLIENT,
	NS_SERVER
} netsrc_t;

typedef struct {
	netadrtype_t	type;

	BYTE	ip[4];

	unsigned short	port;

	BYTE	ipx[10];
} netadr_t;

// vars
extern cvar_t* sv_rconPassword;

// internal functions (custom)
void Com_BeginRedirect( char *buffer, int buffersize, void ( *flush )( char * ) );
void        Com_EndRedirect( void );

int	Cmd_Argc( void );
char *Cmd_Argv( int );

int	Cmd_ArgcSV( void );
char *Cmd_ArgvSV( int );