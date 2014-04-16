#ifndef GAME_FLOW_MGR_H
#define GAME_FLOW_MGR_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Singleton.h"
#include "gslib/Util/HsmUtil.h"
#include "gslib/Math/Vector2.h"

const Vector2I UseMapSpawnPosition(-1, -1);

class GameFlowMgr : public Singleton<GameFlowMgr>, public IHsmOwner
{
	friend class Singleton<GameFlowMgr>;
	GameFlowMgr();

public:
	void Init();
	void Update(GameTimeType deltaTime);

	void SetTargetWorldMap(const char* worldMapFilePath, const Vector2I& initialPos = UseMapSpawnPosition);

private:
	hsm::StateMachine mStateMachine;
	struct GameFlowSharedStateData* mpGameFlowStateData;
};

#endif // GAME_FLOW_H
