/*=========================================================================
 *
 *  Copyright The University of North Carolina at Chapel Hill
 *  All rights reserved.
 *
 *  Licensed under the MADAI Software License. You may obtain a copy of
 *  this license at
 *
 *         https://madai-public.cs.unc.edu/software/license/
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

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
