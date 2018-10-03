# MemoryOrder
Demos of 3 ways even the strong memory model of x86 can exhibit architectural memory reordering, leading to bugs.

NOTE: make sure to leave whole-program optimization and incremental linking OFF for this program, as there is a horrible bug in MSVC in which changing intrinsics can fail to trigger a function recompile. In this demo, changing the memory fence type exhibits this bug, leading to running with stale code. Ugh.
