#ifndef UTIL_HPP
#define UTIL_HPP



#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <omp.h>

__inline__
int get_num_omp_threads(){
	int num_threads = 1;
	//get available number of threads
	#pragma omp parallel
	{
		#pragma omp master
		num_threads = omp_get_num_threads();
	}	
	return num_threads;
}


template<int index, int ndims, typename = void>
struct IncreasingEnumerator{

    template<class Func, class Coords>
    void enumerate(const Coords& bounds, Coords& coords, Func handle_coords){
        for(int i = 0; i < int(bounds[index]); i++){
            coords[index] = i;
			IncreasingEnumerator<index+1, ndims>{}.enumerate(bounds, coords, handle_coords);
        }
    }
};


template<int index, int ndims>
struct IncreasingEnumerator<index, ndims, typename std::enable_if<index == ndims-1>::type>{

    template<class Func, class Coords>
    void enumerate(const Coords& bounds, Coords& coords, Func handle_coords){
        for(int i = 0; i < int(bounds[index]); i++){
            coords[index] = i;
            handle_coords(coords);
        }
    }
};

//enumerate all points with integer coordinates in a grid with dimensions given in bounds
//each valid point is consumed by function handle_coords
template<int ndims, class Func, class Container>
void enumerate_coords_increasing(const Container& bounds, Func handle_coords){
    Container coords;
    std::fill(coords.begin(), coords.end(), 0);

    IncreasingEnumerator<0, ndims>{}.enumerate(bounds, coords, handle_coords);
}


template<int index, int ndims, typename = void>
struct IncreasingManhattanEnumerator{

    template<class Func, class Coords, class Prefix>
    void enumerate(const Coords& dimensions, const Prefix& prefixdiff, int manhattandistance, Coords& coords, Func handle_coords){
        const int maxLoopIterInclusive = std::min(int(dimensions[index]-1), manhattandistance);

        for(int i = 0; i <= maxLoopIterInclusive; i++){
            coords[index] = i;
            if(prefixdiff[index] >= manhattandistance - i){
                IncreasingManhattanEnumerator<index+1, ndims>{}.enumerate(dimensions, prefixdiff, manhattandistance - i, coords, handle_coords);
            }
        }
    }
};

template<int index, int ndims>
struct IncreasingManhattanEnumerator<index, ndims, typename std::enable_if<index == ndims-1>::type>{

    template<class Func, class Coords, class Prefix>
    void enumerate(const Coords& dimensions, const Prefix& prefixdiff, int manhattandistance, Coords& coords, Func handle_coords){
        if(manhattandistance < dimensions[index]){
            coords[index] = manhattandistance;
            handle_coords(coords);
        }
    }
};

//enumerate all points with integer coordinates with a manhatten distance of manhattendistance to origin in a grid with dimensions given in bounds
//each valid point is consumed by function handle_coords
template<int ndims, class Func, class Coords>
void enumerate_manhattan_coords_increasing(const Coords& dimensions, int manhattandistance, Func handle_coords, int num_threads = 1){
    Coords coords;
    std::fill(coords.begin(), coords.end(), 0);

    const int dimsum = std::accumulate(dimensions.begin(), dimensions.end(), 0, std::plus<int>{});
    std::array<int, ndims> prefixdiff;
    prefixdiff[0] = dimsum-dimensions[0];
    for(int i = 1; i < ndims; i++){
        prefixdiff[i] = prefixdiff[i-1] - dimensions[i];
    }

    //IncreasingManhattanEnumerator<0, ndims>{}.enumerate(dimensions, prefixdiff, manhattandistance, coords, handle_coords);
    
    const int maxLoopIterInclusive = std::min(int(dimensions[0]-1), manhattandistance);
	
	int old_num_threads = get_num_omp_threads();
	
	omp_set_num_threads(num_threads);

	#pragma omp parallel for private(coords) schedule(dynamic,1)
	for(int i = 0; i <= maxLoopIterInclusive; i++){
		coords[0] = i;
		if(prefixdiff[0] >= manhattandistance - i){
			IncreasingManhattanEnumerator<0+1, ndims>{}.enumerate(dimensions, prefixdiff, manhattandistance - i, coords, handle_coords);
		}
	}
	
	omp_set_num_threads(old_num_threads);
}


//convert multi-dimensional index to linear index
template<class Container1, class Container2>
std::size_t to_linear_index(const Container1& indices, const Container2& bounds){
	assert(bounds.size() != 0);
	
	std::size_t linear_index = indices.back();
	
	std::size_t factor = 1;
	
	for(int i = indices.size() - 2; i >= 0; i--){
		factor *= bounds[i+1];
		linear_index += factor * indices[i];
	}
	
	return linear_index;
}

//convert linear index to multi-dimensional index
template<class Array, class Container2>
Array to_multidimensional_index(std::size_t linear_index, const Container2& bounds){
	assert(bounds.size() != 0);
	
	std::size_t product = std::accumulate(bounds.begin()+1, bounds.end(), std::size_t(1), std::multiplies<std::size_t>{});
	
	Array result;
	for(std::size_t i = 0; i < bounds.size()-1; i++){
		result[i] = linear_index / product;
		linear_index = linear_index - result[i] * product;
		product = product / bounds[i+1];
	}
	result.back() = linear_index;
	
	return result;
}



//multiply two polynoms whose coefficients are given in vectors a and b
__inline__
std::vector<std::size_t> polynom_mult(const std::vector<std::size_t>& a, const std::vector<std::size_t>& b){
	std::vector<std::size_t> result(a.size() + b.size() - 1, 0);
	
	for(std::size_t i = 0; i < a.size(); i++){
		for(std::size_t j = 0; j < b.size(); j++){
			result[i+j] += a[i]*b[j];
		}
	}
	
	return result;
}

//for a grid of certain dimension, for each possible manhattan distance to origin, calculate number of elements with the same manhatten distance
template<int n, class T>
std::vector<std::size_t> get_numbers_of_coords_with_distance(const T* dimensions){
	//see https://math.stackexchange.com/questions/877236/how-to-calculate-the-number-of-integer-solution-of-a-linear-equation-with-constr 
	std::vector<std::size_t> a;
	std::vector<std::size_t> b;
	std::vector<std::size_t> c;
		
	c.resize(int(dimensions[0]));
	std::fill(c.begin(), c.end(), 1);	
	
	for(int dim = 1; dim < n; dim++){
		
		std::swap(b,c);
		a.clear();
		a.resize(dimensions[dim],1);
		
		c = polynom_mult(a,b);
	}
	
	return c;
}


template<class Container1, class Container2>
bool container_less_than_elementwise(const Container1& l, const Container2& r){
	auto pair = std::mismatch(l.begin(), l.end(), r.begin());
	if(pair.first == l.end() && pair.second == r.end()) 
		return false;
	return *(pair.first) < *(pair.second);
}



#endif
