#ifndef OGAME_HPP
#define OGAME_HPP

#include <map>
#include <cstdint>
#include <array>
#include <vector>
#include <iostream>
#include <string>

#include <functional>
#include <cassert>
#include <iomanip>

namespace ogamehelpers{
 
enum class EntityType {Building, Research, Ship, None};
enum class Entity {Metalmine, Crystalmine, Deutsynth, Solar, Fusion, Robo, Nanite, Shipyard, Metalstorage, Crystalstorage, Deutstorage, Lab, Terra, Alliancedepot, Spacedock, Lunarbase, Phalanx, Jumpgate, Silo,
					Espionage, Computer, Weapons, Shielding, Armour, Energy, Hyperspacetech, Combustion, Impulse, Hyperspacedrive, Laser, Ion, Plasma, Researchnetwork, Astro, Graviton, 
                    Coloship, None};
enum class ItemRarity {Bronze, Silver, Gold, None};

constexpr float plasma_factor_met = 1.0f;
constexpr float plasma_factor_crys = 0.66f;
constexpr float plasma_factor_deut = 0.33f;

constexpr std::int64_t default_production_met = 30;
constexpr std::int64_t default_production_crys = 15;
constexpr std::int64_t default_production_deut = 0;

struct EntityInfo{
	constexpr EntityInfo() : EntityInfo(0,0,0,0,0.f,0.f, EntityType::None, Entity::None, ""){}
        
    template<int N>
    constexpr EntityInfo(std::int64_t m, std::int64_t k, std::int64_t d, std::int64_t e, float ef, float cf, EntityType type, Entity entity, const char(&name)[N])
        :metBaseCosts(m), crysBaseCosts(k), deutBaseCosts(d), energyBaseCosts(e), energyFactor(ef), costFactor(cf), type(type), entity(entity), name(name){}
        
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
    
    bool operator==(const EntityInfo& rhs) const{
        return type == rhs.type && entity == rhs.entity;
    }
    
    bool operator!=(const EntityInfo& rhs) const{
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





constexpr EntityInfo Metalmine = EntityInfo(60,15,0,0,10,1.5, EntityType::Building, Entity::Metalmine, "Metal Mine");
constexpr EntityInfo Crystalmine = EntityInfo(48,24,0,0,10,1.6, EntityType::Building, Entity::Crystalmine, "Crystal Mine");
constexpr EntityInfo Deutsynth = EntityInfo(225,75,0,0,20,1.5, EntityType::Building, Entity::Deutsynth, "Deuterium Synthesizer");
constexpr EntityInfo Solar = EntityInfo(75,30,0,0,0,1.5, EntityType::Building, Entity::Solar, "Solar Plant");
constexpr EntityInfo Fusion = EntityInfo(900,360,180,0,0,1.8, EntityType::Building, Entity::Fusion, "Fusion Reactor");
constexpr EntityInfo Robo = EntityInfo(400,120,200,0,0,2, EntityType::Building, Entity::Robo, "Robotics Factory");
constexpr EntityInfo Nanite = EntityInfo(1000000,500000,100000,0,0,2, EntityType::Building, Entity::Nanite, "Nanite Factory");
constexpr EntityInfo Shipyard = EntityInfo(400,200,100,0,0,2, EntityType::Building, Entity::Shipyard, "Shipyard");
constexpr EntityInfo Metalstorage = EntityInfo(1000,0,0,0,0,2, EntityType::Building, Entity::Metalstorage, "Metal Storage");
constexpr EntityInfo Crystalstorage = EntityInfo(1000,500,0,0,0,2, EntityType::Building, Entity::Crystalstorage, "Crystal Storage");
constexpr EntityInfo Deutstorage = EntityInfo(1000,1000,0,0,0,2, EntityType::Building, Entity::Deutstorage, "Deuterium Tank");
constexpr EntityInfo Lab = EntityInfo(200,400,200,0,0,2, EntityType::Building, Entity::Lab, "Research Lab");
constexpr EntityInfo Terra = EntityInfo(0,50000,100000,1000,0,2, EntityType::Building, Entity::Terra, "Terraformer");
constexpr EntityInfo Alliancedepot = EntityInfo(20000,40000,0,0,0,2, EntityType::Building, Entity::Alliancedepot, "Alliance Depot");
constexpr EntityInfo Spacedock = EntityInfo(200,0,50,50,0,5, EntityType::Building, Entity::Spacedock, "Space Dock");
constexpr EntityInfo Lunarbase = EntityInfo(20000,40000,20000,0,0,2, EntityType::Building, Entity::Lunarbase, "Lunar Base");
constexpr EntityInfo Phalanx = EntityInfo(20000,40000,20000,0,0,2, EntityType::Building, Entity::Phalanx, "Sensor Phalanx");
constexpr EntityInfo Jumpgate = EntityInfo(2000000,4000000,2000000,0,0,2, EntityType::Building, Entity::Jumpgate, "Jump Gate");
constexpr EntityInfo Silo = EntityInfo(20000,20000,1000,0,0,2, EntityType::Building, Entity::Silo, "Missile Silo");

constexpr EntityInfo Espionage = EntityInfo(200,1000,200,0,0,2, EntityType::Research, Entity::Espionage, "Espionage Technology");
constexpr EntityInfo Computer = EntityInfo(0,400,600,0,0,2, EntityType::Research, Entity::Computer, "Computer Technology");
constexpr EntityInfo Weapons = EntityInfo(800,200,0,0,0,2, EntityType::Research, Entity::Weapons, "Weapons Technology");
constexpr EntityInfo Shielding = EntityInfo(200,600,0,0,0,2, EntityType::Research, Entity::Shielding, "Shielding Technology");
constexpr EntityInfo Armour = EntityInfo(1000,0,0,0,0,2, EntityType::Research, Entity::Armour, "Armor Technology");
constexpr EntityInfo Energy = EntityInfo(0,800,400,0,0,2, EntityType::Research, Entity::Energy, "Energy Technology");
constexpr EntityInfo Hyperspacetech = EntityInfo(0,4000,2000,0,0,2, EntityType::Research, Entity::Hyperspacetech, "Hyperspace Technology");
constexpr EntityInfo Combustion = EntityInfo(400,0,600,0,0,2, EntityType::Research, Entity::Combustion, "Combustion Drive");
constexpr EntityInfo Impulse = EntityInfo(2000,4000,600,0,0,2, EntityType::Research, Entity::Impulse, "Impulse Drive");
constexpr EntityInfo Hyperspacedrive = EntityInfo(1000,20000,6000,0,0,2, EntityType::Research, Entity::Hyperspacedrive, "Hyperspace Drive");
constexpr EntityInfo Laser = EntityInfo(200,100,0,0,0,2, EntityType::Research, Entity::Laser, "Laser Technology");
constexpr EntityInfo Ion = EntityInfo(1000,300,100,0,0,2, EntityType::Research, Entity::Ion, "Ion Technology");
constexpr EntityInfo Plasma = EntityInfo(2000,4000,1000,0,0,2, EntityType::Research, Entity::Plasma, "Plasma Technology");
constexpr EntityInfo Researchnetwork = EntityInfo(240000,400000,160000,0,0,2, EntityType::Research, Entity::Researchnetwork, "Intergalactic Research Network");
constexpr EntityInfo Astro = EntityInfo(4000,8000,4000,0,0,1.75, EntityType::Research, Entity::Astro, "Astrophysics");
constexpr EntityInfo Graviton = EntityInfo(0,0,0,300000,0,3, EntityType::Research, Entity::Graviton, "Graviton Technology");

constexpr EntityInfo Coloship = EntityInfo(10000,20000,10000,0,0,0, EntityType::Ship, Entity::Coloship, "Colonization Ship");

constexpr EntityInfo Noentity = EntityInfo();













struct Resources{
	std::int64_t met = 0;
	std::int64_t crystal = 0;
	std::int64_t deut = 0;
	
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

//time dependent production
struct Production{
	int met = 0;
	int crystal = 0;
	int deut = 0;	
	
	Production& operator+=(const Production& rhs);
	Production& operator-=(const Production& rhs);
	//multiply production by unitless factor, e.g. double production
	Production& operator*=(float f);
	
	Resources produce(float time) const;
};

Production operator+(Production l, const Production& r);
Production operator-(Production l, const Production& r);
//multiply production by unitless factor, e.g. double production
Production operator*(Production l, float r);
Production operator*(float l, const Production& r);

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

float getConstructionTimeInDays(const EntityInfo& info, int level, int roboLevel, int naniLevel, int shipyardLevel, int flabLevel, int speedfactor);

int getTotalLabLevel(const std::vector<int>& labsPerPlanet, int igrnLevel);

std::int64_t getFKWConsumption(int fusionLevel, int fusionPercent);
std::int64_t getDailyFKWConsumption(int fusionLevel, int fusionPercent);

std::int64_t getEnergyConsumption(int metlvl, int metPercent,
										int cryslvl, int crysPercent,
										int deutlvl, int deutPercent);



// 100%, no officer
std::int64_t getEnergyOfSats(int count, int planetTemperatur);

// 100%, no officer
std::int64_t getEnergyOfSolarPlant(int level);

// 100%, no officer
std::int64_t getEnergyOfFKW(int level, int etechLevel);

std::int64_t applyEnergyFactor(std::int64_t energy, float productionfactor, bool hasEngineer, bool hasStaff);

int getItemProductionPercent(ItemRarity item);

Production getDefaultProduction();
Production getDailyDefaultProduction();

double getMineProductionFactor(int metLevel, int metPercent,
								  int crysLevel, int crysPercent,
								  int deutLevel, int deutPercent,
								  int solarplantLevel, int solarplantPercent,
								  int fusionLevel, int fusionPercent, int etechLevel, 
								  int sats, int satsPercent, int temperature,
								  bool engineer, bool staff);

Production getDailyProduction(int metLevel, ItemRarity metItem, int metPercent, 
								int crysLevel, ItemRarity crysItem, int crysPercent, 
								int deutLevel, ItemRarity deutItem, int deutPercent, 
								int solarLevel, int solarplantPercent,
								int fusionLevel, int fusionPercent, int etechLevel,
								int temperature, int sats, int satsPercent,
								int plasmaLevel, int speedfactor, 
								bool engineer, bool geologist, bool staff);

float get_save_duration_symmetrictrade(const std::int64_t hm, const std::uint64_t hk, const std::int64_t hd, /*have*/
			const std::int64_t wm, const std::int64_t wk, const std::int64_t wd, /*want*/
			const std::int64_t pm, const std::int64_t pk,const std::int64_t pd, /*production*/
			const std::array<float, 3>& traderate /*e.g 3:2:1*/);
		

std::int64_t getConstructionTimeReductionDM(float totalConstructionTimeDays);

std::int64_t getNonResearchTimeReductionDM(float totalConstructionTimeDays);

std::int64_t getResearchTimeReductionDM(float totalConstructionTimeDays);

int getMaxPossiblePlanets(int astroLevel);

int getMinPossibleAstro(int num_planets);


EntityInfo parseEntityName(const std::string& name);









	



} //end namespace ogamehelpers

#endif
