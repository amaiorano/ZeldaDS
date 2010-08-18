#ifndef GAME_FLOW_MGR_H
#define GAME_FLOW_MGR_H

#include "gslib/Core/Core.h"
#include "gslib/Core/Singleton.h"
#include "gslib/Hsm/HsmStateMachine.h"

class GameFlowMgr : public Singleton<GameFlowMgr>
{
	friend class Singleton<GameFlowMgr>;
	GameFlowMgr();

public:
	void Init();
	void Update(GameTimeType deltaTime);

	void SetTargetWorldMap(const char* worldMapFilePath);

private:
	StateMachine mStateMachine;
	struct GameFlowSharedStateData* mpSharedStateData;
};

#endif // GAME_FLOW_H
