#ifndef PARALLEL_PERMUTATION_HPP
#define PARALLEL_PERMUTATION_HPP

#include <algorithm>
#include <thread>
#include <vector>
#include <mutex>
#include <numeric>


template<class Container, class Func>
void parallel_for_each_permutation(const Container& container, int num_threads, Func func){
    /*
     * 
     * https://stackoverflow.com/questions/7918806/finding-n-th-permutation-without-computing-others
     *
     */
    
    auto ithPermutation = [](int n, size_t i) -> std::vector<size_t>{
        std::vector<size_t> fact(n);
        std::vector<size_t> perm(n);
        
        fact[0] = 1;
        for(int k = 1; k < n; k++)
            fact[k] = fact[k-1] * k;
        
        for(int k = 0; k < n; k++){
            perm[k] = i / fact[n-1-k];
            i = i % fact[n-1-k];
        }
        
        
        for(int k = n-1; k > 0; k--){
            for(int j = k-1; j >= 0; j--){
                if(perm[j] <= perm[k])
                    perm[k]++;
            }
        }
        
        return perm;
    };
    
    size_t totalNumPermutations = 1;
    for(size_t i = 1; i <= container.size(); i++)
        totalNumPermutations *= i;
    
    std::vector<std::thread> threads;

    for(int threadId = 0; threadId < num_threads; threadId++){
        threads.emplace_back([&, threadId](){
            const size_t firstPerm = size_t(float(threadId) * totalNumPermutations / num_threads);
            const size_t last_excl = std::min(totalNumPermutations, size_t(float(threadId+1) * totalNumPermutations / num_threads));
                        
            Container permutation(container);
            
            auto permIndices = ithPermutation(container.size(), firstPerm);
            
            size_t count = firstPerm;
            do{
                for(int i = 0; i < int(permIndices.size()); i++){
                    permutation[i] = container[permIndices[i]];
                }
                
                func(threadId, permutation);
                std::next_permutation(permIndices.begin(), permIndices.end());
                ++count;
            }while(count < last_excl);
        });
    }
    
    for(auto& thread : threads)
        thread.join();
}




#endif
