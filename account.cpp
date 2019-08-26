#include "account.hpp"
#include "ogame.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <sstream>
#include <vector>
#include <iterator>

namespace ogh = ogamehelpers;

ogh::ItemRarity PlanetState::getMetItem() const {
    constexpr auto zero = std::chrono::seconds::zero();

    if (metItemDuration > zero)
        return metItem;
    return ogh::ItemRarity::None;
}

ogh::ItemRarity PlanetState::getCrysItem() const {
    constexpr auto zero = std::chrono::seconds::zero();

    if (crysItemDuration > zero)
        return crysItem;
    return ogh::ItemRarity::None;
}

ogh::ItemRarity PlanetState::getDeutItem() const {
    constexpr auto zero = std::chrono::seconds::zero();

    if (deutItemDuration > zero)
        return deutItem;
    return ogh::ItemRarity::None;
}

void PlanetState::advanceTime(std::chrono::seconds timestep) {
    constexpr auto zero = std::chrono::seconds::zero();

    assert(timestep >= zero);

    metItemDuration = std::max(zero, metItemDuration - timestep);
    crysItemDuration = std::max(zero, crysItemDuration - timestep);
    deutItemDuration = std::max(zero, deutItemDuration - timestep);

    if (constructionInProgress()) {
        buildingQueue = std::max(zero, buildingQueue - timestep);
        if (!constructionInProgress())
            accountPtr->buildingFinishedCallback(*this);
    }
}

bool PlanetState::constructionInProgress() const {
    constexpr auto zero = std::chrono::seconds::zero();

    return buildingQueue > zero;
}

int PlanetState::getLevel(const ogh::Entity& entity) const {
    switch (entity) {
    case ogh::Entity::Metalmine:
        return buildingLevels.metLevel;
    case ogh::Entity::Crystalmine:
        return buildingLevels.crysLevel;
    case ogh::Entity::Deutsynth:
        return buildingLevels.deutLevel;
    case ogh::Entity::Solar:
        return buildingLevels.solarLevel;
    case ogh::Entity::Fusion:
        return buildingLevels.fusionLevel;
    case ogh::Entity::Lab:
        return buildingLevels.labLevel;
    case ogh::Entity::Robo:
        return buildingLevels.roboLevel;
    case ogh::Entity::Nanite:
        return buildingLevels.naniteLevel;
    case ogh::Entity::Shipyard:
        return buildingLevels.shipyardLevel;
    case ogh::Entity::Metalstorage:
        return buildingLevels.metalStorageLevel;
    case ogh::Entity::Crystalstorage:
        return buildingLevels.crystalStorageLevel;
    case ogh::Entity::Deutstorage:
        return buildingLevels.deutStorageLevel;
    case ogh::Entity::Alliancedepot:
        return buildingLevels.allianceDepotLevel;
    case ogh::Entity::Silo:
        return buildingLevels.missileSiloLevel;
    case ogh::Entity::None:
        return 0;
    default:
        throw std::runtime_error("planetstate getLevel error " + ogh::getEntityName(entity));
    }
}

PlanetState::Levels PlanetState::getAllCurrentLevels() const{
    return buildingLevels;
}

PlanetState::Levels PlanetState::getAllLevelsAfterConstruction() const{
    Levels l = buildingLevels;

    switch (entityInQueue) {
    case ogh::Entity::Metalmine:
        l.metLevel++; break;
    case ogh::Entity::Crystalmine:
        l.crysLevel++; break;
    case ogh::Entity::Deutsynth:
        l.deutLevel++; break;
    case ogh::Entity::Solar:
        l.solarLevel++; break;
    case ogh::Entity::Fusion:
        l.fusionLevel++; break;
    case ogh::Entity::Lab:
        l.labLevel++; break;
    case ogh::Entity::Robo:
        l.roboLevel++; break;
    case ogh::Entity::Nanite:
        l.naniteLevel++; break;
    case ogh::Entity::Shipyard:
        l.shipyardLevel++; break;
    case ogh::Entity::Metalstorage:
        l.metalStorageLevel++; break;
    case ogh::Entity::Crystalstorage:
        l.crystalStorageLevel++; break;
    case ogh::Entity::Deutstorage:
        l.deutStorageLevel++; break;
    case ogh::Entity::Alliancedepot:
        l.allianceDepotLevel++; break;
    case ogh::Entity::Silo:
        l.missileSiloLevel++; break;
    case ogh::Entity::None:
        break;
    default:
        break;
    }

    return l;
}

//increase level by one and return new level;
int PlanetState::increaseLevel(ogamehelpers::Entity entity) {
    switch (entity) {
    case ogh::Entity::Metalmine:
        return ++buildingLevels.metLevel;
    case ogh::Entity::Crystalmine:
        return ++buildingLevels.crysLevel;
    case ogh::Entity::Deutsynth:
        return ++buildingLevels.deutLevel;
    case ogh::Entity::Solar:
        return ++buildingLevels.solarLevel;
    case ogh::Entity::Fusion:
        return ++buildingLevels.fusionLevel;
    case ogh::Entity::Lab:
        return ++buildingLevels.labLevel;
    case ogh::Entity::Robo:
        return ++buildingLevels.roboLevel;
    case ogh::Entity::Nanite:
        return ++buildingLevels.naniteLevel;
    case ogh::Entity::Shipyard:
        return ++buildingLevels.shipyardLevel;
    case ogh::Entity::Metalstorage:
        return ++buildingLevels.metalStorageLevel;
    case ogh::Entity::Crystalstorage:
        return ++buildingLevels.crystalStorageLevel;
    case ogh::Entity::Deutstorage:
        return ++buildingLevels.deutStorageLevel;
    case ogh::Entity::Alliancedepot:
        return ++buildingLevels.allianceDepotLevel;
    case ogh::Entity::Silo:
        return ++buildingLevels.missileSiloLevel;
    case ogh::Entity::None:
        return 0;
    default:
        throw std::runtime_error("planetstate increaseLevel error " + ogh::getEntityName(entity));
    }
}

void PlanetState::startConstruction(std::chrono::seconds durationOfConstruction, const ogh::Entity& entity) {
    constexpr auto zero = std::chrono::seconds::zero();

    assert(durationOfConstruction >= zero);
    assert(!constructionInProgress());

    buildingQueue = durationOfConstruction;
    entityInQueue = entity;
}

ogamehelpers::Entity PlanetState::getBuildingInConstruction() const{
    return entityInQueue;
}

void PlanetState::calculateDailyProduction() {
    dailyProduction = ogh::getDailyProduction(getLevel(ogh::Entity::Metalmine), getMetItem(), metPercent,
                                                getLevel(ogh::Entity::Crystalmine), getCrysItem(), crysPercent,
                                                getLevel(ogh::Entity::Deutsynth), getDeutItem(), deutPercent,
                                                getLevel(ogh::Entity::Solar), solarplantPercent,
                                                getLevel(ogh::Entity::Fusion), fusionPercent, accountPtr->getResearchLevel(ogh::Entity::Energy),
                                                temperature, sats, satsPercent,
                                                accountPtr->getResearchLevel(ogh::Entity::Plasma), accountPtr->speedfactor,
                                                accountPtr->hasEngineer(), accountPtr->hasGeologist(), accountPtr->hasStaff());

}

ogh::Production PlanetState::getCurrentDailyProduction() const {
    return dailyProduction;
}

PlanetState::SetPercentsResult PlanetState::setPercentToMaxProduction() {
    constexpr int metPercentBegin = 70;
    constexpr int crysPercentBegin = 70;
    constexpr int deutPercentBegin = 70;
    constexpr int fusionPercentBegin = 70;

    using ogh::Production;

    const int oldMetPercent = metPercent;
    const int oldCrysPercent = crysPercent;
    const int oldDeutPercent = deutPercent;
    const int oldFusionPercent = fusionPercent;

    const Production oldProd = getCurrentDailyProduction();
    Production bestProd = oldProd;
    const double oldDSE = oldProd.produce(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours{24})).dse(accountPtr->traderate);

    int bestMetPercent = metPercent;
    int bestCrysPercent = crysPercent;
    int bestDeutPercent = deutPercent;
    int bestFusionPercent = fusionPercent;
    double bestDSE = 0;

    const int etechLevel = accountPtr->getResearchLevel(ogh::Entity::Energy);
    const int plasmaLevel = accountPtr->getResearchLevel(ogh::Entity::Plasma);
    const bool hasGeologist = accountPtr->hasGeologist();
    const bool hasEngineer = accountPtr->hasEngineer();
    const bool hasStaff = accountPtr->hasStaff();

    const int geologistpercent = hasGeologist ? 10 : 0;
    const int staffpercent = hasStaff ? 2 : 0;
    const double officerfactor = (geologistpercent + staffpercent) / 100.;

    const Production defaultProduction = ogh::getDefaultProduction();

    const int metLevel = getLevel(ogh::Entity::Metalmine);
    const int crysLevel = getLevel(ogh::Entity::Crystalmine);
    const int deutLevel = getLevel(ogh::Entity::Deutsynth);
    const int solarLevel = getLevel(ogh::Entity::Solar);
    const int fusionLevel = getLevel(ogh::Entity::Fusion);

    const double metBaseProd = 30 * metLevel * std::pow(1.1, metLevel);
    const double crysBaseProd = 20 * crysLevel * std::pow(1.1, crysLevel);
    const double deutBaseProd = 10 * deutLevel * std::pow(1.1, deutLevel) * (1.44 - 0.004 * temperature);

    

    for (int newMetPercent = 100; newMetPercent >= metPercentBegin; newMetPercent -= 10) {

        double simpleProduction_met = metBaseProd * newMetPercent / 100.;

        for (int newCrysPercent = 100; newCrysPercent >= crysPercentBegin; newCrysPercent -= 10) {

            double simpleProduction_crystal = crysBaseProd * newCrysPercent / 100.;

            for (int newDeutPercent = 100; newDeutPercent >= deutPercentBegin; newDeutPercent -= 10) {

                double simpleProduction_deut = deutBaseProd * newDeutPercent / 100.;

                for (int newFusionPercent = 100; newFusionPercent >= fusionPercentBegin; newFusionPercent -= 10) {
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

                    const double itemProduction_met = simpleProduction_met * getItemProductionPercent(metItem) / 100.;
                    const double itemProduction_crystal = simpleProduction_crystal * getItemProductionPercent(crysItem) / 100.;
                    const double itemProduction_deut = simpleProduction_deut * getItemProductionPercent(deutItem) / 100.;

                    const double plasmaProduction_met = simpleProduction_met / 100. * ogh::plasma_factor_met * plasmaLevel;
                    const double plasmaProduction_crystal = simpleProduction_crystal / 100. * ogh::plasma_factor_crys * plasmaLevel;
                    const double plasmaProduction_deut = simpleProduction_deut / 100. * ogh::plasma_factor_deut * plasmaLevel;

                    const double extraOfficerProduction_met = simpleProduction_met * officerfactor;
                    const double extraOfficerProduction_crystal = simpleProduction_crystal * officerfactor;
                    const double extraOfficerProduction_deut = simpleProduction_deut * officerfactor;

                    double result_met = (simpleProduction_met + itemProduction_met + plasmaProduction_met + extraOfficerProduction_met);
                    double result_crystal = (simpleProduction_crystal + itemProduction_crystal + plasmaProduction_crystal + extraOfficerProduction_crystal);
                    double result_deut = (simpleProduction_deut + itemProduction_deut + plasmaProduction_deut + extraOfficerProduction_deut);

                    result_met += defaultProduction.metal();
                    result_crystal += defaultProduction.crystal();
                    result_deut += defaultProduction.deuterium();

                    const std::int64_t fkwdeutconsumption = ogh::getFKWConsumption(fusionLevel, newFusionPercent);
                    result_deut -= fkwdeutconsumption;

                    result_met *= accountPtr->speedfactor;
                    result_crystal *= accountPtr->speedfactor;
                    result_deut *= accountPtr->speedfactor;

                    Production newProd = Production::makeProductionPerDay(round(result_met), round(result_crystal), round(result_deut));

                    newProd *= 24;

                    const double newDSE = newProd.produce(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours{24})).dse(accountPtr->traderate);
                    if (newDSE > bestDSE) {
                        bestDSE = newDSE;
                        bestProd = newProd;
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

    std::stringstream ss;
    ss << dailyProduction.metal() << " " << dailyProduction.crystal() << " " << dailyProduction.deuterium() << " -> " 
        << bestProd.metal() << " " << bestProd.crystal() << " " << bestProd.deuterium() 
        << " ::: " << metLevel << " " << crysLevel << " " << deutLevel << " " << fusionLevel;
    accountPtr->log(ss.str());

    if(dailyProduction != bestProd){
        dailyProduction = bestProd;
        accountPtr->planetProductionChanged(oldProd, bestProd);
    }

    if (metPercent != oldMetPercent || crysPercent != oldCrysPercent || deutPercent != oldDeutPercent || fusionPercent != oldFusionPercent) {

        const double oldmineproductionfactor = ogh::getMineProductionFactor(metLevel, oldMetPercent,
                                                                            crysLevel, oldCrysPercent,
                                                                            deutLevel, oldDeutPercent,
                                                                            solarLevel, solarplantPercent,
                                                                            fusionLevel, oldFusionPercent, accountPtr->getResearchLevel(ogh::Entity::Energy),
                                                                            sats, satsPercent, temperature,
                                                                            accountPtr->hasEngineer(), accountPtr->hasStaff());

        const double newmineproductionfactor = ogh::getMineProductionFactor(metLevel, metPercent,
                                                                            crysLevel, crysPercent,
                                                                            deutLevel, deutPercent,
                                                                            solarLevel, solarplantPercent,
                                                                            fusionLevel, fusionPercent, accountPtr->getResearchLevel(ogh::Entity::Energy),
                                                                            sats, satsPercent, temperature,
                                                                            accountPtr->hasEngineer(), accountPtr->hasStaff());

        SetPercentsResult result{
            true,
            metPercent,
            crysPercent,
            deutPercent,
            fusionPercent,
            planetId,
            oldDSE,
            bestDSE,
            oldmineproductionfactor,
            newmineproductionfactor};

        return result;

    } else {
        SetPercentsResult result;
        result.changedPercents = false;
        return result;
    }
}

bool ResearchState::researchInProgress() const {
    constexpr auto zero = std::chrono::seconds::zero();

    return researchQueue > zero;
}

void ResearchState::advanceTime(std::chrono::seconds timestep) {
    constexpr auto zero = std::chrono::seconds::zero();

    assert(timestep >= zero);

    if (researchInProgress()) {
        researchQueue = std::max(std::chrono::seconds::zero(), researchQueue - timestep);
        if (!researchInProgress())
            accountPtr->researchFinishedCallback();
    }
}

int ResearchState::getLevel(ogh::Entity entity) const {
    switch (entity) {
    case ogh::Entity::Espionage:
        return espionageLevel;
    case ogh::Entity::Computer:
        return computerLevel;
    case ogh::Entity::Weapons:
        return weaponsLevel;
    case ogh::Entity::Shielding:
        return shieldingLevel;
    case ogh::Entity::Armour:
        return armourLevel;
    case ogh::Entity::Energy:
        return etechLevel;
    case ogh::Entity::Hyperspacetech:
        return hyperspacetechLevel;
    case ogh::Entity::Combustion:
        return combustionLevel;
    case ogh::Entity::Impulse:
        return impulseLevel;
    case ogh::Entity::Hyperspacedrive:
        return hyperspacedriveLevel;
    case ogh::Entity::Laser:
        return laserLevel;
    case ogh::Entity::Ion:
        return ionLevel;
    case ogh::Entity::Plasma:
        return plasmaLevel;
    case ogh::Entity::Researchnetwork:
        return igrnLevel;
    case ogh::Entity::Astro:
        return astroLevel;
    case ogh::Entity::None:
        return 0;
    default:
        throw std::runtime_error("researchstate getLevel error");
    }
}

int ResearchState::increaseLevel(ogamehelpers::Entity entity) {
    switch (entity) {
    case ogh::Entity::Espionage:
        return ++espionageLevel;
    case ogh::Entity::Computer:
        return ++computerLevel;
    case ogh::Entity::Weapons:
        return ++weaponsLevel;
    case ogh::Entity::Shielding:
        return ++shieldingLevel;
    case ogh::Entity::Armour:
        return ++armourLevel;
    case ogh::Entity::Energy:
        return ++etechLevel;
    case ogh::Entity::Hyperspacetech:
        return ++hyperspacetechLevel;
    case ogh::Entity::Combustion:
        return ++combustionLevel;
    case ogh::Entity::Impulse:
        return ++impulseLevel;
    case ogh::Entity::Hyperspacedrive:
        return ++hyperspacedriveLevel;
    case ogh::Entity::Laser:
        return ++laserLevel;
    case ogh::Entity::Ion:
        return ++ionLevel;
    case ogh::Entity::Plasma:
        return ++plasmaLevel;
    case ogh::Entity::Researchnetwork:
        return ++igrnLevel;
    case ogh::Entity::Astro:
        return ++astroLevel;
    case ogh::Entity::None:
        return 0;
    default:
        throw std::runtime_error("researchstate getLevel error");
    }
}

void ResearchState::startResearch(std::chrono::seconds constructionDuration, const ogh::Entity& entity) {
    constexpr auto zero = std::chrono::seconds::zero();

    assert(constructionDuration >= zero);
    assert(!researchInProgress());

    researchQueue = constructionDuration;
    entityInQueue = entity;
}

void OfficerState::advanceTime(std::chrono::seconds timestep) {
    constexpr auto zero = std::chrono::seconds::zero();

    assert(timestep >= zero);    

    commanderDuration = std::max(zero, commanderDuration - timestep);
    engineerDuration = std::max(zero, engineerDuration - timestep);
    technocratDuration = std::max(zero, technocratDuration - timestep);
    geologistDuration = std::max(zero, geologistDuration - timestep);
    admiralDuration = std::max(zero, admiralDuration - timestep);
}

Account::Account() : Account(1, std::chrono::seconds::zero()) {}

Account::Account(int ecospeed, std::chrono::seconds initialtime)
    : speedfactor(ecospeed), accountTime(initialtime) {
    researches.accountPtr = this;
}

Account::Account(const Account& rhs) {
    operator=(rhs);
}

Account& Account::operator=(const Account& rhs) {
    planets = rhs.planets;
    researches = rhs.researches;
    officers = rhs.officers;
    resources = rhs.resources;
    dailyFarmIncome = rhs.dailyFarmIncome;
    dailyExpeditionIncome = rhs.dailyExpeditionIncome;
    dailyFarmIncomePerSlot = rhs.dailyFarmIncomePerSlot;
    dailyExpeditionIncomePerSlot = rhs.dailyExpeditionIncomePerSlot;
    dailyMineProduction = rhs.dailyMineProduction;
    accountInitialized = rhs.accountInitialized;
    traderate = rhs.traderate;
    speedfactor = rhs.speedfactor;
    saveslots = rhs.saveslots;
    accountTime = rhs.accountTime;
    logRecords = rhs.logRecords;
    eventTimes = rhs.eventTimes;

    for (auto& planet : planets) {
        planet.accountPtr = this;
    }

    researches.accountPtr = this;

    return *this;
}

void Account::log(const std::string& msg) {
    logRecords.emplace_back(accountTime, msg);
}

void Account::buildingFinishedCallback(PlanetState& p) {
    auto handleProductionChangingBuilding = [&]() {
        auto result = p.setPercentToMaxProduction();
        const int level = p.getLevel(p.entityInQueue);
        recordPercentageChange(result, p.entityInQueue, level);
    };

    p.increaseLevel(p.entityInQueue);

    switch (p.entityInQueue) {
    case ogh::Entity::Metalmine:
        handleProductionChangingBuilding();
        break;
    case ogh::Entity::Crystalmine:
        handleProductionChangingBuilding();
        break;
    case ogh::Entity::Deutsynth:
        handleProductionChangingBuilding();
        break;
    case ogh::Entity::Solar:
        handleProductionChangingBuilding();
        break;
    case ogh::Entity::Fusion:
        handleProductionChangingBuilding();
        break;
    case ogh::Entity::Lab:
        break;
    case ogh::Entity::Robo:
        break;
    case ogh::Entity::Nanite:
        break;
    case ogh::Entity::Shipyard:
        break;
    case ogh::Entity::Metalstorage:
        break;
    case ogh::Entity::Crystalstorage:
        break;
    case ogh::Entity::Deutstorage:
        break;
    case ogh::Entity::Alliancedepot:
        break;
    case ogh::Entity::Silo:
        break;
    case ogh::Entity::None:
        break;
    default:
        throw std::runtime_error("No building finished callback for this building " + std::string{ogh::getEntityName(p.entityInQueue)});
    }

    p.entityInQueue = ogh::Entity::None;
}

void Account::researchFinishedCallback() {
    auto handleMineProductionChangingResearch = [&]() {
        const int level = getResearchLevel(researches.entityInQueue);

        auto handlePlanet = [&](auto& planet) {
            auto result = planet.setPercentToMaxProduction();
            recordPercentageChange(result, researches.entityInQueue, level);
        };

        std::for_each(planets.begin(), planets.end(), handlePlanet);
    };

    auto handleFleetIncomeChangingResearch = [&]() {
        updateDailyFarmIncome();
        updateDailyExpeditionIncome();
    };

    auto& r = researches;
    r.increaseLevel(r.entityInQueue);

    //update state after research is finished
    switch (r.entityInQueue) {
    case ogh::Entity::Energy:
        handleMineProductionChangingResearch();
        break;
    case ogh::Entity::Plasma:
        handleMineProductionChangingResearch();
        break;
    case ogh::Entity::Researchnetwork:
        break;
    case ogh::Entity::Astro:
        handleFleetIncomeChangingResearch();
        break;
    case ogh::Entity::Computer:
        handleFleetIncomeChangingResearch();
        break;
    case ogh::Entity::Espionage:
        break;
    case ogh::Entity::Weapons:
        break;
    case ogh::Entity::Shielding:
        break;
    case ogh::Entity::Armour:
        break;
    case ogh::Entity::Hyperspacetech:
        break;
    case ogh::Entity::Combustion:
        break;
    case ogh::Entity::Impulse:
        break;
    case ogh::Entity::Hyperspacedrive:
        break;
    case ogh::Entity::Laser:
        break;
    case ogh::Entity::Ion:
        break;
    case ogh::Entity::None:
        break;
    default:
        throw std::runtime_error("No research finished callback for this research " + std::string{ogh::getEntityName(r.entityInQueue)});
    }

    researches.entityInQueue = ogh::Entity::None;
}

void Account::planetProductionChanged(const ogamehelpers::Production& oldProd, const ogamehelpers::Production& newProd){
    dailyMineProduction -= oldProd;
    dailyMineProduction += newProd;
}

void Account::addNewPlanet() {
    planets.emplace_back();
    planets.back().planetId = int(planets.size());
    planets.back().accountPtr = this;
}

void Account::initEventTimes() {
    eventTimes.reserve(getNumPlanets() * 4 + 6);

    for (const auto& planet : planets) {
        if (planet.constructionInProgress()) {
            eventTimes.emplace_back(planet.buildingQueue);
        }

        if (planet.getMetItem() != ogh::ItemRarity::None) {
            eventTimes.emplace_back(planet.metItemDuration);
        }

        if (planet.getCrysItem() != ogh::ItemRarity::None) {
            eventTimes.emplace_back(planet.crysItemDuration);
        }

        if (planet.getDeutItem() != ogh::ItemRarity::None) {
            eventTimes.emplace_back(planet.deutItemDuration);
        }
    }

    if (researches.researchInProgress()) {
        eventTimes.emplace_back(researches.researchQueue);
    }

    if (hasCommander()) {
        eventTimes.emplace_back(officers.commanderDuration);
    }

    if (hasEngineer()) {
        eventTimes.emplace_back(officers.engineerDuration);
    }

    if (hasTechnocrat()) {
        eventTimes.emplace_back(officers.technocratDuration);
    }

    if (hasGeologist()) {
        eventTimes.emplace_back(officers.geologistDuration);
    }

    if (hasAdmiral()) {
        eventTimes.emplace_back(officers.admiralDuration);
    }

    std::sort(eventTimes.begin(), eventTimes.end());
}

void Account::registerNewEvent(std::chrono::seconds when){
    //std::cerr << "register " << when << '\n';
    auto it = std::lower_bound(eventTimes.begin(), eventTimes.end(), when);
    eventTimes.insert(it, when);
    //std::copy(eventTimes.begin(), eventTimes.end(), std::ostream_iterator<float>(std::cerr, " "));
   // std::cerr << '\n';
}

//must not advance further than next finished event
void Account::advanceTime(std::chrono::seconds timestep) {
    constexpr auto zero = std::chrono::seconds::zero();

    assert(timestep >= zero);
    assert(timestep <= getTimeUntilNextFinishedEvent());

    const ogh::Production currentProduction = getCurrentDailyProduction();
    addResources(currentProduction.produce(timestep));

    for (auto& planet : planets)
        planet.advanceTime(timestep);
    researches.advanceTime(timestep);
    officers.advanceTime(timestep);

    accountTime += timestep;

    //TODO instead of loop, find range to erase, then erase it
    while(!eventTimes.empty() && eventTimes.front() <= accountTime){
        eventTimes.erase(eventTimes.begin());
    }
    //std::cerr << "after advance " << days << " account time " << time << '\n';
   // std::copy(eventTimes.begin(), eventTimes.end(), std::ostream_iterator<float>(std::cerr, " "));
    //std::cerr << '\n';
}

std::chrono::seconds Account::getTimeUntilNextFinishedEvent() const {
    std::chrono::seconds timeUntilNext = std::chrono::seconds::max();
    if(!eventTimes.empty()){
        timeUntilNext = eventTimes.front() - accountTime;
    }
    //return timeUntilNext;
    std::chrono::seconds time2 = std::chrono::seconds::max();
    bool any = false;
    for (const auto& planet : planets) {
        if (planet.constructionInProgress()) {
            time2 = std::min(time2, planet.buildingQueue);
            any = true;
        }

        if (planet.getMetItem() != ogh::ItemRarity::None) {
            time2 = std::min(time2, planet.metItemDuration);
            any = true;
        }

        if (planet.getCrysItem() != ogh::ItemRarity::None) {
            time2 = std::min(time2, planet.crysItemDuration);
            any = true;
        }

        if (planet.getDeutItem() != ogh::ItemRarity::None) {
            time2 = std::min(time2, planet.deutItemDuration);
            any = true;
        }
    }

    if (researches.researchInProgress()) {
        time2 = std::min(time2, researches.researchQueue);
        any = true;
    }

    if (hasCommander()) {
        time2 = std::min(time2, officers.commanderDuration);
        any = true;
    }

    if (hasEngineer()) {
        time2 = std::min(time2, officers.engineerDuration);
        any = true;
    }

    if (hasTechnocrat()) {
        time2 = std::min(time2, officers.technocratDuration);
        any = true;
    }

    if (hasGeologist()) {
        time2 = std::min(time2, officers.geologistDuration);
        any = true;
    }

    if (hasAdmiral()) {
        time2 = std::min(time2, officers.admiralDuration);
        any = true;
    }

    if (any) {
        assert(time2 != std::chrono::seconds::max());
        // if(std::abs(time2 - timeUntilNext) > 1e-4){
        //     std::cerr << "time2 = " << time2 << ", timeUntilNext = " << timeUntilNext << "\n";
        // }
        //assert(std::abs(time2 - timeUntilNext) <= 1e-4);
    }
    
    return time2;
}

bool Account::hasUnfinishedConstructionEvent() const {
    auto hasOngoingConstruction = [](const auto& p) { return p.constructionInProgress(); };

    bool result = researches.researchInProgress() ||
                  std::any_of(planets.begin(), planets.end(), hasOngoingConstruction);

    return result;
}

int Account::getTotalLabLevel() const {
    std::vector<int> labsPerPlanet;
    labsPerPlanet.reserve(getNumPlanets());
    for (const auto& p : planets)
        labsPerPlanet.emplace_back(p.getLevel(ogh::Entity::Lab));

    return ogh::getTotalLabLevel(labsPerPlanet, getResearchLevel(ogh::Entity::Researchnetwork));
}

void Account::calculateDailyProduction(){
    using ogh::Production;

    auto addProductions = [](const auto& l, const auto& r) {
        return l + r.getCurrentDailyProduction();
    };

    dailyMineProduction = std::accumulate(planets.begin(),
                                            planets.end(),
                                            Production{},
                                            addProductions);
}

ogh::Production Account::getCurrentDailyMineProduction() const {
    // using ogh::Production;

    // auto addProductions = [](const auto& l, const auto& r) {
    //     return l + r.getCurrentDailyProduction();
    // };

    // const Production currentProduction = std::accumulate(planets.begin(),
    //                                                      planets.end(),
    //                                                      Production{},
    //                                                      addProductions);

    // return currentProduction;
    return dailyMineProduction;
}

ogh::Production Account::getCurrentDailyFarmIncome() const {
    return dailyFarmIncome;
}

ogh::Production Account::getCurrentDailyExpeditionIncome() const {
    return dailyExpeditionIncome;
}

ogh::Production Account::getCurrentDailyProduction() const {
    using ogh::Production;

    const Production currentProduction = getCurrentDailyMineProduction() + getCurrentDailyFarmIncome() + getCurrentDailyExpeditionIncome();

    return currentProduction;
}

void Account::updateDailyFarmIncome() {
    const int slotsInAccount = ogh::getNumberOfFleetSlotsWithOfficers(getResearchLevel(ogh::Entity::Computer), hasAdmiral(), hasStaff());
    const int fleetSlots = std::max(0, slotsInAccount - saveslots);
    const int expoSlots = ogh::getNumberOfExpeditionSlotsWithOfficers(getResearchLevel(ogh::Entity::Astro), hasAdmiral(), hasStaff());
    const int slots = std::max(0, fleetSlots - expoSlots);

    dailyFarmIncome = dailyFarmIncomePerSlot * slots;
}

void Account::updateDailyExpeditionIncome() {
    const int slotsInAccount = ogh::getNumberOfFleetSlotsWithOfficers(getResearchLevel(ogh::Entity::Computer), hasAdmiral(), hasStaff());
    const int fleetSlots = std::max(0, slotsInAccount - saveslots);
    const int expoSlots = ogh::getNumberOfExpeditionSlotsWithOfficers(getResearchLevel(ogh::Entity::Astro), hasAdmiral(), hasStaff());
    const int slots = std::min(fleetSlots, expoSlots);

    dailyExpeditionIncome = dailyExpeditionIncomePerSlot * slots;
}

void Account::recordPercentageChange(const PlanetState::SetPercentsResult& result, ogh::Entity entity, int level) {
    if (result.changedPercents) {

        PercentageChange pchange;
        pchange.metPercent = result.metPercent;
        pchange.crysPercent = result.crysPercent;
        pchange.deutPercent = result.deutPercent;
        pchange.fusionPercent = result.fusionPercent;
        pchange.planetId = result.planetId;
        pchange.finishedLevel = level;
        pchange.time = accountTime;
        pchange.oldDSE = result.oldDSE;
        pchange.newDSE = result.newDSE;
        pchange.oldMineProductionFactor = result.oldMineProductionFactor;
        pchange.newMineProductionFactor = result.newMineProductionFactor;
        pchange.finishedName = ogh::getEntityName(entity);

        percentageChanges.emplace_back(std::move(pchange));

        std::stringstream sstream;
        sstream << "Planet " << pchange.planetId << ": Changed percents to "
                << "m " << pchange.metPercent << ", c " << pchange.crysPercent << ", d " << pchange.deutPercent << ", f " << pchange.fusionPercent
                << " after construction of " << pchange.finishedName << " " << pchange.finishedLevel
                << ". Production factor: " << pchange.oldMineProductionFactor << "->" << pchange.newMineProductionFactor
                << ". Production increased by " << (((double(pchange.newDSE) / pchange.oldDSE) - 1) * 100) << " %" << '\n';

        log(sstream.str());
    }
}

void Account::startConstruction(int planet, std::chrono::seconds duration, const ogh::Entity& entity, const ogh::Resources& constructionCosts) {
    assert(planet >= 0);
    assert(planet < getNumPlanets());

    planets[planet].startConstruction(duration, entity);
    updateAccountResourcesAfterConstructionStart(constructionCosts);
    registerNewEvent(accountTime + duration);
}

void Account::startResearch(std::chrono::seconds duration, const ogh::Entity& entity, const ogh::Resources& constructionCosts) {

    researches.startResearch(duration, entity);
    updateAccountResourcesAfterConstructionStart(constructionCosts);
    registerNewEvent(accountTime + duration);
}

int Account::getNumPlanets() const {
    return int(planets.size());
}

int Account::getResearchLevel(ogamehelpers::Entity entity) const {
    return researches.getLevel(entity);
}

int Account::getBuildingLevel(int planetId, ogamehelpers::Entity entity) const {
    assert(planetId > 0 && planetId <= getNumPlanets());
    //planetId is 1-based
    const int index = planetId - 1;
    return planets[index].getLevel(entity);
}

ogamehelpers::Entity Account::getResearchInConstruction() const{
    return researches.entityInQueue;
}

ogamehelpers::Entity Account::getBuildingInConstruction(int planetId) const{
    assert(planetId > 0 && planetId <= getNumPlanets());
    //planetId is 1-based
    const int index = planetId - 1;
    return planets[index].getBuildingInConstruction();
}

void Account::addResources(const ogh::Resources& res) {
    resources += res;
}

void Account::updateAccountResourcesAfterConstructionStart(const ogh::Resources& constructionCosts) {
    //update available account resources
    const std::int64_t diffm = resources.metal() - constructionCosts.metal();
    const std::int64_t diffk = resources.crystal() + (traderate[1] * diffm) / traderate[0] - constructionCosts.crystal();
    const std::int64_t diffd = resources.deuterium() + (traderate[2] * diffk) / traderate[1] - constructionCosts.deuterium();
    resources.setMetal(0);
    resources.setCrystal(0);
    resources.setDeuterium(std::max(std::int64_t(0), diffd));
}

void Account::printQueues(std::ostream& os) const {
    os << "Building queues (days): [";
    for (const auto& p : planets) {
        os << std::setprecision(5) << std::fixed << p.buildingQueue.count() << ",";
    }
    os << "]\n";
    os << "Research queue (days): " << std::setprecision(5) << std::fixed << researches.researchQueue.count() << '\n';
}

void Account::waitForAllConstructions() {
    std::stringstream sstream;

    bool b = hasUnfinishedConstructionEvent();

    //std::cout << "Begin wait for finish: " << resources.met << " " << resources.crystal << " " << resources.deut << "\n";

    while (b) {
        sstream << "Waiting until all queues are finished";
        log(sstream.str());
        sstream.str("");

        std::chrono::seconds timeToSkip = getTimeUntilNextFinishedEvent();

        advanceTime(timeToSkip);
        b = hasUnfinishedConstructionEvent();
    }

    //std::cout << "End wait for finish: " << resources.met << " " << resources.crystal << " " << resources.deut << "\n";

    sstream << "All queues are empty";
    log(sstream.str());
    sstream.str("");
}

std::chrono::seconds Account::waitUntilCostsAreAvailable(const ogamehelpers::Resources& constructionCosts) {
    std::stringstream sstream;
    ogamehelpers::Production currentProduction = getCurrentDailyProduction();
    std::chrono::seconds saveTimeDaysForJob{0};

    // std::chrono::seconds saveTimeDays = ogh::get_save_duration_symmetrictrade(resources.metal(), resources.crystal(), resources.deuterium(),
    //                                                            constructionCosts.metal(), constructionCosts.crystal(), constructionCosts.deuterium(),
    //                                                            currentProduction.metal(), currentProduction.crystal(), currentProduction.deuterium(),
    //                                                            traderate);
    std::chrono::seconds saveTimeDays = ogh::get_save_duration_symmetrictrade(resources,
                                                               constructionCosts,
                                                               currentProduction,
                                                               traderate);

    auto makelog = [&]() {
        sstream << "Saving for job. Elapsed saving time: " << saveTimeDaysForJob.count() << " seconds. Current production per day: "
                << currentProduction.metal() << " " << currentProduction.crystal() << " " << currentProduction.deuterium() << "\n";

        log(sstream.str());
        sstream.str("");

        printQueues(sstream);

        log(sstream.str());
        sstream.str("");
    };

    makelog();

    std::chrono::seconds nextEventFinishedInDays = getTimeUntilNextFinishedEvent();

    while (saveTimeDays > nextEventFinishedInDays) {
        const std::chrono::seconds timeToSkip = nextEventFinishedInDays;

        saveTimeDaysForJob += timeToSkip;

        advanceTime(timeToSkip);
        currentProduction = getCurrentDailyProduction();

        nextEventFinishedInDays = getTimeUntilNextFinishedEvent();

        // std::chrono::seconds saveTimeDays = ogh::get_save_duration_symmetrictrade(resources.metal(), resources.crystal(), resources.deuterium(),
        //                                                            constructionCosts.metal(), constructionCosts.crystal(), constructionCosts.deuterium(),
        //                                                            currentProduction.metal(), currentProduction.crystal(), currentProduction.deuterium(),
        //                                                            traderate);
        saveTimeDays = ogh::get_save_duration_symmetrictrade(resources,
                                                               constructionCosts,
                                                               currentProduction,
                                                               traderate);

        if (saveTimeDays == std::chrono::seconds::max())
            log("ERROR : PRODUCTION IS NEGATIVE");

        makelog();
    }

    saveTimeDaysForJob += saveTimeDays;

    advanceTime(saveTimeDays);

    return saveTimeDaysForJob;
}

std::vector<Account::PercentageChange> Account::getPercentageChanges() const {
    return percentageChanges;
}

bool Account::hasCommander() const {
    return officers.commanderDuration > std::chrono::seconds::zero();
}

bool Account::hasEngineer() const {
    return officers.engineerDuration > std::chrono::seconds::zero();
}

bool Account::hasTechnocrat() const {
    return officers.technocratDuration > std::chrono::seconds::zero();
}

bool Account::hasGeologist() const {
    return officers.geologistDuration > std::chrono::seconds::zero();
}

bool Account::hasAdmiral() const {
    return officers.admiralDuration > std::chrono::seconds::zero();
}

bool Account::hasStaff() const {
    return hasCommander() && hasEngineer() && hasTechnocrat() && hasGeologist() && hasAdmiral();
}

Account::UpgradeStats Account::processResearchJob(ogh::Entity entity) {
    using ogamehelpers::Entity;
    using ogamehelpers::EntityInfo;
    using ogamehelpers::EntityType;
    using ogamehelpers::ItemRarity;
    using ogamehelpers::Production;
    using ogamehelpers::Resources;

    if(!accountInitialized){
        for(auto& planet : planets){
            planet.calculateDailyProduction();
        }
        calculateDailyProduction();
        initEventTimes();
        accountInitialized = true;
    }

    std::stringstream sstream;

    const EntityInfo entityInfo = ogh::getEntityInfo(entity);
    assert(entityInfo.type == EntityType::Research);

    const int upgradeLevel = 1 + getResearchLevel(entity) + (researches.entityInQueue == entity ? 1 : 0);

    sstream << "Processing " << ogh::getEntityName(entity) << " " << upgradeLevel << '\n';
    log(sstream.str());
    sstream.str("");

    UpgradeStats stats;
    stats.level = upgradeLevel;

    const Resources constructionCosts = ogh::getBuildCosts(entityInfo, upgradeLevel);

    sstream << "construction costs: " << constructionCosts.metal() << " " << constructionCosts.crystal() << " " << constructionCosts.deuterium() << '\n';
    printQueues(sstream);
    log(sstream.str());
    sstream.str("");

    stats.savePeriodDaysBegin = accountTime;

    const std::chrono::seconds saveTimeDaysForJob = waitUntilCostsAreAvailable(constructionCosts);

    stats.waitingPeriodDaysBegin = accountTime;

    //wait until no research lab is in construction
    auto labInConstruction = [](const auto& p) {
        return p.entityInQueue == ogh::Entity::Lab;
    };

    while (std::any_of(planets.begin(), planets.end(), labInConstruction)) {
        log("Waiting for finished construction of research labs. This does not count as saving time\n");

        std::chrono::seconds timeToSkip = getTimeUntilNextFinishedEvent();
        advanceTime(timeToSkip);

        printQueues(sstream);
    }

    //wait until the research queue is empty
    while (researches.researchInProgress()) {
        log("Waiting for previous research to finish. This does not count as saving time\n");

        //wait for the next event to complete, this may change the production
        std::chrono::seconds timeToSkip = getTimeUntilNextFinishedEvent();
        advanceTime(timeToSkip);

        printQueues(sstream);
        log(sstream.str());
        sstream.str("");
    }

    const int roboLevel = 0;     //not used for research
    const int naniteLevel = 0;   //not used for research
    const int shipyardLevel = 0; //not used for research
    const int totalLabLevel = getTotalLabLevel();

    const std::chrono::seconds researchTimeNoOfficer = ogh::getConstructionTime(entityInfo, upgradeLevel, roboLevel, naniteLevel, shipyardLevel, totalLabLevel, speedfactor);
    const std::chrono::seconds researchTime = hasTechnocrat() ? std::chrono::duration_cast<std::chrono::seconds>(std::chrono::duration<double>{std::ceil(researchTimeNoOfficer.count() * 0.75)}) : researchTimeNoOfficer;

    sstream << "Research time in seconds: " << researchTime.count();

    log(sstream.str());
    sstream.str("");

    stats.constructionBeginDays = accountTime;
    stats.constructionTimeDays = researchTime;

    startResearch(researchTime, entity, constructionCosts);

    if (entity == ogh::Entity::Astro) {
        //if Astrophysics research is started which will increase the planet count uppon completion,
        //a new planet is added immediatly. To prevent constructions before the research is actually complete,
        // the building queue is blocked with an empty task with a duration equal to the research duration of Astrophysics.
        if (getNumPlanets() + 1 == ogh::getMaxPossiblePlanets(getResearchLevel(ogh::Entity::Astro) + 1)) {
            addNewPlanet();
            planets.back().startConstruction(researchTime, ogh::Entity::None);
        }
    }

    sstream << "Total Elapsed time: " << accountTime.count() << " seconds - Starting research. Elapsed saving time: " << saveTimeDaysForJob.count() 
            << " seconds. Elapsed waiting time: " << (stats.constructionBeginDays - stats.waitingPeriodDaysBegin).count() << " seconds\n";
    sstream << "Account resources after start: " << resources.metal() << " " << resources.crystal() << " " << resources.deuterium() << '\n';
    printQueues(sstream);
    sstream << "\n";
    sstream << "--------------------------------------------------\n\n";

    log(sstream.str());
    sstream.str("");

    stats.success = true;

    return stats;
}

Account::UpgradeStats Account::processBuildingJob(int planetId, ogh::Entity entity) {
    using ogamehelpers::Entity;
    using ogamehelpers::EntityInfo;
    using ogamehelpers::EntityType;
    using ogamehelpers::ItemRarity;
    using ogamehelpers::Production;
    using ogamehelpers::Resources;

    if(!accountInitialized){
        for(auto& planet : planets){
            planet.calculateDailyProduction();
        }
        calculateDailyProduction();
        initEventTimes();

        accountInitialized = true;
    }

    std::stringstream sstream;

    const EntityInfo entityInfo = ogh::getEntityInfo(entity);
    const int upgradeLocation = planetId;

    assert(entityInfo.type == EntityType::Building && upgradeLocation >= 0 && upgradeLocation < getNumPlanets());

    const int upgradeLevel = 1 + planets[upgradeLocation].getLevel(entity) + (planets[upgradeLocation].entityInQueue == entity ? 1 : 0);

    sstream << "Planet " << (upgradeLocation + 1) << " processing " << ogh::getEntityName(entity) << " " << upgradeLevel << '\n';
    log(sstream.str());
    sstream.str("");

    const int totalLabLevel = 0; //not used for buildings

    const Resources constructionCosts = getBuildCosts(entityInfo, upgradeLevel);

    sstream << "construction costs: " << constructionCosts.metal() << " " << constructionCosts.crystal() << " " << constructionCosts.deuterium() << '\n';
    printQueues(sstream);
    log(sstream.str());
    sstream.str("");

    UpgradeStats stats;
    stats.level = upgradeLevel;

    //calculate saving time
    stats.savePeriodDaysBegin = accountTime;

    auto waitForResearchBeforeLabStart = [&]() {
        if (entity == ogh::Entity::Lab && researches.entityInQueue != ogh::Entity::None) {
            log("Waiting for finished research before building research lab. This does not count as saving time\n");

            const std::chrono::seconds timeToSkip = getTimeUntilNextFinishedEvent();
            advanceTime(timeToSkip);

            printQueues(sstream);
        }
    };

    const std::chrono::seconds saveTimeDaysForJob = waitUntilCostsAreAvailable(constructionCosts);
    stats.waitingPeriodDaysBegin = accountTime;

    waitForResearchBeforeLabStart();

    //wait until the building queue of current planet is free
    const auto& planetState = planets[upgradeLocation];

    while (planetState.constructionInProgress()) {
        log("Waiting for previous construction to finish. This does not count as saving time\n");

        //wait for the next event to complete, this may change the production
        const std::chrono::seconds timeToSkip = getTimeUntilNextFinishedEvent();
        advanceTime(timeToSkip);

        printQueues(sstream);
        log(sstream.str());
        sstream.str("");
    }

    const std::chrono::seconds constructionTime = ogh::getConstructionTime(entityInfo, 
                                                                            upgradeLevel, 
                                                                            planetState.getLevel(ogh::Entity::Robo), 
                                                                            planetState.getLevel(ogh::Entity::Nanite), 
                                                                            planetState.getLevel(ogh::Entity::Shipyard), 
                                                                            totalLabLevel, 
                                                                            speedfactor);

    sstream << "Construction time in seconds: " << constructionTime.count();
    log(sstream.str());
    sstream.str("");

    stats.constructionBeginDays = accountTime;
    stats.constructionTimeDays = constructionTime;

    startConstruction(upgradeLocation, constructionTime, entity, constructionCosts);

    sstream << "Total Elapsed time: " << accountTime.count() << " days - Starting building on planet. Elapsed saving time: " << saveTimeDaysForJob.count() 
            << " days. Elapsed waiting time: " << (stats.constructionBeginDays - stats.waitingPeriodDaysBegin).count() << " days\n";
    sstream << "Account resources after start: " << resources.metal() << " " << resources.crystal() << " " << resources.deuterium() << '\n';
    printQueues(sstream);
    sstream << "\n";
    sstream << "--------------------------------------------------\n\n";

    log(sstream.str());
    sstream.str("");

    stats.success = true;

    return stats;
}
