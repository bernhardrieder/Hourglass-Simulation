#pragma once

namespace MargolusNeighborhood
{
	namespace Sand
	{
		//3 is a special case!! -> leads to 3 or 12
		const char RulesLUT[16] =
		{
			0,4,8,3,4,12,12,13,8,12,12,14,12,13,14,15
		};

		//see above.. if index number == number in array, then there aren't any changes
		const bool ChangesAvailableLUT[16] =
		{
			false, true, true, true, false, true, true, true, false, true, true, true, false, false, false, false
		};
	}
}
