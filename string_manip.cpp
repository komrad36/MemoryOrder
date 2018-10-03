#include <iostream>
#include <thread>
#include <Windows.h>

// rep string instructions are StoreStore reorderable, but only
// within their own stores, not relative to ones outside that instruction.
// Even though normally x86 grants us StoreStore order for free,
// this means we need to use an external flag (which
// will get proper StoreStore ordering) instead of assuming we can check the
// last element of the store to know when it's done.
//
// if this sounds like a pure academic exercise, it isn't. If you use a
// hand-rolled memcpy/memset implementation, there is a decent chance that
// at least one of its paths uses string instructions, which then means it is
// not safe to check the completion of that memcpy by checking its last element,
// potentially leading to horrific bugs.
//
// this is a hard one to reproduce: it may take some time before this
// demo shows an example of an incorrect internal element. it may help
// to load your system by starting a memory-heavy operation like starting
// a browser with lots of tabs, running Prime95, etc.

constexpr size_t size = 1024*1024*4;
constexpr size_t sentinel_element = size - 1;
constexpr size_t check_element = size - 1 - 64;

__declspec(align(64)) volatile char data[size];

extern "C" void string_store(const size_t size, volatile char* data);

static void f() {
	for (;;) {
		// wait here until the last element has been set
		while (!data[sentinel_element]);

		////////////////////////////////
		// check an element in the middle
		const char c = data[check_element];
		if (c != 1) {
			// if we get here, it means the stores occurred out of order; the last element was set
			// prior to the check element
			std::cout << "Oops! Internal element is wrong! Expected 1, got " << +c << "!" << std::endl;
		}
		////////////////////////////////

		data[sentinel_element] = 0;
	}
}

void string_manip() {
	SetThreadAffinityMask(GetCurrentThread(), 1);

	data[sentinel_element] = 0;

	std::thread t(f);
	// different core. same core always sees its own stores and loads in-order.
	SetThreadAffinityMask(t.native_handle(), 4);

	std::cout
		<< "This one may take a while." << std::endl
		<< "It may help to load your system by starting" << std::endl
		<< "a memory-heavy operation like starting" << std::endl
		<< "a browser with lots of tabs, running Prime95, etc." << std::endl << std::endl;

	for (;;) {
		// set all of data to 0
		memset((void*)data, 0, size);

		////////////////////////////////
		// set all of data to 1 using a fast string store
		string_store(size / 8, data);
		////////////////////////////////

		while (data[sentinel_element]);
	}
}
