#ifndef HEALTH_H
#define HEALTH_H

#include "gslib/Core/Core.h"

class Health
{
public:
	Health()
	{
		Init(6);
	}

	void Init(int16 maxValue)
	{
		SetInvincible(false);
		mMaxValue = maxValue;
		SetValue(mMaxValue);
	}

	bool IsAlive() const { return mValue > 0; }
	bool IsDead() const { return mValue == 0; }
	bool IsInvincible() const { return mInvincible; }

	int16 GetValue() const
	{
		return mValue;
	}

	void SetValue(int16 newValue)
	{
		ASSERT(newValue >= 0 && newValue <= mMaxValue);
		
		// While invincible, don't expect value to be modified (could just early out
		// if that's more useful)
		ASSERT(!IsInvincible());

		mValue = newValue;

		if (IsDead()) // If we just died, reset invincibility
		{
			SetInvincible(false);
		}
	}

	void OffsetValue(int16 offset)
	{
		ASSERT(!IsInvincible());
		SetValue( MathEx::Clamp(mValue + offset, 0, mMaxValue) );
	}

	void SetInvincible(bool bEnabled, GameTimeType invincibleTime = 0)
	{
		mInvincible = bEnabled;
		mInvincibleTime = invincibleTime;
		mElapsedInvincibleTime = 0;
	}

	void Resurrect()
	{
		Init(mMaxValue);
	}

	void Update(GameTimeType deltaTime)
	{
		if (IsInvincible() && mInvincibleTime > 0)
		{
			mElapsedInvincibleTime += deltaTime;
			if (mElapsedInvincibleTime >= mInvincibleTime)
			{
				SetInvincible(false);
			}
		}
	}

private:
	int16 mMaxValue;
	int16 mValue;

	// When damaged, some characters become invincible for a time
	bool mInvincible;
	GameTimeType mInvincibleTime; // Set to 0 for infinite invincibility (god mode)
	GameTimeType mElapsedInvincibleTime;
};

#endif // HEALTH_H
