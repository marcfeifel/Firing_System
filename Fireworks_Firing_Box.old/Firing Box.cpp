// Firing Box.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <conio.h>
#include <windows.h>

#define VERSION (2)

#define LPT0_BASE ((short) 0x3bc)
#define LPT1_BASE ((short) 0x378)
#define LPT2_BASE ((short) 0x278)

unsigned int Select( unsigned char Socket, unsigned char Queue );
unsigned char Sense( void );
unsigned int Fire( void );
void TestAllQueues( void );
void RunProgram( void );
void TestProgram( void );
void FireSpecificQueue( void );
bool GetConfirmation( void );

// Stuff for the DLL
typedef short (_stdcall *inpfuncPtr)(short portaddr);
typedef void (_stdcall *oupfuncPtr)(short portaddr, short datum);
inpfuncPtr inp32fp;
oupfuncPtr oup32fp;
short  Inp32 (short portaddr)
{
	return (inp32fp)(portaddr);
}
void  Out32 (short portaddr, short datum)
{
	(oup32fp)(portaddr,datum);
} 

typedef struct _QUEUE
{
	unsigned char Socket;
	unsigned char Queue;
	long int Delay;
	char Desc[1024];
} QUEUE;

short PPORT_BASE;
unsigned int SelectDelay;
unsigned int FireDelay;
QUEUE theSequence[128];
int totalQueues;
int _tmain(int argc, _TCHAR* argv[])
{
	HINSTANCE hLib;
	FILE *theProgram;
	int result, i;
	char dummy[1024];

	printf( "***********************\n" );
	printf( "*** Firing Box v%03d ***\n", VERSION );
	printf( "***********************\n\n" );

	if( argc < 2 )
	{
		printf( "Please specify a file name." );
		return( 0 );
	}

	theProgram = fopen( argv[1], "r" );
	if( theProgram == NULL )
	{
		printf( "File could not be opened." );
		return( 0 );
	}

	PPORT_BASE = 0;
	while( PPORT_BASE == 0 )
	{
		printf( "Please select a port:\n" );
		printf( "0. LPT0 (0x3bc)\n" );
		printf( "1. LPT1 (0x378)\n" );
		printf( "2. LPT2 (0x278)\n" );
		switch( getch() )
		{
		case '0':
			PPORT_BASE = LPT0_BASE;
			printf( "Port 0 Selected - 0x%x.\n\n", PPORT_BASE );
			break;
		case '1':
			PPORT_BASE = LPT1_BASE;
			printf( "Port 1 Selected - 0x%x.\n\n", PPORT_BASE );
			break;
		case '2':
			PPORT_BASE = LPT2_BASE;
			printf( "Port 2 Selected - 0x%x.\n\n", PPORT_BASE );
			break;
		default:
			printf( "\n" );
			fflush( stdin );
			break;
		}
	}

	result = fscanf( theProgram, "%s%d", dummy, &SelectDelay );
	if( result < 2 || result == EOF )
	{
		printf( "Unable to determine Selection Delay." );
		return( 0 );
	}
	printf( "Selection Delay: %dms\n", SelectDelay );

	result = fscanf( theProgram, "%s%d", dummy, &FireDelay );
	if( result < 2 || result == EOF )
	{
		printf( "Unable to determine Firing Delay." );
		return( 0 );
	}
	printf( "Firing Delay: %dms\n", FireDelay );

	i = 0;
	while( (result = fscanf( theProgram, "%s%d %s%d %s%d %s%s", dummy, &theSequence[i].Socket, dummy, &theSequence[i].Queue, dummy, &theSequence[i].Delay, dummy, theSequence[i].Desc )) != EOF && i < 128 )
	{
		if( result < 8 )
			printf( "Malformed queue information, item %d.\n", i );
		else if( theSequence[i].Socket < 0 || theSequence[i].Socket > 7 )
			printf( "Illegal socket, item %d.\n", i );
		else if( theSequence[i].Queue < 0 || theSequence[i].Queue > 15 )
			printf( "Illegal queue, item %d.\n", i );
		else if( theSequence[i].Delay < 0 )
			printf( "Illegal delay (negative), item %d.\n", i );
		else
		{
			printf( "Socket: %d ", theSequence[i].Socket );
			printf( "Queue: %d ", theSequence[i].Queue );
			printf( "Delay: %d ", theSequence[i].Delay );
			printf( "Description: %s\n", theSequence[i].Desc );
			i++;
		}
	}
	printf( "Program items: %d\n", i );
	totalQueues = i;

	// Setup DLL
	hLib = LoadLibrary("inpout32.dll");
	if (hLib == NULL) {
		fprintf(stderr,"LoadLibrary Failed.\n");
		return -1;
	}

	inp32fp = (inpfuncPtr) GetProcAddress(hLib, "Inp32");
	if (inp32fp == NULL) {
		fprintf(stderr,"GetProcAddress for Inp32 Failed.\n");
		return -1;
	}

	oup32fp = (oupfuncPtr) GetProcAddress(hLib, "Out32");
	if (oup32fp == NULL) {
		fprintf(stderr,"GetProcAddress for Oup32 Failed.\n");
		return -1;
	}
    
	while( 1 )
	{
		printf( "\n\n" );
		printf( "Test (A)ll, (T)est Program, (R)un Program, (F)ire Individual, or (Q)uit.\n" );
		switch( getch() )
		{
		case 'a':
		case 'A':
			TestAllQueues();
			break;

		case 't':
		case 'T':
			TestProgram();
			break;

		case 'r':
		case 'R':
			RunProgram();
			break;

		case 'f':
		case 'F':
			FireSpecificQueue();
			break;

		case 'q':
		case 'Q':
			printf( "Quiting.\n" );
			return( 0 );
		}
	}
}

unsigned int Select( unsigned char Socket, unsigned char Queue )
{
	unsigned char index;

	index = Socket << 4 | Queue;
	Out32( PPORT_BASE+0, index );
	Out32( PPORT_BASE+2, 0x01 );
	Out32( PPORT_BASE+2, 0x00 );

	Sleep( SelectDelay );
	return( SelectDelay );
}

unsigned char Sense( void )
{
	return( Inp32( PPORT_BASE+1 ) & 0x10 );
}

unsigned int Fire( void )
{
	unsigned char index;

	index = (unsigned char)Inp32( PPORT_BASE+0 ); // Get the previous address

	Out32( PPORT_BASE+0, index | 0x80 ); // Fire socket (set high-order bit)
	Out32( PPORT_BASE+2, 0x01 ); // Begin strobe of STROBE
	Out32( PPORT_BASE+2, 0x00 ); // Begin strobe of STROBE

	Sleep( FireDelay );

	Out32( PPORT_BASE+0, index & ~0x80 ); // Fire socket
	Out32( PPORT_BASE+2, 0x01 ); // Begin strobe of STROBE
	Out32( PPORT_BASE+2, 0x00 ); // Begin strobe of STROBE

	return( FireDelay );
}

void TestAllQueues( void )
{
	int i, j;

	for( i = 0; i < 8; i++ )
	{
		printf( "Socket %d:\n", i );
		for( j = 0; j < 16; j++ )
		{
			Select( i, j );
			if( Sense() )
				printf( "Q%d, ", j );
		}
		printf( "\n" );
	}
}

void RunProgram( void )
{
	int i;
	bool CheckingInt;
	int CurrentDelay;

	printf( "\n\nPress Y to confirm firing.\n" );
	if( GetConfirmation() )
	{
		printf( "\nFiring...\n" );
		for( i = 0; i < totalQueues; i++ )
		{
			CurrentDelay = theSequence[i].Delay;
			CurrentDelay -= SelectDelay;
			CurrentDelay -= FireDelay;
			while( CurrentDelay >= 0 )
			{
				if( CurrentDelay < 1000 )
					Sleep( CurrentDelay );
				else
					Sleep( 1000 );
				CurrentDelay -= 1000;

				if( kbhit() )
				{
					getch();
					CheckingInt = 1;
					while( CheckingInt )
					{
						printf( "\n" );
						printf( "Show paused.\n" );
						printf( "(C)ontinue show\n" );
						printf( "(E)nd show\n" );
						printf( "(R)etry the last queue\n" );
						printf( "(F)ire the next queue immediately\n" );
						switch( getch() )
						{
						case 'C':
						case 'c':
							printf( "Press (Y) to CONTINUE the show.\n" );
							if( GetConfirmation() )
							{
								printf( "\n" );
								printf( "Continueing show.\n" );
								CheckingInt = 0;
							}
							break;

						case 'E':
						case 'e':
							printf( "Press (Y) to END the show.\n" );
							if( GetConfirmation() )
							{
								printf( "Show ended.\n" );
								return;
							}
							break;

						case 'R':
						case 'r':
							printf( "Press (Y) to RETRY the last queue and remain paused.\n" );
							if( GetConfirmation() )
							{
								Select( theSequence[i-1].Socket, theSequence[i-1].Queue );
								Fire();
								printf( "Socket %d, queue %d retried.\n", theSequence[i-1].Socket, theSequence[i-1].Queue );
							}
							break;

						case 'F':
						case 'f':
							printf( "Press (Y) to FIRE the next queue immediately and then continue the show.\n" );
							if( GetConfirmation() )
							{
								printf( "Firing next queue and continueing show.\n" );
								CurrentDelay = 0;
								CheckingInt = 0;
							}
							break;
						}
					}
				}
			}

			Select( theSequence[i].Socket, theSequence[i].Queue );
			Fire();
			printf( "Socket %d, Queue %d - %s: FIRED\n", theSequence[i].Socket, theSequence[i].Queue, theSequence[i].Desc );
		}
	}
}

void TestProgram( void )
{
	int i, result;

	printf( "\nTesting...\n" );
	for( i = 0; i < totalQueues; i++ )
	{
		Select( theSequence[i].Socket, theSequence[i].Queue );
		result = Sense();
		printf( "Socket %d, Queue %d - %s: ", theSequence[i].Socket, theSequence[i].Queue, theSequence[i].Desc );
		if( result )
			printf( "present\n" );
		else
			printf( "MISSING\n" );
	}
}

void FireSpecificQueue( void )
{
	int i, j, result;

	printf( "Socket: " );
	result = scanf( "%d", &i );
	if( !result || i < 0 || i > 7 )
	{
		fflush( stdin );
		return;
	}

	printf( "Queue: " );
	result = scanf( "%d", &j );
	if( !result || j < 0 || j > 15 )
	{
		fflush( stdin );
		return;
	}
    
	printf( "\nConfirm socket %d, queue %d: (Y)\n", i, j );
	result = getch();
	if( result == 'y' || result == 'Y' )
	{
		printf( "Socket %d, queue %d: FIRED.\n", i, j );
		Select( i, j );
		Fire();
	}
}

bool GetConfirmation( void )
{
	switch( getch() )
	{
	case 'Y':
	case 'y':
		return( 1 );
	
	default:
		return( 0 );
	}
}

