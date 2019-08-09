#include "account.hpp"
#include "ogame.hpp"

#include <cassert>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

	namespace ogh = ogamehelpers;

	std::unique_ptr<std::ofstream> Account::nullfile = std::make_unique<std::ofstream>("/dev/null");
	
	PlanetState::PlanetState(){}
	
	PlanetState::PlanetState(const PlanetState& rhs){
		operator=(rhs);
	}
	
	PlanetState& PlanetState::operator=(const PlanetState& rhs){
		planetId = rhs.planetId;
		metLevel = rhs.metLevel;
		crysLevel = rhs.crysLevel;
		deutLevel = rhs.deutLevel;
		solarLevel = rhs.solarLevel;
		fusionLevel = rhs.fusionLevel;
		labLevel = rhs.labLevel;
		roboLevel = rhs.roboLevel;
		naniteLevel = rhs.naniteLevel;
		shipyardLevel = rhs.shipyardLevel;
		temperature = rhs.temperature;
		metPercent = rhs.metPercent;
		crysPercent = rhs.crysPercent;
		deutPercent = rhs.deutPercent;
		fusionPercent = rhs.fusionPercent;
		metItem = rhs.metItem;
		crysItem = rhs.crysItem;
		deutItem = rhs.deutItem;
		metItemDurationDays = rhs.metItemDurationDays;
		crysItemDurationDays = rhs.crysItemDurationDays;
		deutItemDurationDays = rhs.deutItemDurationDays;
		sats = rhs.sats;
		researchStatePtr = rhs.researchStatePtr;
		officerStatePtr = rhs.officerStatePtr;
		accountPtr = rhs.accountPtr;
		traderatePtr = rhs.traderatePtr;
		buildingQueue = 0.0f;
        dailyProductionNeedsUpdate = rhs.dailyProductionNeedsUpdate;
        percentageChanges = rhs.percentageChanges;
		
		startConstruction(rhs.buildingQueue, rhs.entityInfoInQueue);
		
		return *this;
	}
	
	ogh::ItemRarity PlanetState::getMetItem() const{
		if(metItemDurationDays > 0)
			return metItem;
		return ogh::ItemRarity::None;
	}
	ogh::ItemRarity PlanetState::getCrysItem() const{
		if(crysItemDurationDays > 0)
			return crysItem;
		return ogh::ItemRarity::None;
	}
	ogh::ItemRarity PlanetState::getDeutItem() const{
		if(deutItemDurationDays > 0)
			return deutItem;
		return ogh::ItemRarity::None;
	}
	
	void PlanetState::advanceTime(float days){
		assert(days >= 0.0f);
		
		metItemDurationDays = std::max(0.0f, metItemDurationDays - days);
		crysItemDurationDays = std::max(0.0f, crysItemDurationDays - days);
		deutItemDurationDays = std::max(0.0f, deutItemDurationDays - days);
		
		if(constructionInProgress()){
			buildingQueue = std::max(0.0f, buildingQueue - days);
			if(!constructionInProgress())
				buildingFinishedCallback();
		}
		
	}
	
	bool PlanetState::constructionInProgress() const{
		return buildingQueue > 0.0f;
	}
	
	int PlanetState::getLevel(const ogh::EntityInfo& info) const{
        int level = 0;
        
		switch(info.entity){
            case ogh::Entity::Metalmine: level = metLevel; break;
            case ogh::Entity::Crystalmine: level = crysLevel; break;
            case ogh::Entity::Deutsynth: level = deutLevel; break;
            case ogh::Entity::Solar: level = solarLevel; break;
            case ogh::Entity::Fusion: level = fusionLevel; break;
            case ogh::Entity::Lab: level = labLevel; break;
            case ogh::Entity::Robo: level = roboLevel; break;
            case ogh::Entity::Nanite: level = naniteLevel; break;
            case ogh::Entity::Shipyard: level = naniteLevel; break;
            default: throw std::runtime_error("planetstate getLevel error " + std::string{info.name});
		}

		return level;
	}

    void PlanetState::buildingFinishedCallback(){
        switch(entityInfoInQueue.entity){
            case ogh::Entity::Metalmine: metLevel++; dailyProductionNeedsUpdate = true; setPercentToMaxProduction(entityInfoInQueue.name, metLevel); break;
            case ogh::Entity::Crystalmine: crysLevel++; dailyProductionNeedsUpdate = true; setPercentToMaxProduction(entityInfoInQueue.name, crysLevel); break;
            case ogh::Entity::Deutsynth: deutLevel++; dailyProductionNeedsUpdate = true; setPercentToMaxProduction(entityInfoInQueue.name, deutLevel); break;
            case ogh::Entity::Solar: solarLevel++; dailyProductionNeedsUpdate = true; setPercentToMaxProduction(entityInfoInQueue.name, solarLevel); break;
            case ogh::Entity::Fusion: fusionLevel++; dailyProductionNeedsUpdate = true; setPercentToMaxProduction(entityInfoInQueue.name, fusionLevel); break;
            case ogh::Entity::Lab: labLevel++; break;
            case ogh::Entity::Robo: roboLevel++; break;
            case ogh::Entity::Nanite: naniteLevel++; break;
            case ogh::Entity::Shipyard: shipyardLevel++; break;
            default: throw std::runtime_error("No building finished callback for this building " + std::string{entityInfoInQueue.name});
        }

        entityInfoInQueue = ogh::EntityInfo{};
    }
	
	void PlanetState::startConstruction(float timeDays, const ogh::EntityInfo& entityInfo){
		assert(timeDays >= 0.0f);
		assert(!constructionInProgress());
		
		buildingQueue = timeDays;
		entityInfoInQueue = entityInfo;
	}
		
	ogh::Production PlanetState::getCurrentDailyProduction() const{
        if(dailyProductionNeedsUpdate){
            dailyProduction = ogh::getDailyProduction(metLevel, getMetItem(), metPercent, 
																	crysLevel, getCrysItem(), crysPercent, 
																	deutLevel, getDeutItem(), deutPercent, 
																	solarLevel, solarplantPercent, 
																	fusionLevel, fusionPercent, researchStatePtr->etechLevel,
																	temperature, sats, satsPercent,
																	researchStatePtr->plasmaLevel, accountPtr->speedfactor, 
																	officerStatePtr->hasEngineer(), officerStatePtr->hasGeologist(), officerStatePtr->hasStaff());
        }
		return dailyProduction;
	}
	
	void PlanetState::setPercentToMaxProduction(const char* name, int level){
        constexpr int metPercentBegin = 70;
        constexpr int crysPercentBegin = 70;
        constexpr int deutPercentBegin = 70;
        constexpr int fusionPercentBegin = 70;
        
        using ogh::Production;
        
        const int oldMetPercent = metPercent;
        const int oldCrysPercent = crysPercent;
        const int oldDeutPercent = deutPercent;
        const int oldFusionPercent = fusionPercent;
        
        const auto oldProd = getCurrentDailyProduction();
        const std::int64_t oldDSE = oldProd.met / (*traderatePtr)[0] * (*traderatePtr)[2] + oldProd.crystal / (*traderatePtr)[1] * (*traderatePtr)[2] + oldProd.deut;
        
        int bestMetPercent = metPercent;
        int bestCrysPercent = crysPercent;
        int bestDeutPercent = deutPercent;
        int bestFusionPercent = fusionPercent;
        std::int64_t bestDSE = 0;
        
        const int etechLevel = researchStatePtr->etechLevel;
        const int plasmaLevel = researchStatePtr->etechLevel;
        const bool hasGeologist = officerStatePtr->hasGeologist();
        const bool hasEngineer = officerStatePtr->hasEngineer();
        const bool hasStaff = officerStatePtr->hasStaff();
        
        const int geologistpercent = hasGeologist ? 10 : 0;
        const int staffpercent = hasStaff ? 2 : 0;
        const double officerfactor = (geologistpercent + staffpercent) / 100.;
        
        const Production defaultProduction = ogh::getDefaultProduction();
        
        const double metBaseProd = 30 * metLevel * std::pow(1.1, metLevel);
        const double crysBaseProd = 20 * crysLevel * std::pow(1.1, crysLevel);
        const double deutBaseProd = 10 * deutLevel * std::pow(1.1, deutLevel) * (1.44 - 0.004 * temperature);
        
        for(int newMetPercent = 100; newMetPercent >= metPercentBegin; newMetPercent -= 10){
            
            double simpleProduction_met = metBaseProd * newMetPercent/100.;
            
            for(int newCrysPercent = 100; newCrysPercent >= crysPercentBegin; newCrysPercent -= 10){
                
                double simpleProduction_crystal = crysBaseProd * newCrysPercent/100.;
                
                for(int newDeutPercent = 100; newDeutPercent >= deutPercentBegin; newDeutPercent -= 10){
                    
                    double simpleProduction_deut = deutBaseProd * newDeutPercent/100.;
                    
                    for(int newFusionPercent = 100; newFusionPercent >= fusionPercentBegin; newFusionPercent -= 10){
                        const double mineproductionfactor = ogh::getMineProductionFactor(metLevel, newMetPercent,
                                                                                         crysLevel, newCrysPercent,
                                                                                         deutLevel, newDeutPercent,
                                                                                    solarLevel, solarplantPercent,
                                                                                    fusionLevel, newFusionPercent, etechLevel, 
                                                                                    sats, satsPercent, temperature,
                                                                                    hasEngineer, hasStaff);
                        simpleProduction_met *= mineproductionfactor;
                        simpleProduction_crystal *= mineproductionfactor;
                        simpleProduction_deut *= mineproductionfactor;
                        
                        const double itemProduction_met = simpleProduction_met * getItemProductionPercent(metItem)/100.;
                        const double itemProduction_crystal = simpleProduction_crystal * getItemProductionPercent(crysItem)/100.;
                        const double itemProduction_deut = simpleProduction_deut * getItemProductionPercent(deutItem)/100.;
                        
                        const double plasmaProduction_met = simpleProduction_met / 100. * ogh::plasma_factor_met * plasmaLevel;
                        const double plasmaProduction_crystal = simpleProduction_crystal / 100. * ogh::plasma_factor_crys * plasmaLevel;
                        const double plasmaProduction_deut = simpleProduction_deut / 100. * ogh::plasma_factor_deut * plasmaLevel;
                        
                        const double extraOfficerProduction_met = simpleProduction_met * officerfactor;
                        const double extraOfficerProduction_crystal = simpleProduction_crystal * officerfactor;
                        const double extraOfficerProduction_deut = simpleProduction_deut * officerfactor;
                        
                        double result_met = (simpleProduction_met + itemProduction_met + plasmaProduction_met + extraOfficerProduction_met);
                        double result_crystal = (simpleProduction_crystal + itemProduction_crystal + plasmaProduction_crystal + extraOfficerProduction_crystal);
                        double result_deut = (simpleProduction_deut + itemProduction_deut + plasmaProduction_deut + extraOfficerProduction_deut);
                        
                        result_met += defaultProduction.met;
                        result_crystal += defaultProduction.crystal;
                        result_deut += defaultProduction.deut;
                        
                        const std::int64_t fkwdeutconsumption = ogh::getFKWConsumption(fusionLevel, newFusionPercent);
                        result_deut -= fkwdeutconsumption;
                        
                        result_met *= accountPtr->speedfactor;
                        result_crystal *= accountPtr->speedfactor;
                        result_deut *= accountPtr->speedfactor;
                        
                        Production newProd;
                        newProd.met = std::round(result_met);
                        newProd.crystal = std::round(result_crystal);
                        newProd.deut = std::round(result_deut);
                        
                        newProd *= 24;
                        
                        const std::int64_t newDSE = newProd.met / (*traderatePtr)[0] * (*traderatePtr)[2] + newProd.crystal / (*traderatePtr)[1] * (*traderatePtr)[2] + newProd.deut;
                        if(newDSE > bestDSE){
                            bestDSE = newDSE;
                            bestMetPercent = newMetPercent;
                            bestCrysPercent = newCrysPercent;
                            bestDeutPercent = newDeutPercent;
                            bestFusionPercent = newFusionPercent;
                        }
                    }
                }
            }
        }
        
        metPercent = bestMetPercent;
        crysPercent = bestCrysPercent;
        deutPercent = bestDeutPercent;
        fusionPercent = bestFusionPercent;
        
        if(metPercent != oldMetPercent || crysPercent != oldCrysPercent || deutPercent != oldDeutPercent || fusionPercent != oldFusionPercent){
            dailyProductionNeedsUpdate = true;
            
            const double oldmineproductionfactor = ogh::getMineProductionFactor(metLevel, oldMetPercent,
                                                                                crysLevel, oldCrysPercent,
                                                                                deutLevel, oldDeutPercent,
                                                                                solarLevel, solarplantPercent,
                                                                                fusionLevel, oldFusionPercent, researchStatePtr->etechLevel, 
                                                                                sats, satsPercent, temperature,
                                                                                officerStatePtr->hasEngineer(), officerStatePtr->hasStaff());
            
            const double newmineproductionfactor = ogh::getMineProductionFactor(metLevel, metPercent,
                                                                                crysLevel, crysPercent,
                                                                                deutLevel, deutPercent,
                                                                                solarLevel, solarplantPercent,
                                                                                fusionLevel, fusionPercent, researchStatePtr->etechLevel, 
                                                                                sats, satsPercent, temperature,
                                                                                officerStatePtr->hasEngineer(), officerStatePtr->hasStaff());

            percentageChanges.emplace_back(PercentageChange{metPercent, 
                                                            crysPercent, 
                                                            deutPercent, 
                                                            fusionPercent, 
                                                            planetId, 
                                                            level,
                                                            name,
                                                            oldDSE,
                                                            bestDSE,
                                                            oldmineproductionfactor, 
                                                            newmineproductionfactor});
            
            std::stringstream sstream;
            sstream << "Planet " << planetId << ": Changed percents to " 
            << "m " << metPercent << ", c " << crysPercent << ", d " << deutPercent << ", f " << fusionPercent 
            << " after construction of " << name << " " << level
            << ". Production factor: " << oldmineproductionfactor << "->" << newmineproductionfactor 
            << ". Production increased by " << (((double(bestDSE)/oldDSE) - 1) * 100) << " %" << '\n';
            
            accountPtr->trace(sstream.str());
        }
	}
	
	
	
	ResearchState::ResearchState(){}
		
	ResearchState::ResearchState(const ResearchState& rhs){
		operator=(rhs);
	}
	
	ResearchState& ResearchState::operator=(const ResearchState& rhs){
		etechLevel = rhs.etechLevel;
		plasmaLevel = rhs.plasmaLevel;
		igrnLevel = rhs.igrnLevel;
        astroLevel = rhs.astroLevel;
		accountPtr = rhs.accountPtr;
		
		researchQueue = 0.0f;
		
		startResearch(rhs.researchQueue, rhs.entityInfoInQueue);
		
		return *this;
	}
	
	bool ResearchState::researchInProgress() const{
		return researchQueue > 0.0f;
	}
	
	void ResearchState::advanceTime(float days){
		assert(days >= 0.0f);
		
		if(researchInProgress()){
			researchQueue = std::max(0.0f, researchQueue - days);
			if(!researchInProgress())
				researchFinishedCallback();
		}			
	}
	
	int ResearchState::getLevel(const ogh::EntityInfo& info) const{
		switch(info.entity){
			case ogh::Entity::Energy: return etechLevel; 
			case ogh::Entity::Plasma: return plasmaLevel;
			case ogh::Entity::Researchnetwork: return igrnLevel;
			case ogh::Entity::Astro: return astroLevel;
			default: throw std::runtime_error("researchstate getLevel error");
		}
	}

    void ResearchState::researchFinishedCallback(){
        //update state after research is finished
        switch(entityInfoInQueue.entity){
        case ogh::Entity::Energy: etechLevel++; accountPtr->invalidatePlanetProductions();
					          accountPtr->setPercentToMaxProduction(entityInfoInQueue.name, etechLevel); break;
        case ogh::Entity::Plasma: plasmaLevel++; accountPtr->invalidatePlanetProductions();
					          accountPtr->setPercentToMaxProduction(entityInfoInQueue.name, plasmaLevel); break;
        case ogh::Entity::Researchnetwork: igrnLevel++; break;
        case ogh::Entity::Astro: astroLevel++; accountPtr->astroPhysicsResearchCompleted(); break;
        default: std::cerr << "Warning. No callback for this research\n";
        }

        entityInfoInQueue = ogh::EntityInfo{};
	}
	
	void ResearchState::startResearch(float timeDays, const ogh::EntityInfo& entityInfo){
		assert(timeDays >= 0.0f);
		assert(!researchInProgress());
		
		researchQueue = timeDays;
		entityInfoInQueue = entityInfo;
	}
	
	void Account::invalidatePlanetProductions(){
        for(auto& planetState : planetStates)
            planetState.dailyProductionNeedsUpdate = true;
    }
	
	
	OfficerState::OfficerState(){}
		
	OfficerState::OfficerState(const OfficerState& rhs){
		operator=(rhs);
	}
	
	OfficerState& OfficerState::operator=(const OfficerState& rhs){
		commanderDurationDays = rhs.commanderDurationDays;
		engineerDurationDays = rhs.engineerDurationDays;
		technocratDurationDays = rhs.technocratDurationDays;
		geologistDurationDays = rhs.geologistDurationDays;
		admiralDurationDays = rhs.admiralDurationDays;			
		return *this;
	}
	
	bool OfficerState::hasCommander() const{
		return commanderDurationDays > 0.0f;
	}
	
	bool OfficerState::hasEngineer() const{
		return engineerDurationDays > 0.0f;
	}
	
	bool OfficerState::hasTechnocrat() const{
		return technocratDurationDays > 0.0f;
	}
	
	bool OfficerState::hasGeologist() const{
		return geologistDurationDays > 0.0f;
	}
	
	bool OfficerState::hasAdmiral() const{
		return admiralDurationDays > 0.0f;
	}
	
	bool OfficerState::hasStaff() const{
		return hasCommander() && hasEngineer() && hasTechnocrat() && hasGeologist() && hasAdmiral();
	}
	
	void OfficerState::advanceTime(float days){
		assert(days >= 0.0f);
		
		commanderDurationDays = std::max(0.0f, commanderDurationDays - days);
		engineerDurationDays = std::max(0.0f, engineerDurationDays - days);
		technocratDurationDays = std::max(0.0f, technocratDurationDays - days);
		geologistDurationDays = std::max(0.0f, geologistDurationDays - days);
		admiralDurationDays = std::max(0.0f, admiralDurationDays - days);
	}
			
	Account::Account() : Account(1, 0.0f){}
	
	Account::Account(int ecospeed, float initialtime)
		:speedfactor(ecospeed), time(initialtime)
	{
		researchState.accountPtr = this;
		logfile = nullfile.get();
	}
	
	Account::Account(const Account& rhs){
		operator=(rhs);
	}
	
	Account& Account::operator=(const Account& rhs){
		planetStates = rhs.planetStates;
		researchState = rhs.researchState;
		officerState = rhs.officerState;
		resources = rhs.resources;
		traderate = rhs.traderate;
		speedfactor = rhs.speedfactor;
		time = rhs.time;
		setLogFile(rhs.logfile);
        astroPhysicsType = rhs.astroPhysicsType;
        postAstroPhysicsAction = rhs.postAstroPhysicsAction;
		
		for(auto& planetState : planetStates){
			planetState.researchStatePtr = &researchState;
			planetState.officerStatePtr = &officerState;
			planetState.accountPtr = this;
			planetState.traderatePtr = &traderate;
		}
		
		researchState.accountPtr = this;
		
		return *this;
	};
	
	void Account::setLogFile(std::ofstream* ptr){
        logfile->flush();
		logfile = ptr;		
	}
	
	void Account::log(const std::string& msg){
		*logfile << msg << '\n';
	}
	
	void Account::trace(const std::string& msg){
		*logfile << time << ": " << msg << '\n';
	}
	
	void Account::addNewPlanet(){
		planetStates.emplace_back();
		planetStates.back().planetId = int(planetStates.size());
		planetStates.back().researchStatePtr = &researchState;
		planetStates.back().officerStatePtr = &officerState;
		planetStates.back().accountPtr = this;
		planetStates.back().traderatePtr = &traderate;
	}
	
	//must not advance further than next finished event
	void Account::advanceTime(float days){
		assert(days >= 0.0f);
		assert(days <= getTimeUntilNextFinishedEvent());
		
		const ogh::Production currentProduction = getCurrentDailyProduction();
		addResources(currentProduction.produce(days));
		
		for(auto& state : planetStates)
			state.advanceTime(days);
		researchState.advanceTime(days);
		officerState.advanceTime(days);
		
		time += days;
	}
	
	float Account::getTimeUntilNextFinishedEvent() const{
		float time = std::numeric_limits<float>::max();
        bool any = false;
		for(const auto& planet : planetStates){
			if(planet.constructionInProgress()){
				time = std::min(time, planet.buildingQueue);
                any = true;
			}
			
			if(planet.getMetItem() != ogh::ItemRarity::None){
				time = std::min(time, planet.metItemDurationDays);
                any = true;
			}
			
			if(planet.getCrysItem() != ogh::ItemRarity::None){
				time = std::min(time, planet.crysItemDurationDays);
                any = true;
			}
			
			if(planet.getDeutItem() != ogh::ItemRarity::None){
				time = std::min(time, planet.deutItemDurationDays);
                any = true;
			}
		}
		
		if(researchState.researchInProgress()){
			time = std::min(time, researchState.researchQueue);
            any = true;
		}
		
		if(officerState.hasCommander()){
			time = std::min(time, officerState.commanderDurationDays);
            any = true;
		}
		
		if(officerState.hasEngineer()){
			time = std::min(time, officerState.engineerDurationDays);
            any = true;
		}
		
		if(officerState.hasTechnocrat()){
			time = std::min(time, officerState.technocratDurationDays);
            any = true;
		}
		
		if(officerState.hasGeologist()){
			time = std::min(time, officerState.geologistDurationDays);
            any = true;
		}
		
		if(officerState.hasAdmiral()){
			time = std::min(time, officerState.admiralDurationDays);
            any = true;
		}
		
		if(any){
            assert(time != std::numeric_limits<float>::max());
        }
		
		return time;
	}
	
	bool Account::hasUnfinishedConstructionEvent() const{
        auto hasOngoingConstruction = [](const auto& p){return p.constructionInProgress();};

        bool result = researchState.researchInProgress() || 
                        std::any_of(planetStates.begin(), planetStates.end(), hasOngoingConstruction);
		
		return result;
	}
	
	int Account::getTotalLabLevel() const{
		std::vector<int> labsPerPlanet;
		labsPerPlanet.reserve(getNumPlanets());
		for(const auto& p : planetStates)
			labsPerPlanet.emplace_back(p.labLevel);
		
		return ogh::getTotalLabLevel(labsPerPlanet, researchState.igrnLevel);
	};

    ogh::Production Account::getCurrentDailyMineProduction() const{
        using ogh::Production;

        auto addProductions = [](const auto& l, const auto& r){
            return l + r.getCurrentDailyProduction();
        };

        const Production currentProduction = std::accumulate(planetStates.begin(), 
                                                            planetStates.end(),
                                                            Production{},
                                                            addProductions);
		
		return currentProduction;
    }

    ogh::Production Account::getCurrentDailyFarmIncome() const{
        return ogh::Production{}; //no farm income for now
    }
	
	ogh::Production Account::getCurrentDailyProduction() const{
        using ogh::Production;

        const Production currentProduction = getCurrentDailyMineProduction() + getCurrentDailyFarmIncome();
		
		return currentProduction;
	}
	
	void Account::setPercentToMaxProduction(const char* name, int level){
        auto setPercent = [&](auto& p){p.setPercentToMaxProduction(name, level);};

        std::for_each(planetStates.begin(), planetStates.end(), setPercent);
	}
	
	void Account::startConstruction(int planet, float timeDays, const ogh::EntityInfo& entityInfo, const ogh::Resources& constructionCosts){
		assert(planet >= 0);
		assert(planet < getNumPlanets());
		
		planetStates[planet].startConstruction(timeDays, entityInfo);
		updateAccountResourcesAfterConstructionStart(constructionCosts);
	}
		
	void Account::startResearch(float timeDays, const ogh::EntityInfo& entityInfo, const ogh::Resources& constructionCosts){
		
		researchState.startResearch(timeDays, entityInfo);
		updateAccountResourcesAfterConstructionStart(constructionCosts);
	}
	
	int Account::getNumPlanets() const{
		return int(planetStates.size());
	}
	
	void Account::addResources(const ogh::Resources& res){
		resources += res;
	}
	
	void Account::updateAccountResourcesAfterConstructionStart(const ogh::Resources& constructionCosts){
		//update available account resources
		const std::int64_t diffm = resources.met - constructionCosts.met;
		const std::int64_t diffk = resources.crystal + (traderate[1]*diffm)/traderate[0] - constructionCosts.crystal;
		const std::int64_t diffd = resources.deut + (traderate[2]*diffk)/traderate[1] - constructionCosts.deut;								
		resources.met = 0;
		resources.crystal = 0;
		resources.deut = std::max(std::int64_t(0), diffd);	
	};
    
    void Account::astroPhysicsResearchCompleted(){
        if(getNumPlanets() < ogh::getMaxPossiblePlanets(researchState.astroLevel)){
            addNewPlanet();
            
            switch(postAstroPhysicsAction){
                   
                case PostAstroAction::SimpleCopyPreviousPlanet:{
                    assert(getNumPlanets() > 1);
                    
                    const PlanetState& src = planetStates[getNumPlanets()-2];
                    PlanetState& dest = planetStates[getNumPlanets()-1];
                                       
                    dest = src;
                    
                    //fix dest
                    dest.planetId = src.planetId + 1;
                    dest.buildingQueue = 0.0f;
                    dest.entityInfoInQueue = ogh::EntityInfo{};
                    
                }break;
                    
                case PostAstroAction::SimpleUpgradeToPreviousPlanet:{
                    assert(getNumPlanets() > 1);
                    
                    const PlanetState& src = planetStates[getNumPlanets()-2];
                    const int newlocation = getNumPlanets()-1;
                    
                    for(int i = 1; i <= src.roboLevel; i++){
                        processBuildingJob(UpgradeJob{newlocation, ogh::Robo});
                    }
                    
                    for(int i = 1; i <= src.naniteLevel; i++){
                        processBuildingJob(UpgradeJob{newlocation, ogh::Nanite});
                    }
                    
                    for(int i = 1; i <= src.metLevel; i++){
                        processBuildingJob(UpgradeJob{newlocation, ogh::Metalmine});
                    }
                    
                    for(int i = 1; i <= src.solarLevel; i++){
                        processBuildingJob(UpgradeJob{newlocation, ogh::Solar});
                    }
                    
                    for(int i = 1; i <= src.fusionLevel; i++){
                        processBuildingJob(UpgradeJob{newlocation, ogh::Fusion});
                    }
                    
                    for(int i = 1; i <= src.crysLevel; i++){
                        processBuildingJob(UpgradeJob{newlocation, ogh::Crystalmine});
                    }
                    
                    for(int i = 1; i <= src.deutLevel; i++){
                        processBuildingJob(UpgradeJob{newlocation, ogh::Deutsynth});
                    }
                    
                    for(int i = 1; i <= src.labLevel; i++){
                        processBuildingJob(UpgradeJob{newlocation, ogh::Lab});
                    }
                    
                    for(int i = 1; i <= src.shipyardLevel; i++){
                        processBuildingJob(UpgradeJob{newlocation, ogh::Shipyard});
                    }
                    
                }break;
                    
                case PostAstroAction::None: break;
                default: std::cerr << "Unhandles case in PostAstroAction switch statement\n"; break;
            }
        } 
    }
	
	void Account::printQueues(std::ostream& os) const{	
		os << "Building queues (days): [";
		for(const auto& p : planetStates){
			os << std::setprecision(5) << std::fixed << p.buildingQueue << ",";
		}			
		os << "]\n";	
		os << "Research queue (days): " << std::setprecision(5) << std::fixed << researchState.researchQueue << '\n';
	}
		
	void Account::waitForAllConstructions(){
		std::stringstream sstream;
		
		bool b = hasUnfinishedConstructionEvent();
		
		//std::cout << "Begin wait for finish: " << resources.met << " " << resources.crystal << " " << resources.deut << "\n";

		while(b){
			sstream << "Waiting until all queues are finished";
			trace(sstream.str());
			sstream.str("");
			
			float timeToSkip = getTimeUntilNextFinishedEvent();
						
			advanceTime(timeToSkip);
			b = hasUnfinishedConstructionEvent();
		}
		
		//std::cout << "End wait for finish: " << resources.met << " " << resources.crystal << " " << resources.deut << "\n";
		
		sstream << "All queues are empty";
		trace(sstream.str());
		sstream.str("");
	}
	
	float Account::waitUntilAstroForNextPlanetIsFinished(){
        float timeWaited = 0.0f;
        
        if(researchState.entityInfoInQueue.entity == ogh::Entity::Astro){
            if(getNumPlanets() < ogh::getMaxPossiblePlanets(researchState.astroLevel + 1)){
                // there is ongoing astrophysics research, and there will be a new planet slot available after research is finished. wait until then.
                
                std::stringstream sstream;
                
                auto makelog = [&](){
                    sstream << "waitUntilAstroForNextPlanetIsFinished. Elapsed waiting time: " << timeWaited << "\n";
                    
                    trace(sstream.str());
                    sstream.str("");
                    
                    printQueues(sstream);	
                    
                    trace(sstream.str());
                    sstream.str("");
                };
                
                makelog();
                
                while(researchState.entityInfoInQueue.entity == ogh::Entity::Astro){
                    const float timeToSkip = getTimeUntilNextFinishedEvent();
                    timeWaited += timeToSkip;                        
                    advanceTime(timeToSkip);
                    
                    makelog();
                }
            }
        }
        
        return timeWaited;
    }
    
    float Account::waitUntilCostsAreAvailable(const ogamehelpers::Resources& constructionCosts){
        std::stringstream sstream;
        ogamehelpers::Production currentProduction = getCurrentDailyProduction();
        float saveTimeDaysForJob = 0.0f;
        
        float saveTimeDays = ogh::get_save_duration_symmetrictrade(resources.met, resources.crystal, resources.deut, 
                                                                   constructionCosts.met, constructionCosts.crystal, constructionCosts.deut,
                                                                   currentProduction.met, currentProduction.crystal, currentProduction.deut,
                                                                   traderate);
        
        auto makelog = [&](){
            sstream << "Saving for job. Elapsed saving time: " << saveTimeDaysForJob << " days. Current production per day: " 
            << currentProduction.met << " " << currentProduction.crystal << " " << currentProduction.deut << "\n";
            
            trace(sstream.str());
            sstream.str("");
            
            printQueues(sstream);	
            
            trace(sstream.str());
            sstream.str("");
        };
        
        makelog();
        
        float nextEventFinishedInDays = getTimeUntilNextFinishedEvent();
        
        while(saveTimeDays > nextEventFinishedInDays){
            const float timeToSkip = nextEventFinishedInDays;				
            
            saveTimeDaysForJob += timeToSkip;
            
            advanceTime(timeToSkip);
            currentProduction = getCurrentDailyProduction();
            
            nextEventFinishedInDays = getTimeUntilNextFinishedEvent();
            
            saveTimeDays = ogh::get_save_duration_symmetrictrade(resources.met, resources.crystal, resources.deut, 
                                                                 constructionCosts.met, constructionCosts.crystal, constructionCosts.deut,
                                                                 currentProduction.met, currentProduction.crystal, currentProduction.deut,
                                                                 traderate);
            
            if(saveTimeDays == std::numeric_limits<float>::max())
                log("ERROR : PRODUCTION IS NEGATIVE");
            
            makelog();
        }
        
        saveTimeDaysForJob += saveTimeDays;
        
        advanceTime(saveTimeDays);	
        
        return saveTimeDaysForJob;
    }

    std::vector<PercentageChange> Account::getPercentageChanges() const{
        std::vector<PercentageChange> result;

        auto copyChangesToResult = [&](const auto& p){
            result.insert(result.end(), 
                            p.percentageChanges.begin(), 
                            p.percentageChanges.end());
        };

        std::for_each(planetStates.begin(), planetStates.end(), copyChangesToResult);

        return result;
    }
    
    Account::UpgradeJobStats Account::processResearchJob(const UpgradeJob& job){
        using ogamehelpers::EntityInfo;
        using ogamehelpers::Entity;
        using ogamehelpers::EntityType;
        using ogamehelpers::ItemRarity;
        using ogamehelpers::Resources;
        using ogamehelpers::Production;
        
        std::stringstream sstream;
        
        UpgradeJobStats stats;
        const auto& entityInfo = job.entityInfo;
        const int upgradeLevel = 1 + researchState.getLevel(entityInfo) + (researchState.entityInfoInQueue.entity == entityInfo.entity ? 1 : 0);
        //const int upgradeLocation = job.location;
        
        sstream << "Processing " << entityInfo.name << " " << upgradeLevel << '\n';
        trace(sstream.str());
        sstream.str("");
        
        assert(entityInfo.type == EntityType::Research);
        
        stats.job = job;
        stats.level = upgradeLevel;
        
        const Resources constructionCosts = ogamehelpers::getBuildCosts(entityInfo, upgradeLevel);
        
        sstream << "construction costs: " << constructionCosts.met << " " << constructionCosts.crystal << " " << constructionCosts.deut << '\n';
        printQueues(sstream);
        log(sstream.str());
        sstream.str("");		
        
        stats.savePeriodDaysBegin = time;
        
        const float saveTimeDaysForJob = waitUntilCostsAreAvailable(constructionCosts);
        
        stats.waitingPeriodDaysBegin = time;
        
        //wait until the research queue is empty
        while(researchState.researchInProgress()){
            trace("Waiting for previous research to finish. This does not count as saving time\n");
            
            //wait for the next event to complete, this may change the production
            float timeToSkip = getTimeUntilNextFinishedEvent();
            advanceTime(timeToSkip);
            
            printQueues(sstream);
            log(sstream.str());
            sstream.str("");
        }		
        
        const int roboLevel = 0; //not used for research
        const int naniteLevel = 0; //not used for research
        const int shipyardLevel = 0; //not used for research
        const int totalLabLevel = getTotalLabLevel();				
        const float researchTime = ogamehelpers::getConstructionTimeInDays(entityInfo, upgradeLevel, roboLevel, naniteLevel, shipyardLevel, totalLabLevel, speedfactor);
        
        sstream << "Research time in days: " << researchTime;
        
        log(sstream.str());
        sstream.str("");
        
        stats.constructionBeginDays = time;
        stats.constructionTimeDays = researchTime;
        
        startResearch(researchTime, entityInfo, constructionCosts);
        
        sstream << "Total Elapsed time: " << time << " days - Starting research. Elapsed saving time: " << saveTimeDaysForJob << " days. Elapsed waiting time: " << (stats.constructionBeginDays - stats.waitingPeriodDaysBegin) << " days\n";
        sstream << "Account resources after start: " << resources.met << " " << resources.crystal << " " << resources.deut << '\n';
        printQueues(sstream);			
        sstream << "\n";			
        sstream << "--------------------------------------------------\n\n";
        
        log(sstream.str());
        sstream.str("");
        
        return stats;
    }
    
    Account::UpgradeJobStats Account::processBuildingJob(const UpgradeJob& job){
        using ogamehelpers::EntityInfo;
        using ogamehelpers::Entity;
        using ogamehelpers::EntityType;
        using ogamehelpers::ItemRarity;
        using ogamehelpers::Resources;
        using ogamehelpers::Production;
        
        std::stringstream sstream;
        
        UpgradeJobStats stats;
        const auto& entityInfo = job.entityInfo;			
        const int upgradeLocation = job.location;
        
        const int upgradeLevel = (upgradeLocation == getNumPlanets() ? 1 : 1 + planetStates[upgradeLocation].getLevel(entityInfo) + (planetStates[upgradeLocation].entityInfoInQueue.entity == entityInfo.entity ? 1 : 0));
        
        sstream << "Planet " << (upgradeLocation+1) << " processing " << entityInfo.name << " " << upgradeLevel << '\n';
        trace(sstream.str());
        sstream.str("");
        
        assert(entityInfo.type == EntityType::Building && upgradeLocation >= 0 && upgradeLocation < getNumPlanets()+1);
        
        
        const int totalLabLevel = 0; //not used for buildings
        
        stats.job = job;			
        stats.level = upgradeLevel;
        stats.savePeriodDaysBegin = time;
        
        //calculate saving time
        const Resources constructionCosts = getBuildCosts(entityInfo, upgradeLevel);
        
        sstream << "construction costs: " << constructionCosts.met << " " << constructionCosts.crystal << " " << constructionCosts.deut << '\n';
        printQueues(sstream);
        log(sstream.str());
        sstream.str("");
        
        float saveTimeDaysForJob = 0.0f;
        
        
        
        
        if(upgradeLocation == getNumPlanets()){
            //if this is a job for a new planet which can be colonized after the currently running astro research
            //wait for research to finish
            
            saveTimeDaysForJob = waitUntilCostsAreAvailable(constructionCosts);
            stats.waitingPeriodDaysBegin = time;
 
            if(researchState.entityInfoInQueue.entity == Entity::Astro
                && getNumPlanets() < ogamehelpers::getMaxPossiblePlanets(researchState.levelInQueue)){
                
                //wait until astro is completed
                while(researchState.researchInProgress()){
                    trace("Waiting for astro research to finish. This does not count as saving time\n");
                    
                    //wait for the next event to complete, this may change the production
                    float timeToSkip = getTimeUntilNextFinishedEvent();
                    advanceTime(timeToSkip);
                    
                    printQueues(sstream);
                    log(sstream.str());
                    sstream.str("");
                }
                }else{
                    sstream << "Planet " << (upgradeLocation+1) << " does not exist and required astrophysics level is not in construction!\n";
                    trace(sstream.str());
                    sstream.str("");
                    assert(false && "Invalid upgrade list!");
                }
        }else{
            
            
            float astrowaitingtime = 0.0f;
            if(astroPhysicsType == AstroType::Blocking){
                astrowaitingtime = waitUntilAstroForNextPlanetIsFinished();
            }
            
            if(astrowaitingtime > 0.0f){
                //there was astro research going on which is now completed and increased the planet count.
                
                //saving begins __after__ astro is finished
                stats.savePeriodDaysBegin = time;
                saveTimeDaysForJob = waitUntilCostsAreAvailable(constructionCosts);
                stats.waitingPeriodDaysBegin = time;
            }else{
                //no astro research going on
                
                saveTimeDaysForJob = waitUntilCostsAreAvailable(constructionCosts);
                stats.waitingPeriodDaysBegin = time;
            }
            
            
            
            //wait until the building queue of current planet is free
            
            while(planetStates[upgradeLocation].constructionInProgress()){
                trace("Waiting for previous construction to finish. This does not count as saving time\n");
                
                //wait for the next event to complete, this may change the production
                const float timeToSkip = getTimeUntilNextFinishedEvent();
                advanceTime(timeToSkip);
                
                printQueues(sstream);
                log(sstream.str());
                sstream.str("");
            }
        }
        
        const auto& planetState = planetStates[upgradeLocation];
        
        const float constructionTimeDays = getConstructionTimeInDays(entityInfo, upgradeLevel, planetState.roboLevel, planetState.naniteLevel, planetState.shipyardLevel, totalLabLevel, speedfactor);
        
        sstream << "Construction time in days: " << constructionTimeDays;
        log(sstream.str());
        sstream.str("");
        
        stats.constructionBeginDays = time;
        stats.constructionTimeDays = constructionTimeDays;
        
        startConstruction(upgradeLocation, constructionTimeDays, entityInfo, constructionCosts);
        
        sstream << "Total Elapsed time: " << time << " days - Starting building on planet. Elapsed saving time: " << saveTimeDaysForJob << " days. Elapsed waiting time: " << (stats.constructionBeginDays - stats.waitingPeriodDaysBegin) << " days\n";
        sstream << "Account resources after start: " << resources.met << " " << resources.crystal << " " << resources.deut << '\n';
        printQueues(sstream);			
        sstream << "\n";			
        sstream << "--------------------------------------------------\n\n";
        
        log(sstream.str());
        sstream.str("");
        
        return stats;
    }

