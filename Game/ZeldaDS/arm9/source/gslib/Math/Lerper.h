#ifndef LERPER_H
#define LERPER_H

#include "gslib/Core/Core.h"
#include "gslib/Math/MathEx.h"

template <typename ValueType, typename TimeType>
class Lerper
{
public:
	Lerper()
	{
		Reset(0, 0, 0, 1, 0);
	}

	void Reset(ValueType min, ValueType max, ValueType curr, ValueType dir, TimeType timeToLerp)
	{
		mMin = min;
		mMax = max;
		mCurr = curr;
		mDir = dir;
		mTimeToLerp = timeToLerp;

		ASSERT(mDir == 1 || mDir == -1);
		ASSERT(mMin <= mMax);

		if (timeToLerp == 0)
		{
			mCurr = mDir > 0? mMax : mMin;
		}
	}

	void Reset(ValueType dir, TimeType timeToLerp)
	{
		Reset(mMin, mMax, mCurr, dir, timeToLerp);
	}

	void Update(TimeType deltaTime)
	{
		if (mTimeToLerp > 0 && deltaTime > 0)
		{
			ValueType inc = (static_cast<ValueType>(deltaTime) / mTimeToLerp) * mDir;
			mCurr = MathEx::Clamp(mCurr + inc, mMin, mMax);

			// Done fading?
			if (mCurr <= mMin || mCurr >= mMax)
			{
				mTimeToLerp = 0;
			}
		}
	}

	ValueType GetCurr() const { return mCurr; }

	bool IsDone() const
	{
		return mTimeToLerp == 0;
	}

private:
	ValueType mMin, mMax;
	ValueType mDir; // +/-1
	ValueType mCurr; // [mMin, mMax]
	TimeType mTimeToLerp;
};


#endif // LERPER_H
