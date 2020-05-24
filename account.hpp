#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP

#include "ogame.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

struct ResearchState;
struct OfficerState;
struct Account;

struct BuildingLevels{
    int metLevel = 0;
    int crysLevel = 0;
    int deutLevel = 0;
    int solarLevel = 0;
    int fusionLevel = 0;
    int labLevel = 0;
    int roboLevel = 0;
    int naniteLevel = 0;
    int shipyardLevel = 0;
    int metalStorageLevel = 0;
    int crystalStorageLevel = 0;
    int deutStorageLevel = 0;
    int allianceDepotLevel = 0;
    int missileSiloLevel = 0;
};

struct PlanetState {

    static constexpr int solarplantPercent = 100;
    static constexpr int satsPercent = 100;

    struct SetPercentsResult {
        bool changedPercents;
        bool changedProductionFactor;
        int oldMetPercent;
        int oldCrysPercent;
        int oldDeutPercent;
        int oldFusionPercent;
        int oldCrawlerPercent;
        int metPercent;
        int crysPercent;
        int deutPercent;
        int fusionPercent;
        int crawlerPercent;
        int planetId;
        double oldDSE;
        double newDSE;
        double oldMineProductionFactor;
        double newMineProductionFactor;
    };

    int planetId = 0;
    int position = 8;
    int temperature = 0;
    int metPercent = 0;
    int crysPercent = 0;
    int deutPercent = 0;
    int fusionPercent = 0;
    int crawlerPercent = 0;
    ogamehelpers::ItemRarity metItem = ogamehelpers::ItemRarity::None;
    ogamehelpers::ItemRarity crysItem = ogamehelpers::ItemRarity::None;
    ogamehelpers::ItemRarity deutItem = ogamehelpers::ItemRarity::None;
    ogamehelpers::ItemRarity energyItem = ogamehelpers::ItemRarity::None;
    std::chrono::seconds metItemDuration{0};
    std::chrono::seconds crysItemDuration{0};
    std::chrono::seconds deutItemDuration{0};
    std::chrono::seconds energyItemDuration{0};
    double mineProductionFactor = 0;
    int sats = 0;
    int crawler = 0;

    std::chrono::seconds buildingQueue{0};
    ogamehelpers::Entity entityInQueue{};

    Account* accountPtr;

    ogamehelpers::Production dailyProduction;

    BuildingLevels buildingLevels;

    PlanetState() = default;
    PlanetState(const PlanetState& rhs) = default;
    PlanetState& operator=(const PlanetState& rhs) = default;

    ogamehelpers::ItemRarity getMetItem() const;
    ogamehelpers::ItemRarity getCrysItem() const;
    ogamehelpers::ItemRarity getDeutItem() const;
    ogamehelpers::ItemRarity getEnergyItem() const;

    void advanceTime(std::chrono::seconds days);

    bool constructionInProgress() const;

    int getLevel(const ogamehelpers::Entity& entity) const;
    BuildingLevels getAllCurrentLevels() const;
    BuildingLevels getAllLevelsAfterConstruction() const;

    //increase level by one and return new level;
    int increaseLevel(ogamehelpers::Entity entity);

    void buildSats(int numsats);
    int getSats() const;

    void buildCrawler(int numCrawler);
    int getCrawler() const;

    void startConstruction(std::chrono::seconds timeDays, const ogamehelpers::Entity& entity);

    ogamehelpers::Entity getBuildingInConstruction() const;

    void calculateDailyProduction();
    ogamehelpers::Production getCurrentDailyProduction() const;
    double getMineProductionFactor() const;

    SetPercentsResult setPercentToMaxProduction(const ogamehelpers::Production& oldProd, double oldProductionFactor);
};

struct ResearchState {
    int espionageLevel = 0;
    int computerLevel = 0;
    int weaponsLevel = 0;
    int shieldingLevel = 0;
    int armourLevel = 0;
    int etechLevel = 0;
    int hyperspacetechLevel = 0;
    int combustionLevel = 0;
    int impulseLevel = 0;
    int hyperspacedriveLevel = 0;
    int laserLevel = 0;
    int ionLevel = 0;
    int plasmaLevel = 0;
    int igrnLevel = 0;
    int astroLevel = 0;

    std::chrono::seconds researchQueue{0};
    ogamehelpers::Entity entityInQueue{};

    Account* accountPtr;

    ResearchState() = default;
    ResearchState(const ResearchState& rhs) = default;
    ResearchState& operator=(const ResearchState& rhs) = default;

    bool researchInProgress() const;

    void advanceTime(std::chrono::seconds days);

    int getLevel(ogamehelpers::Entity entity) const;

    //increase level by one and return new level;
    int increaseLevel(ogamehelpers::Entity entity);

    void startResearch(std::chrono::seconds timeDays, const ogamehelpers::Entity& entity);
};

struct OfficerState {
    std::chrono::seconds commanderDuration{0};
    std::chrono::seconds engineerDuration{0};
    std::chrono::seconds technocratDuration{0};
    std::chrono::seconds geologistDuration{0};
    std::chrono::seconds admiralDuration{0};

    OfficerState() = default;
    OfficerState(const OfficerState&) = default;
    OfficerState& operator=(const OfficerState&) = default;

    void advanceTime(std::chrono::seconds days);
};

struct Account {

    struct UpgradeStats {
        bool success;
        int level;
        std::chrono::seconds waitingPeriodDaysBegin{0};
        std::chrono::seconds savePeriodDaysBegin{0};
        std::chrono::seconds constructionBeginDays{0};
        std::chrono::seconds constructionTimeDays{0};

        bool operator==(const UpgradeStats& rhs) const {
            return success == rhs.success && level == rhs.level && waitingPeriodDaysBegin == rhs.waitingPeriodDaysBegin && savePeriodDaysBegin == rhs.savePeriodDaysBegin && constructionBeginDays == rhs.constructionBeginDays && constructionTimeDays == rhs.constructionTimeDays;
        }

        bool operator!=(const UpgradeStats& rhs) const {
            return !(operator==(rhs));
        }
    };

    struct PercentageChange {
        int oldMetPercent;
        int oldCrysPercent;
        int oldDeutPercent;
        int oldFusionPercent;
        int oldCrawlerPercent;
        int metPercent;
        int crysPercent;
        int deutPercent;
        int fusionPercent;
        int crawlerPercent;
        int planetId;
        int finishedLevel;
        std::chrono::seconds time;
        double oldDSE;
        double newDSE;
        double oldMineProductionFactor;
        double newMineProductionFactor;
        std::string finishedName;
    };

    struct LogRecord {
        std::chrono::seconds time;
        std::string msg;
        LogRecord() = default;
        LogRecord(std::chrono::seconds t, std::string m)
            : time(t), msg(std::move(m)) {}
    };

    std::vector<PlanetState> planets{};
    ResearchState researches{};
    OfficerState officers{};
    ogamehelpers::CharacterClass characterClass = ogamehelpers::CharacterClass::None;

    ogamehelpers::Resources resources{};
    ogamehelpers::Production dailyFarmIncomePerSlot{};
    ogamehelpers::Production dailyExpeditionIncomePerSlot{};
    ogamehelpers::Production dailyFarmIncome{};
    ogamehelpers::Production dailyExpeditionIncome{};
    ogamehelpers::Production dailyMineProduction{};

    bool accountInitialized = false;
    bool canTradeResources = true;

    std::array<float, 3> traderate{{3.0f, 2.0f, 1.0f}};

    
    int researchDurationDivisor = 1;
    int speedfactor = 1;
    int saveslots = 1;

    std::chrono::seconds accountTime{0};

    std::vector<PercentageChange> percentageChanges;

    std::vector<LogRecord> logRecords;

    std::vector<std::chrono::seconds> eventTimes;

    Account();

    Account(int ecospeed, std::chrono::seconds initialtime);

    Account(const Account& rhs);

    Account& operator=(const Account& rhs);

    void log(const std::string& msg);

    void buildingFinishedCallback(PlanetState& planet);

    void researchFinishedCallback();

    void planetProductionChanged(const ogamehelpers::Production& oldProd, const ogamehelpers::Production& newProd);

    void addNewPlanet();

    void initEventTimes();

    //timepoint must be given in accounttime
    void registerNewEvent(std::chrono::seconds when);

    //must not advance further than next finished event
    void advanceTime(std::chrono::seconds days);

    std::chrono::seconds getTimeUntilNextFinishedEvent() const;

    bool hasUnfinishedConstructionEvent() const;

    int getTotalLabLevel() const;

    void calculateDailyProduction();

    ogamehelpers::Production getCurrentDailyMineProduction() const;

    ogamehelpers::Production getCurrentDailyFarmIncome() const;

    ogamehelpers::Production getCurrentDailyExpeditionIncome() const;

    ogamehelpers::Production getCurrentDailyProduction() const;

    void updateDailyFarmIncome();

    void updateDailyExpeditionIncome();

    void recordPercentageChange(const PlanetState::SetPercentsResult& res, ogamehelpers::Entity entity, int level);

    std::vector<PercentageChange> getPercentageChanges() const;

    void startConstruction(int planetNumber, std::chrono::seconds timeDays, const ogamehelpers::Entity& entity, const ogamehelpers::Resources& constructionCosts);

    void startResearch(std::chrono::seconds timeDays, const ogamehelpers::Entity& entity, const ogamehelpers::Resources& constructionCosts);

    int getNumPlanets() const;

    int getResearchLevel(ogamehelpers::Entity entity) const;

    int getBuildingLevel(int planetNumber, ogamehelpers::Entity entity) const;

    BuildingLevels getAllCurrentLevelsOfPlanet(int planetNumber) const;

    BuildingLevels getAllLevelsOfPlanetAfterConstruction(int planetNumber) const;

    ogamehelpers::Entity getResearchInConstruction() const;

    ogamehelpers::Entity getBuildingInConstruction(int planetNumber) const;

    std::chrono::seconds getSaveDuration(
        const ogamehelpers::Resources& have,
        const ogamehelpers::Resources& want,
        const ogamehelpers::Production& production
    );

    void addResources(const ogamehelpers::Resources& res);

    void updateAccountResourcesAfterConstructionStart(const ogamehelpers::Resources& constructionCosts);

    void printQueues(std::ostream& os) const;

    void waitForAllConstructions();

    std::chrono::seconds waitUntilCostsAreAvailable(const ogamehelpers::Resources& constructionCosts);

    bool hasCommander() const;

    bool hasEngineer() const;

    bool hasTechnocrat() const;

    bool hasGeologist() const;

    bool hasAdmiral() const;

    bool hasStaff() const;

    ogamehelpers::CharacterClass getCharacterClass() const;

    void buildSats(int planetNumber, int numsats);

    int getSats(int planetNumber) const;

    UpgradeStats processResearchJob(ogamehelpers::Entity entity);

    UpgradeStats processBuildingJob(int planetNumber, ogamehelpers::Entity entity);
};

#endif
