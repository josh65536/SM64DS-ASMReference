#ifndef SM64DS_COMMON_INCLUDED
#define SM64DS_COMMON_INCLUDED

#include <cstdint>
#include <algorithm>
#include <type_traits>
#include <nds.h>



class Actor;
class Player;

template<typename T>
struct bigger_internal {T val;};
template<> struct bigger_internal<unsigned> {uint64_t val;};
template<> struct bigger_internal< int    > { int64_t val;};
template<> struct bigger_internal<uint16_t> {unsigned val;};
template<> struct bigger_internal<short   > { int     val;};
template<typename T>
using bigger_t = decltype(bigger_internal<T>::val);

extern "C"
{
	int DivQ24(int n, int d);						//Division function for Q24
	uint16_t DecIfAbove0_Short(uint16_t& counter); //returns the counter's new value
	uint8_t DecIfAbove0_Byte(uint8_t& counter); //returns the counter's new value
	bool Lerp16(short& counterPtr, short dest, short step); //linear interpolation
	bool Lerp(int& counterPtr, int dest, int step);
}

namespace cstd {
	int fdiv(int n, int d);			//Same fdiv, just with int to avoid problems when calling it from Fix<T, Q>
}

template<typename T, unsigned Q>
struct Fix
{
	T val;
	inline Fix() {}
	inline constexpr explicit Fix(int value, bool raw = false) : val(raw ? value : value << Q) {}
	template<typename U, unsigned R> inline constexpr explicit Fix(Fix<U, R> fix)
	{
		val = Q > R ? (fix.val) << (Q - R) : (fix.val + (1 << (R - Q - 1))) >> (R - Q);
	}

	inline constexpr Fix<T, Q> operator-  ()              const { return  Fix<T, Q>(-val, true); }
	inline void                operator+= (Fix<T, Q> fix) { val += fix.val; }
	inline constexpr Fix<T, Q> operator+  (Fix<T, Q> fix) const { return  Fix<T, Q>(val + fix.val, true); }
	inline void                operator-= (Fix<T, Q> fix) { val -= fix.val; }
	inline constexpr Fix<T, Q> operator-  (Fix<T, Q> fix) const { return  Fix<T, Q>(val - fix.val, true); }
	inline void                operator*= (Fix<T, Q> fix) { val = ((bigger_t<T>)val * fix.val + (1 << (Q - 1))) >> Q; }
	inline void                operator*= (int integer) { val *= integer; }
	inline constexpr Fix<T, Q> operator*  (Fix<T, Q> fix) const { return  Fix<T, Q>(((bigger_t<T>)val * fix.val + (1 << (Q - 1))) >> Q, true); }
	inline constexpr Fix<T, Q> operator*  (int integer)   const { return  Fix<T, Q>(val * integer, true); }

	inline void                operator/= (Fix<T, Q> fix)
	{
		static_assert(Q == 12 || Q == 24, "Division function missing");
		val = (Q == 12 ? cstd::fdiv(val, fix.val) : DivQ24(val, fix.val));
	}
	inline Fix<T, Q>           operator/  (Fix<T, Q> fix) const
	{
		static_assert(Q == 12 || Q == 24, "Division function missing");
		return Q == 12 ? Fix<T, Q>(cstd::fdiv(val, fix.val), true) : Fix<T, Q>(DivQ24(val, fix.val), true);
	}

	inline void                operator/= (int integer) { val /= integer; }
	inline constexpr Fix<T, Q> operator/  (int integer)   const { return  Fix<T, Q>(val / integer, true); }
	inline void                operator<<=(int amount) { val <<= amount; }
	inline constexpr Fix<T, Q> operator<< (int amount)    const { return  Fix<T, Q>(val << amount, true); }
	inline void                operator>>=(int amount) { val >>= amount; }
	inline constexpr Fix<T, Q> operator>> (int amount)    const { return  Fix<T, Q>(val >> amount, true); }
	inline constexpr bool      operator== (Fix<T, Q> fix) const { return  val == fix.val; }
	inline constexpr bool      operator!= (Fix<T, Q> fix) const { return  val != fix.val; }
	inline constexpr bool      operator<  (Fix<T, Q> fix) const { return  val < fix.val; }
	inline constexpr bool      operator<= (Fix<T, Q> fix) const { return  val <= fix.val; }
	inline constexpr bool      operator>  (Fix<T, Q> fix) const { return  val > fix.val; }
	inline constexpr bool      operator>= (Fix<T, Q> fix) const { return  val >= fix.val; }

	inline constexpr Fix<T, Q> Abs() const { return this->val >= 0 ? *this : -*this; }
	inline constexpr explicit operator int() const { return val >> Q; } //warning! Floors!
	inline bool lerp(Fix<T, Q> dest, Fix<T, Q> step) { return Lerp(val, dest.val, step.val); }
};

template<typename T, unsigned Q> inline constexpr Fix<T, Q> operator* (int integer, Fix<T, Q> fix) { return Fix<T, Q>(integer * fix.val, true); }
template<typename T, unsigned Q> inline constexpr Fix<T, Q> operator/ (int integer, Fix<T, Q> fix) { return Fix<T, Q>(cstd::fdiv(integer << Q, fix.val), true); }
using Fix12i = Fix<int, 12>;
using Fix12s = Fix<short, 12>;
using Fix24i = Fix<int, 24>;

constexpr Fix12i operator""_f(unsigned long long val) { return Fix12i(val, true); }
constexpr Fix12s operator""_fs(unsigned long long val) { return Fix12s(val, true); }
constexpr Fix24i operator""_f24(unsigned long long val) { return Fix24i(val, true); }


namespace cstd {

	int div(int numerator, int denominator);			//32 bit signed division = 0x02052f4c
	int mod(int numerator, int denominator);			//32 bit signed mod = 0x02052ef4
	Fix12i fdiv(Fix12i numerator, Fix12i denominator);	//Fix12i division (32 bit)
	int64_t ldiv(Fix12i numerator, Fix12i denominator); //Fix12i division (64 bit)
	unsigned sqrt(uint64_t x);							//64 bit unsigned sqrt
	inline Fix12i sqrtQ24(Fix12i lower, Fix12i upper) { return Fix12i(sqrt((static_cast<uint64_t>(upper.val) << 32) | lower.val), true); }

	int strcmp(const char* str1, const char* str2);		//Compares two strings and returns the difference (0 when equal)
	char* strncpy(char* dest, const char* src, unsigned count);	//Copies n bytes from src to dest and returns a pointer to dest
	char* strchr(const char* str, char c);				//Searches for c in str and returns a pointer to the first occurence, or 0 if c could not be found
	unsigned strlen(const char* str);					//Returns the length of the string or -1 if no null-terminator has been found

	void fdiv_async(Fix12i numerator, Fix12i denominator);
	Fix12i fdiv_result();								//Returns the division result
	int64_t ldiv_result();								//Returns the 64 bit division result (which type?)
	void reciprocal_async(Fix12i x);					//Computes 1/x

	[[noreturn]] void _start();							//ROM entry point, resets the NDS on return = 0x02004800
	void __builtin_trap();								//Abort functionality. Triggers an undefined instruction (UDF)
	void __assert(const char* file, const char* line, const char* exp, int eval);	//Assertion that causes hangup if eval != 0

	unsigned sine_table;								//u16 per value, 4096 values, 2 bytes stride
	unsigned cosine_table;								//sine_table + 2
	unsigned atan_table;								//u16 per value, 1024 values

	int atan2(Fix12i y, Fix12i x);						//atan2 function, what about 0x020538b8?
	int abs(int x);										//Returns the absolute value of x

}


struct Vector3;
struct Vector3_16;
struct Matrix4x3;
struct Matrix3x3;
struct SharedFilePtr;
extern "C"
{
	extern Matrix4x3 MATRIX_4x3_IDENTITY;
	
	extern uint16_t POWERS_OF_TEN[3]; //100, 10, 1
	extern char DIGIT_ENC_ARR[10];
	
	extern int RNG_STATE; //x => x * 0x0019660d + 0x3c6ef35f
	
	extern Matrix4x3 MATRIX_SCRATCH_PAPER;
	extern unsigned* HEAP_PTR;
	extern unsigned FRAME_COUNTER;
	
	void UnloadObjBankOverlay(int ovID);
	bool LoadObjBankOverlay(int ovID);
	char* LoadFile	(int ov0FileID);
	
	void Crash();

	void FreeFileAllocation(void* ptr);
	void FreeHeapAllocation(void* ptr, unsigned* heapPtr);
	void* AllocateFileSpace(unsigned amount);
	
	short AngleDiff(short ang0, short ang1) __attribute__((const));
	void Vec3_RotateYAndTranslate(Vector3* vF, const Vector3* translation, short angY, const Vector3* v0); //vF and v0 cannot alias.
	short Vec3_VertAngle(const Vector3* v1, const Vector3* v0) __attribute__((pure));
	short Vec3_HorzAngle(const Vector3* v0, const Vector3* v1) __attribute__((pure));
	int RandomIntInternal(int* randomIntStatePtr);
	void Matrix4x3_FromTranslation(Matrix4x3* mF, Fix12i x, Fix12i y, Fix12i z);
	void Matrix4x3_FromRotationZ(Matrix4x3* mF, short angZ);
	void Matrix4x3_FromRotationY(Matrix4x3* mF, short angY);
	void Matrix4x3_FromRotationX(Matrix4x3* mF, short angX);
	void Matrix4x3_FromRotationZXYExt(Matrix4x3* mF, short angX, short angY, short angZ); //yxz intrinsic = zxy extrinsic
	void Matrix4x3_FromRotationXYZExt(Matrix4x3* mF, short angX, short angY, short angZ); //zyx intrinsic = xyz extrinsic
	void Matrix4x3_ApplyInPlaceToRotationZ(Matrix4x3* mF, short angZ); //transforms a rotation matrix using matrix mF.
	void Matrix4x3_ApplyInPlaceToRotationY(Matrix4x3* mF, short angY); //does not apply a rotation matrix.
	void Matrix4x3_ApplyInPlaceToRotationX(Matrix4x3* mF, short angX); //don't get the two confused.
	
	Fix12i Vec3_HorzDist(const Vector3* v0, const Vector3* v1) __attribute__((pure));
	Fix12i Vec3_HorzLen(const Vector3* v0) __attribute__((pure));
	Fix12i Vec3_Dist(const Vector3* v0, const Vector3* v1) __attribute__((pure));
	bool Vec3_Equal(const Vector3* v0, const Vector3* v1) __attribute__((pure));
	void Vec3_LslInPlace(Vector3* vF, int amount);
	void Vec3_Lsl(Vector3* vF, const Vector3* v, int amount);
	void Vec3_AsrInPlace(Vector3* vF, int amount);
	void Vec3_Asr(Vector3* vF, const Vector3* v, int amount);
	void Vec3_DivScalarInPlace(Vector3* vF, Fix12i scalar);
	void Vec3_MulScalarInPlace(Vector3* vF, Fix12i scalar);
	void Vec3_MulScalar(Vector3* vF, const Vector3* v, Fix12i scalar);
	void Vec3_Sub(Vector3* vF, const Vector3* v0, const Vector3* v1);
	void Vec3_Add(Vector3* vF, const Vector3* v0, const Vector3* v1);
	
	void Matrix3x3_LoadIdentity(Matrix3x3* mF);
	void Math_MulVec3Mat3x3(const Vector3* v, const Matrix3x3* m, Vector3* vF);
	void Math_MulMat3x3Mat3x3(const Matrix3x3* m1, const Matrix3x3* m0, Matrix3x3* mF); //m0 is applied to m1, so it's m0*m1=mF
	void Matrix4x3_LoadIdentity(Matrix4x3* mF);
	 // long call to force gcc to actually call the off-by-one address and therefore set the mode to thumb.
	void Matrix4x3_FromScale(Matrix4x3* mF, Fix12i x, Fix12i y, Fix12i z) __attribute__((long_call, target("thumb")));
	void Math_MulVec3Mat4x3(const Vector3* v, const Matrix4x3* m, Vector3* vF);
	void Math_MulMat4x3Mat4x3(const Matrix4x3* m1, const Matrix4x3* m0, Matrix4x3* mF); //m0 is applied to m1, so it's m0*m1=mF
	void Math_InvMat4x3(const Matrix4x3* m0, Matrix4x3* mF);		//Loads inverse of m0 into mF
	void Math_NormalizeVec3(const Vector3* v, Vector3* vF);
	Fix12i Math_LenVec3(const Vector3* v);
	void Math_CrossVec3(const Vector3* v0, const Vector3* v1, Vector3* vF);		//the cross product between v0 and v1 is placed into vF
	Fix12i Math_DotVec3(const Vector3* v0, const Vector3* v1) __attribute__((pure));
	void Math_AddVec3(const Vector3* v0, const Vector3* v1, Vector3* vF);

	void Matrix3x3_SetRotationX(Matrix3x3* m, Fix12i sinTheta, Fix12i cosTheta) __attribute__((long_call, target("thumb")));	//Resets m to an X rotation matrix
	void Matrix3x3_SetRotationY(Matrix3x3* m, Fix12i sinTheta, Fix12i cosTheta) __attribute__((long_call, target("thumb")));	//Resets m to a Y rotation matrix
	void Matrix3x3_SetRotationZ(Matrix3x3* m, Fix12i sinTheta, Fix12i cosTheta) __attribute__((long_call, target("thumb")));	//Resets m to a Z rotation matrix
	
	void MultiStore_Int(int val, void* dest, int byteSize);
	void MultiCopy_Int(void* source, void* dest, int byteSize);
	
	int String_Compare(const char* str1, const char* str2); //returns 0 if equal, a positive number if str1 comes after str2, and a negative number otherwise
	
	uint16_t Color_Interp(uint16_t* dummyArg, uint16_t startColor, uint16_t endColor, Fix12i time) __attribute__((const));
}

inline int RandomInt() {return RandomIntInternal(&RNG_STATE);}

//Matrix is column-major!
struct Matrix2x2
{
    Fix12i r0c0, r1c0;
    Fix12i r0c1, r1c1;
};

//Matrix is column-major!
struct Matrix3x3
{
    Fix12i r0c0, r1c0, r2c0;
    Fix12i r0c1, r1c1, r2c1;
    Fix12i r0c2, r1c2, r2c2;
	
	static Matrix3x3 IDENTITY;
	
	inline void LoadIdentity() {Matrix3x3_LoadIdentity(this);}
	
	inline Matrix3x3 operator*(const Matrix3x3& m) const {Matrix3x3 res; Math_MulMat3x3Mat3x3(&m, this, &res); return res;}
};

//Matrix is column-major!
struct Matrix4x3
{
	Fix12i r0c0, r1c0, r2c0;
	Fix12i r0c1, r1c1, r2c1;
	Fix12i r0c2, r1c2, r2c2;
	Fix12i r0c3, r1c3, r2c3;
	
	static Matrix4x3 IDENTITY;
	
	inline void LoadIdentity() {Matrix4x3_LoadIdentity(this);}
	inline void ThisFromScale(Fix12i x, Fix12i y, Fix12i z) {Matrix4x3_FromScale(this, x, y, z);}
	inline void ThisFromRotationZ(short angZ) {Matrix4x3_FromRotationZ(this, angZ);}
	inline void ThisFromRotationY(short angY) {Matrix4x3_FromRotationY(this, angY);}
	inline void ThisFromRotationX(short angX) {Matrix4x3_FromRotationX(this, angX);}
	inline void ThisFromRotationZXYExt(short angX, short angY, short angZ) {Matrix4x3_FromRotationZXYExt(this, angX, angY, angZ);}
	inline void ThisFromRotationXYZExt(short angX, short angY, short angZ) {Matrix4x3_FromRotationXYZExt(this, angX, angY, angZ);}
	inline void ThisFromTranslation(Fix12i x, Fix12i y, Fix12i z) {Matrix4x3_FromTranslation(this, x, y, z);}
	inline void ApplyInPlaceToRotationZ(short angZ) {Matrix4x3_ApplyInPlaceToRotationZ(this, angZ);}
	inline void ApplyInPlaceToRotationY(short angY) {Matrix4x3_ApplyInPlaceToRotationY(this, angY);}
	inline void ApplyInPlaceToRotationX(short angX) {Matrix4x3_ApplyInPlaceToRotationX(this, angX);}
	
	static inline Matrix4x3 FromScale(Fix12i x, Fix12i y, Fix12i z) {Matrix4x3 res; Matrix4x3_FromScale(&res, x, y, z); return res;}
	static inline Matrix4x3 FromRotationZ(short angZ) {Matrix4x3 res; Matrix4x3_FromRotationZ(&res, angZ); return res;}
	static inline Matrix4x3 FromRotationY(short angY) {Matrix4x3 res; Matrix4x3_FromRotationY(&res, angY); return res;}
	static inline Matrix4x3 FromRotationX(short angX) {Matrix4x3 res; Matrix4x3_FromRotationX(&res, angX); return res;}
	static inline Matrix4x3 FromRotationZXYExt(short angX, short angY, short angZ) {Matrix4x3 res; Matrix4x3_FromRotationZXYExt(&res, angX, angY, angZ); return res;}
	static inline Matrix4x3 FromRotationXYZExt(short angX, short angY, short angZ) {Matrix4x3 res; Matrix4x3_FromRotationXYZExt(&res, angX, angY, angZ); return res;}
	static inline Matrix4x3 FromTranslation(Fix12i x, Fix12i y, Fix12i z) {Matrix4x3 res; Matrix4x3_FromTranslation(&res, x, y, z); return res;}
	
	inline Matrix4x3 Inv() const {Matrix4x3 res; Math_InvMat4x3(this, &res); return res;}
	inline Matrix4x3 operator*(const Matrix4x3& m) const {Matrix4x3 res; Math_MulMat4x3Mat4x3(&m, this, &res); return res;}
};

struct Vector2
{
	Fix12i x, y;
};
struct Vector3
{
	Fix12i x, y, z;
	
	inline void    operator+= (const Vector3& v)       {Vec3_Add(this, this, &v);}
	inline Vector3 operator+  (const Vector3& v) const {Vector3 res; Vec3_Add(&res, this, &v); return res;}
	inline void    operator-= (const Vector3& v)       {Vec3_Sub(this, this, &v);}
	inline Vector3 operator-  (const Vector3& v) const {Vector3 res; Vec3_Sub(&res, this, &v); return res;}
	inline void    operator*= (Fix12i scalar   )       {Vec3_MulScalarInPlace(this, scalar);}
	inline Vector3 operator*  (Fix12i scalar   ) const {Vector3 res; Vec3_MulScalar(&res, this, scalar); return res;}
	inline void    operator/= (Fix12i scalar   )       {Vec3_DivScalarInPlace(this, scalar);}
	inline Vector3 operator/  (Fix12i scalar   ) const {Vector3 res = *this; res /= scalar; return res;} //no DivScalar exists.
	inline void    operator<<=(int amount)             {Vec3_LslInPlace(this, amount);}
	inline Vector3 operator<< (int amount)       const {Vector3 res; Vec3_Lsl(&res, this, amount); return res;}
	inline void    operator>>=(int amount)             {Vec3_AsrInPlace(this, amount);}
	inline Vector3 operator>> (int amount)       const {Vector3 res; Vec3_Asr(&res, this, amount); return res;}
	inline bool    operator== (const Vector3& v) const {return Vec3_Equal(this, &v);}
	
	inline Fix12i HorzDist(const Vector3& v) const {return Vec3_HorzDist(this, &v);}
	inline Fix12i HorzLen() const {return Vec3_HorzLen(this);}
	inline Fix12i Dist(const Vector3& v) const {return Vec3_Dist(this, &v);}
	inline Fix12i Len() const {return Math_LenVec3(this);}
	inline void Normalize() {Math_NormalizeVec3(this, this);}
	inline Vector3 Normalized() const {Vector3 res; Math_NormalizeVec3(this, &res); return res;}
	inline short HorzAngle(const Vector3& v) const {return Vec3_HorzAngle(this, &v);}
	inline short VertAngle(const Vector3& v) const {return Vec3_VertAngle(this, &v);}
	inline Fix12i Dot(const Vector3& v) const {return Math_DotVec3(this, &v);}
	inline Vector3 Cross(const Vector3& v) const {Vector3 res; Math_CrossVec3(this, &v, &res); return res;}
	inline void TransformThis(const Matrix3x3& m) {Math_MulVec3Mat3x3(this, &m, this);}
	inline Vector3 Transform(const Matrix3x3& m) const {Vector3 res; Math_MulVec3Mat3x3(this, &m, &res); return res;}
	inline void TransformThis(const Matrix4x3& m) {Math_MulVec3Mat4x3(this, &m, this);}
	inline Vector3 Transform(const Matrix4x3& m) const {Vector3 res; Math_MulVec3Mat4x3(this, &m, &res); return res;}
	inline Vector3 RotateYAndTranslate(const Vector3& trans, short angY) const {Vector3 res; Vec3_RotateYAndTranslate(&res, &trans, angY, this); return res;}
};
struct Vector3_16
{
	short x, y, z;
};
struct Vector3_16f
{
	Fix12s x, y, z;
};
struct Vector2_16
{
	short x, y;
};

struct SharedFilePtr
{
	uint16_t fileID;
	uint8_t numRefs;
	char* filePtr;
	
	SharedFilePtr& Construct(unsigned ov0FileID);
	char* Load();
	void Release();
};

template<typename T>
struct BaseSpawnInfo
{
	T*(*spawnFunc)();
	short behavPriority;
	short renderPriority;
};
template<typename T>
struct SpawnInfo
{
	T*(*spawnFunc)();
	short behavPriority;
	short renderPriority;
	unsigned flags;
	Fix12i rangeOffsetY;
	Fix12i range;
	Fix12i drawDist;
	Fix12i unkc0;
};

inline Fix12i Sin(uint16_t angle)
{
    return Fix12i(*(short*)(0x02082214 + 4 * ((angle + 8) / 0x10)), true);
}
inline Fix12i Cos(uint16_t angle)
{
    return Fix12i(*(short*)(0x02082216 + 4 * ((angle + 8) / 0x10)), true);
}

constexpr uint16_t Color5Bit(uint8_t r, uint8_t g, uint8_t b) //0x00 to 0xff each
{
	return (uint16_t)r >> 3 << 0 |
		   (uint16_t)g >> 3 << 5 |
		   (uint16_t)b >> 3 << 10;
}

constexpr uint16_t Arr3_5Bit(uint8_t val0, uint8_t val1, uint8_t val2)
{
	return (uint16_t)val0 << 0 |
		   (uint16_t)val1 << 5 |
		   (uint16_t)val2 << 10;
}


#endif
