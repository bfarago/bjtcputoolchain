
#include "Det.h"
#include <stdio.h>

#if (1==DET_ENABLE)
void Det_ReportRuntimeError(
	uint8 ModulId,
	uint8 InstanceId,
	uint8 ApiId,
	uint8 ErrorId
	)
{
	printf("Det(0x%02x, 0x%02x, 0x%02x, 0x%02x)\n",ModulId, InstanceId, ApiId, ErrorId);
}
#endif

#if (1==DEM_ENABLE)
void Dem_ReportEvent(
	uint8 EventId,
	uint8 EventStatus
	)
{
	printf("Dem(0x%02x, 0x%02x)\n",EventId, EventStatus);
}
#endif
