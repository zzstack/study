#include <algorithm>
#include <ctime>
#include <iostream>

/*
Release运行
                                                                                sorted     unsorted
MacBook(Retain,12-inch,2017)    1.3 GHz Intel Core i5   10.13.6     XCode9.4    2246906    2179816
MacBook Air (13-inch, 2017)     2.2 GHz Intel Core i7   10.13.6     XCode9.4    1596941    1682151
MacBook Pro (15-inch, 2018)     2.2 GHz Intel Core i7   10.14.5     XCode10.2   1901891    1866985
*/
int main()
{
    std::cout << "start test ..." << std::endl;
    static const unsigned arraySize = 32768;
    int data[arraySize] = { 0 };
    
    for (unsigned int i = 0; i < arraySize; ++i) {
        data[i] = std::rand() % 256;
    }
    
    // !!! With this, the next loop runs faster.
    std::sort(data, data + arraySize);
    
    // Test
    clock_t start = clock();
    long long sum = 0;
    
    for (unsigned i = 0; i < 100000; ++i)
    {
        // Primary loop
        for (unsigned c = 0; c < arraySize; ++c)
        {
            if (data[c] >= 128)
                sum += data[c];
        }
    }
    
    long elapsedTime = static_cast<long>(clock() - start);
    
    std::cout << "sum = " << sum << ",cost = " << elapsedTime << std::endl;
    return 0;
}
