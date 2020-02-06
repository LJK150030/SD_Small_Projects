#include "Game/GameCommon.hpp"

int tick_idx = 0;
double tick_sum = 0.0;
double tick_list[MAX_SAMPLE] = {};

double CalcRollingAvgTick(double new_tick)
{
	tick_sum -= tick_list[tick_idx];  
	tick_sum += new_tick;             
	tick_list[tick_idx] = new_tick;   
	if (++tick_idx == MAX_SAMPLE)
	{
		tick_idx = 0;
	}

	/* return average */
	return (tick_sum / MAX_SAMPLE);
}