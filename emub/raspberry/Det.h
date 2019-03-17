#ifndef _DET_H_
#define _DET_H_

#include "Std_Types.h"

#define Det_ReportError Det_ReportRuntimeError

void Det_ReportRuntimeError(
	uint8 ModulId,
	uint8 InstanceId,
	uint8 ApiId,
	uint8 ErrorId
	);

void Dem_ReportEvent(
	uint8 EventId,
	uint8 EventStatus
	);

#endif
