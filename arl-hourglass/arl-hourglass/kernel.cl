bool hasPixelDesiredColor(constant uchar* inputPixel, constant uchar* desiredColor)
{
	return	*(inputPixel + 0) == *(desiredColor + 0) && 
			*(inputPixel + 1) == *(desiredColor + 1) && 
			*(inputPixel + 2) == *(desiredColor + 2) && 
			*(inputPixel + 3) == *(desiredColor + 3);
}

void applyColorToPixel(global uchar* inOutPixel, constant uchar* color)
{
	*(inOutPixel + 0) = *(color + 0);
	*(inOutPixel + 1) = *(color + 1);
	*(inOutPixel + 2) = *(color + 2);
	*(inOutPixel + 3) = *(color + 3);
}

bool isBitSet(char* bits, char* desiredBit)
{
	return *bits & (1 << *desiredBit);
}

void printColor(constant char* name, constant uchar* color)
{
	printf("%s = {%d, %u, %u, %u} \n", name,  *(color+0), *(color+1), *(color+2), *(color+3));
}

__kernel void simple_iteration(
	__constant uchar* pixelptr,
	__constant int* pixelOffset,
	__constant int* xDimension,
	__constant int* yDimension,
	__global uchar* tempData,
	__constant uchar* particleColor,
	__constant uchar* obstacleColor,
	__constant uchar* idleColor,
	__constant char* rulesLUT,
	__constant bool* changesAvailableLUT
)
{
	//printf("pixelOffset = %i\n", *pixelOffset);
	//printf("xDimension = %i\n", *xDimension);
	//printf("yDimension = %i\n", *yDimension);
	//printTest("dasdasdasda", *(particleColor+2));
	//int id = get_global_id(0); 
	//if (id >= 499)
	//{
	//	//printColor("particleColor", particleColor);
	//	//printColor("obstacleColor", obstacleColor);
	//	//printColor("idleColor", idleColor);
	//	for (int i = 0; i < 16; ++i)
	//		printf("%u ", *(rulesLUT + i));
	//	printf("\n");
	//	for (int i = 0; i < 16; ++i)
	//		printf("%s ", *(changesAvailableLUT + i) ? "true" : "false" );
	//	printf("\n");
	//}
	//char test[4];
	//test[0] = '0';
	//test[1] = '1';
	//test[2] = '2';
	//test[3] = '3';

	//uchar testPixel[4];
	//testPixel[0] = 123;
	//testPixel[1] = 123;
	//testPixel[2] = 123;
	//testPixel[3] = 123;

	//uchar testPixel2[4];
	//testPixel2[0] = 123;
	//testPixel2[1] = 123;
	//testPixel2[2] = 123;
	//testPixel2[3] = 123;

	//uchar testPixel3[4];
	//testPixel3[0] = 123;
	//testPixel3[1] = 124;
	//testPixel3[2] = 123;
	//testPixel3[3] = 123;

	//printf("%s\n", hasPixelDesiredColor(testPixel, testPixel2) ? "true" : "false");
	//applyColorToPixel(testPixel3, testPixel);
	//printf("%s\n", hasPixelDesiredColor(testPixel, testPixel3) ? "true" : "false");

	//char abc = 2;
	//char bit = 1;
	//printf("%s\n", isBitSet(&abc,&bit) ? "true" : "false");

	//for (int i = 0; i < 4; ++i)
	//	printf("%s", test+i);

	int id = get_global_id(0); // 0 .... 1000 -> image size

	//printf("%i\n", id);

	int x = (id * 2) + *pixelOffset;
	if (x < *xDimension - *pixelOffset)
	{
		for (int y = *pixelOffset; y < *yDimension - *pixelOffset; y += 2)
		{
			int row1 = 4 * (y * (*xDimension) + x);
			int row2 = row1 + (4 * (*xDimension));
			int pixelPositions[4];
			pixelPositions[0] = row1;
			pixelPositions[1] = row1 + 4;
			pixelPositions[2] = row2;
			pixelPositions[3] = row2 + 4;

			/******************* CHECK SAND CONSTELLATION AND WRITE BITS *******************/
			/*
			* bits represent:
			* 0 = upper left
			* 1 = upper right
			* 2 = lower left
			* 3 = lower right
			*/
			char particleBits = 0;
			for (char bitIndex = 0; bitIndex < 4; ++bitIndex)
				particleBits |= hasPixelDesiredColor(pixelptr + pixelPositions[bitIndex], particleColor) ? 1 << bitIndex : 0;


			/******************* GET MARGULOS NEIGHBORHOOD *******************/
			if (!*(changesAvailableLUT + particleBits))
				continue;

			char ruleBits = *(rulesLUT + particleBits);
			if (particleBits == 3)
			{
				//determine random if it remains 3 or will be 12
				ruleBits = 12;
			}

			/******************* CHECK FOR OBSTACLES AND WRITE BITS *******************/
			char obstacleBits = 0;
			for (char bitIndex = 0; bitIndex < 4; ++bitIndex)
			{
				if (!isBitSet(&particleBits, &bitIndex))
					obstacleBits |= hasPixelDesiredColor(pixelptr + pixelPositions[bitIndex], obstacleColor) ? 1 << bitIndex : 0;
			}

			/******************* CHECK IF NEW CONSTELLATION IS DOABLE *******************/
			// ~wallBits -> 0 = wall, 1 = usable
			if ((~obstacleBits & ruleBits) != ruleBits)
				continue;

			/******************* APPLY NEW COLORS/ MARGOLUS RULES *******************/
			for (char bitIndex = 0; bitIndex < 4; ++bitIndex)
			{
				//if current bit isn't wall then go on
				if (!isBitSet(&obstacleBits, &bitIndex))
					applyColorToPixel(tempData + pixelPositions[bitIndex], isBitSet(&ruleBits, &bitIndex) ? particleColor : idleColor);
			}
		}
	}
}

