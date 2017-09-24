#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <chrono>

#include <iostream>
#include <vector>

using namespace std::chrono;

/// ‘ункци€ ReducerMaxTest() определ€ет максимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMaxTest(int *mass_pointer, const long int size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long int i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	printf("Maximal element = %d has index = %d\n",
		maximum->get_reference(), maximum->get_index_reference());
}

/// ‘ункци€ ReducerMinTest() определ€ет минимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	printf("Minimum element = %d has index = %d\n",
		minimum->get_reference(), minimum->get_index_reference());
}

/// ‘ункци€ ParallelSort() сортирует массив в пор€дке возрастани€
/// begin - указатель на первый элемент исходного массива
/// end - указатель на последний элемент исходного массива
void ParallelSort(int *begin, int *end)
{
	if (begin != end)
	{
		--end;
		int *middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle);
		cilk_spawn ParallelSort(begin, middle);
		ParallelSort(++middle, ++end);
		cilk_sync;
	}
}

/// ‘ункци€ ParallelSortWithTime() сортирует массив в пор€дке возрастани€ c подсчетом затраченного времени
/// begin - указатель на первый элемент исходного массива
/// end - указатель на последний элемент исходного массива
void ParallelSortWithTime(int *begin, int *end)
{
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	ParallelSort(begin, end);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	duration<double> duration = (t2 - t1);
	std::cout << "Parallel sort with time is: " << duration.count() << " seconds" << std::endl;
}

/// ‘ункци€ CompareForAndCilk_For() выводит на консоль врем€ работы стандартного цикла for и врем€ работы параллельного цикла cilk_for (дл€ векторов)
/// sz - размер вектора
void CompareForAndCilk_For(size_t sz)
{
	high_resolution_clock::time_point t1,t2;
	duration<double> duration;
	std::vector<int> vecFor;

	t1 = high_resolution_clock::now();
	for (size_t i = 0; i < sz; ++i)
	{
		vecFor.push_back(rand() % 20000 + 1);
	}
	t2 = high_resolution_clock::now();
	duration = (t2 - t1);
	std::cout << "Mass_size: " << sz << std::endl;
	std::cout << "for: " << duration.count() << " seconds" << std::endl;
	
	t1 = high_resolution_clock::now();
	cilk::reducer<cilk::op_vector<int>>red_vec;
	cilk_for(size_t i = 0; i < sz; ++i)
	{
		red_vec->push_back(rand() % 20000 + 1);
	}
	t2 = high_resolution_clock::now();
	duration = (t2 - t1);
	std::cout << "cilk_for: " << duration.count() << " seconds" << std::endl << std::endl;
}

int main()
{
	srand((unsigned)time(0));

	// устанавливаем количество работающих потоков = 4
	__cilkrts_set_param("nworkers", "4");

	int massSize[] = { 1000000, 100000, 10000, 1000, 500, 100, 50, 10 };
	for (int i = 0; i < 8; ++i)
	{
		CompareForAndCilk_For(massSize[i]);
	}

	int *mass, *mass_begin, *mass_end;
	for (int i = 0; i < 8; ++i)
	{
		mass = new int[massSize[i]];
		std::cout << "Size mass: " << massSize[i] << std::endl;
		for (int j = 0; j < massSize[i]; ++j)
		{
			mass[j] = rand() % 1000000 + 1;
		}
		mass_begin = mass;
		mass_end = mass_begin + massSize[i];
		ReducerMinTest(mass, massSize[i]);
		ReducerMaxTest(mass, massSize[i]);

		ParallelSortWithTime(mass_begin, mass_end);
		std::cout << "Sorting is done." << std::endl; 
		
		ReducerMinTest(mass, massSize[i]);
		ReducerMaxTest(mass, massSize[i]);
		std::cout << std::endl;
		delete[] mass;
	}

	system("pause");
	return 0;
}