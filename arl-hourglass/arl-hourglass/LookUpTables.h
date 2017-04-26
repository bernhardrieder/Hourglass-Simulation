#pragma once


//char MargolusNeighborhoodPairs[16][4] = 
//{
//	// 0
//	 { 0,0,
//	   0,0},
//	// 1
//	 { 1,0,
//	   0,0},
//	// 2
//	 { 0,1,
//	   0,0 },
//	// 3
//	 { 1,1,
//	   0,0 },
//	// 4
//	 { 0,0,
//	   1,0 },
//	// 5
//	 { 1,0,
//	   1,0 },
//	// 6
//	 { 0,1,
//	   1,0 },
//	// 7
//	 { 1,1,
//	   1,0 },
//	// 8
//	 { 0,0,
//	   0,1 },
//	// 9
//	 { 1,0,
//	   0,1 },
//	// 10
//	 { 0,1,
//	   0,1 },
//	// 11
//	 { 1,1,
//	   0,1 },
//	// 12
//	 { 0,0,
//	   1,1 },
//	// 13
//	 { 1,0,
//	   1,1 },
//	// 14
//	 { 0,1,
//	   1,1 },
//	// 15
//	 { 1,1,
//	   1,1 }
//};


//3 is a special case!! -> leads to 3 or 12
char MargolusNeighborhoodRulesLUT[16] =
{
	0,4,8,3,4,12,12,13,8,12,12,14,12,13,14,15
};

//see above.. if index number == number in array, then there aren't changes
bool AreThereAnyMargolusNeighborhoodChangesLUT[16] =
{
	false, true, true, true, false, true, true, true, false, true, true, true, false, false, false, false
};