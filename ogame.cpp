#include "ogame.hpp"
#include "hpc_helpers.cuh"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <locale>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace ogamehelpers {

/*const EntityInfo Metalmine = EntityInfo(60,15,0,0,10,1.5, EntityType::Building, Entity::Metalmine, "Metal Mine");
    const EntityInfo Crystalmine = EntityInfo(48,24,0,0,10,1.6, EntityType::Building, Entity::Crystalmine, "Crystal Mine");
    const EntityInfo Deutsynth = EntityInfo(225,75,0,0,20,1.5, EntityType::Building, Entity::Deutsynth, "Deuterium Synthesizer");
    const EntityInfo Solar = EntityInfo(75,30,0,0,0,1.5, EntityType::Building, Entity::Solar, "Solar Plant");
    const EntityInfo Fusion = EntityInfo(900,360,180,0,0,1.8, EntityType::Building, Entity::Fusion, "Fusion Reactor");
    const EntityInfo Robo = EntityInfo(400,120,200,0,0,2, EntityType::Building, Entity::Robo, "Robotics Factory");
    const EntityInfo Nanite = EntityInfo(1000000,500000,100000,0,0,2, EntityType::Building, Entity::Nanite, "Nanite Factory");
    const EntityInfo Shipyard = EntityInfo(400,200,100,0,0,2, EntityType::Building, Entity::Shipyard, "Shipyard");
    const EntityInfo Metalstorage = EntityInfo(1000,0,0,0,0,2, EntityType::Building, Entity::Metalstorage, "Metal Storage");
    const EntityInfo Crystalstorage = EntityInfo(1000,500,0,0,0,2, EntityType::Building, Entity::Crystalstorage, "Crystal Storage");
    const EntityInfo Deutstorage = EntityInfo(1000,1000,0,0,0,2, EntityType::Building, Entity::Deutstorage, "Deuterium Tank");
    const EntityInfo Lab = EntityInfo(200,400,200,0,0,2, EntityType::Building, Entity::Lab, "Research Lab");
    const EntityInfo Terra = EntityInfo(0,50000,100000,1000,0,2, EntityType::Building, Entity::Terra, "Terraformer");
    const EntityInfo Alliancedepot = EntityInfo(20000,40000,0,0,0,2, EntityType::Building, Entity::Alliancedepot, "Alliance Depot");
    const EntityInfo Spacedock = EntityInfo(200,0,50,50,0,5, EntityType::Building, Entity::Spacedock, "Space Dock");
    const EntityInfo Lunarbase = EntityInfo(20000,40000,20000,0,0,2, EntityType::Building, Entity::Lunarbase, "Lunar Base");
    const EntityInfo Phalanx = EntityInfo(20000,40000,20000,0,0,2, EntityType::Building, Entity::Phalanx, "Sensor Phalanx");
    const EntityInfo Jumpgate = EntityInfo(2000000,4000000,2000000,0,0,2, EntityType::Building, Entity::Jumpgate, "Jump Gate");
    const EntityInfo Silo = EntityInfo(20000,20000,1000,0,0,2, EntityType::Building, Entity::Silo, "Missile Silo");
    
    const EntityInfo Espionage = EntityInfo(200,1000,200,0,0,2, EntityType::Research, Entity::Espionage, "Espionage Technology");
    const EntityInfo Computer = EntityInfo(0,400,600,0,0,2, EntityType::Research, Entity::Computer, "Computer Technology");
    const EntityInfo Weapons = EntityInfo(800,200,0,0,0,2, EntityType::Research, Entity::Weapons, "Weapons Technology");
    const EntityInfo Shielding = EntityInfo(200,600,0,0,0,2, EntityType::Research, Entity::Shielding, "Shielding Technology");
    const EntityInfo Armour = EntityInfo(1000,0,0,0,0,2, EntityType::Research, Entity::Armour, "Armor Technology");
    const EntityInfo Energy = EntityInfo(0,800,400,0,0,2, EntityType::Research, Entity::Energy, "Energy Technology");
    const EntityInfo Hyperspacetech = EntityInfo(0,4000,2000,0,0,2, EntityType::Research, Entity::Hyperspacetech, "Hyperspace Technology");
    const EntityInfo Combustion = EntityInfo(400,0,600,0,0,2, EntityType::Research, Entity::Combustion, "Combustion Drive");
    const EntityInfo Impulse = EntityInfo(2000,4000,600,0,0,2, EntityType::Research, Entity::Impulse, "Impulse Drive");
    const EntityInfo Hyperspacedrive = EntityInfo(1000,20000,6000,0,0,2, EntityType::Research, Entity::Hyperspacedrive, "Hyperspace Drive");
    const EntityInfo Laser = EntityInfo(200,100,0,0,0,2, EntityType::Research, Entity::Laser, "Laser Technology");
    const EntityInfo Ion = EntityInfo(1000,300,100,0,0,2, EntityType::Research, Entity::Ion, "Ion Technology");
    const EntityInfo Plasma = EntityInfo(2000,4000,1000,0,0,2, EntityType::Research, Entity::Plasma, "Plasma Technology");
    const EntityInfo Researchnetwork = EntityInfo(240000,400000,160000,0,0,2, EntityType::Research, Entity::Researchnetwork, "Intergalactic Research Network");
    const EntityInfo Astro = EntityInfo(4000,8000,4000,0,0,1.75, EntityType::Research, Entity::Astro, "Astrophysics");
    const EntityInfo Graviton = EntityInfo(0,0,0,300000,0,3, EntityType::Research, Entity::Graviton, "Graviton Technology");
    
    const EntityInfo Coloship = EntityInfo(10000,20000,10000,0,0,0, EntityType::Ship, Entity::Coloship, "Colonization Ship");
    
    const EntityInfo Noentity = EntityInfo();*/

EntityInfo getEntityInfo(Entity entity) {
    switch (entity) {
    case Entity::Metalmine:
        return Metalmine;
    case Entity::Crystalmine:
        return Crystalmine;
    case Entity::Deutsynth:
        return Deutsynth;
    case Entity::Solar:
        return Solar;
    case Entity::Fusion:
        return Fusion;
    case Entity::Robo:
        return Robo;
    case Entity::Nanite:
        return Nanite;
    case Entity::Shipyard:
        return Shipyard;
    case Entity::Metalstorage:
        return Metalstorage;
    case Entity::Crystalstorage:
        return Crystalstorage;
    case Entity::Deutstorage:
        return Deutstorage;
    case Entity::Lab:
        return Lab;
    case Entity::Terra:
        return Terra;
    case Entity::Alliancedepot:
        return Alliancedepot;
    case Entity::Spacedock:
        return Spacedock;
    case Entity::Lunarbase:
        return Lunarbase;
    case Entity::Phalanx:
        return Phalanx;
    case Entity::Jumpgate:
        return Jumpgate;
    case Entity::Silo:
        return Silo;
    case Entity::Espionage:
        return Espionage;
    case Entity::Computer:
        return Computer;
    case Entity::Weapons:
        return Weapons;
    case Entity::Shielding:
        return Shielding;
    case Entity::Armour:
        return Armour;
    case Entity::Energy:
        return Energy;
    case Entity::Hyperspacetech:
        return Hyperspacetech;
    case Entity::Combustion:
        return Combustion;
    case Entity::Impulse:
        return Impulse;
    case Entity::Hyperspacedrive:
        return Hyperspacedrive;
    case Entity::Laser:
        return Laser;
    case Entity::Ion:
        return Ion;
    case Entity::Plasma:
        return Plasma;
    case Entity::Researchnetwork:
        return Researchnetwork;
    case Entity::Astro:
        return Astro;
    case Entity::Graviton:
        return Graviton;
    case Entity::None:
        return Noentity;
    default:
        return Noentity;
    }
}

// struct related functions
Resources& Resources::operator+=(const Resources& rhs) {
    met += rhs.met;
    crystal += rhs.crystal;
    deut += rhs.deut;
    return *this;
}

Resources& Resources::operator-=(const Resources& rhs) {
    met -= rhs.met;
    crystal -= rhs.crystal;
    deut -= rhs.deut;
    return *this;
}

Resources& Resources::operator*=(int i) {
    return operator*=(float(i));
}

Resources& Resources::operator*=(float f) {
    met = std::int64_t(met * f);
    crystal = std::int64_t(crystal * f);
    deut = std::int64_t(deut * f);

    return *this;
}

Resources operator+(Resources l, const Resources& r) {
    l += r;
    return l;
}

Resources operator-(Resources l, const Resources& r) {
    l -= r;
    return l;
}

Resources operator*(Resources l, float r) {
    l *= r;
    return l;
}

Resources operator*(Resources l, int r) {
    l *= r;
    return l;
}

Resources operator*(float l, Resources r) {
    return operator*(r, l);
}

Resources operator*(int l, Resources r) {
    return operator*(r, l);
}

bool operator==(const Resources& l, const Resources& r){
    return l.met == r.met && l.crystal == r.crystal && l.deut == r.deut;
}

bool operator!=(const Resources& l, const Resources& r){
    return !operator==(l, r);
}

Production& Production::operator+=(const Production& rhs) {
    met += rhs.met;
    crystal += rhs.crystal;
    deut += rhs.deut;
    return *this;
}

Production& Production::operator-=(const Production& rhs) {
    met -= rhs.met;
    crystal -= rhs.crystal;
    deut -= rhs.deut;
    return *this;
}

Production& Production::operator*=(float f) {
    met = met * f;
    crystal = crystal * f;
    deut = deut * f;

    return *this;
}

Resources Production::produce(float time) const {
    Resources res;
    res.met = std::int64_t(met) * time;
    res.crystal = std::int64_t(crystal) * time;
    res.deut = std::int64_t(deut) * time;
    return res;
}

Resources Production::produce(std::chrono::seconds period) const{
    constexpr auto zero = std::chrono::seconds::zero();
    
    assert(period >= zero);

    double hours = period.count() / 60.0 / 60.0 / 24.0;

    Resources res;
    res.met = std::int64_t(met) * hours;
    res.crystal = std::int64_t(crystal) * hours;
    res.deut = std::int64_t(deut) * hours;

    return res;
}

Production operator+(Production l, const Production& r) {
    l += r;
    return l;
}

Production operator-(Production l, const Production& r) {
    l -= r;
    return l;
}

Production operator*(Production l, float r) {
    l *= r;
    return l;
}

Production operator*(float l, const Production& r) {
    return r * l;
}

bool operator==(const Production& l, const Production& r){
    return l.met == r.met && l.crystal == r.crystal && l.deut == r.deut;
}

bool operator!=(const Production& l, const Production& r){
    return !operator==(l, r);
}


std::string thousandsSeparator(std::int64_t num) {
    std::ostringstream ss;
    ss.imbue(std::locale(""));
    ss << num;
    return ss.str();
}

Resources getBuildCosts(const EntityInfo& info, int level) {

    Resources result;

    if (level <= 0)
        return result;

    if (info.type == EntityType::Ship) {
        result.met = info.metBaseCosts * level;
        result.crystal = info.crysBaseCosts * level;
        result.deut = info.deutBaseCosts * level;
    } else {
        result.met = (std::int64_t)(info.metBaseCosts * std::pow(info.costFactor, level - 1));
        result.crystal = (std::int64_t)(info.crysBaseCosts * std::pow(info.costFactor, level - 1));
        result.deut = (std::int64_t)(info.deutBaseCosts * std::pow(info.costFactor, level - 1));
        //result.energy = (std::int64_t)(info.energyBaseCosts * std::pow(info.costFactor, level-1));
    }

    if (result.met < 0)
        result.met = std::numeric_limits<std::int64_t>::max();
    if (result.crystal < 0)
        result.crystal = std::numeric_limits<std::int64_t>::max();
    if (result.deut < 0)
        result.deut = std::numeric_limits<std::int64_t>::max();

    return result;
}

Resources getTotalCosts(const EntityInfo& info, int level) {

    Resources result;

    if (level <= 0)
        return result;

    result.met = (std::int64_t)(info.metBaseCosts * (1 - std::pow(info.costFactor, level)) / (-(info.costFactor - 1)));
    result.crystal = (std::int64_t)(info.crysBaseCosts * (1 - std::pow(info.costFactor, level)) / (-(info.costFactor - 1)));
    result.deut = (std::int64_t)(info.deutBaseCosts * (1 - std::pow(info.costFactor, level)) / (-(info.costFactor - 1)));
    //result.energy = (std::int64_t)(info.energyBaseCosts * (1 - std::pow(info.costFactor, level)) / (-info.costFactor - 1));

    if (result.met < 0)
        result.met = std::numeric_limits<std::int64_t>::max();
    if (result.crystal < 0)
        result.crystal = std::numeric_limits<std::int64_t>::max();
    if (result.deut < 0)
        result.deut = std::numeric_limits<std::int64_t>::max();

    return result;
}

std::chrono::seconds getConstructionTime(const EntityInfo& info, int level, int roboLevel, int naniLevel, int shipyardLevel, int flabLevel, int speedfactor) {

    const std::array<Entity, 4> buildingTimeExceptions = {Entity::Nanite, Entity::Lunarbase, Entity::Phalanx, Entity::Jumpgate};

    if (level <= 0){
        return std::chrono::seconds::zero();
    }

    double duration{};

    const Resources costs = getBuildCosts(info, level);

    switch (info.type) {
    case EntityType::Ship: {
        if (shipyardLevel == 0) {
            return std::chrono::seconds::max();
        } else {
            const double hours = (costs.met + costs.crystal) / (2500.f * (1 + shipyardLevel) * std::pow(2, naniLevel)) / speedfactor;
            const double seconds = hours * 60 * 60;

            duration = seconds;
        }
        break;
    }
    case EntityType::Building: {
        const bool exceptionalBuilding = std::find(buildingTimeExceptions.begin(), buildingTimeExceptions.end(), info.entity) != buildingTimeExceptions.end();
        const float factor = exceptionalBuilding ? 1.0f : std::max(4.f - level / 2.f, 1.0f);
        const double seconds = (costs.met + costs.crystal) * 1.44f / factor / (1 + roboLevel) / std::pow(2, naniLevel) / speedfactor;
        duration = seconds;

        break;
    }
    case EntityType::Research: {
        if (flabLevel == 0) {
            return std::chrono::seconds::max();
        }else{
            const double hours = (costs.met + costs.crystal) / (1000.f * (1 + flabLevel)) / speedfactor;
            const double seconds = std::ceil(hours * 60 * 60);

            duration = seconds;
        }
        break;
    }
    case EntityType::None:
        return std::chrono::seconds::zero();
        break;
    }

    duration = std::ceil(duration);

    if (duration < 0) //overflow
        return std::chrono::seconds::max();
    else
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::duration<double>{duration});
}

int getTotalLabLevel(const std::vector<int>& labsPerPlanet, int igrnLevel) {
    const int planets = int(labsPerPlanet.size());
    const int maxResearchPlanets = std::min(igrnLevel + 1, planets);

    if (maxResearchPlanets == planets) {
        return std::accumulate(labsPerPlanet.begin(), labsPerPlanet.end(), 0, std::plus<int>{});
    } else {
        std::vector<int> tmp = labsPerPlanet;
        std::nth_element(tmp.begin(), tmp.begin() + maxResearchPlanets, tmp.end(), std::greater<int>());
        return std::accumulate(tmp.begin(), tmp.begin() + maxResearchPlanets, 0, std::plus<int>{});
    }
}

std::int64_t getFKWConsumption(int fusionLevel, int fusionPercent) {
    if (fusionLevel <= 0)
        return 0;
    assert(fusionPercent >= 0);
    assert(fusionPercent <= 100);

    const std::int64_t consumptionPerHour = (std::int64_t)ceil(10. * fusionLevel * std::pow(1.1, fusionLevel) * fusionPercent / 100.0);

    assert(consumptionPerHour >= 0);

    return consumptionPerHour;
}

std::int64_t getDailyFKWConsumption(int fusionLevel, int fusionPercent) {
    if (fusionLevel <= 0)
        return 0;
    assert(fusionPercent >= 0);
    assert(fusionPercent <= 100);

    const std::int64_t consumption = getFKWConsumption(fusionLevel, fusionPercent) * 24;

    assert(consumption >= 0);

    return consumption;
}

std::int64_t getEnergyConsumption(int metlvl, int metPercent,
                                  int cryslvl, int crysPercent,
                                  int deutlvl, int deutPercent) {
    assert(metlvl >= 0);
    assert(cryslvl >= 0);
    assert(deutlvl >= 0);
    assert(metPercent >= 0);
    assert(crysPercent >= 0);
    assert(deutPercent >= 0);
    assert(metPercent <= 100);
    assert(crysPercent <= 100);
    assert(deutPercent <= 100);

    const double metFactor = metPercent / 100.0;
    const double crysFactor = crysPercent / 100.0;
    const double deutFactor = deutPercent / 100.0;

    const auto& metInfo = Metalmine;
    const auto& crysInfo = Crystalmine;
    const auto& deutInfo = Deutsynth;

    const std::int64_t metConsumption = ceil(metInfo.energyFactor * metlvl * std::pow(1.1, metlvl) * metFactor);
    const std::int64_t crysConsumption = ceil(crysInfo.energyFactor * cryslvl * std::pow(1.1, cryslvl) * crysFactor);
    const std::int64_t deutConsumption = ceil(deutInfo.energyFactor * deutlvl * std::pow(1.1, deutlvl) * deutFactor);

    const std::int64_t consumption = metConsumption + crysConsumption + deutConsumption;

    assert(consumption >= 0);

    return consumption;
}

// 100%, no officer
std::int64_t getEnergyOfSats(int count, int planetTemperatur) {
    std::int64_t energy = (std::int64_t)((planetTemperatur + 140) / 6.f) * count;
    return energy;
}

// 100%, no officer
std::int64_t getEnergyOfSolarPlant(int level) {
    std::int64_t energy = (std::int64_t)(20 * level * std::pow(1.1f, level));
    return energy;
}

// 100%, no officer
std::int64_t getEnergyOfFKW(int level, int etechLevel) {
    std::int64_t energy = (std::int64_t)(30 * level * std::pow((1.05f + etechLevel * 0.01f), level));
    return energy;
}

std::int64_t applyEnergyFactor(std::int64_t energy, float productionfactor, bool hasEngineer, bool hasStaff) {
    const int engineerPercent = hasEngineer ? 10 : 0;
    const int staffPercent = hasStaff ? 2 : 0;
    const float officerfactor = 1.0f + (engineerPercent + staffPercent) / 100.f;
    return std::int64_t(std::round(energy * productionfactor * officerfactor));
}

std::int64_t getTotalEnergy(int solarplantLevel, int solarplantPercent,
                            int fusionLevel, int fusionPercent, int etechLevel,
                            int sats, int satsPercent, int temperature,
                            bool engineer, bool staff) {

    assert(solarplantPercent >= 0);
    assert(fusionPercent >= 0);
    assert(satsPercent >= 0);
    assert(solarplantPercent <= 100);
    assert(fusionPercent <= 100);
    assert(satsPercent <= 100);

    std::int64_t solarenergy = getEnergyOfSolarPlant(solarplantLevel);
    solarenergy = applyEnergyFactor(solarenergy, solarplantPercent / 100.0f, engineer, staff);

    std::int64_t fusionenergy = getEnergyOfFKW(fusionLevel, etechLevel);
    fusionenergy = applyEnergyFactor(fusionenergy, fusionPercent / 100.0f, engineer, staff);

    std::int64_t satsenergy = getEnergyOfSats(sats, temperature);
    satsenergy = applyEnergyFactor(satsenergy, satsPercent / 100.0f, engineer, staff);

    const std::int64_t totalenergy = solarenergy + fusionenergy + satsenergy;

    return totalenergy;
}

int getItemProductionPercent(ItemRarity item) {
    switch (item) {
    case ItemRarity::Bronze:
        return 10;
    case ItemRarity::Silver:
        return 20;
    case ItemRarity::Gold:
        return 30;
    case ItemRarity::None:
        return 0;
    }
    return 0;
}

Production getDefaultProduction() {
    Production result{default_production_met, default_production_crys, default_production_deut};
    return result;
}

Production getDailyDefaultProduction() {
    Production result{default_production_met * 24, default_production_crys * 24, default_production_deut * 24};
    return result;
}

double getMineProductionFactor(int metLevel, int metPercent,
                               int crysLevel, int crysPercent,
                               int deutLevel, int deutPercent,
                               int solarplantLevel, int solarplantPercent,
                               int fusionLevel, int fusionPercent, int etechLevel,
                               int sats, int satsPercent, int temperature,
                               bool engineer, bool staff) {
    assert(metPercent >= 0);
    assert(crysPercent >= 0);
    assert(deutPercent >= 0);
    assert(solarplantPercent >= 0);
    assert(fusionPercent >= 0);
    assert(satsPercent >= 0);
    assert(metPercent <= 100);
    assert(crysPercent <= 100);
    assert(deutPercent <= 100);
    assert(solarplantPercent <= 100);
    assert(fusionPercent <= 100);
    assert(satsPercent <= 100);

    const std::int64_t requiredenergy = getEnergyConsumption(metLevel, metPercent, crysLevel, crysPercent, deutLevel, deutPercent);

    const std::int64_t totalenergy = getTotalEnergy(solarplantLevel, solarplantPercent,
                                                    fusionLevel, fusionPercent, etechLevel,
                                                    sats, satsPercent, temperature,
                                                    engineer, staff);

    const double mineproductionfactor = totalenergy == 0 ? 0.0f : std::min(1.0, double(totalenergy) / double(requiredenergy));
    return mineproductionfactor;
}

Production getDailyProduction(int metLevel, ItemRarity metItem, int metPercent,
                              int crysLevel, ItemRarity crysItem, int crysPercent,
                              int deutLevel, ItemRarity deutItem, int deutPercent,
                              int solarLevel, int solarplantPercent,
                              int fusionLevel, int fusionPercent, int etechLevel,
                              int temperature, int sats, int satsPercent,
                              int plasmaLevel, int speedfactor,
                              bool engineer, bool geologist, bool staff) {

    assert(metLevel >= 0);
    assert(crysLevel >= 0);
    assert(deutLevel >= 0);
    assert(metPercent >= 0);
    assert(crysPercent >= 0);
    assert(deutPercent >= 0);
    assert(plasmaLevel >= 0);
    assert(speedfactor >= 1);

    const int geologistpercent = geologist ? 10 : 0;
    const int staffpercent = staff ? 2 : 0;
    const double officerfactor = (geologistpercent + staffpercent) / 100.;

    const Production defaultProduction = getDefaultProduction();

    double simpleProduction_met = 30 * metLevel * std::pow(1.1, metLevel) * metPercent / 100.;
    double simpleProduction_crystal = 20 * crysLevel * std::pow(1.1, crysLevel) * crysPercent / 100.;
    double simpleProduction_deut = (10 * deutLevel * std::pow(1.1, deutLevel) * (1.44 - 0.004 * temperature) * deutPercent / 100.);

    const double mineproductionfactor = getMineProductionFactor(metLevel, metPercent,
                                                                crysLevel, crysPercent,
                                                                deutLevel, deutPercent,
                                                                solarLevel, solarplantPercent,
                                                                fusionLevel, fusionPercent, etechLevel,
                                                                sats, satsPercent, temperature,
                                                                engineer, staff);
    simpleProduction_met *= mineproductionfactor;
    simpleProduction_crystal *= mineproductionfactor;
    simpleProduction_deut *= mineproductionfactor;

    const double itemProduction_met = simpleProduction_met * getItemProductionPercent(metItem) / 100.;
    const double itemProduction_crystal = simpleProduction_crystal * getItemProductionPercent(crysItem) / 100.;
    const double itemProduction_deut = simpleProduction_deut * getItemProductionPercent(deutItem) / 100.;

    const double plasmaProduction_met = simpleProduction_met / 100. * plasma_factor_met * plasmaLevel;
    const double plasmaProduction_crystal = simpleProduction_crystal / 100. * plasma_factor_crys * plasmaLevel;
    const double plasmaProduction_deut = simpleProduction_deut / 100. * plasma_factor_deut * plasmaLevel;

    const double extraOfficerProduction_met = simpleProduction_met * officerfactor;
    const double extraOfficerProduction_crystal = simpleProduction_crystal * officerfactor;
    const double extraOfficerProduction_deut = simpleProduction_deut * officerfactor;

    double result_met = (simpleProduction_met + itemProduction_met + plasmaProduction_met + extraOfficerProduction_met);
    double result_crystal = (simpleProduction_crystal + itemProduction_crystal + plasmaProduction_crystal + extraOfficerProduction_crystal);
    double result_deut = (simpleProduction_deut + itemProduction_deut + plasmaProduction_deut + extraOfficerProduction_deut);

    result_met += defaultProduction.met;
    result_crystal += defaultProduction.crystal;
    result_deut += defaultProduction.deut;

    const std::int64_t fkwdeutconsumption = getFKWConsumption(fusionLevel, fusionPercent);
    result_deut -= fkwdeutconsumption;

    result_met *= speedfactor;
    result_crystal *= speedfactor;
    result_deut *= speedfactor;

    Production result;
    result.met = round(result_met);
    result.crystal = round(result_crystal);
    result.deut = round(result_deut);

    if (result.met < 0)
        result.met = std::numeric_limits<int>::max();
    if (result.crystal < 0)
        result.crystal = std::numeric_limits<int>::max();
    //if(result.deut < 0) result.deut = std::numeric_limits<std::int64_t>::max();

    result *= 24;

    return result;
}

std::chrono::seconds get_save_duration_symmetrictrade(const std::int64_t hm, const std::uint64_t hk, const std::int64_t hd, /*have*/
                                       const std::int64_t wm, const std::int64_t wk, const std::int64_t wd,  /*want*/
                                       const std::int64_t pm, const std::int64_t pk, const std::int64_t pd,  /*production*/
                                       const std::array<float, 3>& traderate /*e.g 3:2:1*/) {
    assert(traderate[0] > 0);
    assert(traderate[1] > 0);
    assert(traderate[2] > 0);

    const std::int64_t nm = wm - hm;
    const std::int64_t nk = wk - hk;
    const std::int64_t nd = wd - hd;

    const std::int64_t n_dse = nm / traderate[0] * traderate[2] + nk / traderate[1] * traderate[2] + nd;

    if (n_dse <= 0.0f)
        return std::chrono::seconds::zero();

    const std::int64_t p_dse = pm / traderate[0] * traderate[2] + pk / traderate[1] * traderate[2] + pd;

    if (p_dse <= 0.0f)
        return std::chrono::seconds::max();

    double save_duration_days = double(n_dse) / p_dse;

    double seconds = std::ceil(save_duration_days * 24 * 60 * 60);
    auto secs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::duration<double>{seconds});

    return secs;
}

float get_save_duration_notrade(const std::int64_t hm, const std::int64_t hk, const std::int64_t hd, /*have*/
                                const std::int64_t wm, const std::int64_t wk, const std::int64_t wd, /*want*/
                                const std::int64_t pm, const std::int64_t pk, const std::int64_t pd /*production*/) {

    assert(hm >= 0);
    assert(hk >= 0);
    assert(hd >= 0);
    assert(wm >= 0);
    assert(wk >= 0);
    assert(wd >= 0);
    assert(pm >= 0);
    assert(pk >= 0);
    assert(pd >= 0);

    const std::int64_t nm = wm - hm;
    const std::int64_t nk = wk - hk;
    const std::int64_t nd = wd - hd;

    if (nm + nk + nd == 0) {
        return 0.0f;
    }

    const float tm = pm == 0 ? (nm == 0 ? 0 : std::numeric_limits<float>::max()) : float(nm) / pm;
    const float tk = pk == 0 ? (nk == 0 ? 0 : std::numeric_limits<float>::max()) : float(nk) / pk;
    const float td = pd == 0 ? (nd == 0 ? 0 : std::numeric_limits<float>::max()) : float(nd) / pd;

    const float save_duration = std::max(tm, std::max(tk, td));

    return save_duration;
}

std::int64_t getConstructionTimeReductionDM(float totalConstructionTimeDays) {
    const std::int64_t minutes = std::int64_t(totalConstructionTimeDays * 60 * 24);
    return SDIV(minutes, 30) * 750;
}

std::int64_t getNonResearchTimeReductionDM(float totalConstructionTimeDays) {
    const std::int64_t minutes = getConstructionTimeReductionDM(totalConstructionTimeDays);
    return std::min(minutes, std::int64_t(72000));
}

std::int64_t getResearchTimeReductionDM(float totalConstructionTimeDays) {
    const std::int64_t minutes = getConstructionTimeReductionDM(totalConstructionTimeDays);
    return std::min(minutes, std::int64_t(108000));
}

int getMaxPossiblePlanets(int astroLevel) {
    return std::ceil(astroLevel / 2.f) + 1;
}

int getMinPossibleAstro(int num_planets) {
    if (num_planets == 1)
        return 0;
    int astro = 1;
    for (int i = 0; i < num_planets - 2; i++)
        astro += 2;
    return astro;
}

int getNumberOfFleetSlots(int computerLevel) {
    return computerLevel + 1;
}

int getNumberOfFleetSlotsWithOfficers(int computerLevel, bool hasAdmiral, bool hasStaff) {
    constexpr int admiralSlots = 2;
    constexpr int staffSlots = 1;

    const int extraAdmiralSlots = hasAdmiral ? admiralSlots : 0;
    const int extraStaffSlots = hasStaff ? staffSlots : 0;
    return getNumberOfFleetSlots(computerLevel) + extraAdmiralSlots + extraStaffSlots;
}

int getNumberOfExpeditionSlots(int astroLevel) {
    return int(std::sqrt(astroLevel));
}

int getNumberOfExpeditionSlotsWithOfficers(int astroLevel, bool hasAdmiral, bool hasStaff) {
    constexpr int admiralSlots = 1;
    constexpr int staffSlots = 0;

    const int extraAdmiralSlots = hasAdmiral ? admiralSlots : 0;
    const int extraStaffSlots = hasStaff ? staffSlots : 0;
    return getNumberOfExpeditionSlots(astroLevel) + extraAdmiralSlots + extraStaffSlots;
}

std::map<Entity, std::string> entityToNameMap = getEntityToNameMap();

std::map<Entity, std::string> getEntityToNameMap() {
    std::map<Entity, std::string> map;
    map[Entity::Metalmine] = "Metal Mine";
    map[Entity::Crystalmine] = "Crystal Mine";
    map[Entity::Deutsynth] = "Deuterium Synthesizer";
    map[Entity::Solar] = "Solar Planet";
    map[Entity::Fusion] = "Fusion Reactor";
    map[Entity::Robo] = "Robotics Factory";
    map[Entity::Nanite] = "Nanite Factory";
    map[Entity::Shipyard] = "Shipyard";
    map[Entity::Metalstorage] = "Metal Storage";
    map[Entity::Crystalstorage] = "Crystal Storage";
    map[Entity::Deutstorage] = "Deuterium Tank";
    map[Entity::Lab] = "Research Lab";
    map[Entity::Terra] = "Terraformer";
    map[Entity::Alliancedepot] = "Alliance Depot";
    map[Entity::Spacedock] = "Space Dock";
    map[Entity::Lunarbase] = "Lunar Base";
    map[Entity::Phalanx] = "Sensor Phalanx";
    map[Entity::Jumpgate] = "Jump Gate";
    map[Entity::Silo] = "Missile Silo";
    map[Entity::Espionage] = "Espionage Technology";
    map[Entity::Computer] = "Computer Technology";
    map[Entity::Weapons] = "Weapons Technology";
    map[Entity::Shielding] = "Shielding Technology";
    map[Entity::Armour] = "Armour Technology";
    map[Entity::Energy] = "Energy Technology";
    map[Entity::Hyperspacetech] = "Hyperspace Technology";
    map[Entity::Combustion] = "Combustion Drive";
    map[Entity::Impulse] = "Impulse Drive";
    map[Entity::Hyperspacedrive] = "Hyperspace Drive";
    map[Entity::Laser] = "Laser Technology";
    map[Entity::Ion] = "Ion Technology";
    map[Entity::Plasma] = "Plasma Technology";
    map[Entity::Researchnetwork] = "Intergalactic Research Network";
    map[Entity::Astro] = "Astrophysics";
    map[Entity::Graviton] = "Graviton Technology";
    map[Entity::None] = "None";
    return map;
}

std::string getEntityName(Entity e) {
    return entityToNameMap.at(e);
}

EntityInfo parseEntityName(const std::string& name) {

    EntityInfo entity;

    if (name == "Metal Mine") {
        entity = Metalmine;
    } else if (name == "Crystal Mine") {
        entity = Crystalmine;
    } else if (name == "Deuterium Synthesizer") {
        entity = Deutsynth;
    } else if (name == "Solar Plant") {
        entity = Solar;
    } else if (name == "Fusion Reactor") {
        entity = Fusion;
    } else if (name == "Robotics Factory") {
        entity = Robo;
    } else if (name == "Nanite Factory") {
        entity = Nanite;
    } else if (name == "Shipyard") {
        entity = Shipyard;
    } else if (name == "Metal Storage") {
        entity = Metalstorage;
    } else if (name == "Crystal Storage") {
        entity = Crystalstorage;
    } else if (name == "Deuterium Tank") {
        entity = Deutstorage;
    } else if (name == "Research Lab") {
        entity = Lab;
    } else if (name == "Terraformer") {
        entity = Terra;
    } else if (name == "Alliance Depot") {
        entity = Alliancedepot;
    } else if (name == "Space Dock") {
        entity = Spacedock;
    } else if (name == "Lunar Base") {
        entity = Lunarbase;
    } else if (name == "Sensor Phalanx") {
        entity = Phalanx;
    } else if (name == "Jump Gate") {
        entity = Jumpgate;
    } else if (name == "Missile Silo") {
        entity = Silo;
    } else if (name == "Espionage Technology") {
        entity = Espionage;
    } else if (name == "Computer Technology") {
        entity = Computer;
    } else if (name == "Weapons Technology") {
        entity = Weapons;
    } else if (name == "Shielding Technology") {
        entity = Shielding;
    } else if (name == "Armor Technology") {
        entity = Armour;
    } else if (name == "Energy Technology") {
        entity = Energy;
    } else if (name == "Hyperspace Technology") {
        entity = Hyperspacetech;
    } else if (name == "Combustion Drive") {
        entity = Combustion;
    } else if (name == "Impulse Drive") {
        entity = Impulse;
    } else if (name == "Hyperspace Drive") {
        entity = Hyperspacedrive;
    } else if (name == "Laser Technology") {
        entity = Laser;
    } else if (name == "Ion Technology") {
        entity = Ion;
    } else if (name == "Plasma Technology") {
        entity = Plasma;
    } else if (name == "Intergalactic Research Network") {
        entity = Researchnetwork;
    } else if (name == "Astrophysics") {
        entity = Astro;
    } else if (name == "Graviton Technology") {
        entity = Graviton;
    } else if (name == "Colonization Ship") {
        entity = Coloship;
    } else if (name == "none" || name == "") {
        entity = Noentity;
    } else {
        std::cout << "Invalid entity name:" << name << std::endl;
        throw std::runtime_error("");
    }
    return entity;
}

ItemRarity parseItemRarityName(const std::string& rarityname) {
    std::string s(rarityname);
    std::transform(s.begin(), s.end(), s.begin(), [](auto c) { return std::tolower(c); });

    if (s == "b" || s == "bronze")
        return ItemRarity::Bronze;
    else if (s == "s" || s == "silver")
        return ItemRarity::Silver;
    else if (s == "g" || s == "gold")
        return ItemRarity::Gold;
    else
        return ItemRarity::None;
}

std::string itemRarityToName(const ItemRarity& r) {
    switch (r) {
    case ItemRarity::Bronze:
        return "bronze";
    case ItemRarity::Silver:
        return "silver";
    case ItemRarity::Gold:
        return "gold";
    default:
        return "none";
    }
}
} // namespace ogamehelpers
