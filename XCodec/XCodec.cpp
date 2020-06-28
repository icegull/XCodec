#include <cstdio>
#include <cstdlib>
#include "ffmpeg.h"
#include <ppl.h>

int main(int argc, char **argv)
{
	if (argc <= 2)
	{
		fprintf(stderr, "Usage: %s <codec name> <output file>\n", argv[0]);
		exit(0);
	}
	const char* codec_name = argv[1];
	const char* filename = argv[2];

	//ffmpeg_encode_frame(0, filename, codec_name, 7680, 4320, 100 * 1024 * 1024);
	concurrency::parallel_for(0, 4, [&](int idx)
		{
			ffmpeg_encode_frame(idx, filename, codec_name, 3840, 2160, 50 * 1024 * 1024);
		});

	getchar();
	return 0;
}