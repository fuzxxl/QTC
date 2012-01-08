#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "image.h"
#include "qti.h"
#include "qtc.h"
#include "ppm.h"

int main( int argc, char *argv[] )
{
	struct image image;
	struct qti compimage;

	int opt, verbose;
	unsigned long int insize, bsize, outsize;
	int transform, colordiff;
	int rangecomp;
	int minsize;
	int maxdepth;
	int lazyness;
	char *infile, *outfile;

	verbose = 0;
	transform = 0;
	colordiff = 0;
	rangecomp = 0;
	minsize = 2;
	maxdepth = 16;
	lazyness = 0;
	infile = NULL;
	outfile = NULL;

	while( ( opt = getopt( argc, argv, "cvy:t:s:d:l:i:o:" ) ) != -1 )
	{
		switch( opt )
		{
			case 't':
				if( sscanf( optarg, "%i", &transform ) != 1 )
					fputs( "ERROR: Can not parse command line\n", stderr );
			break;

			case 'c':
				rangecomp = 1;
			break;

			case 'y':
				if( sscanf( optarg, "%i", &colordiff ) != 1 )
					fputs( "ERROR: Can not parse command line\n", stderr );
			break;

			case 'v':
				verbose = 1;
			break;

			case 's':
				if( sscanf( optarg, "%i", &minsize ) != 1 )
					fputs( "ERROR: Can not parse command line\n", stderr );
			break;

			case 'd':
				if( sscanf( optarg, "%i", &maxdepth ) != 1 )
					fputs( "ERROR: Can not parse command line\n", stderr );
			break;

			case 'l':
				if( sscanf( optarg, "%i", &lazyness ) != 1 )
					fputs( "ERROR: Can not parse command line\n", stderr );
			break;

			case 'i':
				infile = strdup( optarg );
			break;

			case 'o':
				outfile = strdup( optarg );
			break;

			default:
			case '?':
				fputs( "ERROR: Can not parse command line\n", stderr );
				return 1;
			break;
		}
	}

	if( ! ppm_read( &image, infile ) )
		return 0;

	insize = image.width * image.height * 3;

	if( colordiff >= 1 )
		image_color_diff( &image );

	if( transform == 1 )
		image_transform_fast( &image );
	else if( transform == 2 )
		image_transform( &image );

	if( colordiff >= 2 )
	{
		if( ! qtc_compress_color_diff( &image, NULL, &compimage, minsize, maxdepth, lazyness ) )
			return 0;
	}
	else
	{
		if( ! qtc_compress( &image, NULL, &compimage, minsize, maxdepth, lazyness ) )
			return 0;
	}

	bsize = qti_getsize( &compimage );

	compimage.transform = transform;
	compimage.colordiff = colordiff;
	
	if( ! ( outsize = qti_write( &compimage, rangecomp, outfile ) ) )
		return 0;
	
	image_free( &image );
	qti_free( &compimage );
	
	if( verbose )
		fprintf( stderr, "In:%luB Buff:%luB,%f%% Out:%luB,%f%%\n", insize, bsize/8, (bsize/8)*100.0/insize, outsize, outsize*100.0/insize );

	free( infile );
	free( outfile );

	return 0;
}
