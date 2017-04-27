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
	__constant bool* changesAvailableLUT,
	__constant int* random
)
{
	int id = get_global_id(0); 

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
				ruleBits = *(random + *pixelOffset) < 0 ? 12 : 3;

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

