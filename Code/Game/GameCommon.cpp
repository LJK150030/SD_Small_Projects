#include "Game/GameCommon.hpp"


float CalcAverageTick(const float new_tick)
{
	g_tickSum -= g_tickList[g_tickIndex];
	g_tickSum += new_tick;
	g_tickList[g_tickIndex] = new_tick;
	if (++g_tickIndex == g_maxTickSample)    /* inc buffer index */
	{
		g_tickIndex = 0;
	}

	/* return average */
	return(g_tickSum / static_cast<float>(g_maxTickSample));
}
