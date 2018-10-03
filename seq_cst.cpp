#include <immintrin.h>
#include <iostream>
#include <thread>
#include <Windows.h>

// we need the stores to precede the loads as seen from
// other cores, which means we need to prevent StoreLoad reordering, which
// means we need a full MFENCE. Other fences or combinations of fences will
// not suffice. We need sequential consistency.
// The compiler can also screw us. The optimal solution is
// a std::atomic_bool stored and loaded with std::memory_order_seq_cst.

// pick a fence, any fence
#define FENCE
//#define FENCE _mm_lfence()
//#define FENCE _mm_sfence()
//#define FENCE _mm_mfence()

// variables used in Peterson's algorithm
__declspec(align(64)) volatile int x;
__declspec(align(64)) volatile int y;

// ignore me; used for test harness
__declspec(align(64)) volatile bool gate1;
__declspec(align(64)) volatile bool gate2;
__declspec(align(64)) volatile bool thd1_entered;
__declspec(align(64)) volatile bool thd2_entered;

static void f1() {
	for (;;) {
		while (!gate1);
		gate1 = true;

		////////////////////////////////
		x = 1;
		_ReadWriteBarrier(); // make sure the compiler doesn't screw us
		FENCE;
		if (y == 0) {
			// enter critical section
			thd1_entered = true;
		}
		////////////////////////////////

		gate1 = false;
	}
}

static void f2() {
	for (;;) {
		while (!gate2);
		gate2 = true;

		////////////////////////////////
		y = 1;
		_ReadWriteBarrier(); // make sure the compiler doesn't screw us
		FENCE;
		if (x == 0) {
			// enter critical section
			thd2_entered = true;
		}
		////////////////////////////////

		gate2 = false;
	}
}

void seq_cst() {
	gate1 = gate2 = false;

	std::thread t1(f1);
	SetThreadAffinityMask(t1.native_handle(), 1);
	std::thread t2(f2);
	// different core. same core always sees its own stores and loads in-order.
	SetThreadAffinityMask(t2.native_handle(), 4);

	for (;;) {
		x = y = 0;
		thd1_entered = thd2_entered = false;
		gate1 = gate2 = true;
		while (gate1 || gate2);
		if (thd1_entered && thd2_entered) {
			std::cout << "Oops! Thd1 and thd2 both entered the critical section!" << std::endl;
		}
	}
}
