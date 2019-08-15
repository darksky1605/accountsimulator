#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP

#include "ogame.hpp"


#include <iostream>
#include <memory>
#include <sstream>
#include <fstream>
#include <vector>


	struct ResearchState;
	struct OfficerState;
	struct Account;

    struct PercentageChange{
        int metPercent;
        int crysPercent;
        int deutPercent;
        int fusionPercent;
        int planetId;
        int finishedLevel;
        const char* finishedName;
        std::int64_t oldDSE;
        std::int64_t newDSE;
        double oldMineProductionFactor;
        double newMineProductionFactor;
    };

	struct PlanetState{
		
		static constexpr int solarplantPercent = 100;
		static constexpr int satsPercent = 100;
		
		int planetId = 0;
	
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
		int temperature = 0;
		int metPercent = 0;
		int crysPercent = 0;
		int deutPercent = 0;
		int fusionPercent = 0;
		ogamehelpers::ItemRarity metItem = ogamehelpers::ItemRarity::None;
		ogamehelpers::ItemRarity crysItem = ogamehelpers::ItemRarity::None;
		ogamehelpers::ItemRarity deutItem = ogamehelpers::ItemRarity::None;
		float metItemDurationDays = 0.0f;
		float crysItemDurationDays = 0.0f;
		float deutItemDurationDays = 0.0f;
		int sats = 0;
		
		float buildingQueue = 0.0f;
		ogamehelpers::EntityInfo entityInfoInQueue{};
		
		ResearchState* researchStatePtr;
		OfficerState* officerStatePtr;
		Account* accountPtr;

        std::vector<PercentageChange> percentageChanges;
        
        mutable ogamehelpers::Production dailyProduction;
        bool dailyProductionNeedsUpdate = true;
		
		PlanetState() = default;		
		PlanetState(const PlanetState& rhs) = default;		
		PlanetState& operator=(const PlanetState& rhs) = default;
		
		ogamehelpers::ItemRarity getMetItem() const;
		ogamehelpers::ItemRarity getCrysItem() const;
		ogamehelpers::ItemRarity getDeutItem() const;
		
		void advanceTime(float days);
		
		bool constructionInProgress() const;
		
		int getLevel(const ogamehelpers::EntityInfo& info) const;

        void buildingFinishedCallback();
		
		void startConstruction(float timeDays, const ogamehelpers::EntityInfo& entityInfo);
		
		ogamehelpers::Production getCurrentDailyProduction() const;

		void setPercentToMaxProduction(const char* name, int level);
	};
	
	struct ResearchState{
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
		
		float researchQueue = 0.0f;
		ogamehelpers::EntityInfo entityInfoInQueue{};
		
		Account* accountPtr;
		
		ResearchState() = default;		
		ResearchState(const ResearchState& rhs) = default;		
		ResearchState& operator=(const ResearchState& rhs) = default;
		
		bool researchInProgress() const;
		
		void advanceTime(float days);
		
		int getLevel(const ogamehelpers::EntityInfo& info) const;

        void researchFinishedCallback();
		
		void startResearch(float timeDays, const ogamehelpers::EntityInfo& entityInfo);
	};

	struct OfficerState{
		float commanderDurationDays = 0.0f;
		float engineerDurationDays = 0.0f;
		float technocratDurationDays = 0.0f;
		float geologistDurationDays = 0.0f;
		float admiralDurationDays = 0.0f;
		
		OfficerState();
		
		OfficerState(const OfficerState& rhs);
		
		OfficerState& operator=(const OfficerState& rhs);
		
		bool hasCommander() const;
		
		bool hasEngineer() const;
		
		bool hasTechnocrat() const;
		
		bool hasGeologist() const;
		
		bool hasAdmiral() const;
		
		bool hasStaff() const;
		
		void advanceTime(float days);
	};
	
	struct Account{
		struct UpgradeJob{			
			int location;
			ogamehelpers::EntityInfo entityInfo;

            UpgradeJob() = default;
            UpgradeJob(int l, ogamehelpers::EntityInfo e)
                : location(l), entityInfo(e){}

			bool isResearch() const{
				return entityInfo.type == ogamehelpers::EntityType::Research;
			}
			
			bool isBuilding() const{
				return entityInfo.type == ogamehelpers::EntityType::Building;
			}
			
			bool operator==(const UpgradeJob& rhs) const{
                return location == rhs.location && entityInfo == rhs.entityInfo;
            }
            
            bool operator!=(const UpgradeJob& rhs) const{
                return !(operator==(rhs));
            }
            
            bool operator<(const UpgradeJob& rhs) const{
                if(location > rhs.location)
                    return false;
                if(location < rhs.location)
                    return true;
                return entityInfo.name < rhs.entityInfo.name;
            }
		};	
			
		struct UpgradeJobStats{
			UpgradeJob job;
			int level;
			float waitingPeriodDaysBegin;
			float savePeriodDaysBegin;
			float constructionBeginDays;
			float constructionTimeDays;	
            
            bool operator==(const UpgradeJobStats& rhs) const{
                return job == rhs.job && level == rhs.level
                    && waitingPeriodDaysBegin == rhs.waitingPeriodDaysBegin
                    && savePeriodDaysBegin == rhs.savePeriodDaysBegin
                    && constructionBeginDays == rhs.constructionBeginDays
                    && constructionTimeDays == rhs.constructionTimeDays;
            }
            
            bool operator!=(const UpgradeJobStats& rhs) const{
                return !(operator==(rhs));
            }
		};

        struct LogRecord{
            float time;
            std::string msg;
            LogRecord() = default;
            LogRecord(float t, std::string m)
            : time(t), msg(std::move(m)){}
        };
        
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
	
		std::vector<PlanetState> planetStates{};
		ResearchState researchState{};
		OfficerState officerState{};
		
		ogamehelpers::Resources resources{};        
		ogamehelpers::Production dailyFarmIncomePerSlot{};
        ogamehelpers::Production dailyExpeditionIncomePerSlot{};
        ogamehelpers::Production dailyFarmIncome{};
        ogamehelpers::Production dailyExpeditionIncome{};

        std::array<float, 3> traderate{{3.0f,2.0f,1.0f}};		
		
        int speedfactor = 1;
        int saveslots = 1;
		
		float time = 0.0f;
                
        AstroType astroPhysicsType = AstroType::Nonblocking;
        PostAstroAction postAstroPhysicsAction = PostAstroAction::None;
        
        std::vector<LogRecord> logRecords;
		
		Account();
		
		Account(int ecospeed, float initialtime);
		
		Account(const Account& rhs);
		
		Account& operator=(const Account& rhs);
		
		void log(const std::string& msg);
		
		void addNewPlanet();
		
		//must not advance further than next finished event
		void advanceTime(float days);
		
		float getTimeUntilNextFinishedEvent() const;
		
		bool hasUnfinishedConstructionEvent() const;
		
		int getTotalLabLevel() const;
        
        void invalidatePlanetProductions();

        ogamehelpers::Production getCurrentDailyMineProduction() const;

        ogamehelpers::Production getCurrentDailyFarmIncome() const;

        ogamehelpers::Production getCurrentDailyExpeditionIncome() const;
		
		ogamehelpers::Production getCurrentDailyProduction() const;

        void updateDailyFarmIncome();
        
        void updateDailyExpeditionIncome();
		
		void setPercentToMaxProduction(const char* name, int level);

        std::vector<PercentageChange> getPercentageChanges() const;
		
		void startConstruction(int planet, float timeDays, const ogamehelpers::EntityInfo& entityInfo, const ogamehelpers::Resources& constructionCosts);
			
		void startResearch(float timeDays, const ogamehelpers::EntityInfo& entityInfo, const ogamehelpers::Resources& constructionCosts);
		
		int getNumPlanets() const;
		
		void addResources(const ogamehelpers::Resources& res);
		
		void updateAccountResourcesAfterConstructionStart(const ogamehelpers::Resources& constructionCosts);
		
		void printQueues(std::ostream& os) const;
        
        void astroPhysicsResearchCompleted();
		
		void waitForAllConstructions();
        
        float waitUntilAstroForNextPlanetIsFinished();
		
		float waitUntilCostsAreAvailable(const ogamehelpers::Resources& constructionCosts);
		
		UpgradeJobStats processResearchJob(const UpgradeJob& job);
		
		UpgradeJobStats processBuildingJob(const UpgradeJob& job);
	};



#endif
