#ifndef OGAME_HPP
#define OGAME_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <cassert>
#include <functional>
#include <iomanip>

#include <chrono>

namespace ogamehelpers {

enum class EntityType { Building,
                        Research,
                        Ship,
                        None };
enum class Entity { Metalmine,
                    Crystalmine,
                    Deutsynth,
                    Solar,
                    Fusion,
                    Robo,
                    Nanite,
                    Shipyard,
                    Metalstorage,
                    Crystalstorage,
                    Deutstorage,
                    Lab,
                    Terra,
                    Alliancedepot,
                    Spacedock,
                    Lunarbase,
                    Phalanx,
                    Jumpgate,
                    Silo,
                    Espionage,
                    Computer,
                    Weapons,
                    Shielding,
                    Armour,
                    Energy,
                    Hyperspacetech,
                    Combustion,
                    Impulse,
                    Hyperspacedrive,
                    Laser,
                    Ion,
                    Plasma,
                    Researchnetwork,
                    Astro,
                    Graviton,
                    Coloship,
                    Crawler,
                    None };
enum class ItemRarity { Bronze,
                        Silver,
                        Gold,
                        Platinum,
                        None };

enum class CharacterClass {
    None,
    Collector,
    General,
    Discoverer,
};


constexpr float plasma_factor_met = 1.0f;
constexpr float plasma_factor_crys = 0.66f;
constexpr float plasma_factor_deut = 0.33f;

constexpr std::int64_t default_production_met = 30;
constexpr std::int64_t default_production_crys = 15;
constexpr std::int64_t default_production_deut = 0;

struct EntityInfo {
    constexpr EntityInfo() : EntityInfo(0, 0, 0, 0, 0.f, 0.f, EntityType::None, Entity::None, "") {}

    template <int N>
    constexpr EntityInfo(std::int64_t m, std::int64_t k, std::int64_t d, std::int64_t e, float ef, float cf, EntityType type, Entity entity, const char (&name)[N])
        : metBaseCosts(m), crysBaseCosts(k), deutBaseCosts(d), energyBaseCosts(e), energyFactor(ef), costFactor(cf), type(type), entity(entity), name(name) {}

    std::int64_t metBaseCosts;
    std::int64_t crysBaseCosts;
    std::int64_t deutBaseCosts;
    std::int64_t energyBaseCosts;
    float energyFactor;
    float costFactor;

    EntityType type;
    Entity entity;
    //std::string name;
    const char* name;

    bool operator==(const EntityInfo& rhs) const {
        return type == rhs.type && entity == rhs.entity;
    }

    bool operator!=(const EntityInfo& rhs) const {
        return !(operator==(rhs));
    }
};

/*extern const EntityInfo Metalmine;
extern const EntityInfo Crystalmine;
extern const EntityInfo Deutsynth;
extern const EntityInfo Solar;
extern const EntityInfo Fusion;
extern const EntityInfo Robo;
extern const EntityInfo Nanite;
extern const EntityInfo Shipyard;
extern const EntityInfo Metalstorage;
extern const EntityInfo Crystalstorage;
extern const EntityInfo Deutstorage;
extern const EntityInfo Lab;
extern const EntityInfo Terra;
extern const EntityInfo Alliancedepot;
extern const EntityInfo Spacedock;
extern const EntityInfo Lunarbase;
extern const EntityInfo Phalanx;
extern const EntityInfo Jumpgate;
extern const EntityInfo Silo;

extern const EntityInfo Espionage;
extern const EntityInfo Computer;
extern const EntityInfo Weapons;
extern const EntityInfo Shielding;
extern const EntityInfo Armour;
extern const EntityInfo Energy;
extern const EntityInfo Hyperspacetech;
extern const EntityInfo Combustion;
extern const EntityInfo Impulse;
extern const EntityInfo Hyperspacedrive;
extern const EntityInfo Laser;
extern const EntityInfo Ion;
extern const EntityInfo Plasma;
extern const EntityInfo Researchnetwork;
extern const EntityInfo Astro;
extern const EntityInfo Graviton;

extern const EntityInfo Coloship;

extern const EntityInfo Noentity;*/

constexpr EntityInfo Metalmine = EntityInfo(60, 15, 0, 0, 10, 1.5, EntityType::Building, Entity::Metalmine, "Metal Mine");
constexpr EntityInfo Crystalmine = EntityInfo(48, 24, 0, 0, 10, 1.6, EntityType::Building, Entity::Crystalmine, "Crystal Mine");
constexpr EntityInfo Deutsynth = EntityInfo(225, 75, 0, 0, 20, 1.5, EntityType::Building, Entity::Deutsynth, "Deuterium Synthesizer");
constexpr EntityInfo Solar = EntityInfo(75, 30, 0, 0, 0, 1.5, EntityType::Building, Entity::Solar, "Solar Plant");
constexpr EntityInfo Fusion = EntityInfo(900, 360, 180, 0, 0, 1.8, EntityType::Building, Entity::Fusion, "Fusion Reactor");
constexpr EntityInfo Robo = EntityInfo(400, 120, 200, 0, 0, 2, EntityType::Building, Entity::Robo, "Robotics Factory");
constexpr EntityInfo Nanite = EntityInfo(1000000, 500000, 100000, 0, 0, 2, EntityType::Building, Entity::Nanite, "Nanite Factory");
constexpr EntityInfo Shipyard = EntityInfo(400, 200, 100, 0, 0, 2, EntityType::Building, Entity::Shipyard, "Shipyard");
constexpr EntityInfo Metalstorage = EntityInfo(1000, 0, 0, 0, 0, 2, EntityType::Building, Entity::Metalstorage, "Metal Storage");
constexpr EntityInfo Crystalstorage = EntityInfo(1000, 500, 0, 0, 0, 2, EntityType::Building, Entity::Crystalstorage, "Crystal Storage");
constexpr EntityInfo Deutstorage = EntityInfo(1000, 1000, 0, 0, 0, 2, EntityType::Building, Entity::Deutstorage, "Deuterium Tank");
constexpr EntityInfo Lab = EntityInfo(200, 400, 200, 0, 0, 2, EntityType::Building, Entity::Lab, "Research Lab");
constexpr EntityInfo Terra = EntityInfo(0, 50000, 100000, 1000, 0, 2, EntityType::Building, Entity::Terra, "Terraformer");
constexpr EntityInfo Alliancedepot = EntityInfo(20000, 40000, 0, 0, 0, 2, EntityType::Building, Entity::Alliancedepot, "Alliance Depot");
constexpr EntityInfo Spacedock = EntityInfo(200, 0, 50, 50, 0, 5, EntityType::Building, Entity::Spacedock, "Space Dock");
constexpr EntityInfo Lunarbase = EntityInfo(20000, 40000, 20000, 0, 0, 2, EntityType::Building, Entity::Lunarbase, "Lunar Base");
constexpr EntityInfo Phalanx = EntityInfo(20000, 40000, 20000, 0, 0, 2, EntityType::Building, Entity::Phalanx, "Sensor Phalanx");
constexpr EntityInfo Jumpgate = EntityInfo(2000000, 4000000, 2000000, 0, 0, 2, EntityType::Building, Entity::Jumpgate, "Jump Gate");
constexpr EntityInfo Silo = EntityInfo(20000, 20000, 1000, 0, 0, 2, EntityType::Building, Entity::Silo, "Missile Silo");

constexpr EntityInfo Espionage = EntityInfo(200, 1000, 200, 0, 0, 2, EntityType::Research, Entity::Espionage, "Espionage Technology");
constexpr EntityInfo Computer = EntityInfo(0, 400, 600, 0, 0, 2, EntityType::Research, Entity::Computer, "Computer Technology");
constexpr EntityInfo Weapons = EntityInfo(800, 200, 0, 0, 0, 2, EntityType::Research, Entity::Weapons, "Weapons Technology");
constexpr EntityInfo Shielding = EntityInfo(200, 600, 0, 0, 0, 2, EntityType::Research, Entity::Shielding, "Shielding Technology");
constexpr EntityInfo Armour = EntityInfo(1000, 0, 0, 0, 0, 2, EntityType::Research, Entity::Armour, "Armor Technology");
constexpr EntityInfo Energy = EntityInfo(0, 800, 400, 0, 0, 2, EntityType::Research, Entity::Energy, "Energy Technology");
constexpr EntityInfo Hyperspacetech = EntityInfo(0, 4000, 2000, 0, 0, 2, EntityType::Research, Entity::Hyperspacetech, "Hyperspace Technology");
constexpr EntityInfo Combustion = EntityInfo(400, 0, 600, 0, 0, 2, EntityType::Research, Entity::Combustion, "Combustion Drive");
constexpr EntityInfo Impulse = EntityInfo(2000, 4000, 600, 0, 0, 2, EntityType::Research, Entity::Impulse, "Impulse Drive");
constexpr EntityInfo Hyperspacedrive = EntityInfo(1000, 20000, 6000, 0, 0, 2, EntityType::Research, Entity::Hyperspacedrive, "Hyperspace Drive");
constexpr EntityInfo Laser = EntityInfo(200, 100, 0, 0, 0, 2, EntityType::Research, Entity::Laser, "Laser Technology");
constexpr EntityInfo Ion = EntityInfo(1000, 300, 100, 0, 0, 2, EntityType::Research, Entity::Ion, "Ion Technology");
constexpr EntityInfo Plasma = EntityInfo(2000, 4000, 1000, 0, 0, 2, EntityType::Research, Entity::Plasma, "Plasma Technology");
constexpr EntityInfo Researchnetwork = EntityInfo(240000, 400000, 160000, 0, 0, 2, EntityType::Research, Entity::Researchnetwork, "Intergalactic Research Network");
constexpr EntityInfo Astro = EntityInfo(4000, 8000, 4000, 0, 0, 1.75, EntityType::Research, Entity::Astro, "Astrophysics");
constexpr EntityInfo Graviton = EntityInfo(0, 0, 0, 300000, 0, 3, EntityType::Research, Entity::Graviton, "Graviton Technology");

constexpr EntityInfo Coloship = EntityInfo(10000, 20000, 10000, 0, 0, 0, EntityType::Ship, Entity::Coloship, "Colony Ship");
constexpr EntityInfo Crawler = EntityInfo(2000, 2000, 1000, 0, 50, 0, EntityType::Ship, Entity::Crawler, "Crawler");

constexpr EntityInfo Noentity = EntityInfo();

extern std::map<Entity, std::string> entityToNameMap;

std::map<Entity, std::string> getEntityToNameMap();
std::string getEntityName(Entity e);

EntityInfo getEntityInfo(Entity entity);

struct Production;

struct Resources {
friend struct Production;    
private:
    double met = 0;
    double crys = 0;
    double deut = 0;
public:
    std::int64_t metal() const;
    std::int64_t crystal() const;
    std::int64_t deuterium() const;
    double dse(const std::array<float, 3>& traderate) const;  

    void setMetal(std::int64_t m);
    void setCrystal(std::int64_t k);
    void setDeuterium(std::int64_t d);

    Resources() = default;
    Resources(const Resources&) = default;  
    Resources(std::int64_t m, std::int64_t k, std::int64_t d);

    Resources& operator+=(const Resources& rhs);
    Resources& operator-=(const Resources& rhs);
    Resources& operator*=(int i);
    Resources& operator*=(float f);
};

Resources operator+(Resources l, const Resources& r);
Resources operator-(Resources l, const Resources& r);
Resources operator*(Resources l, float r);
Resources operator*(Resources l, int r);
Resources operator*(float l, Resources r);
Resources operator*(int r, Resources l);
bool operator==(const Resources& l, const Resources& r);
bool operator!=(const Resources& l, const Resources& r);

//time dependent production
struct Production {
    std::chrono::seconds r{1}; //how many seconds need to pass to produce met amount of metal
private:
    double met = 0;
    double crys = 0;
    double deut = 0;

    static Production makeProduction(std::chrono::seconds r, std::int64_t m, std::int64_t c, std::int64_t d);
    Resources produceForSeconds(std::chrono::seconds period) const;
public:
    std::int64_t metal() const;
    std::int64_t crystal() const;
    std::int64_t deuterium() const;

    static Production makeProductionPerSeconds();
    static Production makeProductionPerSeconds(std::int64_t m, std::int64_t c, std::int64_t d);

    static Production makeProductionPerMinute();
    static Production makeProductionPerMinute(std::int64_t m, std::int64_t c, std::int64_t d);

    static Production makeProductionPerHour();
    static Production makeProductionPerHour(std::int64_t m, std::int64_t c, std::int64_t d);

    static Production makeProductionPerDay();
    static Production makeProductionPerDay(std::int64_t m, std::int64_t c, std::int64_t d);

    

    Production& operator+=(const Production& rhs);
    Production& operator-=(const Production& rhs);
    //multiply production by unitless factor, e.g. double production
    Production& operator*=(float f);

    Resources produce(std::chrono::seconds period) const;
    Resources produce(std::chrono::minutes period) const;
    Resources produce(std::chrono::hours period) const;
};

Production operator+(Production l, const Production& r);
Production operator-(Production l, const Production& r);
//multiply production by unitless factor, e.g. double production
Production operator*(Production l, float r);
Production operator*(float l, const Production& r);
bool operator==(const Production& l, const Production& r);
bool operator!=(const Production& l, const Production& r);

/*
struct EntityMap{
	std::map<Entity, EntityInfo> map;
	
	EntityMap(){
		map[Entity::Metalmine] = EntityInfo(60,15,0,0,10,1.5, EntityType::Building, Entity::Metalmine, "Metal Mine");
		map[Entity::Crystalmine] = EntityInfo(48,24,0,0,10,1.6, EntityType::Building, Entity::Crystalmine, "Crystal Mine");
		map[Entity::Deutsynth] = EntityInfo(225,75,0,0,20,1.5, EntityType::Building, Entity::Deutsynth, "Deuterium Synthesizer");
		map[Entity::Solar] = EntityInfo(75,30,0,0,0,1.5, EntityType::Building, Entity::Solar, "Solar Plant");
		map[Entity::Fusion] = EntityInfo(900,360,180,0,0,1.8, EntityType::Building, Entity::Fusion, "Fusion Reactor");
		map[Entity::Robo] = EntityInfo(400,120,200,0,0,2, EntityType::Building, Entity::Robo, "Robotics Factory");
		map[Entity::Nanite] = EntityInfo(1000000,500000,100000,0,0,2, EntityType::Building, Entity::Nanite, "Nanite Factory");
		map[Entity::Shipyard] = EntityInfo(400,200,100,0,0,2, EntityType::Building, Entity::Shipyard, "Shipyard");
		map[Entity::Metalstorage] = EntityInfo(1000,0,0,0,0,2, EntityType::Building, Entity::Metalstorage, "Metal Storage");
		map[Entity::Crystalstorage] = EntityInfo(1000,500,0,0,0,2, EntityType::Building, Entity::Crystalstorage, "Crystal Storage");
		map[Entity::Deutstorage] = EntityInfo(1000,1000,0,0,0,2, EntityType::Building, Entity::Deutstorage, "Deuterium Tank");
		map[Entity::Lab] = EntityInfo(200,400,200,0,0,2, EntityType::Building, Entity::Lab, "Research Lab");
		map[Entity::Terra] = EntityInfo(0,50000,100000,1000,0,2, EntityType::Building, Entity::Terra, "Terraformer");
		map[Entity::Alliancedepot] = EntityInfo(20000,40000,0,0,0,2, EntityType::Building, Entity::Alliancedepot, "Alliance Depot");
		map[Entity::Spacedock] = EntityInfo(200,0,50,50,0,5, EntityType::Building, Entity::Spacedock, "Space Dock");
		map[Entity::Lunarbase] = EntityInfo(20000,40000,20000,0,0,2, EntityType::Building, Entity::Lunarbase, "Lunar Base");
		map[Entity::Phalanx] = EntityInfo(20000,40000,20000,0,0,2, EntityType::Building, Entity::Phalanx, "Sensor Phalanx");
		map[Entity::Jumpgate] = EntityInfo(2000000,4000000,2000000,0,0,2, EntityType::Building, Entity::Jumpgate, "Jump Gate");
		map[Entity::Silo] = EntityInfo(20000,20000,1000,0,0,2, EntityType::Building, Entity::Silo, "Missile Silo");
        
		map[Entity::Espionage] = EntityInfo(200,1000,200,0,0,2, EntityType::Research, Entity::Espionage, "Espionage Technology");
		map[Entity::Computer] = EntityInfo(0,400,600,0,0,2, EntityType::Research, Entity::Computer, "Computer Technology");
		map[Entity::Weapons] = EntityInfo(800,200,0,0,0,2, EntityType::Research, Entity::Weapons, "Weapons Technology");
		map[Entity::Shielding] = EntityInfo(200,600,0,0,0,2, EntityType::Research, Entity::Shielding, "Shielding Technology");
		map[Entity::Armour] = EntityInfo(1000,0,0,0,0,2, EntityType::Research, Entity::Armour, "Armor Technology");
		map[Entity::Energy] = EntityInfo(0,800,400,0,0,2, EntityType::Research, Entity::Energy, "Energy Technology");
		map[Entity::Hyperspacetech] = EntityInfo(0,4000,2000,0,0,2, EntityType::Research, Entity::Hyperspacetech, "Hyperspace Technology");
		map[Entity::Combustion] = EntityInfo(400,0,600,0,0,2, EntityType::Research, Entity::Combustion, "Combustion Drive");
		map[Entity::Impulse] = EntityInfo(2000,4000,600,0,0,2, EntityType::Research, Entity::Impulse, "Impulse Drive");
		map[Entity::Hyperspacedrive] = EntityInfo(1000,20000,6000,0,0,2, EntityType::Research, Entity::Hyperspacedrive, "Hyperspace Drive");
		map[Entity::Laser] = EntityInfo(200,100,0,0,0,2, EntityType::Research, Entity::Laser, "Laser Technology");
		map[Entity::Ion] = EntityInfo(1000,300,100,0,0,2, EntityType::Research, Entity::Ion, "Ion Technology");
		map[Entity::Plasma] = EntityInfo(2000,4000,1000,0,0,2, EntityType::Research, Entity::Plasma, "Plasma Technology");
		map[Entity::Researchnetwork] = EntityInfo(240000,400000,160000,0,0,2, EntityType::Research, Entity::Researchnetwork, "Intergalactic Research Network");
		map[Entity::Astro] = EntityInfo(4000,8000,4000,0,0,1.75, EntityType::Research, Entity::Astro, "Astrophysics");
		map[Entity::Graviton] = EntityInfo(0,0,0,300000,0,3, EntityType::Research, Entity::Graviton, "Graviton Technology");
        
        map[Entity::Coloship] = EntityInfo(10000,20000,10000,0,0,0, EntityType::Ship, Entity::Coloship, "Colonization Ship");
        
		map[Entity::None] = EntityInfo();
	}
	
	const EntityInfo& getInfo(Entity entity){
		return map[entity];
	}
};
*/

Resources getBuildCosts(const EntityInfo& info, int level);

Resources getTotalCosts(const EntityInfo& info, int level);

std::chrono::seconds getConstructionTime(const EntityInfo& info, int level, int roboLevel, int naniLevel, int shipyardLevel, int flabLevel, int speedfactor);

int getTotalLabLevel(const std::vector<int>& labsPerPlanet, int igrnLevel);

std::int64_t getFKWConsumption(int fusionLevel, int fusionPercent);
std::int64_t getDailyFKWConsumption(int fusionLevel, int fusionPercent);

std::int64_t getEnergyConsumption(int metlvl, int metPercent,
                                  int cryslvl, int crysPercent,
                                  int deutlvl, int deutPercent,
                                  int crawler, int crawlerPercent,
                                  CharacterClass cclass);

// 100%, no officer
std::int64_t getEnergyOfSats(int count, int planetTemperatur);

// 100%, no officer
std::int64_t getEnergyOfSolarPlant(int level);

// 100%, no officer
std::int64_t getEnergyOfFKW(int level, int etechLevel);

std::int64_t applyEnergyFactor(std::int64_t energy, float productionfactor, 
                                ItemRarity energyItem, bool hasEngineer, 
                                bool hasStaff, CharacterClass cclass);

std::int64_t getTotalEnergy(int solarplantLevel, int solarplantPercent,
                            int fusionLevel, int fusionPercent, int etechLevel,
                            int sats, int satsPercent, int temperature,
                            ItemRarity energyItem, 
                            bool engineer, bool staff,
                            CharacterClass cclass);                                

int getItemProductionPercent(ItemRarity item);
int getEnergyItemProductionPercent(ItemRarity item);

Production getDefaultProduction();
Production getDailyDefaultProduction();

struct ProductionCalculator{

    

    ProductionCalculator(
        int metLevel, ItemRarity metItem, 
        int crysLevel, ItemRarity crysItem,
        int deutLevel, ItemRarity deutItem,
        int solarplantLevel,
        int fusionLevel,int etechLevel,
        ItemRarity energyItem,
        int temperature, int sats,
        int planetPosition,
        int crawler,
        int plasmaLevel,
        bool engineer, bool geologist, bool staff,
        CharacterClass cclass
    )
    : metLevel(metLevel), crysLevel(crysLevel),
    deutLevel(deutLevel),
    solarplantLevel(solarplantLevel), fusionLevel(fusionLevel),
    etechLevel(etechLevel), sats(sats), temperature(temperature),
    crawler(crawler),plasmaLevel(plasmaLevel),engineer(engineer),geologist(geologist),
    staff(staff),cclass(cclass), metItem(metItem),
    crysItem(crysItem),deutItem(deutItem), energyItem(energyItem), planetPosition(planetPosition),
    defaultProduction(getDefaultProduction()){

        geologistpercent = geologist ? 10 : 0;
        staffpercent = staff ? 2 : 0;
        officerfactor = (geologistpercent + staffpercent) / 100.;

        classPercent = cclass == CharacterClass::Collector ? 25 : 0;
        classFactor = (classPercent) / 100.;

        constexpr double crawler_boost_factor = 0.0002;
        constexpr int crawler_y_factor = 8;
        //constexpr double crawler_maxTotalBoost = 0.5;        

        const double crawlerClassFactor = cclass == CharacterClass::Collector ? 1.5 : 1.0;    
        crawlerFactor = crawler_boost_factor * crawlerClassFactor;
        //const int maxCrawlersByProduction = std::ceil(crawler_maxTotalBoost / (crawler_boost_factor * crawlerClassFactor));
        const int totalMineLevel = metLevel + crysLevel + deutLevel;
        usablecrawler = std::min(crawler, totalMineLevel * crawler_y_factor);
        //usablecrawler = std::min(usablecrawler, maxCrawlersByProduction);

        metBaseProd = 30 * metLevel * std::pow(1.1, metLevel);
        crysBaseProd = 20 * crysLevel * std::pow(1.1, crysLevel);
        deutBaseProd = 10 * deutLevel * std::pow(1.1, deutLevel) * (1.44 - 0.004 * temperature);

    }

    double getMineProductionFactor(
        int metPercent,
        int crysPercent,
        int deutPercent,
        int solarplantPercent,
        int fusionPercent,
        int satsPercent,
        int crawlerPercent
    ) const {
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
            usablecrawler, crawlerPercent,
            cclass
        );

        const std::int64_t totalenergy = getTotalEnergy(solarplantLevel, solarplantPercent,
                                                        fusionLevel, fusionPercent, etechLevel,
                                                        sats, satsPercent, temperature,
                                                        energyItem, 
                                                        engineer, staff,
                                                        cclass);

        const double mineproductionfactor = totalenergy == 0 ? 0.0f : std::min(1.0, double(totalenergy) / double(requiredenergy));
        return mineproductionfactor;
    }



    Production getDailyProduction(
        int metPercent,
        int crysPercent,
        int deutPercent,
        int solarplantPercent,
        int fusionPercent,
        int satsPercent,
        int crawlerPercent,
        int speedfactor
    ) const {

        assert(metLevel >= 0);
        assert(crysLevel >= 0);
        assert(deutLevel >= 0);
        assert(metPercent >= 0);
        assert(crysPercent >= 0);
        assert(deutPercent >= 0);
        assert(plasmaLevel >= 0);
        assert(speedfactor >= 1);
        assert(planetPosition >= 1);
        assert(planetPosition <= 15);

        constexpr std::array<double, 15> crysBoostByPosition{0.3, 0.225, 0.15, 0,0,0,0,0,0,0,0,0,0,0,0};

        const double positionBonus_met = 0;
        const double positionBonus_crystal = crysBaseProd * crysBoostByPosition[planetPosition-1];
        const double positionBonus_deut = 0;

        double simpleProduction_met = (metBaseProd + positionBonus_met) * metPercent / 100.;
        double simpleProduction_crystal = (crysBaseProd + positionBonus_crystal) * crysPercent / 100.;
        double simpleProduction_deut = (deutBaseProd + positionBonus_deut) * deutPercent / 100.;

        const double mineproductionfactor = getMineProductionFactor(
            metPercent,
            crysPercent,
            deutPercent,
            solarplantPercent,
            fusionPercent,
            satsPercent,
            crawlerPercent
        );
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

        const double crawlerPercentFactor = crawlerPercent / 100.0;

        const double crawlerProduction_met = (simpleProduction_met * crawlerFactor * usablecrawler) * crawlerPercentFactor;
        const double crawlerProduction_crystal = (simpleProduction_crystal * crawlerFactor * usablecrawler) * crawlerPercentFactor;
        const double crawlerProduction_deut = (simpleProduction_deut * crawlerFactor * usablecrawler) * crawlerPercentFactor;



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

    int metLevel;
    int crysLevel;
    int deutLevel;
    int solarplantLevel;
    int fusionLevel;
    int etechLevel;
    int sats;
    int temperature;
    int crawler;
    int plasmaLevel;
    bool engineer;
    bool geologist;
    bool staff;
    CharacterClass cclass;
    ItemRarity metItem;
    ItemRarity crysItem;
    ItemRarity deutItem;
    ItemRarity energyItem;
    int planetPosition;
    int geologistpercent;
    int staffpercent;
    double officerfactor;
    int classPercent;
    double classFactor;
    int usablecrawler;
    double metBaseProd;
    double crysBaseProd;
    double deutBaseProd;
    double crawlerFactor;

    Production defaultProduction;

};


std::chrono::seconds get_save_duration_symmetrictrade(const Resources& have, /*have*/
                                       const Resources& want,  /*want*/
                                       const Production& production,  /*production*/
                                       const std::array<float, 3>& traderate /*e.g 3:2:1*/);                                       

std::chrono::seconds get_save_duration_notrade(const Resources& have,
                                       const Resources& want,
                                       const Production& production);

std::int64_t getConstructionTimeReductionDM(float totalConstructionTimeDays);

std::int64_t getNonResearchTimeReductionDM(float totalConstructionTimeDays);

std::int64_t getResearchTimeReductionDM(float totalConstructionTimeDays);

int getMaxPossiblePlanets(int astroLevel);

int getMinPossibleAstro(int num_planets);

int getNumberOfFleetSlots(int computerLevel);
int getNumberOfFleetSlots(int computerLevel, bool hasAdmiral, bool hasStaff, CharacterClass cclass);

int getNumberOfExpeditionSlots(int astroLevel);
int getNumberOfExpeditionSlots(int astroLevel, bool hasAdmiral, bool hasStaff, CharacterClass cclass);

EntityInfo parseEntityName(const std::string& name);

ItemRarity parseItemRarityName(const std::string& rarityname);

std::string itemRarityToName(const ItemRarity& r);

CharacterClass parseCharacterClassName(const std::string& classname);
std::string characterClassToName(const CharacterClass& cclass);

} //end namespace ogamehelpers

#endif
