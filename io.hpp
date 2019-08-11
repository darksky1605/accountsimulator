#ifndef IO_HPP
#define IO_HPP

#include "account.hpp"
#include "ogame.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using UpgradeJobList = std::vector<Account::UpgradeJob>;

bool operator==(const UpgradeJobList& l, const UpgradeJobList& r);
bool operator!=(const UpgradeJobList& l, const UpgradeJobList& r);
bool operator<(const UpgradeJobList& l, const UpgradeJobList& r);

struct UpgradeTask{
	static int researchLocation;
	static int allCurrentPlanetsLocation;
		
	ogamehelpers::EntityInfo entityInfo{};
	std::vector<int> locations{};

	UpgradeTask(){}
	UpgradeTask(const ogamehelpers::EntityInfo e, const std::vector<int>& l) : entityInfo(e), locations(l){}

	bool isResearch() const{
		return entityInfo.type == ogamehelpers::EntityType::Research;
	}

	const std::vector<int>& getLocations() const{
		return locations;
	}
};

struct UpgradeGroup{
	bool transposed = false;
	std::vector<UpgradeTask> tasks;
	
	UpgradeGroup(){}
	UpgradeGroup(bool t, const std::vector<UpgradeTask>& l) : transposed(t), tasks(l){}
	
	bool isTransposed() const{
		return transposed;
	}

    bool valid() const{
        if(tasks.empty()) return true;

        auto equalsfirstloc = [&](const auto& t){
            return tasks[0].getLocations() == t.getLocations();
        };

        if(isTransposed()){
            return std::all_of(tasks.begin()+1, tasks.end(), equalsfirstloc);
        }else{
            return true;
        }
    }
	
    const std::vector<UpgradeTask>& getTasks() const{
        assert(valid());
        return tasks;
    }

	std::vector<UpgradeTask> getTasks(int numPlanets) const{
		assert(valid());

        if(tasks.empty() || tasks[0].getLocations().empty()){
            return {};
        }

        if(isTransposed()){
            std::vector<std::size_t> posInTask(tasks.size(), 0);
            
            if(tasks[0].getLocations()[0] == UpgradeTask::allCurrentPlanetsLocation){
                std::vector<UpgradeTask> result;
                int maxPos = numPlanets;
                for(int pos = 0; pos < maxPos; pos++){
                    for(const auto& task : tasks){
                        result.emplace_back(task);
                        result.back().locations = std::vector<int>{pos};
                    }
                }
                return result;
            }else{
                std::vector<UpgradeTask> result;
                int maxPos = int(tasks[0].getLocations().size());
                for(int pos = 0; pos < maxPos; pos++){
                    for(const auto& task : tasks){
                        result.emplace_back(task);
                        result.back().locations = std::vector<int>{task.getLocations()[pos]};
                    }
                }
                return result;
            }
        }else{
            if(tasks[0].getLocations()[0] == UpgradeTask::allCurrentPlanetsLocation){
                std::vector<UpgradeTask> result;
                int maxPos = numPlanets;                
                for(const auto& task : tasks){
                    for(int pos = 0; pos < maxPos; pos++){
                        result.emplace_back(task);
                        result.back().locations = std::vector<int>{pos};
                    }
                }
                return result;
            }else{
		        return tasks;
            }
        }
	}
};

struct PermutationGroup{
	std::vector<UpgradeGroup> groups;
	
	PermutationGroup(){}
	PermutationGroup(const std::vector<UpgradeGroup>& g) : groups(g){}
};

std::ostream& operator<<(std::ostream& os, const UpgradeTask& r);
std::ostream& operator<<(std::ostream& os, const UpgradeGroup& r);
std::ostream& operator<<(std::ostream& os, const PermutationGroup& r);

bool operator==(const UpgradeTask& l, const UpgradeTask& r);
bool operator==(const UpgradeGroup& l, const UpgradeGroup& r);
bool operator==(const PermutationGroup& l, const PermutationGroup& r);

bool operator!=(const UpgradeTask& l, const UpgradeTask& r);
bool operator!=(const UpgradeGroup& l, const UpgradeGroup& r);
bool operator!=(const PermutationGroup& l, const PermutationGroup& r);

bool operator<(const UpgradeTask& l, const UpgradeTask& r);
bool operator<(const UpgradeGroup& l, const UpgradeGroup& r);
bool operator<(const PermutationGroup& l, const PermutationGroup& r);

Account parseAccountJsonFile(const std::string& filename);

std::vector<UpgradeJobList> parseUpgradeFile2(const std::string& filename);
std::vector<PermutationGroup> parseUpgradeFile3(const std::string& filename);

#endif
