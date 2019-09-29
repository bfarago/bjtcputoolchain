#ifndef _DET_H_
#define _DET_H_

#include "Std_Types.h"
#define DET_ENABLE 1
#define DEM_ENABLE 0

#if (1==DET_ENABLE)
#define Det_ReportError Det_ReportRuntimeError

void Det_ReportRuntimeError(
	uint8 ModulId,
	uint8 InstanceId,
	uint8 ApiId,
	uint8 ErrorId
	);
#else
#define Det_ReportError(modulid, instanceid, apiid, errorid)
#define Det_ReportRuntimeError(modulid, instanceid, apiid, errorid)
#endif


#if (1==DEM_ENABLE)
void Dem_ReportEvent(
	uint8 EventId,
	uint8 EventStatus
	);
#else
#define Dem_ReportEvent(eventid, eventstatus)
#endif

#endif
