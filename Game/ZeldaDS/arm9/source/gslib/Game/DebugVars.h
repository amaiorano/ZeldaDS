#ifndef DEBUG_VARS_H
#define DEBUG_VARS_H

#include "gslib/Core/Config.h"

#if DEBUG_VARS_ENABLED

// Contains all debug vars, access via macros below
struct DebugVars
{
	DebugVars()
		: DrawCollisionBounds(false)
	{
	}

	bool DrawCollisionBounds;
};

extern DebugVars gDebugVars;

#define DEBUG_VAR_SET(var, value) gDebugVars.var = value;
#define DEBUG_VAR_TOGGLE(var) gDebugVars.var = !gDebugVars.var;
#define DEBUG_VAR_GET(var) (gDebugVars.var)

#endif // DEBUG_VARS_ENABLED

#endif // DEBUG_VARS_H