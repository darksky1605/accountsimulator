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

std::int64_t Resources::metal() const{
    return std::floor(met);
}

std::int64_t Resources::crystal() const{
    return std::floor(crys);
}

std::int64_t Resources::deuterium() const{
    return std::floor(deut);
}

double Resources::dse(const std::array<float, 3>& traderate) const{
    return met / traderate[0] * traderate[2] + crys / traderate[1] * traderate[2] + deut;
}

void Resources::setMetal(std::int64_t m){
    met = m;
}

void Resources::setCrystal(std::int64_t k){
    crys = k;
}

void Resources::setDeuterium(std::int64_t d){
    deut = d;
}


Resources::Resources(std::int64_t m, std::int64_t k, std::int64_t d){
    setMetal(m);
    setCrystal(k);
    setDeuterium(d);
}

Resources& Resources::operator+=(const Resources& rhs) {
    met += rhs.met;
    crys += rhs.crys;
    deut += rhs.deut;
    return *this;
}

Resources& Resources::operator-=(const Resources& rhs) {
    met -= rhs.met;
    crys -= rhs.crys;
    deut -= rhs.deut;
    return *this;
}

Resources& Resources::operator*=(int i) {
    return operator*=(float(i));
}

Resources& Resources::operator*=(float f) {
    met = met * f;
    crys = crys * f;
    deut = deut * f;

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
    return l.metal() == r.metal() && l.crystal() == r.crystal() && l.deuterium() == r.deuterium();
}

bool operator!=(const Resources& l, const Resources& r){
    return !operator==(l, r);
}










std::int64_t Production::metal() const{
    return met;
}

std::int64_t Production::crystal() const{
    return crys;
}

std::int64_t Production::deuterium() const{
    return deut;
}


Production Production::makeProductionPerSeconds(){
    return makeProductionPerSeconds(0,0,0);
}

Production Production::makeProductionPerSeconds(std::int64_t m, std::int64_t c, std::int64_t d){
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::seconds{1});
    return makeProduction(seconds, m, c, d);
}

Production Production::makeProductionPerMinute(){
    return makeProductionPerMinute(0,0,0);
}

Production Production::makeProductionPerMinute(std::int64_t m, std::int64_t c, std::int64_t d){
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::minutes{1});
    return makeProduction(seconds, m, c, d);
}

Production Production::makeProductionPerHour(){
    return makeProductionPerHour(0,0,0);
}

Production Production::makeProductionPerHour(std::int64_t m, std::int64_t c, std::int64_t d){
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours{1});
    return makeProduction(seconds, m, c, d);
}

Production Production::makeProductionPerDay(){
    return makeProductionPerDay(0,0,0);
}

Production Production::makeProductionPerDay(std::int64_t m, std::int64_t c, std::int64_t d){
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::hours{24});
    return makeProduction(seconds, m, c, d);
}

Production Production::makeProduction(std::chrono::seconds r, std::int64_t m, std::int64_t c, std::int64_t d){
    Production p;
    p.r = r;
    p.met = m;
    p.crys = c;
    p.deut = d;
    return p;
}

Production& Production::operator+=(const Production& rhs) {
    const double ratio = double(r.count()) / double(rhs.r.count());
    met += rhs.met * ratio;
    crys += rhs.crys * ratio;
    deut += rhs.deut * ratio;
    return *this;
}

Production& Production::operator-=(const Production& rhs) {
    const double ratio = double(r.count()) / double(rhs.r.count());
    met -= rhs.met * ratio;
    crys -= rhs.crys * ratio;
    deut -= rhs.deut * ratio;
    return *this;
}

Production& Production::operator*=(float f) {
    met = met * f;
    crys = crys * f;
    deut = deut * f;

    return *this;
}

Resources Production::produceForSeconds(std::chrono::seconds period) const{
    constexpr auto zero = std::chrono::seconds::zero();
    
    assert(period >= zero);

    const double ratio = double(period.count()) / double(r.count());

    Resources res;
    res.met = met * ratio;
    res.crys = crys * ratio;
    res.deut = deut * ratio;

    return res;
}

Resources Production::produce(std::chrono::seconds period) const{
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(period);
    return produceForSeconds(seconds);
}

Resources Production::produce(std::chrono::minutes period) const{
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(period);
    return produceForSeconds(seconds);
}

Resources Production::produce(std::chrono::hours period) const{
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(period);
    return produceForSeconds(seconds);
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
    return l.metal() == r.metal() && l.crystal() == r.crystal() && l.deuterium() == r.deuterium();
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
        result.setMetal(info.metBaseCosts * level);
        result.setCrystal(info.crysBaseCosts * level);
        result.setDeuterium(info.deutBaseCosts * level);
    } else {
        std::int64_t met = (std::int64_t)(info.metBaseCosts * std::pow(info.costFactor, level - 1));
        std::int64_t crys = (std::int64_t)(info.crysBaseCosts * std::pow(info.costFactor, level - 1));
        std::int64_t deut = (std::int64_t)(info.deutBaseCosts * std::pow(info.costFactor, level - 1));

        if(info.entity == Entity::Astro){
            met = (met / 100) * 100;
            crys = (crys / 100) * 100;
            deut = (deut / 100) * 100;
        }

        result.setMetal(met);
        result.setCrystal(crys);
        result.setDeuterium(deut);        
    }

    if (result.metal() < 0)
        result.setMetal(std::numeric_limits<std::int64_t>::max());
    if (result.crystal() < 0)
        result.setCrystal(std::numeric_limits<std::int64_t>::max());
    if (result.deuterium() < 0)
        result.setDeuterium(std::numeric_limits<std::int64_t>::max());

    return result;
}

Resources getTotalCosts(const EntityInfo& info, int level) {

    Resources result;

    if (level <= 0)
        return result;

    result.setMetal((std::int64_t)(info.metBaseCosts * (1 - std::pow(info.costFactor, level)) / (-(info.costFactor - 1))));
    result.setCrystal((std::int64_t)(info.crysBaseCosts * (1 - std::pow(info.costFactor, level)) / (-(info.costFactor - 1))));
    result.setDeuterium((std::int64_t)(info.deutBaseCosts * (1 - std::pow(info.costFactor, level)) / (-(info.costFactor - 1))));
    //result.energy = (std::int64_t)(info.energyBaseCosts * (1 - std::pow(info.costFactor, level)) / (-info.costFactor - 1));

    if (result.metal() < 0)
        result.setMetal(std::numeric_limits<std::int64_t>::max());
    if (result.crystal() < 0)
        result.setCrystal(std::numeric_limits<std::int64_t>::max());
    if (result.deuterium() < 0)
        result.setDeuterium(std::numeric_limits<std::int64_t>::max());

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
            const double hours = (costs.metal() + costs.crystal()) / (2500.f * (1 + shipyardLevel) * std::pow(2, naniLevel)) / speedfactor;
            const double seconds = hours * 60 * 60;

            duration = seconds;
        }
        break;
    }
    case EntityType::Building: {
        const bool exceptionalBuilding = std::find(buildingTimeExceptions.begin(), buildingTimeExceptions.end(), info.entity) != buildingTimeExceptions.end();
        const float factor = exceptionalBuilding ? 1.0f : std::max(4.f - level / 2.f, 1.0f);
        const double seconds = (costs.metal() + costs.crystal()) * 1.44f / factor / (1 + roboLevel) / std::pow(2, naniLevel) / speedfactor;
        duration = seconds;

        break;
    }
    case EntityType::Research: {
        if (flabLevel == 0) {
            return std::chrono::seconds::max();
        }else{
            const double hours = (costs.metal() + costs.crystal()) / (1000.f * (1 + flabLevel)) / speedfactor;
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
                                  int deutlvl, int deutPercent,
                                  int numcrawler, int crawlerPercent) {
    assert(metlvl >= 0);
    assert(cryslvl >= 0);
    assert(deutlvl >= 0);
    assert(numcrawler >= 0);
    assert(metPercent >= 0);
    assert(crysPercent >= 0);
    assert(deutPercent >= 0);
    assert(crawlerPercent >= 0);
    assert(metPercent <= 100);
    assert(crysPercent <= 100);
    assert(deutPercent <= 100);
    assert(crawlerPercent <= 100);

    const double metFactor = metPercent / 100.0;
    const double crysFactor = crysPercent / 100.0;
    const double deutFactor = deutPercent / 100.0;
    const double crawlerFactor = crawlerPercent / 100.0;

    const auto& metInfo = Metalmine;
    const auto& crysInfo = Crystalmine;
    const auto& deutInfo = Deutsynth;
    const auto& crawlerInfo = Crawler;

    const std::int64_t metConsumption = std::ceil(metInfo.energyFactor * metlvl * std::pow(1.1, metlvl) * metFactor);
    const std::int64_t crysConsumption = std::ceil(crysInfo.energyFactor * cryslvl * std::pow(1.1, cryslvl) * crysFactor);
    const std::int64_t deutConsumption = std::ceil(deutInfo.energyFactor * deutlvl * std::pow(1.1, deutlvl) * deutFactor);
    const std::int64_t crawlerConsumption = std::ceil(crawlerInfo.energyFactor * numcrawler * crawlerFactor);

    const std::int64_t consumption = metConsumption + crysConsumption + deutConsumption + crawlerConsumption;

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

std::int64_t applyEnergyFactor(std::int64_t energy, float productionfactor, 
                                bool hasEngineer, bool hasStaff, CharacterClass cclass){
    const int engineerPercent = hasEngineer ? 10 : 0;
    const int staffPercent = hasStaff ? 2 : 0;
    const int classPercent = cclass == CharacterClass::Collector ? 10 : 0;
    const float bonusfactor = 1.0f + (engineerPercent + staffPercent + classPercent) / 100.f;
    return std::int64_t(std::round(energy * productionfactor * bonusfactor));
}

std::int64_t getTotalEnergy(int solarplantLevel, int solarplantPercent,
                            int fusionLevel, int fusionPercent, int etechLevel,
                            int sats, int satsPercent, int temperature,
                            bool engineer, bool staff,
                            CharacterClass cclass) {

    assert(solarplantPercent >= 0);
    assert(fusionPercent >= 0);
    assert(satsPercent >= 0);
    assert(solarplantPercent <= 100);
    assert(fusionPercent <= 100);
    assert(satsPercent <= 100);

    std::int64_t solarenergy = getEnergyOfSolarPlant(solarplantLevel);
    solarenergy = applyEnergyFactor(solarenergy, solarplantPercent / 100.0f, engineer, staff, cclass);

    std::int64_t fusionenergy = getEnergyOfFKW(fusionLevel, etechLevel);
    fusionenergy = applyEnergyFactor(fusionenergy, fusionPercent / 100.0f, engineer, staff, cclass);

    std::int64_t satsenergy = getEnergyOfSats(sats, temperature);
    satsenergy = applyEnergyFactor(satsenergy, satsPercent / 100.0f, engineer, staff, cclass);

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
    //Production result{default_production_met, default_production_crys, default_production_deut};
    //return result;
    return Production::makeProductionPerHour(default_production_met, default_production_crys, default_production_deut);
}

Production getDailyDefaultProduction() {
    //Production result{default_production_met * 24, default_production_crys * 24, default_production_deut * 24};
    //return result;
    return Production::makeProductionPerHour(default_production_met * 24, default_production_crys * 24, default_production_deut * 24);
}

double getMineProductionFactor(int metLevel, int metPercent,
                               int crysLevel, int crysPercent,
                               int deutLevel, int deutPercent,
                               int solarplantLevel, int solarplantPercent,
                               int fusionLevel, int fusionPercent, int etechLevel,
                               int sats, int satsPercent, int temperature,
                               int crawler, int crawlerPercent,
                               bool engineer, bool staff,
                               CharacterClass cclass) {
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

    const std::int64_t requiredenergy = getEnergyConsumption(
        metLevel, metPercent, 
        crysLevel, crysPercent, 
        deutLevel, deutPercent,
        crawler, crawlerPercent
    );

    const std::int64_t totalenergy = getTotalEnergy(solarplantLevel, solarplantPercent,
                                                    fusionLevel, fusionPercent, etechLevel,
                                                    sats, satsPercent, temperature,
                                                    engineer, staff,
                                                    cclass);

    const double mineproductionfactor = totalenergy == 0 ? 0.0f : std::min(1.0, double(totalenergy) / double(requiredenergy));
    return mineproductionfactor;
}

Production getDailyProduction(int metLevel, ItemRarity metItem, int metPercent,
                              int crysLevel, ItemRarity crysItem, int crysPercent,
                              int deutLevel, ItemRarity deutItem, int deutPercent,
                              int solarLevel, int solarplantPercent,
                              int fusionLevel, int fusionPercent, int etechLevel,
                              int temperature, int sats, int satsPercent,
                              int numcrawler, int crawlerPercent,
                              int plasmaLevel, int speedfactor,
                              bool engineer, bool geologist, bool staff,
                              CharacterClass cclass) {

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

    const int classPercent = cclass == CharacterClass::Collector ? 25 : 0;
    const double classFactor = (classPercent) / 100.;

    constexpr int crawler_y_factor = 8;
    const int totalMineLevel = metLevel + crysLevel + deutLevel;
    numcrawler = std::min(numcrawler, totalMineLevel * crawler_y_factor);

    constexpr int crawler_boost_factor = 0.0002;
    const double crawlerClassFactor = cclass == CharacterClass::Collector ? 1.5 : 1.0;    
    const double crawlerFactor = crawler_boost_factor * crawlerClassFactor;

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
                                                                numcrawler, crawlerPercent,
                                                                engineer, staff,
                                                                cclass);
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

    const double classProduction_met = simpleProduction_met * classFactor;
    const double classProduction_crystal = simpleProduction_crystal * classFactor;
    const double classProduction_deut = simpleProduction_deut * classFactor;

    const double crawlerProduction_met = std::round(simpleProduction_met * crawlerFactor * numcrawler);
    const double crawlerProduction_crystal = std::round(simpleProduction_crystal * crawlerFactor * numcrawler);
    const double crawlerProduction_deut = std::round(simpleProduction_deut * crawlerFactor * numcrawler);

    double result_met = (simpleProduction_met 
                        + itemProduction_met 
                        + plasmaProduction_met 
                        + extraOfficerProduction_met
                        + classProduction_met
                        + crawlerProduction_met);
    double result_crystal = (simpleProduction_crystal 
                        + itemProduction_crystal 
                        + plasmaProduction_crystal 
                        + extraOfficerProduction_crystal
                        + classProduction_crystal
                        + crawlerProduction_crystal);
    double result_deut = (simpleProduction_deut 
                        + itemProduction_deut 
                        + plasmaProduction_deut 
                        + extraOfficerProduction_deut
                        + classProduction_deut
                        + crawlerProduction_deut);

    result_met += defaultProduction.metal();
    result_crystal += defaultProduction.crystal();
    result_deut += defaultProduction.deuterium();

    const std::int64_t fkwdeutconsumption = getFKWConsumption(fusionLevel, fusionPercent);
    result_deut -= fkwdeutconsumption;

    result_met *= speedfactor;
    result_crystal *= speedfactor;
    result_deut *= speedfactor;

    Production result = Production::makeProductionPerDay(round(result_met), round(result_crystal), round(result_deut));
    
    if (result.metal() < 0){
        std::cerr << "Error, met production is negative\n";
        assert(false);
        return result;
    }
    if (result.crystal() < 0){
        std::cerr << "Error, crys production is negative\n";
        assert(false);
        return result;
    }

    result *= 24;

    return result;
}


std::chrono::seconds get_save_duration_symmetrictrade(const Resources& have, /*have*/
                                       const Resources& want,  /*want*/
                                       const Production& production,  /*production*/
                                       const std::array<float, 3>& traderate /*e.g 3:2:1*/){
    assert(traderate[0] > 0);
    assert(traderate[1] > 0);
    assert(traderate[2] > 0);

    const Resources need = want - have;

    const double n_dse = need.dse(traderate);

    if (n_dse <= 0.0)
        return std::chrono::seconds::zero();

    const double p_dse = production.produce(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::seconds{1})).dse(traderate);

    if (p_dse <= 0.0)
        return std::chrono::seconds::max();

    double save_duration_seconds = std::ceil(n_dse / p_dse);

    auto secs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::duration<double>{save_duration_seconds});

    return secs;
}

std::chrono::seconds get_save_duration_notrade(const Resources& have,
                                       const Resources& want,
                                       const Production& production){

    const Resources need = want - have;

    if(need.metal() + need.crystal() + need.deuterium() <= 0){
        return std::chrono::seconds::zero();
    }

    const Resources resPerSecond = production.produce(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::seconds{1}));

    const double tm = resPerSecond.metal() == 0 ? (need.metal() == 0 ? 0 : std::numeric_limits<double>::max()) : double(need.metal()) / resPerSecond.metal();
    const double tk = resPerSecond.crystal() == 0 ? (need.crystal() == 0 ? 0 : std::numeric_limits<double>::max()) : double(need.crystal()) / resPerSecond.crystal();
    const double td = resPerSecond.deuterium() == 0 ? (need.deuterium() == 0 ? 0 : std::numeric_limits<double>::max()) : double(need.deuterium()) / resPerSecond.deuterium();

    const double seconds = std::max(tm, std::max(tk,td));

    const auto secs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::duration<double>{seconds});

    return secs;
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

int getNumberOfFleetSlots(int computerLevel, bool hasAdmiral, bool hasStaff, CharacterClass cclass) {
    constexpr int admiralSlots = 2;
    constexpr int staffSlots = 1;
    constexpr int classGeneralSlots = 2;

    const int extraAdmiralSlots = hasAdmiral ? admiralSlots : 0;
    const int extraStaffSlots = hasStaff ? staffSlots : 0;
    const int extraGeneralSlots = cclass == CharacterClass::General ? classGeneralSlots : 0;

    return getNumberOfFleetSlots(computerLevel) + extraAdmiralSlots + extraStaffSlots + extraGeneralSlots;
}

int getNumberOfExpeditionSlots(int astroLevel) {
    return int(std::sqrt(astroLevel));
}

int getNumberOfExpeditionSlots(int astroLevel, bool hasAdmiral, bool hasStaff, CharacterClass cclass) {
    constexpr int admiralSlots = 1;
    constexpr int staffSlots = 0;
    constexpr int classDiscovererSlots = 2;

    const int extraAdmiralSlots = hasAdmiral ? admiralSlots : 0;
    const int extraStaffSlots = hasStaff ? staffSlots : 0;
    const int extraDiscovererSlots = cclass == CharacterClass::Discoverer ? classDiscovererSlots : 0;

    return getNumberOfExpeditionSlots(astroLevel) + extraAdmiralSlots + extraStaffSlots + extraDiscovererSlots;
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
    map[Entity::Coloship] = "Colony Ship";
    map[Entity::Crawler] = "Crawler";
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
    } else if (name == "Colony Ship") {
        entity = Coloship;
    } else if (name == "Crawler") {
        entity = Crawler;  
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

CharacterClass parseCharacterClassName(const std::string& classname){
    std::string s(classname);
    std::transform(s.begin(), s.end(), s.begin(), [](auto c) { return std::tolower(c); });

    if(s == "collector")
        return CharacterClass::Collector;
    else if(s == "discoverer")
        return CharacterClass::Discoverer;
    else if(s == "general")
        return CharacterClass::General;
    else
        return CharacterClass::None;
}

std::string characterClassToName(const CharacterClass& cclass){
    switch(cclass){
    case CharacterClass::Collector:
            return "Collector";
    case CharacterClass::Discoverer:
        return "Discoverer";
    case CharacterClass::General:
        return "General";
    default:
        return "None";
    }
}


} // namespace ogamehelpers
