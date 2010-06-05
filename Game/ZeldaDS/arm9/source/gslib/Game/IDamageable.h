#ifndef IDAMAGEABLE_H
#define IDAMAGEABLE_H

namespace DamageEffect
{
	enum Type
	{
		Hurt,
		Stun,

		NumTypes,
		Default = Hurt
	};
}

struct DamageInfo
{
	DamageInfo() { Reset(); }
	
	void Reset()
	{
		mEffect = DamageEffect::Default;
		mAmount = 0;
		mPushVector.Reset(InitZero);
	}

	bool IsSet() const
	{
		return mAmount > 0;
	}

	DamageEffect::Type mEffect;
	uint16 mAmount;
	Vector2I mPushVector;
};

class IDamageable
{
public:
	virtual void OnDamage(const DamageInfo& damageInfo) = 0;
};

#endif // IDAMAGEABLE_H
