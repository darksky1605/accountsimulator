#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "ogame.hpp"

#include <vector>
#include <array>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <map>
#include <numeric>
#include <functional>
#include <atomic>
#include <mutex>
#include <cstring>

#include <omp.h>

namespace ogamesolver{
	
/*template<int index, int ndims>
struct IncreasingEnumerator{

    template<class Func, class Coords>
    void enumerate(const Coords& bounds, Coords& coords, Func handle_coords){
        for(int i = 0; i < int(bounds[index]); i++){
            coords[index] = i;
			IncreasingEnumerator<index+1, ndims>{}.enumerate(bounds, coords, handle_coords);
        }
    }
};

template<int i>
struct minusone{
	static constexpr int value = i-1;
};

template<int ndims>
struct IncreasingEnumerator<minusone<ndims>::value, ndims>{
    static constexpr int index = ndims-1;

    template<class Func, class Coords>>
    void enumerate(const Coords& bounds, Coords& coords, Func handle_coords){
        for(int i = 0; i < int(bounds[index]); i++){
            coords[index] = i;
            handle_coords(coords);
        }
    }
};
*/

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

template<int ndims, class Func, class Coords>
void enumerate_manhattan_coords_increasing(const Coords& dimensions, int manhattandistance, Func handle_coords){
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

#pragma omp parallel for private(coords) schedule(dynamic,1)
	for(int i = 0; i <= maxLoopIterInclusive; i++){
		coords[0] = i;
		if(prefixdiff[0] >= manhattandistance - i){
			IncreasingManhattanEnumerator<0+1, ndims>{}.enumerate(dimensions, prefixdiff, manhattandistance - i, coords, handle_coords);
		}
	}
}


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


std::vector<std::size_t> polynom_mult(const std::vector<std::size_t>& a, const std::vector<std::size_t>& b){
	std::vector<std::size_t> result(a.size() + b.size() - 1, 0);
	
	for(std::size_t i = 0; i < a.size(); i++){
		for(std::size_t j = 0; j < b.size(); j++){
			result[i+j] += a[i]*b[j];
		}
	}
	
	return result;
}

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


void print_time(std::ostream& os, float daysfloat){
	std::uint64_t seconds = daysfloat * 24ULL * 60ULL * 60ULL;
	std::uint64_t days = seconds / (24ULL*60ULL*60ULL);
	seconds = seconds % (24ULL*60ULL*60ULL);
	std::uint64_t hours = seconds / (60ULL*60ULL);
	seconds = seconds % (60ULL * 60ULL);
	std::uint64_t minutes = seconds / 60ULL;
	seconds = seconds % 60ULL;
	os << days << "d " << hours << "h " << minutes << "m " << seconds << "s";
}

template<class Container1, class Container2>
bool container_less_than_elementwise(const Container1& l, const Container2& r){
	auto pair = std::mismatch(l.begin(), l.end(), r.begin());
	if(pair.first == l.end() && pair.second == r.end()) 
		return false;
	return *(pair.first) < *(pair.second);
}
		
		


struct SinglePlanetSolver{
	
	static constexpr int index_metmine = 0;
	static constexpr int index_crystalmine = 1;
	static constexpr int index_deutsynth = 2;
	static constexpr int index_solarplant = 3;
	static constexpr int index_fusionplant = 4;	
	static constexpr int index_energytech = 5;
	static constexpr int index_plasmatech = 6;
	static constexpr int dimensions = 7;
	
	using Coords = std::array<int, dimensions>;
	
	struct State{
		const State* prev = nullptr;
		float time = std::numeric_limits<float>::max();
	};
	
	struct PathNode{
		Coords coords;
		float time;
		
		PathNode() : PathNode({}, 0.0f){}
		PathNode(const Coords& c, float t) : coords(c), time(t){}
	};
	
	
	
	
	
	void solve(int metminelvl, int crystalminelvl, int deutsynthlvl, int solarplantlvl, int fusionplantlvl, int energytechlvl, int plasmatechlvl, int temperature, int speedfactor){
		using ogamehelpers::Entity;
		using ogamehelpers::EntityInfo;
		using ogamehelpers::EntityMap;
		using ogamehelpers::Resources;
		using ogamehelpers::Production;
		using ogamehelpers::ItemRarity;
		
		constexpr bool geologist = false;
		constexpr bool staff = false;
		constexpr bool engineer = false;
		
		Coords bounds;
		
		bounds[index_metmine] = metminelvl+1;
		bounds[index_crystalmine] = crystalminelvl+1;
		bounds[index_deutsynth] = deutsynthlvl+1;
		bounds[index_fusionplant] = fusionplantlvl+1;
		bounds[index_solarplant] = solarplantlvl+1;
		bounds[index_energytech] = energytechlvl+1;
		bounds[index_plasmatech] = plasmatechlvl+1;
		
		
		/*bounds[index_metmine] = 20;
		bounds[index_crystalmine] = 20;
		bounds[index_deutsynth] = 20;
		bounds[index_fusionplant] = 10;
		bounds[index_solarplant] = 20;
		bounds[index_energytech] = 10;
		bounds[index_plasmatech] = 10;*/
		
		EntityMap tmpEntityMap;
		
		std::array<EntityInfo, dimensions> entityInfoMap;
		
		entityInfoMap[index_metmine] = tmpEntityMap.getInfo(Entity::Metalmine);
		entityInfoMap[index_crystalmine] = tmpEntityMap.getInfo(Entity::Crystalmine);
		entityInfoMap[index_deutsynth] = tmpEntityMap.getInfo(Entity::Deutsynth);
		entityInfoMap[index_fusionplant] = tmpEntityMap.getInfo(Entity::Fusion);
		entityInfoMap[index_solarplant] = tmpEntityMap.getInfo(Entity::Solar);
		entityInfoMap[index_energytech] = tmpEntityMap.getInfo(Entity::Energy);
		entityInfoMap[index_plasmatech] = tmpEntityMap.getInfo(Entity::Plasma);
		
		std::size_t numcells = 1;
		for(const auto& i : bounds)
			numcells *= i;
		
		std::vector<State> dpmatrix(numcells);
		
		dpmatrix[0].time = 0;
		
		auto print_coords = [&](const Coords& coords){
            std::cout << "[";
            for(const auto i : coords){
                std::cout  << int(i) << ',';
            }
            std::cout << "]";
        };

		std::size_t calculated_cells = 0;
		auto handle_coords = [&](const Coords& coords){
			calculated_cells++;
			//std::cout << "Progress: " << float(calculated_cells) / numcells * 100 << "%\r";
			//std::cout.flush();
			if(coords == Coords{4,4,0,1,0,0,0}){
					calculated_cells += 0;
			}
			
			
			int mycoordinatesum = std::accumulate(coords.begin(), coords.end(), 0, std::plus<Coords::value_type>{});
			std::size_t myindex = to_linear_index(coords, bounds);
			
			//check upgrade by one level for each dimension
			for(int dim = 0; dim < dimensions; dim++){
				if(coords[dim] > 0){
					auto neighborcoords = coords;
					neighborcoords[dim]--;
					
					int neighborcoordinatesum = std::accumulate(neighborcoords.begin(), neighborcoords.end(), 0, std::plus<Coords::value_type>{});
					
					//get cost of upgrade
					Resources buildcosts = ogamehelpers::getBuildCosts(entityInfoMap[dim], coords[dim]);
					
					Production dailyProduction = ogamehelpers::getDailyProduction(neighborcoords[index_metmine], ItemRarity::None, 100, 
																						neighborcoords[index_crystalmine], ItemRarity::None, 100, 
																						neighborcoords[index_deutsynth], ItemRarity::None, 100, 
																						neighborcoords[index_solarplant], 100,
																						neighborcoords[index_fusionplant], 100, neighborcoords[index_energytech], 
																						temperature, 0, 100,
																						neighborcoords[index_plasmatech], speedfactor, 
																						engineer, geologist, staff);
					
					/*float saveDays = ogamehelpers::get_save_duration_notrade(0,0,0, 
															   buildcosts.met, buildcosts.crystal, buildcosts.deut, 
															   dailyProduction.met, dailyProduction.crystal, dailyProduction.deut);
					*/
					
					float saveDays = ogamehelpers::get_save_duration_symmetrictrade(0,0,0, 
															   buildcosts.met, buildcosts.crystal, buildcosts.deut, 
															   dailyProduction.met, dailyProduction.crystal, dailyProduction.deut,
															   {3,2,1});
					
					
					const std::size_t neighborindex = to_linear_index(neighborcoords, bounds);					
					const State& neighborstate = dpmatrix[neighborindex];
					const float newtime = saveDays + neighborstate.time;
					
					if(dpmatrix[myindex].time > newtime){
						dpmatrix[myindex].time = newtime;
						dpmatrix[myindex].prev = &neighborstate;
					}					
				}else{
					//not a valid neighbor
				}
			}
		};
		
		enumerate_coords_increasing<dimensions>(bounds, handle_coords);
		
		Coords resultcoords = bounds;
		/*float resulttime = std::numeric_limits<float>::max();
		enumerate_manhattan_coords_increasing<dimensions>(bounds, 33, [&](const auto& currentcoords){
			std::size_t currentindex = to_linear_index(currentcoords, bounds);
			if(dpmatrix[currentindex].time < resulttime){
				resulttime = dpmatrix[currentindex].time;
				resultcoords = currentcoords;
			}
		});*/
		
		for(auto& i : resultcoords) i = std::max(0, i-1);
		
		//backtrack to origin to find step-by-step list
		std::vector<PathNode> path;
		Coords currentcoords = resultcoords;
		std::size_t currentindex = to_linear_index(currentcoords, bounds);
		const State* currentstateptr = &dpmatrix[currentindex];
		path.emplace_back(currentcoords, currentstateptr->time);
		while(currentstateptr->prev != nullptr){
			currentstateptr = currentstateptr->prev;
			currentindex = currentstateptr - dpmatrix.data();
			
			currentcoords = to_multidimensional_index<Coords>(currentindex, bounds);
			
			path.emplace_back(currentcoords, currentstateptr->time);
		}	
		
		std::reverse(path.begin(), path.end());
		
		std::for_each(path.begin(), path.end(), [](const auto& node){
            std::cout << "[";
            for(const auto i : node.coords){
                std::cout  << i << ',';
            }
            std::cout << "] ";
			float mineproductionfactor = ogamehelpers::getMineProductionFactor(int(node.coords[index_metmine]), 100,
																					int(node.coords[index_crystalmine]), 100,
																					int(node.coords[index_deutsynth]), 100,
																					int(node.coords[index_solarplant]), 100,
																					int(node.coords[index_fusionplant]), 100,
																					int(node.coords[index_energytech]), 
																					0,0,0,
																					engineer, staff);
			std::cout << mineproductionfactor << " ";
			print_time(std::cout, node.time);
			std::cout << '\n';
        });

		
		const std::size_t resultindex = to_linear_index(resultcoords, bounds);
		
		
		
		std::cout << "Time: " << dpmatrix[resultindex].time << std::endl;
	}
	
};






















struct SinglePlanetSolver2{
	
	static constexpr int8_t index_metmine = 0;
	static constexpr int8_t index_crystalmine = 1;
	static constexpr int8_t index_deutsynth = 2;
	static constexpr int8_t index_solarplant = 3;
	static constexpr int8_t index_fusionplant = 4;	
	static constexpr int8_t index_energytech = 5;
	static constexpr int8_t index_plasmatech = 6;

	static constexpr int8_t dimensions = 7;
	static constexpr int8_t noprev = 128;
	
	static constexpr float savetime_hidden = std::numeric_limits<float>::max() - 1;
	
	using Coords = std::array<int8_t, dimensions>;
	
	struct State{
		ogamehelpers::Resources ress;
		ogamehelpers::Production production;
		Coords coords;
		int8_t prev = noprev;
		
		//times are given in days
		float time = std::numeric_limits<float>::max();
		float constructiontime = 0; //time stores previous time + this.savetime + this.constructiontime. however, the savetime of next cell may overlap with construction time of this cell
		float totalConstructionTime = 0;
		float savetime = 0;
	};
	
	struct PathNode{
		Coords coords;
		/*float time;
		float constructiontime;
		float totalConstructionTime;
		
		PathNode() : PathNode({}, 0.0f, 0.0f, 0.0f){}
		PathNode(const Coords& c, float t, float t2, float t3) : coords(c), time(t), constructiontime(t2), totalConstructionTime(t3){}*/
		
		const State* state;
		PathNode() : PathNode({}, nullptr){}
		PathNode(const Coords& c, const State* s) : coords(c), state(s){}
	};
	

	
	
	void solve(int metminelvl, int crystalminelvl, int deutsynthlvl, int solarplantlvl, int fusionplantlvl, int energytechlvl, int plasmatechlvl, int temperature, int speedfactor){
		using ogamehelpers::Entity;
		using ogamehelpers::EntityInfo;
		using ogamehelpers::EntityMap;
		using ogamehelpers::Resources;
		using ogamehelpers::Production;
		using ogamehelpers::ItemRarity;
		
		constexpr bool geologist = false;
		constexpr bool staff = false;
		constexpr bool engineer = false;
		constexpr int roboLevel = 0;
		constexpr int naniLevel = 0;
		constexpr int shipyardLevel = 0;
		constexpr int flabLevel = 10;
		
		Coords bounds;
		
		/*bounds[index_metmine] = metminelvl+1;
		bounds[index_crystalmine] = crystalminelvl+1;
		bounds[index_deutsynth] = deutsynthlvl+1;
		bounds[index_fusionplant] = fusionplantlvl+1;
		bounds[index_solarplant] = solarplantlvl+1;
		bounds[index_energytech] = energytechlvl+1;
		bounds[index_plasmatech] = plasmatechlvl+1;*/
		
		
		bounds[index_metmine] = 100;
		bounds[index_crystalmine] = 100;
		bounds[index_deutsynth] = 100;
		bounds[index_fusionplant] = 1;
		bounds[index_solarplant] = 80;
		bounds[index_energytech] = 1;
		bounds[index_plasmatech] = 3;
		
		const std::vector<std::size_t> number_of_elements_at_diagonal = get_numbers_of_coords_with_distance<dimensions>(bounds.data());
		int max_diag = 0;
		std::size_t total_elements = 0;
		const std::size_t max_bytes = 8ULL * (1ULL << 30);
		
		for(int i = 0; i < int(number_of_elements_at_diagonal.size()); i++){
			std::size_t new_elements = total_elements + number_of_elements_at_diagonal[i];
			
			if(sizeof(State) * new_elements < max_bytes){
				max_diag = i;
				total_elements += number_of_elements_at_diagonal[i];
			}
		}
		
		auto largest_diag_iter = std::max_element(number_of_elements_at_diagonal.begin(), number_of_elements_at_diagonal.end());
		int largest_diag = int(std::distance(number_of_elements_at_diagonal.begin(), largest_diag_iter));
		
		std::cout << "max_diag: " << max_diag << ", total_elements: " << total_elements << std::endl;		
		std::cout << "largest_diag: " << largest_diag << std::endl;
		
		
		EntityMap tmpEntityMap;
		
		std::array<EntityInfo, dimensions> entityInfoMap;
		
		entityInfoMap[index_metmine] = tmpEntityMap.getInfo(Entity::Metalmine);
		entityInfoMap[index_crystalmine] = tmpEntityMap.getInfo(Entity::Crystalmine);
		entityInfoMap[index_deutsynth] = tmpEntityMap.getInfo(Entity::Deutsynth);
		entityInfoMap[index_fusionplant] = tmpEntityMap.getInfo(Entity::Fusion);
		entityInfoMap[index_solarplant] = tmpEntityMap.getInfo(Entity::Solar);
		entityInfoMap[index_energytech] = tmpEntityMap.getInfo(Entity::Energy);
		entityInfoMap[index_plasmatech] = tmpEntityMap.getInfo(Entity::Plasma);
		
		std::size_t numcells = 1;
		for(const auto& i : bounds)
			numcells *= i;
		
		std::vector<State> dpmatrix(numcells);
		
		//initialize first cell 
		dpmatrix[0].time = 0;
		dpmatrix[0].ress.met = 500;
		dpmatrix[0].ress.crystal = 500;
		dpmatrix[0].ress.deut = 0;

		auto print_coords = [&](const Coords& coords){
            std::cout << "[";
            for(const auto i : coords){
                std::cout  << int(i) << ',';
            }
            std::cout << "]";
        };
		
		auto compare_states_by_time = [](const auto& state1, const Coords& coords1, const auto& state2, const Coords& coords2){
				return state1.time < state2.time;
			};
			
		auto compare_states_by_production_per_time = [&](const auto& state1, const Coords& coords1, const auto& state2, const Coords& coords2){				
			
			float dseproduction1 = float(state1.production.met) / 3 + float(state1.production.crystal) / 2 + float(state1.production.deut); 
			float dseproduction2 = float(state2.production.met) / 3 + float(state2.production.crystal) / 2 + float(state2.production.deut);
			
			return (dseproduction1 / state1.time) > (dseproduction2 / state2.time);
		};
		
		auto compare_states_by_production = [&](const auto& state1, const Coords& coords1, const auto& state2, const Coords& coords2){				
			
			float dseproduction1 = float(state1.production.met) / 3 + float(state1.production.crystal) / 2 + float(state1.production.deut); 
			float dseproduction2 = float(state2.production.met) / 3 + float(state2.production.crystal) / 2 + float(state2.production.deut);
			
			return (dseproduction1) > (dseproduction2);
		};
		
		auto compare_states_by_production_per_costs = [&](const auto& state1, const Coords& coords1, const auto& state2, const Coords& coords2){				
			
			float dseproduction1 = float(state1.production.met) / 3 + float(state1.production.crystal) / 2 + float(state1.production.deut); 
			float dseproduction2 = float(state2.production.met) / 3 + float(state2.production.crystal) / 2 + float(state2.production.deut);
			
			Resources costs1;
			Resources costs2;
			
			for(int dim = 0; dim < dimensions; dim++){
					Resources totalinvestedcosts1 = getTotalCosts(entityInfoMap[dim], coords1[dim]);
					Resources totalinvestedcosts2 = getTotalCosts(entityInfoMap[dim], coords2[dim]);
					costs1.met += totalinvestedcosts1.met;
					costs1.crystal += totalinvestedcosts1.crystal;
					costs1.deut += totalinvestedcosts1.deut;
					costs2.met += totalinvestedcosts2.met;
					costs2.crystal += totalinvestedcosts2.crystal;
					costs2.deut += totalinvestedcosts2.deut;
			}
			
			float dsecosts1 = float(costs1.met) / 3 + float(costs1.crystal) / 2 + float(costs1.deut); 
			float dsecosts2 = float(costs2.met) / 3 + float(costs2.crystal) / 2 + float(costs2.deut);
			
			return (dseproduction1 / dsecosts1) > (dseproduction2 / dsecosts2);
		};
		
		auto compare_states = compare_states_by_production_per_costs; //compare_states_by_time;
		
		auto is_invalid_float = [](float f){
			return f == std::numeric_limits<float>::max() || f < 0;
		};

		std::size_t calculated_cells = 0;
		std::atomic<int> atLeastOneCellRelaxed{0};

		auto handle_coords = [&](const Coords& coords){
			
			
			
			
			calculated_cells++;
			//std::cout << "Progress: " << float(calculated_cells) / numcells * 100 << "%\r";
			//std::cout.flush();
			//if(coords == Coords{4,4,0,1,0,0,0}){
			//		calculated_cells += 0;
			//}
			
			//find the correponding dp state to coords
			//int mycoordinatesum = std::accumulate(coords.begin(), coords.end(), 0, std::plus<Coords::value_type>{});
			std::size_t myindex = to_linear_index(coords, bounds);
			State& mystate = dpmatrix[myindex];
			
			Production myDailyProduction = ogamehelpers::getDailyProduction(coords[index_metmine], ItemRarity::None, 100, 
																						coords[index_crystalmine], ItemRarity::None, 100, 
																						coords[index_deutsynth], ItemRarity::None, 100, 
																						coords[index_solarplant], 100,
																						coords[index_fusionplant], 100, coords[index_energytech], 
																						temperature, 0, 100,
																						coords[index_plasmatech], speedfactor, 
																						engineer, geologist, staff);
			mystate.production = myDailyProduction;
			
			//check upgrade by one level for each dimension
			bool isrelaxed = false;
			for(int dim = 0; dim < dimensions; dim++){
				//if coords[dim] == 0, this dimension does not have a neighbor. skip it
				if(coords[dim] > 0 && coords[dim] < 50){
					//find neighbor information
					Coords neighborcoords = coords;
					neighborcoords[dim]--;
					
					//int neighborcoordinatesum = std::accumulate(neighborcoords.begin(), neighborcoords.end(), 0, std::plus<Coords::value_type>{});
					const std::size_t neighborindex = to_linear_index(neighborcoords, bounds);					
					const State& neighborstate = dpmatrix[neighborindex];
					
					Coords neighborsneighborcoords;
					std::fill(neighborsneighborcoords.begin(), neighborsneighborcoords.end(), 0);
					std::size_t neighborsneighborindex = 0;
					if(neighborstate.prev != noprev){
						neighborsneighborcoords = neighborcoords;
						neighborsneighborcoords[neighborstate.prev]--;
						neighborsneighborindex = to_linear_index(neighborsneighborcoords, bounds);
					}
					
					const State& neighborsneighborstate = dpmatrix[neighborsneighborindex];
					
					
					//get cost of upgrade
					Resources buildcosts = ogamehelpers::getBuildCosts(entityInfoMap[dim], coords[dim]);
					//get construction time of upgrade
					float constructionDays = ogamehelpers::getConstructionTimeInDays(entityInfoMap[dim], coords[dim], roboLevel, naniLevel, shipyardLevel, flabLevel, speedfactor);
					
					if(is_invalid_float(constructionDays))
						continue;
					
					//0. Calculate save time with the neighborsneighbor's production.
					//1. if this is less than the neighbor's construction time, then the new time is neighborstime + myconstructiontime
					//2. if it is not less than the neighbor's construction time, we have to take into account two different saving steps.
					//	-- 2a) neighborsneighbor's production for the time it takes neighbor to finish construction
					//	-- 2b) neighbor's production until buildcosts are available.
					
					
					//0.
										
					Production dailyNeighborsNeighborProduction = neighborsneighborstate.production;
					
					// we need neighbor's ress here since that's the state whose ressources we use.
					float neighborsNeighborSaveDays = ogamehelpers::get_save_duration_symmetrictrade(neighborstate.ress.met, neighborstate.ress.crystal, neighborstate.ress.deut, 
																									buildcosts.met, buildcosts.crystal, buildcosts.deut, 
																									dailyNeighborsNeighborProduction.met, dailyNeighborsNeighborProduction.crystal, dailyNeighborsNeighborProduction.deut,
																									{3,2,1});
					
					if(is_invalid_float(neighborsNeighborSaveDays))
						continue;
					
					if(neighborsNeighborSaveDays <= neighborstate.constructiontime){
						//1.
						
						State newstate = mystate;
						newstate.time = constructionDays + neighborstate.time;
						newstate.constructiontime = constructionDays;
						newstate.totalConstructionTime = neighborstate.totalConstructionTime + constructionDays;
						newstate.savetime = savetime_hidden;
						//newstate.dseproduction = 
						
						if(is_invalid_float(newstate.time))
							continue;
						
						//relax
						if(!isrelaxed || compare_states(newstate, coords, mystate, coords)){
							mystate = newstate;
							mystate.prev = dim;
							
							if(neighborsNeighborSaveDays == 0){
								std::int64_t diffm = neighborstate.ress.met - buildcosts.met;
								std::int64_t diffk = neighborstate.ress.crystal + (2*diffm)/3 - buildcosts.crystal;
								std::int64_t diffd = neighborstate.ress.deut + (1*diffk)/2 - buildcosts.deut;
								
								mystate.ress.met = 0;
								mystate.ress.crystal = 0;
								mystate.ress.deut = std::max(std::int64_t(0), diffd);
								
								
								/*mystate.ress.met = neighborstate.ress.met - buildcosts.met;
								mystate.ress.crystal = neighborstate.ress.crystal - buildcosts.crystal;
								mystate.ress.deut = neighborstate.ress.deut - buildcosts.deut;*/
							}else{
								mystate.ress.met = 0;
								mystate.ress.crystal = 0;
								mystate.ress.deut = 0;
							}
							isrelaxed = true;
						}

					}else{
						//2a) neighborsneighbor's production for the time it takes neighbor to finish construction
						
						Resources produced; //the neighborsneighbor produced resources
						produced.met = dailyNeighborsNeighborProduction.met * neighborstate.constructiontime;
						produced.crystal = dailyNeighborsNeighborProduction.crystal * neighborstate.constructiontime;
						produced.deut = dailyNeighborsNeighborProduction.deut * neighborstate.constructiontime;
						
						//2b) neighbor's production until buildcosts are available
						
						
						Production dailyNeighborProduction = neighborstate.production;
						
						
						float neighborSaveDays = ogamehelpers::get_save_duration_symmetrictrade(neighborstate.ress.met + produced.met, neighborstate.ress.crystal + produced.crystal, neighborstate.ress.deut + produced.deut, 
																									buildcosts.met, buildcosts.crystal, buildcosts.deut, 
																									dailyNeighborProduction.met, dailyNeighborProduction.crystal, dailyNeighborProduction.deut,
																									{3,2,1});
						
						if(is_invalid_float(neighborSaveDays))
							continue;
						
						
						State newstate = mystate;
						newstate.time = neighborsNeighborSaveDays + neighborSaveDays + constructionDays + neighborstate.time;
						
						newstate.constructiontime = constructionDays;
						newstate.totalConstructionTime = neighborstate.totalConstructionTime + constructionDays;
						newstate.savetime = neighborsNeighborSaveDays + neighborSaveDays;
						
						if(is_invalid_float(newstate.time))
							continue;
						
						//relax
						if(!isrelaxed || compare_states(newstate, coords, mystate, coords)){
							mystate = newstate;
							mystate.prev = dim;
							
							/*if(neighborsNeighborSaveDays == 0){
								std::int64_t diffm = neighborstate.ress.met - buildcosts.met;
								std::int64_t diffk = neighborstate.ress.crystal - (2*diffm)/3 - buildcosts.crystal;
								std::int64_t diffd = neighborstate.ress.deut - (1*diffk)/2 - buildcosts.deut;
								
								mystate.ress.met = std::max(std::int64_t(0), diffm);
								mystate.ress.crystal = std::max(std::int64_t(0), diffk);
								mystate.ress.deut = std::max(std::int64_t(0), diffd);
								*/
								/*mystate.ress.met = neighborstate.ress.met - buildcosts.met;
								mystate.ress.crystal = neighborstate.ress.crystal - buildcosts.crystal;
								mystate.ress.deut = neighborstate.ress.deut - buildcosts.deut;*/
							/*}else{
								mystate.ress.met = 0;
								mystate.ress.crystal = 0;
								mystate.ress.deut = 0;
							}*/
							mystate.ress.met = 0;
							mystate.ress.crystal = 0;
							mystate.ress.deut = 0;
							isrelaxed = true;
						}
					}
				
				}else{
					//not a valid neighbor
				}
			}
			if(isrelaxed)
				atLeastOneCellRelaxed = 1;
		};
		
		int largest_relaxed_diag = 0;
		for(int diag = 0; diag <= 150/*largest_diag*/; diag++){
			std::cout << "diag: " << diag << std::endl;
			atLeastOneCellRelaxed = 0;
			TIMERSTARTCPU(manhattandiag);
			enumerate_manhattan_coords_increasing<dimensions>(bounds, diag, handle_coords);
			TIMERSTOPCPU(manhattandiag);

			/*if(diag == 0 || atLeastOneCellRelaxed.load()){
				largest_relaxed_diag = diag;
			}else{
				break;
			}*/
		}
		
		largest_relaxed_diag = 150;
				
		/*Coords resultcoords = bounds;
		float resulttime = std::numeric_limits<float>::max();
		enumerate_manhattan_coords_increasing<dimensions>(bounds, largest_diag, [&](const auto& currentcoords){
			std::size_t currentindex = to_linear_index(currentcoords, bounds);
			if(dpmatrix[currentindex].time < resulttime){
				resulttime = dpmatrix[currentindex].time;
				resultcoords = currentcoords;
			}
		});*/
		
		Coords resultcoords;
		std::fill(resultcoords.begin(), resultcoords.end(), 0);
		std::size_t resultindex = 0;
		float ratio = 0;
		bool found = false;
		std::mutex m;
		enumerate_manhattan_coords_increasing<dimensions>(bounds, largest_relaxed_diag /*largest_diag*/, [&](const auto& currentcoords){
			std::lock_guard<std::mutex> lg(m);
			std::size_t currentindex = to_linear_index(currentcoords, bounds);
			
			/*Resources dailyProduction = ogamehelpers::getDailyProduction(currentcoords[index_metmine], ItemRarity::None, 100, 
																				currentcoords[index_crystalmine], ItemRarity::None, 100, 
																				currentcoords[index_deutsynth], ItemRarity::None, 100, 
																				currentcoords[index_solarplant], currentcoords[index_fusionplant], currentcoords[index_energytech],
																				temperature, currentcoords[index_plasmatech], speedfactor, 
																				engineer, geologist, staff);
			
			float dseproduction = float(dailyProduction.met) / 3 + float(dailyProduction.crystal) / 2 + float(dailyProduction.deut);
			
			float currentratio = dseproduction / dpmatrix[currentindex].time;
			print_coords(currentcoords);
			std::cout << " : " << currentratio << ", " << dailyProduction.met << " " << dailyProduction.crystal << " " << dailyProduction.deut << " " << dpmatrix[currentindex].time << std::endl;*/
			
			/*Resources costs1;
			
			for(int dim = 0; dim < dimensions; dim++){
					Resources totalinvestedcosts1 = getTotalCosts(entityInfoMap[dim], currentcoords[dim]);
					costs1.met += totalinvestedcosts1.met;
					costs1.crystal += totalinvestedcosts1.crystal;
					costs1.deut += totalinvestedcosts1.deut;
			}
			
			float dsecosts1 = float(costs1.met) / 3 + float(costs1.crystal) / 2 + float(costs1.deut); 
			
			float dseproduction = float(dpmatrix[currentindex].production.met) / 3 + float(dpmatrix[currentindex].production.crystal) / 2 + float(dpmatrix[currentindex].production.deut);
			
			print_coords(currentcoords);
			std::cout << dseproduction << ", ";
			print_time(std::cout, dpmatrix[currentindex].time);
			std::cout << ", ";
			std::cout << (dseproduction / dsecosts1);*/
			//print_time(std::cout, dpmatrix[currentindex].totalConstructionTime);
			//std::cout << ", ";
			//print_time(std::cout, dpmatrix[currentindex].constructiontime);
			//std::cout << ", ";
			//if(dpmatrix[currentindex].savetime == savetime_hidden)
			//	std::cout << "overlapped savetime ";
			//else 
			//	print_time(std::cout, dpmatrix[currentindex].savetime);
			//std::cout << std::endl;
			
			if(!found || compare_states(dpmatrix[currentindex], currentcoords, dpmatrix[resultindex], resultcoords)){
				resultcoords = currentcoords;
				resultindex = currentindex;
				//ratio = currentratio;
				found = true;
			}
		});
		
		//for(auto& i : resultcoords) i = std::max(0, i-1);
		
		//backtrack to origin to find step-by-step list
		std::vector<PathNode> path;
		Coords currentcoords = resultcoords;
		std::size_t currentindex = to_linear_index(currentcoords, bounds);
		const State* currentstateptr = &dpmatrix[currentindex];
		//path.emplace_back(currentcoords, currentstateptr->time, currentstateptr->constructiontime, currentstateptr->totalConstructionTime);
		path.emplace_back(currentcoords, currentstateptr);
		while(currentstateptr->prev != noprev){

			currentcoords[currentstateptr->prev] -= 1;
			currentindex = to_linear_index(currentcoords, bounds);
			
			currentstateptr = &dpmatrix[currentindex];
			
			//path.emplace_back(currentcoords, currentstateptr->time, currentstateptr->constructiontime, currentstateptr->totalConstructionTime);
			path.emplace_back(currentcoords, currentstateptr);
		}	
		
		std::reverse(path.begin(), path.end());
		
		std::for_each(path.begin(), path.end(), [&](const auto& node){
            print_coords(node.coords);
			
			std::cout << ", m: " << node.state->ress.met;
			std::cout << ", k: " << node.state->ress.crystal;
			std::cout << ", d: " << node.state->ress.deut << " ";
			float mineproductionfactor = ogamehelpers::getMineProductionFactor(int(node.coords[index_metmine]), 100,
																					int(node.coords[index_crystalmine]), 100,
																					int(node.coords[index_deutsynth]), 100,
																					int(node.coords[index_solarplant]), 100,
																					int(node.coords[index_fusionplant]), 100,
																					int(node.coords[index_energytech]), 
																					0,0,0,
																					engineer, staff);
			Resources costs1;
			
			for(int dim = 0; dim < dimensions; dim++){
					Resources totalinvestedcosts1 = getTotalCosts(entityInfoMap[dim], node.coords[dim]);
					costs1.met += totalinvestedcosts1.met;
					costs1.crystal += totalinvestedcosts1.crystal;
					costs1.deut += totalinvestedcosts1.deut;
			}
			
			float dsecosts1 = float(costs1.met) / 3 + float(costs1.crystal) / 2 + float(costs1.deut); 
			
			float dseproduction = float(dpmatrix[currentindex].production.met) / 3 + float(dpmatrix[currentindex].production.crystal) / 2 + float(dpmatrix[currentindex].production.deut);
			
			//std::cout << mineproductionfactor << " ";
			print_time(std::cout, node.state->time);
			std::cout << ", ";
			std::cout <<(dseproduction / dsecosts1);
			/*print_time(std::cout, node.state->totalConstructionTime);
			std::cout << ", ";
			print_time(std::cout, node.state->constructiontime);
			std::cout << ", ";
			if(node.state->savetime == savetime_hidden)
				std::cout << "overlapped savetime ";
			else 
				print_time(std::cout, node.state->savetime);
			
			std::cout << node.state->time;*/
			
			
			std::cout << '\n';
        });

		
		//const std::size_t resultindex = to_linear_index(resultcoords, bounds);
		
		
		
		std::cout << "Time: " << dpmatrix[resultindex].time << std::endl;
	}
	
};















































template<class T, class C, class P>
	struct DPDiagonals{
		using State_t = T;
		using Coords_t = C;
		using Prev_t = P;
		
		struct Wrapper{
			Coords_t* coords = nullptr;
			State_t* state = nullptr;
			Prev_t* prev = nullptr;
			
			bool operator==(const Wrapper& r) const{
				return (0 == std::memcmp(this, &r, sizeof(Wrapper)));
			}
			
			bool operator!=(const Wrapper& r) const{
				return !operator==(r);
			}
		};
		
		Wrapper defaultwrapper;
		
		State_t defaultstate;
		Coords_t defaultcoords;
		Prev_t defaultprev;
		
			
		std::vector<std::vector<Coords_t>> dpcoords;
		std::vector<std::vector<State_t>> dpstates; //each state vector is sorted by coords
		std::vector<std::vector<Prev_t>> dpprevs; //each prev vector is sorted by coords

		mutable std::mutex mutex;
		
		DPDiagonals(){}
		
		DPDiagonals(State_t s, Coords_t c, Prev_t p) : defaultstate(s), defaultcoords(c), defaultprev(p){}
		
		bool is_valid_cell(const Wrapper& cellwrapper) const{
			return cellwrapper != defaultwrapper;
		}
										
		void set_diagonal_coords_and_default_others(int diagonal, std::vector<Coords_t>&& newdiag){
			dpcoords.resize(diagonal + 1);
			dpstates.resize(diagonal + 1);
			dpprevs.resize(diagonal + 1);
			
			dpcoords[diagonal] = std::move(newdiag);
			dpstates[diagonal].clear();
			dpstates[diagonal].resize(dpcoords[diagonal].size(), defaultstate);
			dpprevs[diagonal].clear();
			dpprevs[diagonal].resize(dpcoords[diagonal].size(), defaultprev);
		}
		
		Wrapper operator[](const Coords_t& coords){
			int diagonal = std::accumulate(coords.begin(), coords.end(), 0, std::plus<int>{});
			if(dpcoords.size() < diagonal + 1){
				std::cerr << "operator[]: out of bounds diagonal\n";
				return Wrapper{nullptr, nullptr, nullptr};
			}
			
			auto& coordsvec = dpcoords[diagonal];
			auto& statesvec = dpstates[diagonal];
			auto& prevsvec = dpprevs[diagonal];
			
			auto lb = std::lower_bound(coordsvec.begin(), coordsvec.end(), coords, container_less_than_elementwise<Coords_t, Coords_t>);
			if((lb != coordsvec.end() && *lb == coords)){
				std::size_t distance = std::distance(coordsvec.begin(), lb);
				return Wrapper{&coordsvec[distance], &statesvec[distance], &prevsvec[distance]};
			}else{
				/*std::lock_guard<std::mutex> lg(mutex);
				std::cerr << "operator[]: coords not found\n";
				print_coords(std::cerr, coords);
				std::cerr << "\n";*/
				return Wrapper{nullptr, nullptr, nullptr};
			}
		}
	};



struct SinglePlanetSolver3{
	
	static constexpr int8_t index_metmine = 0;
	static constexpr int8_t index_crystalmine = 1;
	static constexpr int8_t index_deutsynth = 2;
	static constexpr int8_t index_solarplant = 3;
	static constexpr int8_t index_fusionplant = 4;	
	static constexpr int8_t index_energytech = 5;
	static constexpr int8_t index_plasmatech = 6;
	static constexpr int8_t index_lab = 7;

	static constexpr int dimensions = 8;
	static constexpr int8_t noprev = 128;
	static constexpr int8_t illegalprev = -1;
	
	static constexpr float savetime_hidden = std::numeric_limits<float>::max() - 1;
	
	using Coords = std::array<int16_t, dimensions>;
	
	static void print_coords (std::ostream& os, const Coords& coords){
		os << "[";
		for(const auto i : coords){
			os  << int(i) << ',';
		}
		os << "]";
	};
	
	struct State{
		ogamehelpers::Resources ress;
		ogamehelpers::Production production;
		
		//times are given in days
		float time = std::numeric_limits<float>::max();
		float constructiontime = 0; //time stores previous time + this.savetime + this.constructiontime. however, the savetime of next cell may overlap with construction time of this cell
		float totalConstructionTime = 0;
		float savetime = 0;
		
		bool operator==(const State& r) const{
			return (0 == std::memcmp(this, &r, sizeof(State)));
		}
		
		bool operator!=(const State& r) const{
			return !operator==(r);
		}
	};
	
	struct PathNode{
		Coords coords;
		/*float time;
		float constructiontime;
		float totalConstructionTime;
		
		PathNode() : PathNode({}, 0.0f, 0.0f, 0.0f){}
		PathNode(const Coords& c, float t, float t2, float t3) : coords(c), time(t), constructiontime(t2), totalConstructionTime(t3){}*/
		
		const State* state;
		PathNode() : PathNode({}, nullptr){}
		PathNode(const Coords& c, const State* s) : coords(c), state(s){}
	};
	
	
	

	
	
	void solve(int metminelvl, int crystalminelvl, int deutsynthlvl, int solarplantlvl, int fusionplantlvl, int energytechlvl, int plasmatechlvl, int temperature, int speedfactor){
		using ogamehelpers::Entity;
		using ogamehelpers::EntityInfo;
		using ogamehelpers::EntityMap;
		using ogamehelpers::Resources;
		using ogamehelpers::Production;
		using ogamehelpers::ItemRarity;
		
		constexpr bool geologist = false;
		constexpr bool staff = false;
		constexpr bool engineer = false;
		constexpr int roboLevel = 0;
		constexpr int naniLevel = 0;
		constexpr int shipyardLevel = 0;
		
			
		/*const std::vector<std::size_t> number_of_elements_at_diagonal = get_numbers_of_coords_with_distance<dimensions>(bounds.data());
		int max_diag = 0;
		std::size_t total_elements = 0;
		const std::size_t max_bytes = 8ULL * (1ULL << 30);
		
		for(int i = 0; i < int(number_of_elements_at_diagonal.size()); i++){
			std::size_t new_elements = total_elements + number_of_elements_at_diagonal[i];
			
			if(sizeof(State) * new_elements < max_bytes){
				max_diag = i;
				total_elements += number_of_elements_at_diagonal[i];
			}
		}
		
		auto largest_diag_iter = std::max_element(number_of_elements_at_diagonal.begin(), number_of_elements_at_diagonal.end());
		int largest_diag = int(std::distance(number_of_elements_at_diagonal.begin(), largest_diag_iter));
		
		std::cout << "max_diag: " << max_diag << ", total_elements: " << total_elements << std::endl;		
		std::cout << "largest_diag: " << largest_diag << std::endl;*/
		
		
		EntityMap tmpEntityMap;
		
		std::array<EntityInfo, dimensions> entityInfoMap;
		
		entityInfoMap[index_metmine] = tmpEntityMap.getInfo(Entity::Metalmine);
		entityInfoMap[index_crystalmine] = tmpEntityMap.getInfo(Entity::Crystalmine);
		entityInfoMap[index_deutsynth] = tmpEntityMap.getInfo(Entity::Deutsynth);
		entityInfoMap[index_fusionplant] = tmpEntityMap.getInfo(Entity::Fusion);
		entityInfoMap[index_solarplant] = tmpEntityMap.getInfo(Entity::Solar);
		entityInfoMap[index_energytech] = tmpEntityMap.getInfo(Entity::Energy);
		entityInfoMap[index_plasmatech] = tmpEntityMap.getInfo(Entity::Plasma);
		entityInfoMap[index_lab] = tmpEntityMap.getInfo(Entity::Lab);
		
		
		
		DPDiagonals<State, Coords, int8_t> dpmatrix(State{}, Coords{-42}, noprev);
		
		//initialize first cell 
		{
			dpmatrix.set_diagonal_coords_and_default_others(0,{Coords{}});
			auto cell = dpmatrix[Coords{}];
			cell.state->time = 0;
			cell.state->ress.met = 500;
			cell.state->ress.crystal = 500;
			cell.state->ress.deut = 0;
		}		
		
		auto compare_states_by_time = [](const auto& state1, const Coords& coords1, const auto& state2, const Coords& coords2){
				return state1.time < state2.time;
			};
			
		auto compare_states_by_production_per_time = [&](const auto& state1, const Coords& coords1, const auto& state2, const Coords& coords2){				
			
			float dseproduction1 = float(state1.production.met) / 3 + float(state1.production.crystal) / 2 + float(state1.production.deut); 
			float dseproduction2 = float(state2.production.met) / 3 + float(state2.production.crystal) / 2 + float(state2.production.deut);
			
			return (dseproduction1 / state1.time) > (dseproduction2 / state2.time);
		};
		
		auto compare_states_by_production = [&](const auto& state1, const Coords& coords1, const auto& state2, const Coords& coords2){				
			
			float dseproduction1 = float(state1.production.met) / 3 + float(state1.production.crystal) / 2 + float(state1.production.deut); 
			float dseproduction2 = float(state2.production.met) / 3 + float(state2.production.crystal) / 2 + float(state2.production.deut);
			
			return (dseproduction1) > (dseproduction2);
		};
		
		auto compare_states_by_production_per_costs = [&](const auto& state1, const Coords& coords1, const auto& state2, const Coords& coords2){				
			
			float dseproduction1 = float(state1.production.met) / 3 + float(state1.production.crystal) / 2 + float(state1.production.deut); 
			float dseproduction2 = float(state2.production.met) / 3 + float(state2.production.crystal) / 2 + float(state2.production.deut);
			
			Resources costs1;
			Resources costs2;
			
			for(int dim = 0; dim < dimensions; dim++){
					Resources totalinvestedcosts1 = getTotalCosts(entityInfoMap[dim], coords1[dim]);
					Resources totalinvestedcosts2 = getTotalCosts(entityInfoMap[dim], coords2[dim]);
					costs1.met += totalinvestedcosts1.met;
					costs1.crystal += totalinvestedcosts1.crystal;
					costs1.deut += totalinvestedcosts1.deut;
					costs2.met += totalinvestedcosts2.met;
					costs2.crystal += totalinvestedcosts2.crystal;
					costs2.deut += totalinvestedcosts2.deut;
			}
			
			float dsecosts1 = float(costs1.met) / 3 + float(costs1.crystal) / 2 + float(costs1.deut); 
			float dsecosts2 = float(costs2.met) / 3 + float(costs2.crystal) / 2 + float(costs2.deut);
			
			return (dseproduction1 / dsecosts1) > (dseproduction2 / dsecosts2);
		};
		
		auto compare_states = compare_states_by_production;
		
		auto is_invalid_float = [](float f){
			return f == std::numeric_limits<float>::max() || f < 0;
		};

		std::atomic<std::size_t> calculated_cells{0};
		std::atomic<int> atLeastOneCellRelaxed{0};

		auto handle_coords = [&](const Coords& coords){

			calculated_cells++;

			auto mycell = dpmatrix[coords];
			
			Production myDailyProduction = ogamehelpers::getDailyProduction(coords[index_metmine], ItemRarity::None, 100, 
																						coords[index_crystalmine], ItemRarity::None, 100, 
																						coords[index_deutsynth], ItemRarity::None, 100, 
																						coords[index_solarplant], 100,
																						coords[index_fusionplant], 100, coords[index_energytech], 
																						temperature, 0, 100,
																						coords[index_plasmatech], speedfactor, 
																						engineer, geologist, staff);
			
			mycell.state->production = myDailyProduction;
			
			//check upgrade by one level for each dimension
			bool isrelaxed = false;
			for(int dim = 0; dim < dimensions; dim++){
				//if coords[dim] == 0, this dimension does not have a neighbor. skip it
				if(coords[dim] > 0){
					
					//find neighbor information
					Coords neighborcoords = coords;
					neighborcoords[dim]--;
					auto neighborcell = dpmatrix[neighborcoords];
					
					if(!dpmatrix.is_valid_cell(neighborcell)){
						continue;
					}
										
					//find neighbor's neighbor, if any. defaults to coords [0,0,...,0]
					Coords neighborsneighborcoords;
					std::fill(neighborsneighborcoords.begin(), neighborsneighborcoords.end(), 0);

					const int8_t neighborstate_prev = *neighborcell.prev;
					if(neighborstate_prev != noprev){
						neighborsneighborcoords = neighborcoords;
						neighborsneighborcoords[neighborstate_prev]--;
					}
					
					auto neighborsneighborcell = dpmatrix[neighborsneighborcoords];
										
					//get cost of upgrade
					Resources buildcosts = ogamehelpers::getBuildCosts(entityInfoMap[dim], coords[dim]);
					//get construction time of upgrade
					float constructionDays = ogamehelpers::getConstructionTimeInDays(entityInfoMap[dim], coords[dim], roboLevel, naniLevel, shipyardLevel, coords[index_lab], speedfactor);
					
					if(is_invalid_float(constructionDays))
						continue;
					
					//0. Calculate save time with the neighborsneighbor's production.
					//1. if this is less than the neighbor's construction time, then the new time is neighborstime + myconstructiontime
					//2. if it is not less than the neighbor's construction time, we have to take into account two different saving steps.
					//	-- 2a) neighborsneighbor's production for the time it takes neighbor to finish construction
					//	-- 2b) neighbor's production until buildcosts are available.
					
					
					//0.
										
					Production dailyNeighborsNeighborProduction = neighborsneighborcell.state->production;
					
					// we need neighbor's ress here since that's the state whose ressources we use.
					float neighborsNeighborSaveDays = ogamehelpers::get_save_duration_symmetrictrade(neighborcell.state->ress.met, neighborcell.state->ress.crystal, neighborcell.state->ress.deut, 
																									buildcosts.met, buildcosts.crystal, buildcosts.deut, 
																									dailyNeighborsNeighborProduction.met, dailyNeighborsNeighborProduction.crystal, dailyNeighborsNeighborProduction.deut,
																									{3,2,1});
					//handle overflow
					if(is_invalid_float(neighborsNeighborSaveDays))
						continue;
					
					if(neighborsNeighborSaveDays <= neighborcell.state->constructiontime){
						//1.
						
						State newstate = *mycell.state;
						newstate.time = constructionDays + neighborcell.state->time;
						newstate.constructiontime = constructionDays;
						newstate.totalConstructionTime = neighborcell.state->totalConstructionTime + constructionDays;
						newstate.savetime = savetime_hidden;
						//newstate.dseproduction = 
						
						if(is_invalid_float(newstate.time))
							continue;
						
						//relax
						if(!isrelaxed || compare_states(newstate, coords, *mycell.state, coords)){
							*mycell.state = newstate;
							*mycell.prev = dim;
							
							if(neighborsNeighborSaveDays == 0){
								std::int64_t diffm = neighborcell.state->ress.met - buildcosts.met;
								std::int64_t diffk = neighborcell.state->ress.crystal + (2*diffm)/3 - buildcosts.crystal;
								std::int64_t diffd = neighborcell.state->ress.deut + (1*diffk)/2 - buildcosts.deut;
								
								mycell.state->ress.met = 0;
								mycell.state->ress.crystal = 0;
								mycell.state->ress.deut = std::max(std::int64_t(0), diffd);
								
								
							}else{
								mycell.state->ress.met = 0;
								mycell.state->ress.crystal = 0;
								mycell.state->ress.deut = 0;
							}
							isrelaxed = true;
						}

					}else{
						//2a) neighborsneighbor's production for the time it takes neighbor to finish construction
						
						Resources produced; //the neighborsneighbor produced resources
						produced.met = dailyNeighborsNeighborProduction.met * neighborcell.state->constructiontime;
						produced.crystal = dailyNeighborsNeighborProduction.crystal * neighborcell.state->constructiontime;
						produced.deut = dailyNeighborsNeighborProduction.deut * neighborcell.state->constructiontime;
						
						//2b) neighbor's production until buildcosts are available
						
						
						Production dailyNeighborProduction = neighborcell.state->production;
						
						
						float neighborSaveDays = ogamehelpers::get_save_duration_symmetrictrade(neighborcell.state->ress.met + produced.met, neighborcell.state->ress.crystal + produced.crystal, neighborcell.state->ress.deut + produced.deut, 
																									buildcosts.met, buildcosts.crystal, buildcosts.deut, 
																									dailyNeighborProduction.met, dailyNeighborProduction.crystal, dailyNeighborProduction.deut,
																									{3,2,1});
						
						if(is_invalid_float(neighborSaveDays))
							continue;
						
						
						State newstate = *mycell.state;
						newstate.time = neighborsNeighborSaveDays + neighborSaveDays + constructionDays + neighborcell.state->time;
						
						newstate.constructiontime = constructionDays;
						newstate.totalConstructionTime = neighborcell.state->totalConstructionTime + constructionDays;
						newstate.savetime = neighborsNeighborSaveDays + neighborSaveDays;
						
						if(is_invalid_float(newstate.time))
							continue;
						
						//relax
						if(!isrelaxed || compare_states(newstate, coords, *mycell.state, coords)){
							*mycell.state = newstate;
							*mycell.prev = dim;
							
							mycell.state->ress.met = 0;
							mycell.state->ress.crystal = 0;
							mycell.state->ress.deut = 0;
							isrelaxed = true;
						}
					}
				
				}else{
					//not a valid neighbor
				}
			}
			if(isrelaxed)
				atLeastOneCellRelaxed = 1;
		};
		
		
		Coords absoluteConstraints{}; //exclusive upper bounds
		absoluteConstraints[index_metmine] = 43;
		absoluteConstraints[index_crystalmine] = 36;
		absoluteConstraints[index_deutsynth] = 38;
		absoluteConstraints[index_fusionplant] = 23;
		absoluteConstraints[index_solarplant] = 30;
		absoluteConstraints[index_energytech] = 24;
		absoluteConstraints[index_plasmatech] = 1;
		absoluteConstraints[index_lab] = 10;
		
		//functions return true if argument coords should be processed
		std::vector<std::function<bool(Coords)>> additionalConstraints;
		
		additionalConstraints.emplace_back([&](const auto& coords){
			//restrict search space to exclusive upper bounds given by absoluteConstraints
			
			return container_less_than_elementwise<Coords, Coords>(coords, absoluteConstraints);
		});
		
		
		
		additionalConstraints.emplace_back([](const auto& coords){
			//crys must not be greater than met and at most 10 levels smaller
			auto diffmk = (coords[index_metmine] - coords[index_crystalmine]);
			if(diffmk < 0 || diffmk > 10) return false;
			
			//deut must not be greater than met and at most 10 levels smaller
			auto diffmd = (coords[index_metmine] - coords[index_deutsynth]);
			if(diffmd < 0 || diffmd > 10) return false;							   
										   
			return true;
		});
		
		additionalConstraints.emplace_back([](const auto& coords){
			//difference between energytech and fusion plant should be 3 or less
			auto diff = std::abs(coords[index_energytech] - coords[index_fusionplant]);
			if(diff > 3) 
				return false;
										   
			//fusion plant requires energy tech >=3
			if(coords[index_fusionplant] > 0 && coords[index_energytech] < 3)
				return false;
										   
			return true;
		});
		
		auto should_discard_coords = [&](const auto& coords){
			return !std::all_of(additionalConstraints.begin(), 
								additionalConstraints.end(), 
								[&](auto func){
									return func(coords);
								});
		};
		
		auto discard_or_handle_coords =  [&](const auto& coords){
			if(should_discard_coords(coords))
				return;
			handle_coords(coords);
		};
		
		int largest_relaxed_diag = 0;
		int max_diag = 100;
		
		int num_threads = 1;
		//get available number of threads
		#pragma omp parallel
		{
			#pragma omp master
			num_threads = omp_get_num_threads();
		
		}
		
		for(int diag = 0; diag <= max_diag; diag++){
			std::cout << "diag: " << diag << std::endl;
			Coords bounds;
			//std::fill(bounds.begin(), bounds.end(), diag+1);
			for(int i = 0; i < bounds.size(); i++){
				bounds[i] = std::min(diag+1, int(absoluteConstraints[i]));
			}

			if(diag != 0){
			
				//init diagonal
				std::vector<std::vector<Coords>> coords_per_thread(num_threads);
					
				TIMERSTARTCPU(first_diag_pass);
				enumerate_manhattan_coords_increasing<dimensions>(bounds, diag, [&](const auto& coords){
					if(should_discard_coords(coords))
						return;
					
					int threadid = omp_get_thread_num();
					coords_per_thread[threadid].emplace_back(coords);
				});
				TIMERSTOPCPU(first_diag_pass);
				
				TIMERSTARTCPU(merging_coords);
				
				std::vector<Coords> merged;
				std::sort(coords_per_thread[0].begin(), coords_per_thread[0].end(), container_less_than_elementwise<Coords, Coords>);
				for(std::size_t i = 1; i < coords_per_thread.size(); i++){
					std::sort(coords_per_thread[i].begin(), coords_per_thread[i].end(), container_less_than_elementwise<Coords, Coords>);
					
					merged.resize(coords_per_thread[0].size() + coords_per_thread[i].size());
					
					std::merge(coords_per_thread[0].begin(), coords_per_thread[0].end(),
								coords_per_thread[i].begin(), coords_per_thread[i].end(),
								merged.begin(), container_less_than_elementwise<Coords, Coords>);
					
					{
						std::vector<Coords> tmp;
						std::swap(coords_per_thread[i],tmp);
					}
					
					std::swap(merged, coords_per_thread[0]);
				}
					
				dpmatrix.set_diagonal_coords_and_default_others(diag, std::move(coords_per_thread[0]));
				
				TIMERSTOPCPU(merging_coords);
			
			}
			
			
			atLeastOneCellRelaxed = 0;
			TIMERSTARTCPU(second_diag_pass);
			enumerate_manhattan_coords_increasing<dimensions>(bounds, diag, discard_or_handle_coords);
			TIMERSTOPCPU(second_diag_pass);

			/*if(diag == 0 || atLeastOneCellRelaxed.load()){
				largest_relaxed_diag = diag;
			}else{
				break;
			}*/
		}
		
		largest_relaxed_diag = 150;
				
		
		Coords resultcoords{};
		decltype(dpmatrix)::Wrapper resultcell = dpmatrix[resultcoords];

		float ratio = 0;
		bool found = false;
		std::mutex m;
		Coords backtrackbounds{};
		//std::fill(backtrackbounds.begin(), backtrackbounds.end(), max_diag+1);

		for(int i = 0; i < backtrackbounds.size(); i++){
			backtrackbounds[i] = std::min(max_diag+1, int(absoluteConstraints[i]));
		}
		
		enumerate_manhattan_coords_increasing<dimensions>(backtrackbounds, max_diag /*largest_diag*/, [&](const auto& currentcoords){
			if(should_discard_coords(currentcoords))
				return;
			
			std::lock_guard<std::mutex> lg(m);
			
			if(!found){
				resultcell = dpmatrix[currentcoords];
			}
			
			/*Resources dailyProduction = ogamehelpers::getDailyProduction(currentcoords[index_metmine], ItemRarity::None, 100, 
																				currentcoords[index_crystalmine], ItemRarity::None, 100, 
																				currentcoords[index_deutsynth], ItemRarity::None, 100, 
																				currentcoords[index_solarplant], currentcoords[index_fusionplant], currentcoords[index_energytech],
																				temperature, currentcoords[index_plasmatech], speedfactor, 
																				engineer, geologist, staff);
			
			float dseproduction = float(dailyProduction.met) / 3 + float(dailyProduction.crystal) / 2 + float(dailyProduction.deut);
			
			float currentratio = dseproduction / dpmatrix[currentindex].time;
			print_coords(currentcoords);
			std::cout << " : " << currentratio << ", " << dailyProduction.met << " " << dailyProduction.crystal << " " << dailyProduction.deut << " " << dpmatrix[currentindex].time << std::endl;*/
			
			/*Resources costs1;
			
			for(int dim = 0; dim < dimensions; dim++){
					Resources totalinvestedcosts1 = getTotalCosts(entityInfoMap[dim], currentcoords[dim]);
					costs1.met += totalinvestedcosts1.met;
					costs1.crystal += totalinvestedcosts1.crystal;
					costs1.deut += totalinvestedcosts1.deut;
			}
			
			float dsecosts1 = float(costs1.met) / 3 + float(costs1.crystal) / 2 + float(costs1.deut); 
			
			float dseproduction = float(dpmatrix[currentindex].production.met) / 3 + float(dpmatrix[currentindex].production.crystal) / 2 + float(dpmatrix[currentindex].production.deut);
			
			print_coords(currentcoords);
			std::cout << dseproduction << ", ";
			print_time(std::cout, dpmatrix[currentindex].time);
			std::cout << ", ";
			std::cout << (dseproduction / dsecosts1);*/
			//print_time(std::cout, dpmatrix[currentindex].totalConstructionTime);
			//std::cout << ", ";
			//print_time(std::cout, dpmatrix[currentindex].constructiontime);
			//std::cout << ", ";
			//if(dpmatrix[currentindex].savetime == savetime_hidden)
			//	std::cout << "overlapped savetime ";
			//else 
			//	print_time(std::cout, dpmatrix[currentindex].savetime);
			//std::cout << std::endl;
			auto currentcell = dpmatrix[currentcoords];
			
			if(!found || compare_states_by_time(*currentcell.state, currentcoords, *resultcell.state, resultcoords)){
				resultcoords = currentcoords;
				resultcell = currentcell;
				//ratio = currentratio;
				found = true;
			}
		});
		
		//for(auto& i : resultcoords) i = std::max(0, i-1);
		
		//backtrack to origin to find step-by-step list
		std::vector<PathNode> path;
		Coords currentcoords = resultcoords;
		auto currentcell = dpmatrix[resultcoords];

		path.emplace_back(currentcoords, currentcell.state);
		while(*currentcell.prev != noprev && *currentcell.prev != illegalprev){

			currentcoords[*currentcell.prev] -= 1;
			currentcell = dpmatrix[currentcoords];
			
			path.emplace_back(currentcoords, currentcell.state);
		}	
		
		std::reverse(path.begin(), path.end());
		
		std::for_each(path.begin(), path.end(), [&](const auto& node){
			//const State& state = dpmatrix.get_state(node.coords);
			
            print_coords(std::cout, node.coords);
			std::cout << ", ";
			
			/*std::cout << ", m: " << node.state->ress.met;
			std::cout << ", k: " << node.state->ress.crystal;
			std::cout << ", d: " << node.state->ress.deut << " ";*/
			float mineproductionfactor = ogamehelpers::getMineProductionFactor(int(node.coords[index_metmine]), 100,
																					int(node.coords[index_crystalmine]), 100,
																					int(node.coords[index_deutsynth]), 100,
																					int(node.coords[index_solarplant]), 100,
																					int(node.coords[index_fusionplant]), 100,
																					int(node.coords[index_energytech]), 
																					0,0,0,
																					engineer, staff);
			Resources costs1;
			
			for(int dim = 0; dim < dimensions; dim++){
					Resources totalinvestedcosts1 = getTotalCosts(entityInfoMap[dim], node.coords[dim]);
					costs1.met += totalinvestedcosts1.met;
					costs1.crystal += totalinvestedcosts1.crystal;
					costs1.deut += totalinvestedcosts1.deut;
			}
			
			float dsecosts1 = float(costs1.met) / 3 + float(costs1.crystal) / 2 + float(costs1.deut); 
			
			float dseproduction = float(node.state->production.met) / 3 + float(node.state->production.crystal) / 2 + float(node.state->production.deut);
			
			//std::cout << mineproductionfactor << " ";
			print_time(std::cout, node.state->time);
			
			std::cout << ", ";
			std::cout << "\tprod factor " << mineproductionfactor;
			std::cout << ", ";
			//std::cout << "\tdse prod per day " <<(dseproduction);
			std::cout << "\t" << node.state->production.met << " " << node.state->production.crystal << " " << node.state->production.deut;
			/*print_time(std::cout, node.state->totalConstructionTime);
			std::cout << ", ";
			print_time(std::cout, node.state->constructiontime);
			std::cout << ", ";
			if(node.state->savetime == savetime_hidden)
				std::cout << "overlapped savetime ";
			else 
				print_time(std::cout, node.state->savetime);
			
			std::cout << node.state->time;*/
			
			
			std::cout << '\n';
        });

		
		//const std::size_t resultindex = to_linear_index(resultcoords, bounds);
		
		
		
		//std::cout << "Time: " << dpmatrix[resultindex].time << std::endl;
	}
	
};


}




#endif
