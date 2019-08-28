#include "account.hpp"
#include "hpc_helpers.cuh"
#include "io.hpp"
#include "json.hpp"
#include "ogame.hpp"
#include "parallel_permutation.hpp"
#include "serialization.hpp"
#include "util.hpp"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <numeric>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

namespace ogh = ogamehelpers;
using json = nlohmann::json;

#if 0
/* 
* Used for Astro researches that enable a new planet slot after research.
* 
* Determine whether constructions on the current planets can be started while Astro research is in progress 
* 
* AstroType::Blocking: No new constructions can be started before research is completed
* AstroType::Nonblocking: New constructions can be started on existing planets
*/
enum class AstroType {Blocking, Nonblocking};

/* 
* Used for Astro researches that enable a new planet slot after research.
* 
* Determines what should be done after research is completed.
* 
* PostAstroAction::SimpleCopyPreviousPlanet: Set levels of new planet to the levels of previous planet. 
*                                            Time is not advanced.
* 
* PostAstroAction::SimpleUpgradeToPreviousPlanet: Perform upgrade jobs to bring the levels of the new planet to the levels of the previous planet.
* 
* PostAstroAction::None: No action is performed. The new planet is colonized, but building levels remain 0. Time is not advanced.
*/
enum class PostAstroAction {SimpleCopyPreviousPlanet, SimpleUpgradeToPreviousPlanet, None};

        AstroType astroPhysicsType = AstroType::Nonblocking;
        PostAstroAction postAstroPhysicsAction = PostAstroAction::None;

#endif

struct UpgradeResult {
    Account::UpgradeStats stats;
    ogh::Entity entity;
    int location;

    bool operator==(const UpgradeResult& rhs) const {
        return stats == rhs.stats && entity == rhs.entity && location == rhs.location;
    }

    bool operator!=(const UpgradeResult& rhs) const {
        return !operator==(rhs);
    }
};

struct UpgradeListResult {
    bool success = false;
    std::chrono::seconds constructionFinishedInDays{0};
    std::chrono::seconds lastConstructionStartedAfterDays{0};
    std::chrono::seconds savingFinishedInDays{0};
    std::chrono::seconds previousUpgradeDelay{0};
    std::vector<UpgradeResult> upgradeResults;

    bool operator==(const UpgradeListResult& rhs) const {
        return success == rhs.success && constructionFinishedInDays == rhs.constructionFinishedInDays && lastConstructionStartedAfterDays == rhs.lastConstructionStartedAfterDays && savingFinishedInDays == rhs.savingFinishedInDays && previousUpgradeDelay == rhs.previousUpgradeDelay && upgradeResults == rhs.upgradeResults;
    }

    bool operator!=(const UpgradeListResult& rhs) const {
        return !(operator==(rhs));
    }
};

struct PerformUpgradeOptions{
    int astroMode = 0;
};

std::vector<UpgradeResult> upgradeNewPlanetAfterAstro(Account& account){
    std::vector<UpgradeResult> result;
    result.reserve(200);

    const int newPlanetNumber = account.getNumPlanets();
    const int referencePlanetNumber = newPlanetNumber - 1;
    if(referencePlanetNumber == 0){
        return {};
    }
    
    BuildingLevels referenceLevels = account.getAllLevelsOfPlanetAfterConstruction(referencePlanetNumber);

    auto process = [&](ogh::Entity entity){
        auto stats = account.processBuildingJob(newPlanetNumber, entity);
        UpgradeResult upgradeResult{
            stats,
            entity,
            newPlanetNumber,
        };
        result.emplace_back(std::move(upgradeResult));

        if (account.accountTime == std::chrono::seconds::max() || !stats.success)
            assert(false);
    };

    /*
    Upgrade plan: 
    Robo 
    Nani
    Shipyard
    Sats
    Met Crys Deut Solar. use Fusion plant instead of solar if max solar level is reached.
    Build remaining buildings in any order
    */

    for(int level = 1; level <= referenceLevels.roboLevel; level++){
        process(ogh::Entity::Robo);
    }
    for(int level = 1; level <= referenceLevels.naniteLevel; level++){
        process(ogh::Entity::Nanite);
    }
    for(int level = 1; level <= referenceLevels.shipyardLevel; level++){
        process(ogh::Entity::Shipyard);
    }

    account.buildSats(newPlanetNumber, account.getSats(referencePlanetNumber));

    int m = 0;
    int k = 0; 
    int d = 0;
    int s = 0;
    int f = 0;
    while(m < referenceLevels.metLevel || k < referenceLevels.crysLevel || d < referenceLevels.deutLevel || s < referenceLevels.solarLevel || f < referenceLevels.fusionLevel){
        if(m < referenceLevels.metLevel){
            process(ogh::Entity::Metalmine);
            m++;
        }
        if(k < referenceLevels.crysLevel){
            process(ogh::Entity::Crystalmine);
            k++;
        }
        if(d < referenceLevels.deutLevel){
            process(ogh::Entity::Deutsynth);
            d++;
        }
        if(s < referenceLevels.solarLevel){
            process(ogh::Entity::Solar);
            s++;
        }else{
            if(f < referenceLevels.fusionLevel){
                process(ogh::Entity::Fusion);
                f++;
            }
        }
    }

    for(int level = 1; level <= referenceLevels.labLevel; level++){
        process(ogh::Entity::Lab);
    }
    for(int level = 1; level <= referenceLevels.metalStorageLevel; level++){
        process(ogh::Entity::Metalstorage);
    }
    for(int level = 1; level <= referenceLevels.crystalStorageLevel; level++){
        process(ogh::Entity::Crystalstorage);
    }
    for(int level = 1; level <= referenceLevels.deutStorageLevel; level++){
        process(ogh::Entity::Deutstorage);
    }
    for(int level = 1; level <= referenceLevels.allianceDepotLevel; level++){
        process(ogh::Entity::Alliancedepot);
    }
    for(int level = 1; level <= referenceLevels.missileSiloLevel; level++){
        process(ogh::Entity::Silo);
    }

    return result;
}

UpgradeListResult perform_upgrades(Account& account,
                                   const std::vector<PermutationGroup>& planned_upgrades,
                                   const PerformUpgradeOptions& options) {

    using ogh::EntityType;

    UpgradeListResult result;
    result.upgradeResults.reserve(planned_upgrades.size());

    auto print_job = [](const auto& job) {
        (void)job;
        //std::cout << ogh::getEntityName(entityInfo.entity) << " at planet " << job.location << std::endl;
    };

    auto submitJobAndCheckTime = [&](const auto& job) {
        if (job.isResearch()) {
            assert(job.entityInfo.type == EntityType::Research && job.location == UpgradeTask::researchLocation);

            print_job(job);

            const int oldNumPlanets = account.getNumPlanets();

            auto stats = account.processResearchJob(job.entityInfo.entity);
            UpgradeResult upgradeResult{
                stats,
                job.entityInfo.entity,
                job.location,
            };
            result.upgradeResults.emplace_back(std::move(upgradeResult));

            bool returnvalue = true;

            if (account.accountTime == std::chrono::seconds::max() || !stats.success)
                returnvalue = false;

            if(returnvalue && options.astroMode == 0){
                if(job.entityInfo.entity == ogh::Entity::Astro && oldNumPlanets + 1 == account.getNumPlanets()){
                    auto newplanetupraderesults = upgradeNewPlanetAfterAstro(account);

                    result.upgradeResults.insert(result.upgradeResults.end(), newplanetupraderesults.begin(), newplanetupraderesults.end());
                }
            }

            return returnvalue;
        } else {
            if (job.location == UpgradeTask::allCurrentPlanetsLocation) {
                auto curJob = job;
                bool ok = true;
                for (int planetNumber = 1; planetNumber <= account.getNumPlanets() && ok; planetNumber++) {
                    curJob.location = planetNumber;

                    assert(curJob.entityInfo.type == EntityType::Building && (curJob.location > 0 && curJob.location <= account.getNumPlanets()));

                    print_job(curJob);

                    auto stats = account.processBuildingJob(curJob.location, curJob.entityInfo.entity);
                    UpgradeResult upgradeResult{
                        stats,
                        curJob.entityInfo.entity,
                        curJob.location,
                    };
                    result.upgradeResults.emplace_back(std::move(upgradeResult));

                    if (account.accountTime == std::chrono::seconds::max() || !stats.success)
                        ok = false;
                }
                return ok;
            } else {
                assert(job.entityInfo.type == EntityType::Building && (job.location >= 0 && job.location < account.getNumPlanets() + 1)); // +1 to account for possible astro physics in progress
                print_job(job);
                auto stats = account.processBuildingJob(job.location, job.entityInfo.entity);
                UpgradeResult upgradeResult{
                    stats,
                    job.entityInfo.entity,
                    job.location,
                };
                result.upgradeResults.emplace_back(std::move(upgradeResult));

                if (account.accountTime == std::chrono::seconds::max() || !stats.success)
                    return false;
                return true;
            }
        }
        assert(false);
        return false;
    };

    auto getNumPlanets = [&]() {
        int numPlanets = account.getNumPlanets();
        if (account.getResearchInConstruction() == ogh::Entity::Astro && numPlanets < ogh::getMaxPossiblePlanets(account.getResearchLevel(ogh::Entity::Astro) + 1)) {
            numPlanets += 1;
        }
        return numPlanets;
    };

    auto submitUpgradeTask = [&](const auto& task) {
        const int numPlanets = getNumPlanets();
        const auto upgradeJobs = task.getUpgradeJobs(numPlanets);
        return std::all_of(upgradeJobs.begin(), upgradeJobs.end(), submitJobAndCheckTime);
    };

    auto submitUpgradeGroup = [&](const auto& upgradeGroup) {
        const int numPlanets = getNumPlanets();
        const auto upgradeTasks = upgradeGroup.getTasks(numPlanets);
        return std::all_of(upgradeTasks.begin(), upgradeTasks.end(), submitUpgradeTask);
    };

    auto submitPermutationGroup = [&](const auto& permGroup) {
        const auto upgradeGroups = permGroup.groups;
        return std::all_of(upgradeGroups.begin(), upgradeGroups.end(), submitUpgradeGroup);
    };

    const bool ok = std::all_of(planned_upgrades.begin(), planned_upgrades.end(), submitPermutationGroup);

    result.success = ok;

    if (ok) {

        result.lastConstructionStartedAfterDays = account.accountTime;
        result.savingFinishedInDays = std::chrono::seconds::zero();
        result.previousUpgradeDelay = std::chrono::seconds::zero();

        for (const auto& res : result.upgradeResults) {
            result.savingFinishedInDays += res.stats.waitingPeriodDaysBegin - res.stats.savePeriodDaysBegin;
            result.previousUpgradeDelay += res.stats.constructionBeginDays - res.stats.waitingPeriodDaysBegin;
        }

        //wait until all planets finished building

        account.waitForAllConstructions();

        result.constructionFinishedInDays = account.accountTime;
    }

    std::cout << std::flush;
    std::cerr << std::flush;

    return result;
}

std::string convert_time(float daysfloat) {
    std::stringstream ss;
    std::uint64_t seconds = daysfloat * 24ULL * 60ULL * 60ULL;
    std::uint64_t days = seconds / (24ULL * 60ULL * 60ULL);
    seconds = seconds % (24ULL * 60ULL * 60ULL);
    std::uint64_t hours = seconds / (60ULL * 60ULL);
    seconds = seconds % (60ULL * 60ULL);
    std::uint64_t minutes = seconds / 60ULL;
    seconds = seconds % 60ULL;
    ss << days << "d " << hours << "h " << minutes << "m " << seconds << "s";
    return ss.str();
}


void printLogRecord(std::ostream& os, const Account::LogRecord& record) {
    os << secondsToDHM(record.time) << ": " << record.msg << '\n';
}

void createAccountFile(const std::string& filename) {
    Account account;
    account.addNewPlanet();
    json j = account;
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("Cannot open new account file " + filename);
    }
    out << std::setw(4) << j;
}

void usage(int argc, char** argv) {
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
    std::cout << "--astromode mode: Default 0.\n\n";
    std::cout << "  mode = 0: A new planet is automatically upgraded to the levels of the second newest planet. This is not done in optimal manner.\n";
    std::cout << "  mode = 1: All buildings on the new planet have to be specified explicitly in the upgrade file. Otherwise the new planet will remain at 0.\n";
    std::cout << "--permute mode: Default 0.\n\n";
    std::cout << "  mode = 0: No permutation.\n";
    std::cout << "  mode = 1: Find permutation with shortest completion time.\n";
    std::cout << "  mode = 2: Find permutation with shortest time until last upgrade is started.\n";
    std::cout << "  mode = 3: Find permutation with shortest save time.\n";
    std::cout << "  mode = 4: Find permutation with shortest blocked queue time.\n\n";
    std::cout << "--permutations n: Display n best permutations. Default 1.\n\n";
    std::cout << "--printlist: Show detailed time table for upgrade.\n\n";
    std::cout << "--printalllists: Show detailed time table for every permutation\n\n";
    std::cout << "--percentages: Show changes of production percentages per planet.\n\n";
    std::cout << "--dhm: Print durations in days, hours, minutes format instead of fractional days.\n\n";

    std::cout << "The initial account state is read from accountfile\n\n";
    std::cout << "The list of upgrades to perform is read from upgradefile\n\n";

    std::cout << "Example: " << argv[0] << " --accountfile account.json --upgradefile upgrades.txt --speed 2" << std::endl;
}

int detailedmultiupgrade(int argc, char** argv) {
    //constexpr bool debugprint = true;
    //constexpr bool nanisweep = false;

    if (argc == 1) {
        usage(argc, argv);
        return 0;
    }

    //check for options which do not start a simulation
    {

        for (int i = 1; i < argc; i++) {
            if (std::string(argv[i]) == "--help") {
                usage(argc, argv);
                return 0;
            }

            if (std::string(argv[i]) == "--createAccount") {
                assert(i + 1 < argc);
                //bool createAccount = true;
                std::string newAccountFile = std::string(argv[i + 1]);
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
    int astroMode = 0;

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--printlist") {
            printList = true;
            continue;
        }

        if (std::string(argv[i]) == "--printalllists") {
            printAllLists = true;
            continue;
        }

        if (std::string(argv[i]) == "--percentages") {
            showPercentageChanges = true;
            continue;
        }

        if (std::string(argv[i]) == "--speed") {
            assert(i + 1 < argc);
            speedfactor = std::atoi(argv[i + 1]);
            i++;
            overwriteSpeed = true;
            continue;
        }

        if (std::string(argv[i]) == "--threads") {
            assert(i + 1 < argc);
            num_threads = std::atoi(argv[i + 1]);
            i++;
            continue;
        }

        if (std::string(argv[i]) == "--accountfile") {
            assert(i + 1 < argc);
            accountFile = std::string(argv[i + 1]);
            i++;
            continue;
        }

        if (std::string(argv[i]) == "--upgradefile") {
            assert(i + 1 < argc);
            upgradeFile = std::string(argv[i + 1]);
            i++;
            continue;
        }

        if (std::string(argv[i]) == "--logfile") {
            assert(i + 1 < argc);
            logFileName = std::string(argv[i + 1]);
            i++;
            continue;
        }

        if (std::string(argv[i]) == "--permute") {
            assert(i + 1 < argc);
            permutationMode = std::atoi(argv[i + 1]);
            i++;
            continue;
        }

        if (std::string(argv[i]) == "--astromode") {
            assert(i + 1 < argc);
            astroMode = std::atoi(argv[i + 1]);
            i++;
            continue;
        }

        if (std::string(argv[i]) == "--permutations") {
            assert(i + 1 < argc);
            num_best_permutations = std::atoi(argv[i + 1]);
            i++;
            continue;
        }

        if (std::string(argv[i]) == "--dhm") {
            use_dhm_format = true;
            continue;
        }

        if (std::string(argv[i]) == "--appendlog") {
            appendLog = true;
            continue;
        }
    }

    if (overwriteSpeed) {
        std::cout << "Speed factor: " << speedfactor << '\n';
    }
    std::cout << "Permutation mode: " << permutationMode << '\n';
    std::cout << "Show the n best permutations: " << num_best_permutations << '\n';
    std::cout << "Astro mode: " << astroMode << '\n';
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
    if (appendLog)
        openmode = std::ios_base::app;

    std::ofstream logFile(logFileName, openmode);

    if (!logFile) {
        throw std::runtime_error("Cannot open log file " + logFileName);
    }

    auto printLog = [&](const auto& record) {
        printLogRecord(logFile, record);
    };

    Account account = parseAccountJsonFile(accountFile);

    if (overwriteSpeed) {
        account.speedfactor = speedfactor;
    }

    auto planned_upgrades = parseUpgradeFile(upgradeFile);

    {
        for (const auto& x : planned_upgrades)
            std::cout << x << std::endl;
    }

    PerformUpgradeOptions upOpts;
    upOpts.astroMode = astroMode;

    if (permutationMode == 0) {
        auto result = perform_upgrades(account, planned_upgrades, upOpts);

        if (result.success) {

            std::for_each(account.logRecords.begin(), account.logRecords.end(), printLog);
            logFile.flush();
            std::cout << std::endl;

            std::cout << "The selected upgrades take " << secondsToDHM(result.constructionFinishedInDays) << " days.\n";
            std::cout << "Last upgrade started after " << secondsToDHM(result.lastConstructionStartedAfterDays) << " days.\n";
            std::cout << "The required saving time is " << secondsToDHM(result.savingFinishedInDays) << " days.\n";
            std::cout << "Days lost because a queue was full: " << secondsToDHM(result.previousUpgradeDelay) << " days.\n";


            std::cout << '\n';

            if (printList) {
                std::cout << "Detailed statistics:\n";

                for (int jobid = 0; jobid < int(result.upgradeResults.size()); jobid++) {
                    const auto& res = result.upgradeResults.at(jobid);
                    const auto& stat = res.stats;
                    const auto& entity = res.entity;
                    const int upgradeLevel = stat.level;
                    const int planetNumber = res.location;
                    std::cout << "Planet " << planetNumber << ": " << ogh::getEntityName(entity) << " " << upgradeLevel << ". Saving period begin: " << secondsToDHM(stat.savePeriodDaysBegin)
                                << ", Waiting period begin: " << secondsToDHM(stat.waitingPeriodDaysBegin) << ", Construction begin: " << secondsToDHM(stat.constructionBeginDays) << ", Construction time: " << secondsToDHM(stat.constructionTimeDays)
                                << ", Save time: " << secondsToDHM(stat.waitingPeriodDaysBegin - stat.savePeriodDaysBegin) << '\n';
                }
            }

            if (showPercentageChanges) {
                auto percentageChanges = account.getPercentageChanges();

                auto printChange = [&](const auto& change) {
                    std::string timestring = secondsToDHM(change.time);
                    auto& stream = std::cout;
                    stream << timestring << " Planet " << change.planetId << ": ";
                    
                    if(change.oldMetPercent != change.metPercent 
                            || change.oldCrysPercent != change.crysPercent 
                            || change.oldDeutPercent != change.deutPercent 
                            || change.oldFusionPercent != change.fusionPercent){

                        stream << "Changed percents from m " << change.oldMetPercent << ", c " << change.oldCrysPercent
                            << ", d " << change.oldDeutPercent << ", f " << change.oldFusionPercent
                            << " to m " << change.metPercent << ", c " << change.crysPercent
                            << ", d " << change.deutPercent << ", f " << change.fusionPercent;

                    }else{
                        stream << "Production factor changed";
                    }

                    stream << " after construction of " << change.finishedName << " " << change.finishedLevel
                           << ". Production factor: " << change.oldMineProductionFactor
                           << "->" << change.newMineProductionFactor
                           << ". Production increased by " << (change.oldDSE != 0 ? (((double(change.newDSE) / change.oldDSE) - 1) * 100) : 0)
                           << " % DSE" << std::endl;
                };

                std::cout << '\n';

                std::for_each(percentageChanges.begin(), percentageChanges.end(), printChange);
            }
        } else {
            std::cout << "Error: Upgrades could not be performed!" << std::endl;
        }
    } else {

        auto resultcomp1 = [](const UpgradeListResult& l, const UpgradeListResult& r) {
            return l.constructionFinishedInDays < r.constructionFinishedInDays;
        };
        auto resultcomp2 = [](const UpgradeListResult& l, const UpgradeListResult& r) {
            return l.lastConstructionStartedAfterDays < r.lastConstructionStartedAfterDays;
        };
        auto resultcomp3 = [](const UpgradeListResult& l, const UpgradeListResult& r) {
            return l.savingFinishedInDays < r.savingFinishedInDays;
        };
        auto resultcomp4 = [](const UpgradeListResult& l, const UpgradeListResult& r) {
            return l.previousUpgradeDelay < r.previousUpgradeDelay;
        };

        std::function<bool(const UpgradeListResult&, const UpgradeListResult&)> resultcomp;
        switch (permutationMode) {
        case 1:
            resultcomp = resultcomp1;
            break;
        case 2:
            resultcomp = resultcomp2;
            break;
        case 3:
            resultcomp = resultcomp3;
            break;
        case 4:
            resultcomp = resultcomp4;
            break;
        default:
            assert(false);
        }

        std::vector<std::vector<UpgradeListResult>> bestResultsPerThread(num_threads);
        std::vector<std::vector<std::vector<PermutationGroup>>> bestUpgradePermutationsPerThread(num_threads);
        std::vector<std::vector<Account>> bestAccountsPerThread(num_threads);

        std::vector<std::chrono::seconds> longestCompletionTimePerThread(num_threads, std::chrono::seconds::zero());

        std::set<std::vector<PermutationGroup>> uniqueProcessedPermutations;
        std::mutex m;

        auto handle_permutation = [&](int threadId, const auto& upgradepermutation){
            //create copy of original account
            auto permutationAccount = account;

            //perform permutation of upgrades on permutation account
            UpgradeListResult nextResult = perform_upgrades(permutationAccount, upgradepermutation, upOpts);

            if (nextResult.success) {

                auto& myBestResults = bestResultsPerThread[threadId];
                auto& myBestUpgradePermutations = bestUpgradePermutationsPerThread[threadId];
                auto& myBestAccounts = bestAccountsPerThread[threadId];
                auto& longestCompletionTime = longestCompletionTimePerThread[threadId];

                //save result if it is better than one of the previously saved results (can be at most num_best_permutations previously saved results)
                auto iter = lower_bound(myBestResults.begin(), myBestResults.end(), nextResult, resultcomp);
                std::size_t distance = std::distance(myBestResults.begin(), iter);

                if (iter == myBestResults.end() && int(myBestResults.size()) < num_best_permutations) {
                    myBestResults.emplace_back(nextResult);
                    myBestUpgradePermutations.emplace_back(upgradepermutation);
                    myBestAccounts.emplace_back(permutationAccount);
                } else if (iter != myBestResults.end() && *iter != nextResult) {
                    myBestResults.insert(iter, nextResult);
                    myBestUpgradePermutations.insert(myBestUpgradePermutations.begin() + distance, upgradepermutation);
                    myBestAccounts.insert(myBestAccounts.begin() + distance, permutationAccount);

                    if (int(myBestResults.size()) > num_best_permutations)
                        myBestResults.resize(num_best_permutations);
                    if (int(myBestUpgradePermutations.size()) > num_best_permutations)
                        myBestUpgradePermutations.resize(num_best_permutations);
                    if (int(myBestAccounts.size()) > num_best_permutations)
                        myBestAccounts.resize(num_best_permutations);
                }

                //find out the worst completion time of all permutations
                longestCompletionTime = std::max(longestCompletionTime, permutationAccount.accountTime);
            }
        };

        parallel_for_each_unique_permutation(planned_upgrades, num_threads, handle_permutation);

        std::vector<UpgradeListResult> bestResults;
        std::vector<std::vector<PermutationGroup>> bestUpgradePermutations;
        std::vector<Account> bestAccounts;

        //combine results of threads
        for (int i = 0; i < num_threads; i++) {
            bestResults.insert(bestResults.end(), bestResultsPerThread[i].begin(), bestResultsPerThread[i].end());
            bestUpgradePermutations.insert(bestUpgradePermutations.end(), bestUpgradePermutationsPerThread[i].begin(), bestUpgradePermutationsPerThread[i].end());
            bestAccounts.insert(bestAccounts.end(), bestAccountsPerThread[i].begin(), bestAccountsPerThread[i].end());
        }

        //sort results (result indices)
        std::vector<int> indices(bestResults.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(), [&](int l, int r) {
            return resultcomp(bestResults[l], bestResults[r]);
        });

        std::unique(indices.begin(), indices.end(), [&](int l, int r) {
            return bestUpgradePermutations[l] == bestUpgradePermutations[r];
        });

        assert(indices.size() == bestResults.size());

        const std::chrono::seconds longestCompletionTime = *std::max_element(longestCompletionTimePerThread.begin(), longestCompletionTimePerThread.end());

        std::cout << "Best permutations:\n";
        for (int i = 0; i < std::min(num_best_permutations, int(indices.size())); i++) {
            int resultIndex = indices[i];
            const auto& bestResult = bestResults[resultIndex];
            const auto& bestUpgradePermutation = bestUpgradePermutations[resultIndex];
            auto& bestAccount = bestAccounts[resultIndex];

            for (const auto& jobList : bestUpgradePermutation) {
                std::cout << "[ ";
                std::cout << jobList;
                std::cout << " ]";
            }
            std::cout << '\n';

            std::chrono::seconds timeToAdvance = std::max(std::chrono::seconds::zero(), longestCompletionTime - bestAccount.accountTime);

            std::int64_t currentResourcesDSE = bestAccount.resources.dse(bestAccount.traderate);
            auto currentProduction = bestAccount.getCurrentDailyProduction();
            std::int64_t currentProductionDSE = currentProduction.produce(std::chrono::hours{1}).dse(bestAccount.traderate);

            std::cout << "Account after " << secondsToDHM(bestAccount.accountTime) << ": Resources: " << currentResourcesDSE << " DSE, Production: " << currentProductionDSE << " DSE/h.\n";
            

            bestAccount.advanceTime(timeToAdvance);

            std::cout << "The selected upgrades take " << secondsToDHM(bestResult.constructionFinishedInDays) << " days.\n";
            std::cout << "Last upgrade started after " << secondsToDHM(bestResult.lastConstructionStartedAfterDays) << " days.\n";
            std::cout << "The required saving time is " << secondsToDHM(bestResult.savingFinishedInDays) << " days.\n";
            std::cout << "Days lost because a queue was full: " << secondsToDHM(bestResult.previousUpgradeDelay) << " days.\n";

            currentResourcesDSE = bestAccount.resources.dse(bestAccount.traderate);
            currentProduction = bestAccount.getCurrentDailyProduction();
            currentProductionDSE = currentProduction.produce(std::chrono::hours{1}).dse(bestAccount.traderate);

            std::cout << "Account after " << secondsToDHM(longestCompletionTime) << ": Resources: " << currentResourcesDSE << " DSE, Production: " << currentProductionDSE << " DSE/h.\n";

            std::cout << '\n';

            if ((printList && i == 0) || printAllLists) {
                std::cout << "Detailed statistics:\n";

                for (int jobid = 0; jobid < int(bestResult.upgradeResults.size()); jobid++) {
                    const auto& res = bestResult.upgradeResults.at(jobid);
                    const auto& stat = res.stats;
                    const auto& entity = res.entity;
                    const int upgradeLevel = stat.level;
                    const int planetNumber = res.location;

                    std::cout << "Planet " << planetNumber << ": " << ogh::getEntityName(entity) << " " << upgradeLevel << ". Saving period begin: " << secondsToDHM(stat.savePeriodDaysBegin)
                                << ", Waiting period begin: " << secondsToDHM(stat.waitingPeriodDaysBegin) << ", Construction begin: " << secondsToDHM(stat.constructionBeginDays) << ", Construction time: " << secondsToDHM(stat.constructionTimeDays)
                                << ", Save time: " << secondsToDHM(stat.waitingPeriodDaysBegin - stat.savePeriodDaysBegin) << '\n';
                }
            }
        }
    }

    std::cout.flush();

    return 0;
}

int main(int argc, char** argv) {

    TIMERSTARTCPU(program_execution);

    int retVal = detailedmultiupgrade(argc, argv);

    TIMERSTOPCPU(program_execution);

    return retVal;
}
