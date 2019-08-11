#include "io.hpp"
#include "util.hpp"
#include "serialization.hpp"
#include "json.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace ogh = ogamehelpers;
using json = nlohmann::json;

std::vector<std::string> split(const std::string& str, char c){
	std::vector<std::string> result;

	std::stringstream ss(str);
	std::string s;

	while (std::getline(ss, s, c)) {
			result.emplace_back(s);
	}

	return result;
}

std::string trim(const std::string& str, const std::string& whitespace = " "){
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

bool is_number(const std::string& s){
    return !s.empty() && std::find_if(s.begin(), 
        s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}



int UpgradeTask::researchLocation = -1;
int UpgradeTask::allCurrentPlanetsLocation = -2;

std::ostream& operator<<(std::ostream& os, const UpgradeTask& r){
	const auto& locs = r.getLocations();
	for(int i = 0; i < int(locs.size()); i++){
		os << locs[i] << " ";
	}
	os << r.entityInfo.name;
	return os;
}

std::ostream& operator<<(std::ostream& os, const UpgradeGroup& r){
	const auto& tasks = r.getTasks();
	if(r.isTransposed())
		os << "( ";
	for(int i = 0; i < int(tasks.size()); i++){
		
		os << tasks[i] << " ";
	}
	if(r.isTransposed())
		os << " )";
	
	return os;
}

std::ostream& operator<<(std::ostream& os, const PermutationGroup& r){
	const auto& groups = r.groups;
	
	for(int i = 0; i < int(groups.size()); i++){		
		os << groups[i] << " ";
	}
	
	return os;
}

bool operator==(const UpgradeTask& l, const UpgradeTask& r){
	return (l.entityInfo.entity == r.entityInfo.entity && l.getLocations() == r.getLocations());
}

bool operator==(const UpgradeGroup& l, const UpgradeGroup& r){
	return (l.isTransposed() == r.isTransposed() && l.getTasks() == r.getTasks());
}

bool operator==(const PermutationGroup& l, const PermutationGroup& r){
	return (l.groups == r.groups);
}

bool operator!=(const UpgradeTask& l, const UpgradeTask& r){
	return !(l == r);
}

bool operator!=(const UpgradeGroup& l, const UpgradeGroup& r){
	return !(l == r);
}

bool operator!=(const PermutationGroup& l, const PermutationGroup& r){
	return !(l == r);
}

bool operator<(const UpgradeTask& l, const UpgradeTask& r){
	if(l.entityInfo.name != r.entityInfo.name)
		return l.entityInfo.name < r.entityInfo.name;
	
	return container_less_than_elementwise(l.getLocations(), r.getLocations());
}

bool operator<(const UpgradeGroup& l, const UpgradeGroup& r){
	if(!l.transposed && r.transposed)
		return true;
	if(l.transposed && !r.transposed)
		return false;
		
	return container_less_than_elementwise(l.tasks, r.tasks);
}

bool operator<(const PermutationGroup& l, const PermutationGroup& r){
	return container_less_than_elementwise(l.groups, r.groups);
} 

Account parseAccountJsonFile(const std::string& filename){
    Account account;
    std::ifstream is(filename);

    if(!is)
        throw std::runtime_error("Cannot open file " + filename);
    
    json j;
    is >> j;
    account = j.get<Account>();

    return account;  
}

ogh::EntityInfo parseUpgradeName(const std::string& name){
    
    ogh::EntityInfo entity;

    if(name == "metalmine" || name == "met"){
	    entity = ogh::Metalmine;
    }else if(name == "crystalmine" || name == "kris" || name == "crys"){
	    entity = ogh::Crystalmine;
    }else if(name == "deutsynth" || name == "deut"){
	    entity = ogh::Deutsynth;
    }else if(name == "solarplant" || name == "skw"){
	    entity = ogh::Solar;
    }else if(name == "fusionplanet" || name == "fkw"){
	    entity = ogh::Fusion;
    }else if(name == "robofactory" || name == "robo"){
	    entity = ogh::Robo;
    }else if(name == "nanitefactory" || name == "nani"){
	    entity = ogh::Nanite;
    }else if(name == "researchlab" || name == "lab"){
	    entity = ogh::Lab;
    }else if(name == "energytech" || name == "etech"){
	    entity = ogh::Energy;
    }else if(name == "plasmatech" || name == "plasma"){
	    entity = ogh::Plasma;
    }else if(name == "astrophysics" || name == "astro"){
	    entity = ogh::Astro;
    }else if(name == "researchnetwork" || name == "igfn" || name == "igrn"){
	    entity = ogh::Researchnetwork;
    }else if(name == "computertech" || name == "comp"){
	    entity = ogh::Computer;
    }else if(name == "none" || name == ""){
        entity = ogh::Noentity;
    }else{
	    std::cout << "Invalid upgrade name:" << name << std::endl;
	    throw std::runtime_error("");
    }
    return entity;
}

std::vector<PermutationGroup> parseUpgradeFile(const std::string& filename){
	
	std::vector<PermutationGroup> upgradeList;
	
	std::ifstream is(filename);
	
	if(!is)
		throw std::runtime_error("Cannot open file " + filename);
		
	std::string line;
	std::string lowerline;
	
	auto nextline = [&](){
		bool b = false;
		while((b = static_cast<bool>(std::getline(is, line)))){
			if(line.size() == 0 || (line.size() > 0 && line[0] == '#'))
				continue; //skip full line comments
			line = line.substr(0, line.find('#')); //remove comments starting with #
			line = trim(line); //remove trailing whitespace
			lowerline.resize(line.size());
			std::transform(line.begin(), line.end(), lowerline.begin(), ::tolower);
			break;
		}
		return b;
	};
	
	while(nextline()){
		auto tokens = split(lowerline, ' ');
		
		if(tokens.size() > 0){
			PermutationGroup permGroup;
			
			if(tokens.size() == 1){
				UpgradeTask job;
				job.entityInfo = parseUpgradeName(tokens[0]);
				
				if(job.isResearch()){
					job.locations.emplace_back(UpgradeTask::researchLocation);
				}else{
					job.locations.emplace_back(UpgradeTask::allCurrentPlanetsLocation);
				}
				
				permGroup.groups.emplace_back(false, std::vector<UpgradeTask>{job});
			}
			
			if(tokens.size() > 1){
				int braces = 0;
				for(const auto& s : tokens){
					if(s == "(") braces++;
					if(s == ")") braces--;
					if(braces < 0) throw std::runtime_error("Error parsing braces");
					if(braces > 1) throw std::runtime_error("Error parsing braces");
				}
				if(braces != 0) throw std::runtime_error("Error parsing braces");
				
				UpgradeGroup upgradeGroup;
			
				std::vector<int> locations;
				
				for(std::size_t i = 0; i < tokens.size(); i++){
					if(tokens[i] == "("){
						//previous group is finished. no nested braces allowed
						if(upgradeGroup.tasks.size() > 0){
							permGroup.groups.emplace_back(upgradeGroup);
							upgradeGroup = UpgradeGroup{};
						}
						upgradeGroup.transposed = true;
					}else if(tokens[i] == ")"){
						//transposed upgrade group is finished
						if(upgradeGroup.tasks.size() > 0){
							permGroup.groups.emplace_back(upgradeGroup);
							upgradeGroup = UpgradeGroup{};
						}
					}else if(is_number(tokens[i])){
						int loc = std::stoi(tokens[i]) - 1;
						assert(loc >= 0);
						locations.emplace_back(loc);
					}else{
						UpgradeTask job;
						job.entityInfo = parseUpgradeName(tokens[i]);
					
						// token is upgrade name. if locations is empty, upgrade is performed on all planets, 
						// else it is performed on the planets given in locations
						if(locations.empty()){
							if(job.isResearch()){
								job.locations.emplace_back(UpgradeTask::researchLocation);
							}else{
								job.locations.emplace_back(UpgradeTask::allCurrentPlanetsLocation);
							}
						}else{
							job.locations = locations;
						}
						
						upgradeGroup.tasks.emplace_back(job);
						
						locations.clear();
					}
				}
				
				if(!locations.empty()){
					std::cout << "Warning. Found upgrade locations without upgrade name! Ignoring this upgrade!" << std::endl;
				}
				
				if(upgradeGroup.tasks.size() > 0){
					permGroup.groups.emplace_back(upgradeGroup);
				}
			}
			
			upgradeList.emplace_back(permGroup);
		}
	}
	
	return upgradeList;
}


