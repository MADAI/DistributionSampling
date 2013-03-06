#include <iostream>
#include "Random.h"
using std::cout;

void test(madai::Random & r){
	cout << r.Integer(10000) << '\t';
	cout << r.Uniform() << '\t';
	cout << r.Uniform(-100.0, 100.0) << '\t';
	cout << r.Gaussian() << '\t';
	cout << r.Gaussian(50.0, 5.0) << '\n' << '\n';
	return;
}
int main(int argc, char ** argv) {
	cout << "\n";

	madai::Random r1;
	test(r1);

	unsigned long int SEED = 34567;

  madai::Random r2(SEED);
	test(r2);

	r1.Reseed(SEED);
	test(r1);

	r2.Reseed();
	test(r2);

	return EXIT_SUCCESS;
}

