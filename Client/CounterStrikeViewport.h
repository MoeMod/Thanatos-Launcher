#include "TeamFortressViewport.h"

class CounterStrikeViewport : public TeamFortressViewport
{
public:
};

#define gViewPortInterface (*(CounterStrikeViewport **)gpViewPortInterface)

void InstallCounterStrikeViewportHook(void);