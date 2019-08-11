#include "ogame.hpp"
#include "account.hpp"
#include "parallel_permutation.hpp"
#include "util.hpp"
#include "hpc_helpers.cuh"
#include "serialization.hpp"
#include "json.hpp"

#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <numeric>
#include <atomic>
#include <mutex>
#include <set>

	namespace ogh = ogamehelpers;
    using json = nlohmann::json;


	struct UpgradeResult{
        bool success = false;
		float constructionFinishedInDays = 0;
		float lastConstructionStartedAfterDays = 0;
		float savingFinishedInDays = 0;
		float previousUpgradeDelay = 0;
		std::vector<Account::UpgradeJobStats> upgradeJobStatistics;
        
        bool operator==(const UpgradeResult& rhs) const{
            return success == rhs.success && constructionFinishedInDays == rhs.constructionFinishedInDays
            && lastConstructionStartedAfterDays == rhs.lastConstructionStartedAfterDays
            && savingFinishedInDays == rhs.savingFinishedInDays
            && previousUpgradeDelay == rhs.previousUpgradeDelay
            && upgradeJobStatistics == rhs.upgradeJobStatistics;
        }
        
        bool operator!=(const UpgradeResult& rhs) const{
            return !(operator==(rhs));
        }
	};
	
	using UpgradeJobList = std::vector<Account::UpgradeJob>;	
    
    bool operator==(const UpgradeJobList& l, const UpgradeJobList& r){
        if(l.size() != r.size()) 
            return false;
        for(int i = 0; i < int(l.size()); i++){
            if(l[i].entityInfo.name != r[i].entityInfo.name)
                return false;
        }
        return true;
    }
    
    bool operator!=(const UpgradeJobList& l, const UpgradeJobList& r){
        return !(l == r);
    }
    
    bool operator<(const UpgradeJobList& l, const UpgradeJobList& r){
        return container_less_than_elementwise(l,r);
    }
    
	UpgradeResult perform_upgrades(Account& account,
									const std::vector<UpgradeJobList>& planned_upgrades){

		using ogh::EntityType;
				
		UpgradeResult result;
		result.upgradeJobStatistics.reserve(planned_upgrades.size());
		
		/*auto print_job = [](auto job){
			std::cout << job.entityInfo.name << " at planet " << job.location << std::endl;
		};*/
        bool ok = true;
        
		for(std::size_t listId = 0; listId < planned_upgrades.size() && ok; listId++){
			const UpgradeJobList& jobList = planned_upgrades[listId];
			auto first = jobList.begin();
			while(first != jobList.end()){
				auto last = first;
								
				//find range with same upgrade type
				while(last != jobList.end() && first->entityInfo.type == last->entityInfo.type){
					last++;
				}				
				
				if(first->isResearch()){
					//process research jobs in range [first, last)
					
					ok = std::all_of(first, last, [&](const auto& job){
						assert(job.entityInfo.type == EntityType::Research && job.location == Account::UpgradeJob::researchLocation);
						
						result.upgradeJobStatistics.emplace_back(account.processResearchJob(job));
						
						//print_job(job);
                        
                        if(account.time == std::numeric_limits<float>::max())
                            return false;
                        return true;
					});
					
				}else{
					
					//find ranges with same upgrade location
					
					auto first2 = first;
					
					while(first2 != last){
						auto last2 = first2;
						while(last2 != last && first2->location == last2->location){
							last2++;
						}
						
						if(first2->location == Account::UpgradeJob::allCurrentPlanetsLocation){

                            
							for(int i = 0; i < account.getNumPlanets() && ok; i++){							
								ok = std::all_of(first2, last2, [&](auto job){
                                        job.location = i;
                                        
                                        assert(job.entityInfo.type == EntityType::Building && (job.location >= 0 && job.location < account.getNumPlanets()+1)); // +1 to account for possible astro physics in progress
                            
                                        result.upgradeJobStatistics.emplace_back(account.processBuildingJob(job));
                                        
                                        //print_job(job);
                                        
                                        if(account.time == std::numeric_limits<float>::max())
                                            return false;
                                        return true;
                                    });
							}
							
						}else{
							
							ok = std::all_of(first2, last2, [&](const auto& job){
								assert(job.entityInfo.type == EntityType::Building && (job.location >= 0 && job.location < account.getNumPlanets()+1)); // +1 to account for possible astro physics in progress
								
								result.upgradeJobStatistics.emplace_back(account.processBuildingJob(job));
								
								//print_job(job);
                                
                                if(account.time == std::numeric_limits<float>::max())
                                    return false;
                                return true;
							});
							
						}
						
						first2 = last2;
					}
				}
				
				
				first = last;
			}
		}
		
		result.success = ok;
		
		if(ok){
            
            result.lastConstructionStartedAfterDays = account.time;
            result.savingFinishedInDays = 0.0f;
            result.previousUpgradeDelay = 0.0f;
            
            for(const auto& stat : result.upgradeJobStatistics){
                result.savingFinishedInDays += stat.waitingPeriodDaysBegin - stat.savePeriodDaysBegin;
                result.previousUpgradeDelay += stat.constructionBeginDays - stat.waitingPeriodDaysBegin;
            }
            
            //wait until all planets finished building
            
            account.waitForAllConstructions();
            
            result.constructionFinishedInDays = account.time;
        }
		
		std::cout << std::flush;
		std::cerr << std::flush;
		
		return result;
		
	}
	
	
// ##########################################################

struct UpgradeTask{
	static int researchLocation;
	static int allCurrentPlanetsLocation;
		
	ogh::EntityInfo entityInfo{};
	std::vector<int> locations{};

	UpgradeTask(){}
	UpgradeTask(const ogh::EntityInfo e, const std::vector<int>& l) : entityInfo(e), locations(l){}

	bool isResearch() const{
		return entityInfo.type == ogh::EntityType::Research;
	}

	const std::vector<int>& getLocations() const{
		return locations;
	}
};

int UpgradeTask::researchLocation = -1;
int UpgradeTask::allCurrentPlanetsLocation = -2;

struct UpgradeGroup{
	bool transposed = false;
	std::vector<UpgradeTask> tasks;
	
	UpgradeGroup(){}
	UpgradeGroup(bool t, const std::vector<UpgradeTask>& l) : transposed(t), tasks(l){}
	
	bool isTransposed() const{
		return transposed;
	}
	
	const std::vector<UpgradeTask>& getTasks() const{
		if(isTransposed()){
			auto first = tasks.begin();
			auto last = first;
			while(last != tasks.end()){
				assert(first->getLocations() == last->getLocations());
				++last;
			}
		}
		return tasks;
	}
};

struct PermutationGroup{
	std::vector<UpgradeGroup> groups;
	
	PermutationGroup(){}
	PermutationGroup(const std::vector<UpgradeGroup>& g) : groups(g){}
};

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


#if 1


std::vector<UpgradeJobList> parseUpgradeFile2(const std::string& filename){
	std::vector<UpgradeJobList> upgradeList;
	
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
		
		if(tokens.size() == 1){
			Account::UpgradeJob job;
			job.entityInfo = parseUpgradeName(tokens[0]);
			
			if(job.isResearch()){
				job.location = Account::UpgradeJob::researchLocation;
			}else{
				job.location = Account::UpgradeJob::allCurrentPlanetsLocation;
			}
			
			upgradeList.emplace_back(UpgradeJobList{job});
		}
		
		if(tokens.size() >= 2){
			UpgradeJobList jobList;
			
			std::vector<int> locations;
			
			for(std::size_t i = 0; i < tokens.size(); i++){
				
				if(!is_number(tokens[i])){
                    // token is upgrade name. if locations is empty, upgrade is performed on all planets, 
					// else it is performed on the planets given in locations

                    const auto entityInfo = parseUpgradeName(tokens[i]);

                    if(locations.empty()){
                        Account::UpgradeJob job;
                        job.entityInfo = entityInfo;
						if(job.isResearch()){
							job.location = Account::UpgradeJob::researchLocation;
						}else{
							job.location = Account::UpgradeJob::allCurrentPlanetsLocation;
						}
                        jobList.emplace_back(job);
					}else{
                        for(const auto& location : locations){
                            Account::UpgradeJob job;
                            job.entityInfo = entityInfo;
                            job.location = location;
                            jobList.emplace_back(job);
                        }
                        locations.clear();
                    }
				}else{
					int loc = std::stoi(tokens[i]) - 1;
					assert(loc >= 0);
					locations.emplace_back(loc);
				}
			}
			
			if(!locations.empty()){
				std::cout << "Warning. Found upgrade locations without upgrade name! Ignoring this upgrade!" << std::endl;
			}
			
			upgradeList.emplace_back(jobList);
		}
	}
	
	return upgradeList;
}




std::vector<PermutationGroup> parseUpgradeFile3(const std::string& filename){
	
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


#endif


std::string convert_time(float daysfloat){
	std::stringstream ss;
	std::uint64_t seconds = daysfloat * 24ULL * 60ULL * 60ULL;
	std::uint64_t days = seconds / (24ULL*60ULL*60ULL);
	seconds = seconds % (24ULL*60ULL*60ULL);
	std::uint64_t hours = seconds / (60ULL*60ULL);
	seconds = seconds % (60ULL * 60ULL);
	std::uint64_t minutes = seconds / 60ULL;
	seconds = seconds % 60ULL;
	ss << days << "d " << hours << "h " << minutes << "m " << seconds << "s";
	return ss.str();
}



void printLogRecord(std::ostream& os, const Account::LogRecord& record){
    os << record.time << ": " << record.msg << '\n';
}

void createAccountFile(const std::string& filename){
    Account account;
    account.addNewPlanet();
    json j = account;
    std::ofstream out(filename);
    if(!out){
        throw std::runtime_error("Cannot open new account file " + filename);
    }
    out << std::setw(4) << j;
}

void usage(int argc, char** argv){
    (void)argc;
    std::cout << "Usage:" << argv[0] << " Options\n\n";
	    
    std::cout << "Calculates how long it takes to perform a series of researches and upgrades in an account\n\n";
    std::cout << "Options:\n";
    std::cout << "--help: Show this message.\n\n";
    std::cout << "--newAccount file. Creates an empty account json file with given name and exists\n\n";
    std::cout << "--accountfile file: Load initial account data from file\n\n";
    std::cout << "--upgradefile file: Load upgrade list from file\n\n";
    std::cout << "--logfile file: Write program trace to file. Default log.txt.\n\n";
    std::cout << "--speed ecospeedfactor: Economy speed factor of universe. Overwrites account setting.\n\n";
    std::cout << "--threads num_threads: Number of CPU threads to use for permutations. Default 1.\n\n";
    std::cout << "--permute mode: Default 0.\n\n";
    std::cout << "  mode = 0: No permutation.\n";
    std::cout << "  mode = 1: Find permutation with shortest completion time.\n";
    std::cout << "  mode = 2: Find permutation with shortest time until last upgrade is started.\n";
    std::cout << "  mode = 3: Find permutation with shortest save time.\n";
    std::cout << "  mode = 4: Find permutation with shortest blocked queue time.\n";
    std::cout << "--permutations n: Display n best permutations. Default 1.\n\n";
    std::cout << "--printlist: Show detailed time table for upgrade\n\n";
    std::cout << "--printalllists: Show detailed time table for every permutation\n\n";
    std::cout << "--showPercentageChanges: Show changes of production percentages per planet\n\n";
    std::cout << "--dhm: Print durations in days, hours, minutes format instead of fractional days\n\n";
	    
    std::cout << "The initial account state is read from accountfile\n\n";
    std::cout << "The list of upgrades to perform is read from upgradefile\n\n";

    std::cout << "Example: " << argv[0] << " --accountfile account.json --upgradefile upgrades.txt --speed 2" << std::endl;
}

int detailedmultiupgrade(int argc, char** argv){
    //constexpr bool debugprint = true;
    //constexpr bool nanisweep = false;
    
    if(argc == 1){
        usage(argc, argv);
        return 0;
    }

    //check for options which do not start a simulation
    {

        for(int i = 1; i < argc; i++){
            if(std::string(argv[i]) == "--help"){
                usage(argc, argv);
                return 0;
            }

            if(std::string(argv[i]) == "--createAccount"){
                assert(i+1 < argc);
                //bool createAccount = true;
                std::string newAccountFile = std::string(argv[i+1]);
                i++;
                createAccountFile(newAccountFile);
                return 0;
            }
        }
        
    }
    
    int speedfactor = 1;
    bool overwriteSpeed = false;
    std::string accountFile("");
    std::string upgradeFile("");
    std::string logFileName("/dev/null");
    int permutationMode = 0;
    bool printList = false;
    bool printAllLists = false;
    int num_best_permutations = 1;
    int num_threads = 1;
    bool use_dhm_format = false;
    bool appendLog = false;
    bool showPercentageChanges = false;
    
    for(int i = 1; i < argc; i++){        
        if(std::string(argv[i]) == "--printlist"){
            printList = true;
            continue;
        }
        
        if(std::string(argv[i]) == "--printalllists"){
            printAllLists = true;
            continue;
        }

        if(std::string(argv[i]) == "--percentages"){
            showPercentageChanges = true;
            continue;
        }
        
        if(std::string(argv[i]) == "--speed"){
            assert(i+1 < argc);
            speedfactor = std::atoi(argv[i+1]);
            i++;
            overwriteSpeed = true;
            continue;
        }
        
        if(std::string(argv[i]) == "--threads"){
            assert(i+1 < argc);
            num_threads = std::atoi(argv[i+1]);
            i++;
            continue;
        }
        
        if(std::string(argv[i]) == "--accountfile"){
            assert(i+1 < argc);
            accountFile = std::string(argv[i+1]);
            i++;
            continue;
        }
        
        if(std::string(argv[i]) == "--upgradefile"){
            assert(i+1 < argc);
            upgradeFile = std::string(argv[i+1]);
            i++;
            continue;
        }
        
        if(std::string(argv[i]) == "--logfile"){
            assert(i+1 < argc);
            logFileName = std::string(argv[i+1]);
            i++;
            continue;
        }
        
        if(std::string(argv[i]) == "--permute"){
            assert(i+1 < argc);
            permutationMode = std::atoi(argv[i+1]);
            i++;
            continue;
        }
        
        if(std::string(argv[i]) == "--permutations"){
            assert(i+1 < argc);
            num_best_permutations = std::atoi(argv[i+1]);
            i++;
            continue;
        }
                
        if(std::string(argv[i]) == "--dhm"){
            use_dhm_format = true;
            continue;
        }
        
        if(std::string(argv[i]) == "--appendlog"){
            appendLog = true;
            continue;
        }
    }
    
    if(overwriteSpeed){
        std::cout << "Speed factor: " << speedfactor << '\n';
    }
    std::cout << "Permutation mode: " << permutationMode << '\n';
    std::cout << "printList: " << printList << '\n';
    std::cout << "printAllLists: " << printAllLists << '\n';
    std::cout << "showPercentageChanges: " << showPercentageChanges << '\n';
    std::cout << "Account file: " << accountFile << '\n';
    std::cout << "Upgrade file: " << upgradeFile << '\n';
    std::cout << "Log file: " << logFileName << '\n';
    std::cout << "Append log file: " << appendLog << '\n';
    std::cout << "DHM time format: " << use_dhm_format << '\n';
    std::cout << "Threads: " << num_threads << '\n';
    
    auto openmode = std::ios_base::out;
    if(appendLog)
        openmode = std::ios_base::app;
    
    std::ofstream logFile(logFileName, openmode);
    
    if(!logFile){
        throw std::runtime_error("Cannot open log file " + logFileName);
    }

    auto printLog = [&](const auto& record){
        printLogRecord(logFile, record);
    };
   
    Account account = parseAccountJsonFile(accountFile);

    if(overwriteSpeed){
        account.speedfactor = speedfactor;
    }

    auto planned_upgrades = parseUpgradeFile2(upgradeFile);

    {
        auto tmp = parseUpgradeFile3(upgradeFile);
        for(const auto& x : tmp)
            std::cout << x << std::endl;
    }
    
    /*for(const auto& upgrade : planned_upgrades){
     *	std::cout << (upgrade.location+1) << " " << upgrade.entityInfo.name << " " << upgrade.level << '\n';
}*/
    
    std::cout << std::endl;
    
                               
    #if 0
    std::vector<UpgradeJobList> upgradejoblist = parseUpgradeFile2("buildlist1.txt");
    auto result = perform_upgrades(account, upgradejoblist);
    
    #endif	
    
    if(permutationMode == 0){
          
        auto result = perform_upgrades(account, planned_upgrades);
        
        if(result.success){

            std::for_each(account.logRecords.begin(), account.logRecords.end(), printLog);
            logFile.flush();
            std::cout << std::endl;

            if(use_dhm_format){
                std::cout << "The selected upgrades take " << convert_time(result.constructionFinishedInDays) << " days.\n";
                std::cout << "Last upgrade started after " << convert_time(result.lastConstructionStartedAfterDays) << " days.\n";
                std::cout << "The required saving time is " << convert_time(result.savingFinishedInDays) << " days.\n";
                std::cout << "Days lost because a queue was full: " << convert_time(result.previousUpgradeDelay) << " days.\n";
            }else{
                std::cout << "The selected upgrades take " << result.constructionFinishedInDays << " days.\n";
                std::cout << "Last upgrade started after " << result.lastConstructionStartedAfterDays << " days.\n";
                std::cout << "The required saving time is " << result.savingFinishedInDays << " days.\n";
                std::cout << "Days lost because a queue was full: " << result.previousUpgradeDelay << " days.\n";
            }
            
            std::cout << '\n';
            
            if(printList){
                std::cout << "Detailed statistics:\n";
                
                for(int jobid = 0; jobid < int(result.upgradeJobStatistics.size()); jobid++){
                    const auto& stat = result.upgradeJobStatistics[jobid];
                    const auto& job = stat.job;
                    const auto& entityInfo = job.entityInfo;
                    const int upgradeLevel = stat.level;
                    const int upgradeLocation = job.location;
                    if(use_dhm_format){
                        std::cout << "Planet " << (upgradeLocation+1) << ": " << entityInfo.name << " " << upgradeLevel << ". Saving period begin: " << convert_time(stat.savePeriodDaysBegin)
                        << ", Waiting period begin: " << convert_time(stat.waitingPeriodDaysBegin) << ", Construction begin: " << convert_time(stat.constructionBeginDays) << ", Construction time: " << convert_time(stat.constructionTimeDays) 
                        << ", Save time: " << convert_time(stat.waitingPeriodDaysBegin - stat.savePeriodDaysBegin)<< '\n';                    
                    }else{
                        std::cout << "Planet " << (upgradeLocation+1) << ": " << entityInfo.name << " " << upgradeLevel << ". Saving period begin: " << stat.savePeriodDaysBegin 
                            << ", Waiting period begin: " << stat.waitingPeriodDaysBegin << ", Construction begin: " << stat.constructionBeginDays << ", Construction time: " << stat.constructionTimeDays 
                            << ", Save time: " << (stat.waitingPeriodDaysBegin - stat.savePeriodDaysBegin)<< '\n';
                    }
                }
            }

            if(showPercentageChanges){
                auto percentageChanges = account.getPercentageChanges();

                auto printChange = [](const auto& change){
                    auto& stream = std::cout;
                    stream << "Planet " << change.planetId << ": Changed percents to " 
                    << "m " << change.metPercent << ", c " << change.crysPercent 
                    << ", d " << change.deutPercent << ", f " << change.fusionPercent 
                    << " after construction of " << change.finishedName << " " << change.finishedLevel
                    << ". Production factor: " << change.oldMineProductionFactor 
                    << "->" << change.newMineProductionFactor 
                    << ". Production increased by " << (((double(change.newDSE)/change.oldDSE) - 1) * 100) 
                    << " % DSE" << std::endl;
                };

                std::cout << '\n';

                std::for_each(percentageChanges.begin(), percentageChanges.end(), printChange);
            }
        }else{
            std::cout << "Error: Upgrades could not be performed!" << std::endl;
        }
    }else{
        
        auto resultcomp1 = [](const UpgradeResult& l, const UpgradeResult& r){
            return l.constructionFinishedInDays < r.constructionFinishedInDays;
        };
        auto resultcomp2 = [](const UpgradeResult& l, const UpgradeResult& r){
            return l.lastConstructionStartedAfterDays < r.lastConstructionStartedAfterDays;
        };
        auto resultcomp3 = [](const UpgradeResult& l, const UpgradeResult& r){
            return l.savingFinishedInDays < r.savingFinishedInDays;
        };
        auto resultcomp4 = [](const UpgradeResult& l, const UpgradeResult& r){
            return l.previousUpgradeDelay < r.previousUpgradeDelay;
        };
        
        std::function<bool(const UpgradeResult&, const UpgradeResult&)> resultcomp;
        switch(permutationMode){
            case 1: resultcomp = resultcomp1; break;
            case 2: resultcomp = resultcomp2; break;
            case 3: resultcomp = resultcomp3; break;
            case 4: resultcomp = resultcomp4; break;
            default: assert(false);
        }
        
        std::vector<std::vector<UpgradeResult>> bestResultsPerThread(num_threads);
        std::vector<std::vector<std::vector<UpgradeJobList>>> bestUpgradePermutationsPerThread(num_threads);
        std::vector<std::vector<Account>> bestAccountsPerThread(num_threads);        
        
        std::vector<float> longestCompletionTimePerThread(num_threads, 0.0f);
        
        std::set<std::vector<UpgradeJobList>> uniqueProcessedPermutations;
        //std::atomic_int permcount{0};
        std::mutex m;
        
        parallel_for_each_permutation(planned_upgrades, num_threads, [&](int threadId, const auto& upgradepermutation){
            
            {
                std::lock_guard<std::mutex> lg(m);
                if(uniqueProcessedPermutations.count(upgradepermutation) > 0){
                    return;
                }else{
                    uniqueProcessedPermutations.insert(upgradepermutation);
                }
            }
            
            //create copy of original account
            auto permutationAccount = account;
            
            //perform permutation of upgrades on permutation account
            UpgradeResult nextResult = perform_upgrades(permutationAccount, upgradepermutation);
            
            if(nextResult.success){
            
                auto& myBestResults = bestResultsPerThread[threadId];
                auto& myBestUpgradePermutations = bestUpgradePermutationsPerThread[threadId];
                auto& myBestAccounts = bestAccountsPerThread[threadId];
                auto& longestCompletionTime = longestCompletionTimePerThread[threadId];
                
                //save result if it is better than one of the previously saved results (can be at most num_best_permutations previously saved results)
                auto iter = lower_bound(myBestResults.begin(), myBestResults.end(), nextResult, resultcomp);
                std::size_t distance = std::distance(myBestResults.begin(),iter);
                
                if(iter == myBestResults.end() && int(myBestResults.size()) < num_best_permutations){
                    myBestResults.emplace_back(nextResult);
                    myBestUpgradePermutations.emplace_back(upgradepermutation);
                    myBestAccounts.emplace_back(permutationAccount);
                }else if(iter != myBestResults.end() && *iter != nextResult){
                    myBestResults.insert(iter, nextResult);
                    myBestUpgradePermutations.insert(myBestUpgradePermutations.begin() + distance, upgradepermutation);
                    myBestAccounts.insert(myBestAccounts.begin() + distance, permutationAccount);
                    
                    if(int(myBestResults.size()) > num_best_permutations)
                        myBestResults.resize(num_best_permutations);
                    if(int(myBestUpgradePermutations.size()) > num_best_permutations)
                        myBestUpgradePermutations.resize(num_best_permutations);
                    if(int(myBestAccounts.size()) > num_best_permutations)
                        myBestAccounts.resize(num_best_permutations);
                }
                
                //find out the worst completion time of all permutations
                longestCompletionTime = std::max(longestCompletionTime, permutationAccount.time);
                
            }
            
            //permcount++;
            
            /*std::lock_guard<std::mutex> lg(m);
            
            std::cout << "thread " << threadId << " : ";
            for(const auto& jobList : upgradepermutation){
                std::cout << "[ ";
                for(const auto& job : jobList)
                    std::cout << job.entityInfo.name << ", ";
                std::cout << " ]";
            }
            std::cout << '\n';*/
        });
        
        //std::cout << permcount << std::endl;
        
        std::vector<UpgradeResult> bestResults;
        std::vector<std::vector<UpgradeJobList>> bestUpgradePermutations;
        std::vector<Account> bestAccounts;
        
        //combine results of threads
        for(int i = 0; i < num_threads; i++){
            bestResults.insert(bestResults.end(), bestResultsPerThread[i].begin(), bestResultsPerThread[i].end());
            bestUpgradePermutations.insert(bestUpgradePermutations.end(), bestUpgradePermutationsPerThread[i].begin(), bestUpgradePermutationsPerThread[i].end());
            bestAccounts.insert(bestAccounts.end(), bestAccountsPerThread[i].begin(), bestAccountsPerThread[i].end());
        }
        
        //sort results (result indices)
        std::vector<int> indices(bestResults.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(), [&](int l, int r){
            return resultcomp(bestResults[l], bestResults[r]);
        });
        
        std::unique(indices.begin(), indices.end(), [&](int l, int r){
            return bestUpgradePermutations[l] == bestUpgradePermutations[r];
        });

        assert(indices.size() == bestResults.size());
        
        
        const float longestCompletionTime = *std::max_element(longestCompletionTimePerThread.begin(), longestCompletionTimePerThread.end());
        
        std::cout << "Best permutations:\n";
        for(int i = 0; i < std::min(num_best_permutations, int(indices.size())); i++){
            int resultIndex = indices[i];
            const auto& bestResult = bestResults[resultIndex];
            const auto& bestUpgradePermutation = bestUpgradePermutations[resultIndex];
            auto& bestAccount = bestAccounts[resultIndex];
            
            for(const auto& jobList : bestUpgradePermutation){
                std::cout << "[ ";
                for(const auto& job : jobList)
                    std::cout << job.entityInfo.name << ", ";
                std::cout << " ]";
            }
            std::cout << '\n';
            
            float timeToAdvance = std::max(0.0f, longestCompletionTime - bestAccount.time);
            
            std::int64_t currentResourcesDSE = bestAccount.resources.met / (bestAccount.traderate)[0] * (bestAccount.traderate)[2] + bestAccount.resources.crystal / (bestAccount.traderate)[1] * (bestAccount.traderate)[2] + bestAccount.resources.deut;
            auto currentProduction = bestAccount.getCurrentDailyProduction();
            std::int64_t currentProductionPerDayDSE = currentProduction.met / (bestAccount.traderate)[0] * (bestAccount.traderate)[2] + currentProduction.crystal / (bestAccount.traderate)[1] * (bestAccount.traderate)[2] + currentProduction.deut;
            std::int64_t currentProductionPerHourDSE = currentProductionPerDayDSE / 24.0f;
            
            if(use_dhm_format){
                std::cout << "Account after " << convert_time(bestAccount.time) << ": Resources: " << currentResourcesDSE << " DSE, Production: " << currentProductionPerHourDSE << " DSE/h.\n";
            }else{
                std::cout << "Account after " << bestAccount.time << " days: Resources: " << currentResourcesDSE << " DSE, Production: " << currentProductionPerHourDSE << " DSE/h.\n";
            }
            
            
            bestAccount.advanceTime(timeToAdvance);
            
            if(use_dhm_format){
                std::cout << "The selected upgrades take " << convert_time(bestResult.constructionFinishedInDays) << " days.\n";
                std::cout << "Last upgrade started after " << convert_time(bestResult.lastConstructionStartedAfterDays) << " days.\n";
                std::cout << "The required saving time is " << convert_time(bestResult.savingFinishedInDays) << " days.\n";
                std::cout << "Days lost because a queue was full: " << convert_time(bestResult.previousUpgradeDelay) << " days.\n";
            }else{
                std::cout << "The selected upgrades take " << bestResult.constructionFinishedInDays << " days.\n";
                std::cout << "Last upgrade started after " << bestResult.lastConstructionStartedAfterDays << " days.\n";
                std::cout << "The required saving time is " << bestResult.savingFinishedInDays << " days.\n";
                std::cout << "Days lost because a queue was full: " << bestResult.previousUpgradeDelay << " days.\n";
            }
            
            
            
            
            currentResourcesDSE = bestAccount.resources.met / (bestAccount.traderate)[0] * (bestAccount.traderate)[2] + bestAccount.resources.crystal / (bestAccount.traderate)[1] * (bestAccount.traderate)[2] + bestAccount.resources.deut;
            currentProduction = bestAccount.getCurrentDailyProduction();
            currentProductionPerDayDSE = currentProduction.met / (bestAccount.traderate)[0] * (bestAccount.traderate)[2] + currentProduction.crystal / (bestAccount.traderate)[1] * (bestAccount.traderate)[2] + currentProduction.deut;
            currentProductionPerHourDSE = currentProductionPerDayDSE / 24.0f;
            
            if(use_dhm_format){
                std::cout << "Account after " << convert_time(longestCompletionTime) << ": Resources: " << currentResourcesDSE << " DSE, Production: " << currentProductionPerHourDSE << " DSE/h.\n";
            }else{
                std::cout << "Account after " << longestCompletionTime << " days: Resources: " << currentResourcesDSE << " DSE, Production: " << currentProductionPerHourDSE << " DSE/h.\n";
            }
            
            std::cout << '\n';
            
            if((printList && i == 0) || printAllLists){
                std::cout << "Detailed statistics:\n";
                
                for(int jobid = 0; jobid < int(bestResult.upgradeJobStatistics.size()); jobid++){
                    const auto& stat = bestResult.upgradeJobStatistics[jobid];
                    const auto& job = stat.job;
                    const auto& entityInfo = job.entityInfo;
                    const int upgradeLevel = stat.level;
                    const int upgradeLocation = job.location;
                    
                    if(use_dhm_format){
                        std::cout << "Planet " << (upgradeLocation+1) << ": " << entityInfo.name << " " << upgradeLevel << ". Saving period begin: " << convert_time(stat.savePeriodDaysBegin)
                        << ", Waiting period begin: " << convert_time(stat.waitingPeriodDaysBegin) << ", Construction begin: " << convert_time(stat.constructionBeginDays) << ", Construction time: " << convert_time(stat.constructionTimeDays) 
                        << ", Save time: " << convert_time(stat.waitingPeriodDaysBegin - stat.savePeriodDaysBegin)<< '\n';
                    }else{
                        std::cout << "Planet " << (upgradeLocation+1) << ": " << entityInfo.name << " " << upgradeLevel << ". Saving period begin: " << stat.savePeriodDaysBegin 
                        << ", Waiting period begin: " << stat.waitingPeriodDaysBegin << ", Construction begin: " << stat.constructionBeginDays << ", Construction time: " << stat.constructionTimeDays 
                        << ", Save time: " << (stat.waitingPeriodDaysBegin - stat.savePeriodDaysBegin)<< '\n';
                    }
                    
                    
                    
                }
            }
        }
    }
    
    std::cout.flush();
        
    
    return 0;
}












int main(int argc, char** argv){

	TIMERSTARTCPU(program_execution);
	
	int retVal = detailedmultiupgrade(argc, argv);
	
	TIMERSTOPCPU(program_execution);
	
	return retVal;

}