#ifndef ARRAY_2D_H
#define ARRAY_2D_H

// Efficient 2d array class, data is stored in row major order
// (i.e. x values from first row, followed by x values from second
// row, etc.)
template <typename T, typename SizeType = size_t>
class Array2d
{
public:
	typedef Array2d ThisType;

	Array2d() : mpArray(0) { }
	Array2d(SizeType sizeX, SizeType sizeY) : mpArray(0) { Reset(sizeX, sizeY); }
	~Array2d() { Reset(); }

	void Reset(SizeType sizeX = 0, SizeType sizeY = 0)
	{
		mSizeX = sizeX;
		mSizeY = sizeY;

		delete [] mpArray;

		if (sizeX > 0 && sizeY > 0)
		{
			mpArray = new T[sizeX * sizeY];
		}
		else
		{
			ASSERT(sizeX == 0 && sizeY == 0);
		}
	}

	SizeType Size() const { return mSizeX * mSizeY; }
	SizeType SizeX() const { return mSizeX; }
	SizeType SizeY() const { return mSizeY; }

	const T& operator()(SizeType x, SizeType y) const
	{
		ASSERT(x < mSizeX && y < mSizeY);
		return mpArray[y * mSizeX + x];
	}

	T& operator()(SizeType x, SizeType y)
	{
		ASSERT(x < mSizeX && y < mSizeY);
		return mpArray[y * mSizeX + x];
	}

	const T* RawPtr() const { return mpArray; }
	T* RawPtr() { return mpArray; }

private:
	T* mpArray;
	SizeType mSizeX, mSizeY;
};

#endif // ARRAY_2D_H
