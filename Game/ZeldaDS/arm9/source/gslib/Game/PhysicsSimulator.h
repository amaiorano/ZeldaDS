#ifndef PHYSICS_SIMULATOR_H
#define PHYSICS_SIMULATOR_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Singleton.h"

class IPhysical;

class PhysicsSimulator : public Singleton<PhysicsSimulator>
{
public:
	void IntegrateAndApplyCollisions(GameTimeType deltaTime);

private:
	PhysicsSimulator() {}
	friend class Singleton<PhysicsSimulator>;
};

#endif // PHYSICS_SIMULATOR_H
