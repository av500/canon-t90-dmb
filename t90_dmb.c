/*
Copyright (c)2008, Vladimir Pantelic
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
Canon T90 Data Memory Back interface code, using a PC parallel port 

written in 2008 using Borland C, conio.h, io.h inp(), outp() etc need to be replaced with something more "modern"

the timing loops are also way off these days :)
*/

#include <conio.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#define PARPORT 0x0378

#define BYTENUM 2068

unsigned char PORTBYTE = 0;

unsigned char Data[4096];


#define CLK 	0x08
#define DRB 	0x04
#define DCA 	0x02
#define DATA 	0x01

#define SetBit( mask ) PORTBYTE |= (mask); outp( PARPORT, PORTBYTE )

#define ClrBit( mask ) PORTBYTE &= ~(mask); outp ( PARPORT, PORTBYTE )

#define GetBit( mask ) ( ( ( inp( PARPORT+1 ) >> 4 ) ^ 0x07 ) & (mask) )

#define Clkhigh (inp( PARPORT+1) & 0x80)

#define WAIT120us for ( j = 0; j < 5400; j ++ ) { j = (j + 1);}

#define WAIT100us for ( j = 0; j < 4500; j ++ ) { j = (j + 1);}

#define WAIT10us for ( j=0; j<375; j++) { j = j+1; }

#define WAIT150us for ( j=0; j<6750; j++) { j = j+1; }

#define WAIT300us for ( j=0; j<13500; j++) { j = j+1; }

#define WAIT1800us for ( j=0; j<83000; j++) { j = j+1; }

#define WAIT2500us for ( j=0; j<114000; j++) { j = j+1; }

void PutByte( unsigned int c )
{
	int i = 7;
	unsigned long j;

	while ( i >= 0 ) {

		ClrBit( CLK );

		if ( (c >> i) & 0x01 ) {
			SetBit( DCA );
		} else {
			ClrBit( DCA );
		}
		WAIT100us;

		SetBit( CLK );

		WAIT100us;

		i--;
	}
}


unsigned char GetByte( void )
{
	unsigned char b = 0;
	unsigned long j;
	int i;


	for ( i = 0; i < 8; i++){

		ClrBit( CLK );

		WAIT100us;

		//b = b << 1;

		SetBit( CLK );

		if ( GetBit( DATA ) != 0 )
			b |= 1 << i ;

		WAIT100us;

	}
	return b;
}


unsigned char ScanByte( int mask )
{
	int i;
	unsigned char b = 0;
	unsigned long j;

	for ( i = 0; i < 8; i++){

		b = b << 1;
/*
		while( GetBit( CLK ) == CLK );

		while( GetBit( CLK ) == 0 );
*/
		while( Clkhigh !=0 );



		while( Clkhigh ==0 );

		if ( GetBit( mask ) != 0 )
			b += 1;


	}
	return b;

}

void main( void )
{
	int pic;

	int frame;
	int tv;
	int av;
	int number;
	int lens;
	int month;
	int year;
	int date;
	int hour;
	int min;
	int expcomp;
	int ISO;
	int flash;
	int program;
	int pattern;
	int manual;
	int stoppeddown;

	int EXPCOMP;
	int SPOT[8];
	int EVR;
	int FEL;


	int i;
	unsigned long j;

	unsigned char b;

	unsigned char Head[3];

	unsigned long sum=0;

	FILE *outfile;

	PORTBYTE = 0;

	// Set all Bits to indicate Idle state
	SetBit( DATA | DCA | DRB | CLK );

	printf("\n\n Press Key to start transfer\n\n");

	while( !kbhit() ) { }

	(void) getch();

	printf("scanning... \n\n");

/*
	while( GetBit ( CLK ) == CLK ) ;

	while( GetBit ( CLK ) == 0 );

	while( GetBit ( CLK ) == CLK ) ;

	while( GetBit ( CLK ) == 0 );


	for ( i = 0; i < 3; i++ ) {
		Head[i] = ScanByte( DCA );
	}

	while( GetBit ( CLK ) == CLK ) ;

	while( GetBit ( CLK ) == 0 );

	for ( i = 0; i < BYTENUM; i++ ) {
		Data[i] = ScanByte( DATA );
	}


	goto END;
*/

	// CLEAR ALL
	ClrBit( DATA | DCA | DRB | CLK );


	WAIT10us;

	SetBit( DRB | CLK );

	WAIT10us;

	WAIT10us;

	if ( GetBit( DRB ) == 1 ) {
		printf("\nNo Camera present\n");
		goto END;
	}

	SetBit ( DCA );

	WAIT10us;

	SetBit( DATA );

	WAIT10us;

	WAIT10us;

	ClrBit( DATA | DRB );

	while ( GetBit( CLK ) == CLK );

	while ( GetBit( CLK ) == 0 );

	WAIT2500us;

	SetBit( DATA | DRB );

	WAIT150us;

	PutByte( 0x5A );

	WAIT300us;

	PutByte( 0xBB );

	WAIT300us;

	PutByte( 0xFF );

	SetBit( CLK | DCA );

	WAIT100us;

	while ( GetBit( CLK ) == 0 );

	WAIT1800us;

	for ( i = 0; i < BYTENUM; i++ ) {

		Data[i] = GetByte();

		WAIT150us;

	}

	SetBit( DATA | DCA | DRB | CLK );

END:

	pic = 0;
	i = 2064;

	printf("Write ... ");
	if ( (outfile = fopen("\\T90.BIN", "w+b" )) == NULL )
	{
		perror("Error:");
		return 1;
	}

	while ( i >= 0 ) {


		if ( Data[i] < 16 ) {
			printf("0%X ", Data[i] );
			fprintf(outfile, "0%X ", Data[i] );
		} else {
			printf( "%X ", Data[i] );
			fprintf(outfile, "%X ", Data[i] );
		}

		if ( Data[i] == 0xFF ) {
			pic++;
			printf(" : %d \n", pic);
			fprintf(outfile, " : %d \n", pic);
		}
		i --;
		if ( i < 0 )
			break;
	}

//	for ( i = 0; i < BYTENUM; i++ ) {
//		fputc( Data[i], outfile);
//	}
	fclose(outfile);


	printf("... file\n");

ENDEND:
	printf("\n\n Press Key to Quit");
	while( !kbhit() ) { }

	return;

}