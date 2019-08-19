#ifndef PARALLEL_PERMUTATION_HPP
#define PARALLEL_PERMUTATION_HPP

#include <algorithm>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>
#include <map>

template <class Container, class Func>
void parallel_for_each_permutation(const Container& container, int num_threads, Func func) {
    /*
     * 
     * https://stackoverflow.com/questions/7918806/finding-n-th-permutation-without-computing-others
     *
     */

    auto ithPermutation = [](int n, size_t i) -> std::vector<size_t> {
        std::vector<size_t> fact(n);
        std::vector<size_t> perm(n);

        fact[0] = 1;
        for (int k = 1; k < n; k++)
            fact[k] = fact[k - 1] * k;

        for (int k = 0; k < n; k++) {
            perm[k] = i / fact[n - 1 - k];
            i = i % fact[n - 1 - k];
        }

        for (int k = n - 1; k > 0; k--) {
            for (int j = k - 1; j >= 0; j--) {
                if (perm[j] <= perm[k])
                    perm[k]++;
            }
        }

        return perm;
    };

    size_t totalNumPermutations = 1;
    for (size_t i = 1; i <= container.size(); i++)
        totalNumPermutations *= i;

    std::vector<std::thread> threads;

    for (int threadId = 0; threadId < num_threads; threadId++) {
        threads.emplace_back([&, threadId]() {
            const size_t firstPerm = size_t(float(threadId) * totalNumPermutations / num_threads);
            const size_t last_excl = std::min(totalNumPermutations, size_t(float(threadId + 1) * totalNumPermutations / num_threads));

            Container permutation(container);

            auto permIndices = ithPermutation(container.size(), firstPerm);

            size_t count = firstPerm;
            do {
                for (int i = 0; i < int(permIndices.size()); i++) {
                    permutation[i] = container[permIndices[i]];
                }

                func(threadId, permutation);
                std::next_permutation(permIndices.begin(), permIndices.end());
                ++count;
            } while (count < last_excl);
        });
    }

    for (auto& thread : threads)
        thread.join();
}






template<class T>
struct MultisetPermutation{
    //code adapted from https://github.com/ekg/multichoose

    struct Result{
        bool valid;
        std::vector<T> permutation;
    };

    struct ListElement{
        T value;
        ListElement* next;

        ListElement() { }

        ListElement(T val, ListElement* n) {
            value = val;
            next = n;
        }

        ListElement* nth(int n) {
            ListElement* o = this;
            int i = 0;
            while (i < n && o->next != NULL) {
                o = o->next;
                ++i;
            }
            return o;
        }

        ~ListElement() {
            if (next != NULL) {
                delete next;
            }
        }

    };

    void list_init(std::vector<T>& multiset) {
        std::sort(multiset.begin(), multiset.end(), comparator); // ensures proper non-increasing order
        auto item = multiset.begin();
        h = new ListElement(*item, NULL);
        ++item;
        while (item != multiset.end()) {
            h = new ListElement(*item, h);
            ++item;
        }
    }

    static std::vector<T> linked_list_to_vector(ListElement* h) {
        ListElement* o = h;
        std::vector<T> l;
        while (o != NULL) {
            l.push_back(o->value);
            o = o->next;
        }
        return l;
    }

    ListElement* h;
    ListElement* i;
    ListElement* j;
    ListElement* s;
    ListElement* t;
    bool firstCall;

    std::function<bool(const T&, const T&)> comparator;

    MultisetPermutation(const std::vector<T>& multiset) 
        : MultisetPermutation(multiset, std::less<T>{}){

    }

    template<class Comp>
    MultisetPermutation(const std::vector<T>& multiset, Comp comp) 
            : firstCall(true), comparator(comp){
        std::vector<T> tmp(multiset);
        list_init(tmp);
        i = h->nth(tmp.size() - 2);
        j = h->nth(tmp.size() - 1);
    }

    ~MultisetPermutation(){
        delete h;
    }

    bool calculateNext(){
        if(j->next != NULL || comparator(j->value, h->value)){
            if (j->next != NULL && !comparator(i->value, j->next->value)) {
                s = j;
            } else {
                s = i;
            }
            t = s->next;
            s->next = t->next;
            t->next = h;
            if (comparator(t->value, h->value)) {
                i = t;
            }
            j = i->next;
            h = t;
            return true;
        }else{
            return false;
        }
    }

    void skip(int n){
        for(int k = 0; k < n; k++){
            calculateNext();
        }
    }

    Result getNextPerm(){
        if(!firstCall){
            if(calculateNext()){
                Result res;
                res.valid = true;
                res.permutation = linked_list_to_vector(h);
                return res;
            }else{
                Result res;
                res.valid = false;
                return res;
            }
        }else{
            firstCall = false;
            Result res;
            res.valid = true;
            res.permutation = linked_list_to_vector(h);
            return res;
        }
    }
};




template <class Container, class Func>
void parallel_for_each_unique_permutation(const Container& container, int num_threads, Func func) {

    auto factorial = [](size_t n){
        size_t f = 1;
        for(size_t i = 2; i <= n; i++){
            f *= i;
        }
        return f;
    };

    auto comparator = [&](int i, int k){
        return container[i] < container[k];
    };

    using Comp_t = decltype(comparator);
    //using value_type = typename Container::value_type;

    const size_t maxNumPermutations = factorial(container.size());

    std::vector<int> indices(container.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::map<int, int, Comp_t> frequencies(comparator);
    for(const auto& i : indices){
        frequencies[i]++;
    }

    size_t numPermutations = maxNumPermutations;
    for(const auto& p : frequencies){
        numPermutations /= factorial(p.second);
    }

    std::cerr << "parallel_for_each_unique_permutation : total number of permutations " << maxNumPermutations << ", unique " << numPermutations << '\n';

    std::vector<std::thread> threads;

    for (int threadId = 0; threadId < num_threads; threadId++) {
        threads.emplace_back([&, threadId, indices]() {
            const size_t firstPerm = size_t(float(threadId) * numPermutations / num_threads);
            const size_t last_excl = std::min(numPermutations, size_t(float(threadId + 1) * numPermutations / num_threads));

            MultisetPermutation<int> permGenerator(indices, comparator);
            permGenerator.skip(firstPerm);
            std::cout << "thread " << threadId << " begin" << std::endl;

            Container permutation(container);

            auto permIndices = permGenerator.getNextPerm();
            size_t count = firstPerm;
            while(permIndices.valid && count < last_excl){
                for (int i = 0; i < int(permIndices.permutation.size()); i++) {
                    permutation[i] = container[permIndices.permutation[i]];
                }

                func(threadId, permutation);
                ++count;
                permIndices = permGenerator.getNextPerm();
            }
            std::cout << "thread " << threadId << " end" << std::endl;
        });
    }

    for (auto& thread : threads)
        thread.join();
}

#endif
