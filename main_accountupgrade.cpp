#include "ogame.hpp"
#include "account.hpp"
#include "parallel_permutation.hpp"
#include "util.hpp"
#include "hpc_helpers.cuh"
#include "serialization.hpp"
#include "json.hpp"
#include "io.hpp"

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


UpgradeResult perform_upgrades(Account& account,
								const std::vector<UpgradeJobList>& planned_upgrades){

	using ogh::EntityType;
			
	UpgradeResult result;
	result.upgradeJobStatistics.reserve(planned_upgrades.size());
	
	auto print_job = [](const auto& job){
        (void)job;
		//std::cout << job.entityInfo.name << " at planet " << job.location << std::endl;
	};


    auto submitJobAndCheckTime = [&](const auto& job){
        if(job.isResearch()){
            assert(job.entityInfo.type == EntityType::Research && job.location == Account::UpgradeJob::researchLocation);

            print_job(job);

            auto stats = account.processResearchJob(job);
            result.upgradeJobStatistics.emplace_back(std::move(stats));
    
            if(account.time == std::numeric_limits<float>::max())
                return false;
            return true;                   
        }else{
            if(job.location == Account::UpgradeJob::allCurrentPlanetsLocation){
                auto curJob = job;
                bool ok = true;
                for(int i = 0; i < account.getNumPlanets() && ok; i++){
                    curJob.location = i;

                    assert(curJob.entityInfo.type == EntityType::Building 
                            && (curJob.location >= 0 
                                   && curJob.location < account.getNumPlanets()+1)); // +1 to account for possible astro physics in progress

                    print_job(curJob);

                    auto stats = account.processBuildingJob(curJob);
                    result.upgradeJobStatistics.emplace_back(std::move(stats));
            
                    if(account.time == std::numeric_limits<float>::max())
                        ok = false;
				}
                return ok;
            }else{
                assert(job.entityInfo.type == EntityType::Building 
                        && (job.location >= 0 
                            && job.location < account.getNumPlanets()+1)); // +1 to account for possible astro physics in progress
                print_job(job);
                auto stats = account.processBuildingJob(job);
                result.upgradeJobStatistics.emplace_back(std::move(stats));
        
                if(account.time == std::numeric_limits<float>::max())
                    return false;
                return true;
            }
        }
        assert(false);
        return false;
    };

    auto submitJobList = [&](const auto& jobList){
        return std::all_of(jobList.begin(), jobList.end(), submitJobAndCheckTime);
    };

    const bool ok = std::all_of(planned_upgrades.begin(), planned_upgrades.end(), submitJobList);
	
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
    std::cout << "--newAccount file. Creates an empty account json file with given name and exists.\n\n";
    std::cout << "--accountfile file: Load initial account data from file.\n\n";
    std::cout << "--upgradefile file: Load upgrade list from file.\n\n";
    std::cout << "--logfile file: Write program trace to file.\n\n";
    std::cout << "--speed ecospeedfactor: Economy speed factor of universe. Overwrites account setting.\n\n";
    std::cout << "--threads num_threads: Number of CPU threads to use for permutations. Default 1.\n\n";
    std::cout << "--permute mode: Default 0.\n\n";
    std::cout << "  mode = 0: No permutation.\n";
    std::cout << "  mode = 1: Find permutation with shortest completion time.\n";
    std::cout << "  mode = 2: Find permutation with shortest time until last upgrade is started.\n";
    std::cout << "  mode = 3: Find permutation with shortest save time.\n";
    std::cout << "  mode = 4: Find permutation with shortest blocked queue time.\n\n";
    std::cout << "--permutations n: Display n best permutations. Default 1.\n\n";
    std::cout << "--printlist: Show detailed time table for upgrade.\n\n";
    std::cout << "--printalllists: Show detailed time table for every permutation\n\n";
    std::cout << "--showPercentageChanges: Show changes of production percentages per planet.\n\n";
    std::cout << "--dhm: Print durations in days, hours, minutes format instead of fractional days.\n\n";
	    
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
