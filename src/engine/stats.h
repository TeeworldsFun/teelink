#ifndef ENGINE_STATS_H
#define ENGINE_STATS_H

#include "kernel.h"

class IStats : public IInterface
{
	MACRO_INTERFACE("stats", 0)
public:

	virtual void Init() = 0;
	virtual void Reset() = 0;
	virtual void Save() = 0;
};

extern IStats *CreateStats();

#endif
