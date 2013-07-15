// mutex (atomic) handling

//********************************************* helpers for other libs

#define STGLIB_MUTEX StMutex _StGLib_Mutex
#ifdef DEBUG_STGLIB_MUTEX
#define STGLIB_MTLOCK(name) {STGLIB_DEBUG("MTwait: %s\n",name);++_StGLib_Mutex;STGLIB_DEBUG("MTLOCK: %s\n",name);}
#define STGLIB_MTUNLOCK(name) {STGLIB_DEBUG("MTUNLOCK: %s\n",name);--_StGLib_Mutex;}
#else
#define STGLIB_MTLOCK(ignore) ++_StGLib_Mutex
#define STGLIB_MTUNLOCK(ignore) --_StGLib_Mutex
#endif

#ifdef WIN32
//############################################# WIN32 VERSION
class StMutex
{
	HANDLE hMutex;
	unsigned locked;

public:
	StMutex()
	{
		locked=0;
		hMutex=CreateMutex(NULL,FALSE,NULL);
		if (!hMutex)
			throw; // a fit
	}
	~StMutex()
	{
		if (locked)
			ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}
	inline void _MutexLock(void)
	{
//printf("MutexWait: %08X by %08X was locked=%d\n",this,GetCurrentThreadId(),locked);
		switch (WaitForSingleObject(hMutex,INFINITE))
		{
		case WAIT_OBJECT_0:
			break;

		case WAIT_FAILED:
		case WAIT_ABANDONED:
		case WAIT_TIMEOUT:
		default:
			throw; // a fit
			break;
		}
//printf("MutexLock: %08X by %08X was locked=%d\n",this,GetCurrentThreadId(),locked);
		++locked;

		// if the same thread ++'s a mutex, locked will go to >1
	}

	inline void _MutexUnlock(void)
	{
//printf("MutexUnLk: %08X by %08X was locked=%d\n",this,GetCurrentThreadId(),locked);
		if (!locked)
			throw;
		--locked;
		if (!ReleaseMutex(hMutex))
			throw; // a fit
	}
	inline void operator++(void)
	{
		_MutexLock();
	}
	inline void operator--(void)
	{
		_MutexUnlock();
	}
	inline operator unsigned()
	{
		return(locked);
	}
};

#elif linux
//############################################# LINUX VERSION


// assume we need to implement SMP compatible code
#define LOCK "lock ; "

/*
 * Make sure gcc doesn't try to be clever and move things around
 * on us. We need to use _exactly_ the address the user gave us,
 * not some alias that contains the same information.
 */
typedef struct { volatile int counter; } atomic_t;

#define ATOMIC_INIT(i)	{ (i) }

/**
 * atomic_read - read atomic variable
 * @v: pointer of type atomic_t
 * 
 * Atomically reads the value of @v.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */ 
#define atomic_read(v)		((v)->counter)

/**
 * atomic_set - set atomic variable
 * @v: pointer of type atomic_t
 * @i: required value
 * 
 * Atomically sets the value of @v to @i.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */ 
#define atomic_set(v,i)		(((v)->counter) = (i))

/**
 * atomic_add - add integer to atomic variable
 * @i: integer value to add
 * @v: pointer of type atomic_t
 * 
 * Atomically adds @i to @v.  Note that the guaranteed useful range
 * of an atomic_t is only 24 bits.
 */
static __inline__ void atomic_add(int i, atomic_t *v)
{
	__asm__ __volatile__(
		LOCK "addl %1,%0"
		:"=m" (v->counter)
		:"ir" (i), "m" (v->counter));
}

/**
 * atomic_sub - subtract the atomic variable
 * @i: integer value to subtract
 * @v: pointer of type atomic_t
 * 
 * Atomically subtracts @i from @v.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */
static __inline__ void atomic_sub(int i, atomic_t *v)
{
	__asm__ __volatile__(
		LOCK "subl %1,%0"
		:"=m" (v->counter)
		:"ir" (i), "m" (v->counter));
}

/**
 * atomic_sub_and_test - subtract value from variable and test result
 * @i: integer value to subtract
 * @v: pointer of type atomic_t
 * 
 * Atomically subtracts @i from @v and returns
 * true if the result is zero, or false for all
 * other cases.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */
static __inline__ int atomic_sub_and_test(int i, atomic_t *v)
{
	unsigned char c;

	__asm__ __volatile__(
		LOCK "subl %2,%0; sete %1"
		:"=m" (v->counter), "=qm" (c)
		:"ir" (i), "m" (v->counter) : "memory");
	return c;
}

/**
 * atomic_inc - increment atomic variable
 * @v: pointer of type atomic_t
 * 
 * Atomically increments @v by 1.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */ 
static __inline__ void atomic_inc(atomic_t *v)
{
	__asm__ __volatile__(
		LOCK "incl %0"
		:"=m" (v->counter)
		:"m" (v->counter));
}

/**
 * atomic_dec - decrement atomic variable
 * @v: pointer of type atomic_t
 * 
 * Atomically decrements @v by 1.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */ 
static __inline__ void atomic_dec(atomic_t *v)
{
	__asm__ __volatile__(
		LOCK "decl %0"
		:"=m" (v->counter)
		:"m" (v->counter));
}

/**
 * atomic_dec_and_test - decrement and test
 * @v: pointer of type atomic_t
 * 
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */ 
static __inline__ int atomic_dec_and_test(atomic_t *v)
{
	unsigned char c;

	__asm__ __volatile__(
		LOCK "decl %0; sete %1"
		:"=m" (v->counter), "=qm" (c)
		:"m" (v->counter) : "memory");
	return c != 0;
}

/**
 * atomic_inc_and_test - increment and test 
 * @v: pointer of type atomic_t
 * 
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */ 
static __inline__ int atomic_inc_and_test(atomic_t *v)
{
	unsigned char c;

	__asm__ __volatile__(
		LOCK "incl %0; sete %1"
		:"=m" (v->counter), "=qm" (c)
		:"m" (v->counter) : "memory");
	return c != 0;
}

/**
 * atomic_add_negative - add and test if negative
 * @v: pointer of type atomic_t
 * @i: integer value to add
 * 
 * Atomically adds @i to @v and returns true
 * if the result is negative, or false when
 * result is greater than or equal to zero.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */ 
static __inline__ int atomic_add_negative(int i, atomic_t *v)
{
	unsigned char c;

	__asm__ __volatile__(
		LOCK "addl %2,%0; sets %1"
		:"=m" (v->counter), "=qm" (c)
		:"ir" (i), "m" (v->counter) : "memory");
	return c;
}

/* These are x86-specific, used by some header files */
#define atomic_clear_mask(mask, addr) \
__asm__ __volatile__(LOCK "andl %0,%1" \
: : "r" (~(mask)),"m" (*addr) : "memory")

#define atomic_set_mask(mask, addr) \
__asm__ __volatile__(LOCK "orl %0,%1" \
: : "r" (mask),"m" (*addr) : "memory")

/* Atomic operations are already serializing on x86 */
#define smp_mb__before_atomic_dec()	barrier()
#define smp_mb__after_atomic_dec()	barrier()
#define smp_mb__before_atomic_inc()	barrier()
#define smp_mb__after_atomic_inc()	barrier()


#define STMUTEX_WAITLIST_SIZE 8

class StMutex
{
	atomic_t locked;
	int waitlist[STMUTEX_WAITLIST_SIZE];

public:
	StMutex()
	{
		atomic_set(&locked,0);
	}
	~StMutex()
	{
		// must insure 
		if (locked)
			ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}
	inline void _MutexLock(void)
	{
//printf("MutexWait: %08X by %08X was locked=%d\n",this,GetCurrentThreadId(),locked);
		switch (WaitForSingleObject(hMutex,INFINITE))
		{
		case WAIT_OBJECT_0:
			break;

		case WAIT_FAILED:
		case WAIT_ABANDONED:
		case WAIT_TIMEOUT:
		default:
			throw; // a fit
			break;
		}
//printf("MutexLock: %08X by %08X was locked=%d\n",this,GetCurrentThreadId(),locked);
		++locked;

		// if the same thread ++'s a mutex, locked will go to >1
	}

	inline void _MutexUnlock(void)
	{
//printf("MutexUnLk: %08X by %08X was locked=%d\n",this,GetCurrentThreadId(),locked);
		if (!locked)
			throw;
		--locked;
		if (!ReleaseMutex(hMutex))
			throw; // a fit
	}
	inline void operator++(void)
	{
		_MutexLock();
	}
	inline void operator--(void)
	{
		_MutexUnlock();
	}
	inline operator unsigned()
	{
		return(locked);
	}
};


#endif
