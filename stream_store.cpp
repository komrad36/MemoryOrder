#include <immintrin.h>
#include <iostream>
#include <thread>
#include <Windows.h>

// stream stores are StoreStore reorderable, so
// even though normally x86 grants us StoreStore order for free,
// in this case we need to enforce it manually via an SFENCE.
// the optimal solution is a std::atomic_bool stored with
// std::memory_order_release, and loaded with
// std::memory_order_acquire.

// pick a fence, any fence
#define FENCE
//#define FENCE _mm_lfence()
//#define FENCE _mm_sfence()
//#define FENCE _mm_mfence()

constexpr size_t num_m128s = 1024;
__declspec(align(64)) float data[4 * num_m128s];

const __m128 ref = _mm_set_ps(1, 2, 3, 4);

// store flag
__declspec(align(64)) volatile bool gate;

static void f() {
	for (;;) {
		// wait for store flag
		while (!gate);

		////////////////////////////////
		// check the last element
		const __m128 v = _mm_load_ps(data + 4 * (num_m128s - 1));
		if (_mm_movemask_ps(_mm_cmpeq_ps(v, ref)) != 0b1111) {
			// if we get here, the stores occurred out of order even outside the stream loop: the setting of the flag
			// became visible to us before the last stream store did
			std::cout << "Oops! Last element is wrong! Expected (1, 2, 3, 4), got (" << v.m128_f32[0] << ", " << v.m128_f32[1] << ", " << v.m128_f32[2] << ", " << v.m128_f32[3] << ")!" << std::endl;
		}
		////////////////////////////////

		gate = false;
	}
}

void stream_store() {
	SetThreadAffinityMask(GetCurrentThread(), 1);

	gate = false;

	std::thread t(f);
	// different core. same core always sees its own stores and loads in-order.
	SetThreadAffinityMask(t.native_handle(), 4);

	for (;;) {
		// set all data to 0
		memset(data, 0, num_m128s * sizeof(__m128));

		////////////////////////////////
		// set all data to vectors of {1, 2, 3, 4} using stream stores
		for (int i = 0; i < num_m128s; ++i) {
			_mm_stream_ps(data + 4 * i, ref);
		}

		// fence
		_ReadWriteBarrier(); // make sure the compiler doesn't screw us
		FENCE;
		////////////////////////////////

		// release gate
		gate = true;
		while (gate);
	}
}
