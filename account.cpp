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
		switch(info.entity){
            case ogh::Entity::Metalmine: return metLevel;
            case ogh::Entity::Crystalmine: return crysLevel;
            case ogh::Entity::Deutsynth: return deutLevel;
            case ogh::Entity::Solar: return solarLevel;
            case ogh::Entity::Fusion: return fusionLevel;
            case ogh::Entity::Lab: return labLevel;
            case ogh::Entity::Robo: return roboLevel;
            case ogh::Entity::Nanite: return naniteLevel;
            case ogh::Entity::Shipyard: return shipyardLevel;
            case ogh::Entity::Metalstorage: return metalStorageLevel;
            case ogh::Entity::Crystalstorage: return crystalStorageLevel;
            case ogh::Entity::Deutstorage: return deutStorageLevel;
            case ogh::Entity::Alliancedepot: return allianceDepotLevel;
            case ogh::Entity::Silo: return missileSiloLevel;
            default: throw std::runtime_error("planetstate getLevel error " + std::string{info.name});
		}
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
            case ogh::Entity::Metalstorage: metalStorageLevel++; break;
            case ogh::Entity::Crystalstorage: crystalStorageLevel++; break;
            case ogh::Entity::Deutstorage: deutStorageLevel++; break;
            case ogh::Entity::Alliancedepot: allianceDepotLevel++; break;
            case ogh::Entity::Silo: missileSiloLevel++; break;
            case ogh::Entity::None: break;
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
        const std::int64_t oldDSE = oldProd.met / (accountPtr->traderate)[0] * (accountPtr->traderate)[2] + oldProd.crystal / (accountPtr->traderate)[1] * (accountPtr->traderate)[2] + oldProd.deut;
        
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
                        
                        const std::int64_t newDSE = newProd.met / (accountPtr->traderate)[0] * (accountPtr->traderate)[2] + newProd.crystal / (accountPtr->traderate)[1] * (accountPtr->traderate)[2] + newProd.deut;
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
            
            accountPtr->log(sstream.str());
        }
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
        case ogh::Entity::Espionage: return espionageLevel; 
        case ogh::Entity::Computer: return computerLevel; 
        case ogh::Entity::Weapons: return weaponsLevel; 
        case ogh::Entity::Shielding: return shieldingLevel; 
        case ogh::Entity::Armour: return armourLevel; 
        case ogh::Entity::Energy: return etechLevel; 
        case ogh::Entity::Hyperspacetech: return hyperspacetechLevel; 
        case ogh::Entity::Combustion: return combustionLevel; 
        case ogh::Entity::Impulse: return impulseLevel; 
        case ogh::Entity::Hyperspacedrive: return hyperspacedriveLevel; 
        case ogh::Entity::Laser: return laserLevel; 
        case ogh::Entity::Ion: return ionLevel; 
        case ogh::Entity::Plasma: return plasmaLevel; 
        case ogh::Entity::Researchnetwork: return igrnLevel; 
        case ogh::Entity::Astro: return astroLevel;
        default: throw std::runtime_error("researchstate getLevel error");
        }
    }

    void ResearchState::researchFinishedCallback(){
        //update state after research is finished
        switch(entityInfoInQueue.entity){
        case ogh::Entity::Energy: etechLevel++; 
                                accountPtr->invalidatePlanetProductions();
					            accountPtr->setPercentToMaxProduction(entityInfoInQueue.name, etechLevel); break;
        case ogh::Entity::Plasma: plasmaLevel++; 
                                accountPtr->invalidatePlanetProductions();
					            accountPtr->setPercentToMaxProduction(entityInfoInQueue.name, plasmaLevel); break;
        case ogh::Entity::Researchnetwork: igrnLevel++; break;
        case ogh::Entity::Astro: astroLevel++; 
                                accountPtr->updateDailyFarmIncome();
                                accountPtr->updateDailyExpeditionIncome(); break;
        case ogh::Entity::Computer: computerLevel++;
                                accountPtr->updateDailyFarmIncome();
                                accountPtr->updateDailyExpeditionIncome(); break;
        case ogh::Entity::Espionage: espionageLevel++; break;
        case ogh::Entity::Weapons: weaponsLevel++; break;
        case ogh::Entity::Shielding: shieldingLevel++; break;
        case ogh::Entity::Armour: armourLevel++; break;
        case ogh::Entity::Hyperspacetech: hyperspacetechLevel++; break;
        case ogh::Entity::Combustion: combustionLevel++; break;
        case ogh::Entity::Impulse: impulseLevel++; break;
        case ogh::Entity::Hyperspacedrive: hyperspacedriveLevel++; break;
        case ogh::Entity::Laser: laserLevel++; break;
        case ogh::Entity::Ion: ionLevel++; break;
        case ogh::Entity::None: break;
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
        for(auto& planet : planets)
            planet.dailyProductionNeedsUpdate = true;
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
	}
	
	Account::Account(const Account& rhs){
		operator=(rhs);
	}
	
	Account& Account::operator=(const Account& rhs){
		planets = rhs.planets;
		researchState = rhs.researchState;
		officerState = rhs.officerState;
		resources = rhs.resources;
		dailyFarmIncome = rhs.dailyFarmIncome;
		dailyExpeditionIncome = rhs.dailyExpeditionIncome;
        dailyFarmIncomePerSlot = rhs.dailyFarmIncomePerSlot;
		dailyExpeditionIncomePerSlot = rhs.dailyExpeditionIncomePerSlot;
		traderate = rhs.traderate;
		speedfactor = rhs.speedfactor;
        saveslots = rhs.saveslots;
		time = rhs.time;
        logRecords = rhs.logRecords;
		
		for(auto& planet : planets){
			planet.researchStatePtr = &researchState;
			planet.officerStatePtr = &officerState;
			planet.accountPtr = this;
		}
		
		researchState.accountPtr = this;
		
		return *this;
	}
	
	void Account::log(const std::string& msg){
        logRecords.emplace_back(time, msg);
	}
	
	void Account::addNewPlanet(){
		planets.emplace_back();
		planets.back().planetId = int(planets.size());
		planets.back().researchStatePtr = &researchState;
		planets.back().officerStatePtr = &officerState;
		planets.back().accountPtr = this;
	}
	
	//must not advance further than next finished event
	void Account::advanceTime(float days){
		assert(days >= 0.0f);
		assert(days <= getTimeUntilNextFinishedEvent());
		
		const ogh::Production currentProduction = getCurrentDailyProduction();
		addResources(currentProduction.produce(days));
		
		for(auto& planet : planets)
			planet.advanceTime(days);
		researchState.advanceTime(days);
		officerState.advanceTime(days);
		
		time += days;
	}
	
	float Account::getTimeUntilNextFinishedEvent() const{
		float time = std::numeric_limits<float>::max();
        bool any = false;
		for(const auto& planet : planets){
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
                        std::any_of(planets.begin(), planets.end(), hasOngoingConstruction);
		
		return result;
	}
	
	int Account::getTotalLabLevel() const{
		std::vector<int> labsPerPlanet;
		labsPerPlanet.reserve(getNumPlanets());
		for(const auto& p : planets)
			labsPerPlanet.emplace_back(p.labLevel);
		
		return ogh::getTotalLabLevel(labsPerPlanet, researchState.igrnLevel);
	}

    ogh::Production Account::getCurrentDailyMineProduction() const{
        using ogh::Production;

        auto addProductions = [](const auto& l, const auto& r){
            return l + r.getCurrentDailyProduction();
        };

        const Production currentProduction = std::accumulate(planets.begin(), 
                                                            planets.end(),
                                                            Production{},
                                                            addProductions);
		
		return currentProduction;
    }

    ogh::Production Account::getCurrentDailyFarmIncome() const{
        return dailyFarmIncome;
    }

    ogh::Production Account::getCurrentDailyExpeditionIncome() const{
        return dailyExpeditionIncome;
    }
	
	ogh::Production Account::getCurrentDailyProduction() const{
        using ogh::Production;

        const Production currentProduction = getCurrentDailyMineProduction() 
                                            + getCurrentDailyFarmIncome()
                                            + getCurrentDailyExpeditionIncome();
		
		return currentProduction;
	}

    void Account::updateDailyFarmIncome(){
        const int slotsInAccount = ogh::getNumberOfFleetSlotsWithOfficers(researchState.computerLevel, officerState.hasAdmiral(), officerState.hasStaff());
        const int fleetSlots = std::max(0, slotsInAccount - saveslots);
        const int expoSlots = ogh::getNumberOfExpeditionSlotsWithOfficers(researchState.astroLevel, officerState.hasAdmiral(), officerState.hasStaff());
        const int slots = std::max(0, fleetSlots - expoSlots);

        dailyFarmIncome = dailyFarmIncomePerSlot * slots;
    }
        
    void Account::updateDailyExpeditionIncome(){
        const int slotsInAccount = ogh::getNumberOfFleetSlotsWithOfficers(researchState.computerLevel, officerState.hasAdmiral(), officerState.hasStaff());
        const int fleetSlots = std::max(0, slotsInAccount - saveslots);
        const int expoSlots = ogh::getNumberOfExpeditionSlotsWithOfficers(researchState.astroLevel, officerState.hasAdmiral(), officerState.hasStaff());
        const int slots = std::min(fleetSlots, expoSlots);

        dailyExpeditionIncome = dailyExpeditionIncomePerSlot * slots;
    }
	
	void Account::setPercentToMaxProduction(const char* name, int level){
        auto setPercent = [&](auto& p){p.setPercentToMaxProduction(name, level);};

        std::for_each(planets.begin(), planets.end(), setPercent);
	}
	
	void Account::startConstruction(int planet, float timeDays, const ogh::EntityInfo& entityInfo, const ogh::Resources& constructionCosts){
		assert(planet >= 0);
		assert(planet < getNumPlanets());
		
		planets[planet].startConstruction(timeDays, entityInfo);
		updateAccountResourcesAfterConstructionStart(constructionCosts);
	}
		
	void Account::startResearch(float timeDays, const ogh::EntityInfo& entityInfo, const ogh::Resources& constructionCosts){
		
		researchState.startResearch(timeDays, entityInfo);
		updateAccountResourcesAfterConstructionStart(constructionCosts);
	}
	
	int Account::getNumPlanets() const{
		return int(planets.size());
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
	}
    	
	void Account::printQueues(std::ostream& os) const{	
		os << "Building queues (days): [";
		for(const auto& p : planets){
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
			log(sstream.str());
			sstream.str("");
			
			float timeToSkip = getTimeUntilNextFinishedEvent();
						
			advanceTime(timeToSkip);
			b = hasUnfinishedConstructionEvent();
		}
		
		//std::cout << "End wait for finish: " << resources.met << " " << resources.crystal << " " << resources.deut << "\n";
		
		sstream << "All queues are empty";
		log(sstream.str());
		sstream.str("");
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
            
            log(sstream.str());
            sstream.str("");
            
            printQueues(sstream);	
            
            log(sstream.str());
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

        std::for_each(planets.begin(), planets.end(), copyChangesToResult);

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
        log(sstream.str());
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

        //wait until no research lab is in construction
        auto labInConstruction = [](const auto& p){
            return p.entityInfoInQueue.entity == ogh::Entity::Lab;
        };

        while(std::any_of(planets.begin(), planets.end(), labInConstruction)){
            log("Waiting for finished construction of research labs. This does not count as saving time\n");

            float timeToSkip = getTimeUntilNextFinishedEvent();
            advanceTime(timeToSkip);

            printQueues(sstream);
        }
        
        //wait until the research queue is empty
        while(researchState.researchInProgress()){
            log("Waiting for previous research to finish. This does not count as saving time\n");
            
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

        if(entityInfo.entity == ogh::Entity::Astro){
            //if Astrophysics research is started which will increase the planet count uppon completion,
            //a new planet is added immediatly. To prevent constructions before the research is actually complete,
            // the building queue is blocked with an empty task with a duration equal to the research duration of Astrophysics.
            if(getNumPlanets() + 1 == ogh::getMaxPossiblePlanets(researchState.astroLevel + 1)){
                addNewPlanet();
                planets.back().startConstruction(researchTime, ogh::Noentity);
            }
        }
        
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
        
        const int upgradeLevel = (upgradeLocation == getNumPlanets() ? 1 : 1 + planets[upgradeLocation].getLevel(entityInfo) + (planets[upgradeLocation].entityInfoInQueue.entity == entityInfo.entity ? 1 : 0));
        
        sstream << "Planet " << (upgradeLocation+1) << " processing " << entityInfo.name << " " << upgradeLevel << '\n';
        log(sstream.str());
        sstream.str("");
        
        assert(entityInfo.type == EntityType::Building && upgradeLocation >= 0 && upgradeLocation < getNumPlanets());        
        
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
        
        auto waitForResearchBeforeLabStart = [&](){
            if(entityInfo.entity == ogh::Entity::Lab && researchState.entityInfoInQueue.entity != ogh::Entity::None){
                log("Waiting for finished research before building research lab. This does not count as saving time\n");

                float timeToSkip = getTimeUntilNextFinishedEvent();
                advanceTime(timeToSkip);

                printQueues(sstream);
            }
        };
            
        saveTimeDaysForJob = waitUntilCostsAreAvailable(constructionCosts);
        stats.waitingPeriodDaysBegin = time;
        
        waitForResearchBeforeLabStart();
        
        //wait until the building queue of current planet is free
        const auto& planetState = planets[upgradeLocation];
        
        while(planetState.constructionInProgress()){
            log("Waiting for previous construction to finish. This does not count as saving time\n");
            
            //wait for the next event to complete, this may change the production
            const float timeToSkip = getTimeUntilNextFinishedEvent();
            advanceTime(timeToSkip);
            
            printQueues(sstream);
            log(sstream.str());
            sstream.str("");
        }

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

