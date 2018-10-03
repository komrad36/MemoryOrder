extern void string_manip();
extern void stream_store();
extern void seq_cst();

// pick a test to run

//auto test = string_manip;
//auto test = stream_store;
auto test = seq_cst;

int main() {
	test();
}