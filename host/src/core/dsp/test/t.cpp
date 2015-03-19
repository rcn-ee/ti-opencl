#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <list>
#include <numeric>
#include <vector>

using namespace std;

template <class T>
void print(T& c){
   for( typename T::iterator i = c.begin(); i != c.end(); i++ ){
      std::cout << *i << endl;
   }
}

int main( )
{
   const float a[] = { 1, 1.3, 1.5};

   vector<float> data( a,a + sizeof( a ) / sizeof( a[0] ) );
   print( data  );

   float mean = accumulate( data.begin(), data.end(), 0.0f )/ data.size();
   cout << mean << endl;

   vector<float> zero_mean( data );
   transform( zero_mean.begin(), zero_mean.end(), zero_mean.begin(),bind2nd( minus<float>(), mean ) );

   float deviation = inner_product( zero_mean.begin(),zero_mean.end(), zero_mean.begin(), 0.0f );
   deviation = sqrt( deviation / ( data.size() - 1 ) );
   cout << deviation;
}
