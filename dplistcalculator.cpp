#include "ogame.hpp"
#include "util.hpp"
#include "hpc_helpers.cuh"


#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <iostream>
#include <map>
#include <mutex>
#include <numeric>
#include <omp.h> 
#include <vector>
#include <numeric>
#include <fstream>
#include <cmath>

namespace ogh = ogamehelpers;


#define Level(X) struct X##Level{\
    int level;\
    X##Level() : X##Level(0){}\
    X##Level(int i) : level(i){}\
    operator int() const {return level;}\
};

Level(Met)
Level(Crys)
Level(Deut)
Level(Solar)
Level(Fusion)
Level(Energy)
Level(Plasma)
Level(Lab)
Level(Robo)
Level(Nani)
Level(PlanetUpgrades)
Level(Spy)
Level(Impulse)
Level(Shipyard)



    


struct ListCalculator{
    static constexpr int8_t index_metmine = 0;
    static constexpr int8_t index_crystalmine = 1;
    static constexpr int8_t index_deutsynth = 2;
    static constexpr int8_t index_solarplant = 3;
    static constexpr int8_t index_fusionplant = 4;	
    static constexpr int8_t index_energytech = 5;
    static constexpr int8_t index_plasmatech = 6;
    static constexpr int8_t index_lab = 7;
    static constexpr int8_t index_robo = 8;
    static constexpr int8_t index_nani = 9;
    static constexpr int8_t index_planetupgrades = 10;
    static constexpr int8_t index_spion = 11;
    static constexpr int8_t index_impuls = 12;
    static constexpr int8_t index_shipyard = 13;
    
    static constexpr int dimensions = 14;
    static constexpr int8_t noprev = 128;
    static constexpr int8_t illegalprev = -1;
    
    static constexpr bool sparse = true;
    
    using CoordEntry_t = int8_t;
    using Coords = std::array<CoordEntry_t, dimensions>; //planet count is Coords[index_planetupgrades]+1
    
    struct Percents{
        std::array<uint8_t, 2> data;
        
        Percents(){
            setMetPercents(uint8_t(100));
            setCrysPercents(uint8_t(100));
            setDeutPercents(uint8_t(100));
            setFusionPercents(uint8_t(100));
        }
        
        int getMetPercents() const{
            return 10 * int((data[0] & 0xF0) >> 4);
        }
        int getCrysPercents() const{
            return 10 * int(data[0] & 0x0F);
        }
        int getDeutPercents() const{
            return 10 * int((data[1] & 0xF0) >> 4);
        }
        int getFusionPercents() const{
            return 10 * int(data[1] & 0x0F);
        }
        
        void setMetPercents(uint8_t p){
            data[0] = (data[0] & 0x0F) | ((p/10) << 4);
        }
        void setCrysPercents(uint8_t p){
            data[0] = (data[0] & 0xF0) | (p/10);
        }
        void setDeutPercents(uint8_t p){
            data[1] = (data[1] & 0x0F) | ((p/10) << 4);
        }
        void setFusionPercents(uint8_t p){
            data[1] = (data[1] & 0xF0) | (p/10);
        }
    };
    
    struct State{
        ogh::Resources ress{};
        ogh::Production prod{};
        
        //times are given in days
        //the time it takes to reach the corresponding state levels on every planet
        float time = std::numeric_limits<float>::max();
        //the time it takes to save resources for the corresponding state levels on every planet
        float savetime = std::numeric_limits<float>::max();
        
        Percents percents;
        
        bool operator==(const State& r) const{
            return (0 == std::memcmp(this, &r, sizeof(State)));
        }
        
        bool operator!=(const State& r) const{
            return !operator==(r);
        }
    };
    
    struct PathNode{
        Coords coords;
        State state;
        
        PathNode() : PathNode({}, {}){}
        PathNode(const Coords& c, const State& t) : coords(c), state(t){}
    };
    
    struct OfficerState{
        float commanderDurationDays = 0.0f;
        float engineerDurationDays = 0.0f;
        float technocratDurationDays = 0.0f;
        float geologistDurationDays = 0.0f;
        float admiralDurationDays = 0.0f;
        
        OfficerState() = default;		
        OfficerState(const OfficerState& rhs) = default;		
        OfficerState& operator=(const OfficerState& rhs) = default;
        
        bool hasCommander() const{
            return commanderDurationDays > 0.0f;
        }
        
        bool hasEngineer() const{
            return engineerDurationDays > 0.0f;
        }
        
        bool hasTechnocrat() const{
            return technocratDurationDays > 0.0f;
        }
        
        bool hasGeologist() const{
            return geologistDurationDays > 0.0f;
        }
        
        bool hasAdmiral() const{
            return admiralDurationDays > 0.0f;
        }
        
        bool hasStaff() const{
            return hasCommander() && hasEngineer() && hasTechnocrat() && hasGeologist() && hasAdmiral();
        }
    };
    
    
    std::vector<State> dpmatrix;
    std::vector<int8_t> prevs;
    std::map<Coords, State> previousDiagonalStates;
    std::map<Coords, State> currentDiagonalStates;
    
    std::vector<std::map<Coords, int8_t>> diagonalPrevs;

    int previousDiagonal = 0;
    int currentDiagonal = 1;
    
    //ogh::EntityMap entityInfoMap;
    std::array<ogh::EntityInfo, dimensions> entityInfoArray;
    
    OfficerState officers{};
    int temperature = 0;
    int speedfactor = 1;
    std::array<float, 3> traderate;
    Coords bounds;
    
    std::vector<std::function<bool(Coords)>> additionalConstraints;
    
    std::string tmpdir = ".";
    
    ListCalculator(){	
        
        entityInfoArray[index_metmine] = ogh::Metalmine;
        entityInfoArray[index_crystalmine] = ogh::Crystalmine;
        entityInfoArray[index_deutsynth] = ogh::Deutsynth;
        entityInfoArray[index_fusionplant] = ogh::Fusion;
        entityInfoArray[index_solarplant] = ogh::Solar;
        entityInfoArray[index_energytech] = ogh::Energy;
        entityInfoArray[index_plasmatech] = ogh::Plasma;
        entityInfoArray[index_lab] = ogh::Lab;
        entityInfoArray[index_robo] = ogh::Robo;
        entityInfoArray[index_nani] = ogh::Nanite;
        entityInfoArray[index_spion] = ogh::Espionage;
        entityInfoArray[index_impuls] = ogh::Impulse;
        entityInfoArray[index_shipyard] = ogh::Shipyard;
        
        make_constraints();
        
        traderate = {3.0f,2.0f,1.0f};
    }
    
    void setTmpDir(const std::string& dir){
        tmpdir = dir;
    }
    
    void make_constraints(){
        
        additionalConstraints.emplace_back([](const auto& coords){
            //crys must not be greater than met and at most 10 levels smaller
            auto diffmk = (coords[index_metmine] - coords[index_crystalmine]);
            if(diffmk < 0 || diffmk > 10) return false;
                                           
                                           //deut must not be greater than met and at most 10 levels smaller
                                           auto diffmd = (coords[index_metmine] - coords[index_deutsynth]);
            if(diffmd < 0 || diffmd > 10) return false;		
                                           
                                           //fusion plant should be at most 3 levels over energy technique
                                           auto diff = std::max(0, coords[index_fusionplant] - coords[index_energytech]);
            if(diff > 3) 
                return false;
            
            
            return true;
        });
        
        additionalConstraints.emplace_back([](const auto& coords){
            // (lab) requirements of researches
            if(coords[index_energytech] > 0 && coords[index_lab] < 1)
                return false;
            
            if(coords[index_plasmatech] > 0 && (coords[index_lab] < 4 || coords[index_energytech] < 8))
                return false;
            
            if(coords[index_spion] > 0 && coords[index_lab] < 3)
                return false;
            
            if(coords[index_impuls] > 0 && (coords[index_lab] < 2 || coords[index_energytech] < 1))
                return false;
            
            const int min_astro = ogh::getMinPossibleAstro(coords[index_planetupgrades]+1);
            if(min_astro > 0 && (coords[index_spion] < 4 || coords[index_impuls] < 3 || coords[index_shipyard] < 4))
                return false;
            
            return true;
        });
        
        additionalConstraints.emplace_back([](const auto& coords){
            // building requirements
            if(coords[index_nani] > 0 && coords[index_robo] < 10)
                return false;
            if(coords[index_fusionplant] > 0 && coords[index_energytech] < 3)
                return false;
            if(coords[index_shipyard] > 0 && coords[index_robo] < 2)
                return false;
            
            return true;
        });
    }
    
    bool isValidCoordinate(const Coords& coords) const{
        return std::all_of(additionalConstraints.begin(), 
                           additionalConstraints.end(), 
                           [&](auto func){
                               return func(coords);
                           });
    }
    
    bool isResearchIndex(int index) const{
        if(index == index_energytech || index == index_plasmatech || index == index_planetupgrades || index == index_spion || index == index_impuls)
            return true;
        return false;
    }
    
    bool isBuildingIndex(int index) const{
        if(index == index_metmine 
            || index == index_crystalmine
            || index == index_deutsynth
            || index == index_solarplant
            || index == index_fusionplant
            || index == index_lab
            || index == index_robo
            || index == index_nani
            || index == index_shipyard)
            
            return true;
        
        return false;
    }
    
    void print_coords(std::ostream& os, const Coords& coords) const{
        os << "[";
        for(const auto i : coords){
            os << int(i) << ',';
        }
        os << "]";
    };
    
    Coords make_coords(MetLevel metLevel, 
                       CrysLevel crysLevel, 
                       DeutLevel deutLevel, 
                       SolarLevel solarLevel, 
                       FusionLevel fusionLevel, 
                       EnergyLevel etechLevel, 
                       PlasmaLevel plasmaLevel, 
                       LabLevel labLevel,
                       RoboLevel roboLevel,
                       NaniLevel naniLevel,
                       PlanetUpgradesLevel planetUpgrades,
                       SpyLevel spyLevel,
                       ImpulseLevel impulseLevel,
                       ShipyardLevel shipyardLevel) const{
                           
        assert(metLevel <= std::numeric_limits<CoordEntry_t>::max());
        assert(crysLevel <= std::numeric_limits<CoordEntry_t>::max());
        assert(deutLevel <= std::numeric_limits<CoordEntry_t>::max());
        assert(solarLevel <= std::numeric_limits<CoordEntry_t>::max());
        assert(fusionLevel <= std::numeric_limits<CoordEntry_t>::max());
        assert(etechLevel <= std::numeric_limits<CoordEntry_t>::max());
        assert(plasmaLevel <= std::numeric_limits<CoordEntry_t>::max());
        assert(labLevel <= std::numeric_limits<CoordEntry_t>::max());
        assert(roboLevel <= std::numeric_limits<CoordEntry_t>::max());
        assert(naniLevel <= std::numeric_limits<CoordEntry_t>::max());
        assert(planetUpgrades <= std::numeric_limits<CoordEntry_t>::max());
        assert(spyLevel <= std::numeric_limits<CoordEntry_t>::max());
        assert(impulseLevel <= std::numeric_limits<CoordEntry_t>::max());
        assert(shipyardLevel <= std::numeric_limits<CoordEntry_t>::max());
        
        assert(metLevel >= 0);
        assert(crysLevel >= 0);
        assert(deutLevel >= 0);
        assert(solarLevel >= 0);
        assert(fusionLevel >= 0);
        assert(etechLevel >= 0);
        assert(plasmaLevel >= 0);
        assert(labLevel >= 0);
        assert(roboLevel >= 0);
        assert(naniLevel >= 0);
        assert(planetUpgrades >= 0);
        assert(spyLevel >= 0);
        assert(impulseLevel >= 0);
        assert(shipyardLevel >= 0);
        
        Coords target;
        
        target[index_metmine] = metLevel;
        target[index_crystalmine] = crysLevel;
        target[index_deutsynth] = deutLevel;
        target[index_solarplant] = solarLevel;
        target[index_fusionplant] = fusionLevel;
        target[index_energytech] = etechLevel;
        target[index_plasmatech] = plasmaLevel;
        target[index_lab] = labLevel;
        target[index_robo] = roboLevel;
        target[index_nani] = naniLevel;
        target[index_planetupgrades] = planetUpgrades;
        target[index_spion] = spyLevel;
        target[index_impuls] = impulseLevel;
        target[index_shipyard] = shipyardLevel;
        
        return target;
    }
    
    State& get_state_of_current_diagonal_sparse(const Coords& coords){
        auto it = currentDiagonalStates.find(coords);
        assert(it != currentDiagonalStates.end());
        return it->second;
    }
    
    State& get_state_of_previous_diagonal_sparse(const Coords& coords){
        auto it = previousDiagonalStates.find(coords);
        assert(it != previousDiagonalStates.end());
        return it->second;
    }
    
    State& get_state_of_current_diagonal_dense(const Coords& coords){
        const std::size_t index = to_linear_index(coords, bounds);
        return dpmatrix[index];
    }
    
    State& get_state_of_previous_diagonal_dense(const Coords& coords){
        const std::size_t index = to_linear_index(coords, bounds);
        return dpmatrix[index];
    }
    
    State& get_state_of_current_diagonal(const Coords& coords){
        if(sparse)
            return get_state_of_current_diagonal_sparse(coords);
        else
            return get_state_of_current_diagonal_dense(coords);
    }
    
    State& get_state_of_previous_diagonal(const Coords& coords){
        if(sparse)
            return get_state_of_previous_diagonal_sparse(coords);
        else
            return get_state_of_previous_diagonal_dense(coords);
    }
    
    
    int8_t get_prev_value(const Coords& coords) const{
        if(sparse){
            const int sum = std::accumulate(coords.begin(), coords.end(), 0);
            
            auto it = diagonalPrevs[sum].find(coords);
            
            if(it == diagonalPrevs[sum].end())
                return illegalprev;
            
            return it->second;
        }else{
            const std::size_t index = to_linear_index(coords, bounds);
            return prevs[index];
        }
    }
    
    void set_prev_value(const Coords& coords, int8_t value){
        if(sparse){
            const int sum = std::accumulate(coords.begin(), coords.end(), 0);
            assert(diagonalPrevs[sum].find(coords) != diagonalPrevs[sum].end());
            diagonalPrevs[sum][coords] = value;
        }else{
            const std::size_t index = to_linear_index(coords, bounds);
            prevs[index] = value;
        }
    }
    
    
    ogh::Production setPercentToMaxProduction(State& state, const Coords& coords) const{
        using ogh::ItemRarity;
        using ogh::Production;
        
        constexpr int metPercentBegin = 100;
        constexpr int crysPercentBegin = 100;
        constexpr int deutPercentBegin = 100;
        constexpr int fusionPercentBegin = 100;

        
        
        const int etechLevel = coords[index_energytech];
        const int plasmaLevel = coords[index_plasmatech];
        const bool hasGeologist = officers.hasGeologist();
        const bool hasEngineer = officers.hasEngineer();
        const bool hasStaff = officers.hasStaff();
        const int geologistpercent = hasGeologist ? 10 : 0;
        const int staffpercent = hasStaff ? 2 : 0;
        const double officerfactor = (geologistpercent + staffpercent) / 100.;
        const int metLevel = coords[index_metmine];
        const int crysLevel = coords[index_crystalmine];
        const int deutLevel = coords[index_deutsynth];
        const int solarLevel = coords[index_solarplant];
        const int solarplantPercent = 100;
        const int fusionLevel = coords[index_fusionplant];
        const int sats = 0;
        const int satsPercent = 0;
        const ogh::ItemRarity metItem = ogh::ItemRarity::None;
        const ogh::ItemRarity crysItem = ogh::ItemRarity::None;
        const ogh::ItemRarity deutItem = ogh::ItemRarity::None;

        const int oldMetPercent = state.percents.getMetPercents();
        const int oldCrysPercent = state.percents.getCrysPercents();
        const int oldDeutPercent = state.percents.getDeutPercents();
        const int oldFusionPercent = state.percents.getFusionPercents();

        int bestMetPercent = oldMetPercent;
        int bestCrysPercent = oldCrysPercent;
        int bestDeutPercent = oldDeutPercent;
        int bestFusionPercent = oldFusionPercent;
        std::int64_t bestDSE = 0;
        Production bestProd;
        
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
                        
                        const double itemProduction_met = simpleProduction_met * ogh::getItemProductionPercent(metItem)/100.;
                        const double itemProduction_crystal = simpleProduction_crystal * ogh::getItemProductionPercent(crysItem)/100.;
                        const double itemProduction_deut = simpleProduction_deut * ogh::getItemProductionPercent(deutItem)/100.;
                        
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
                        
                        result_met *= speedfactor;
                        result_crystal *= speedfactor;
                        result_deut *= speedfactor;
                        
                        Production newProd;
                        newProd.met = round(result_met);
                        newProd.crystal = round(result_crystal);
                        newProd.deut = round(result_deut);
                        
                        newProd *= 24;
                        
                        const std::int64_t newDSE = newProd.met / (traderate)[0] * (traderate)[2] + newProd.crystal / (traderate)[1] * (traderate)[2] + newProd.deut;
                        if(newDSE > bestDSE){
                            bestDSE = newDSE;
                            bestMetPercent = newMetPercent;
                            bestCrysPercent = newCrysPercent;
                            bestDeutPercent = newDeutPercent;
                            bestFusionPercent = newFusionPercent;
                            bestProd = newProd;
                        }
                    }
                }
            }
        }

        state.percents.setMetPercents(bestMetPercent);
        state.percents.setCrysPercents(bestCrysPercent);
        state.percents.setDeutPercents(bestDeutPercent);
        state.percents.setFusionPercents(bestFusionPercent);
        
        return bestProd;
    }
    
    void relax(const Coords& coords, const Coords& offset){
        using ogh::Entity;
        using ogh::EntityInfo;
        using ogh::EntityType;
        using ogh::Resources;
        using ogh::Production;
        using ogh::ItemRarity;
        
        
        
        auto& currentState = get_state_of_current_diagonal(coords);//dpmatrix[currentindex];
        //auto& currentRess = currentState.ress;
        
        auto realcoords = coords;
        for(int i = 0; i < dimensions; i++){
            realcoords[i] += offset[i];
        }
        
        bool isRelaxed = false;
                
        for(int entry = 0; entry < dimensions; entry++){
            if(coords[entry] > 0){
                
                Coords neighborcoords = coords;
                neighborcoords[entry]--;
                auto realneighborcoords = realcoords;
                realneighborcoords[entry]--;	
                
                if(get_prev_value(neighborcoords) == illegalprev){
                    continue;
                }
                
                const auto& neighborState = get_state_of_previous_diagonal(neighborcoords);//dpmatrix[neighborindex];
                const int neighborplanets = realneighborcoords[index_planetupgrades]+1;
                const Production neighborProduction = neighborState.prod;
                
                
                if(entry == index_planetupgrades){
                    
                    
                    const int reqAstro = ogh::getMinPossibleAstro(neighborplanets+1);
                    Resources astroBuildCosts = ogh::getBuildCosts(ogh::Astro, reqAstro);
                    if(reqAstro > 1)
                        astroBuildCosts += ogh::getBuildCosts(ogh::Astro, reqAstro - 1);
                    
                    astroBuildCosts += ogh::getBuildCosts(ogh::Coloship, 1);
                    
                    const float savetimeDaysAstro = ogh::get_save_duration_symmetrictrade(neighborState.ress.met, neighborState.ress.crystal, neighborState.ress.deut, /*have*/
                                                                                          astroBuildCosts.met, astroBuildCosts.crystal, astroBuildCosts.deut, /*want*/
                                                                                     neighborProduction.met, neighborProduction.crystal, neighborProduction.deut, /*production*/
                                                                                     traderate);
                    
                    if(savetimeDaysAstro == std::numeric_limits<float>::max())
                        continue;
                    
                    // produce resources in save time, then subtract buildcosts for the upgrade
                    Resources newRess = neighborState.ress + neighborProduction.produce(savetimeDaysAstro);
                    const std::int64_t diffm = newRess.met - astroBuildCosts.met;
                    const std::int64_t diffk = newRess.crystal + (traderate[1]*diffm)/traderate[0] - astroBuildCosts.crystal;
                    const std::int64_t diffd = newRess.deut + (traderate[2]*diffk)/traderate[1] - astroBuildCosts.deut;								
                    newRess.met = 0;
                    newRess.crystal = 0;
                    newRess.deut = std::max(std::int64_t(0), diffd);
                    
                    //research time of astro
                    float constructionTimeDaysAstro = ogh::getConstructionTimeInDays(ogh::Astro, reqAstro, 
                                                                                     realneighborcoords[index_robo], realneighborcoords[index_nani], realneighborcoords[index_shipyard], 
                                                                                     realneighborcoords[index_lab] * neighborplanets, speedfactor);
                    if(reqAstro > 1)
                        constructionTimeDaysAstro = ogh::getConstructionTimeInDays(ogh::Astro, reqAstro - 1, 
                                                                                   realneighborcoords[index_robo], realneighborcoords[index_nani], realneighborcoords[index_shipyard], 
                                                                                   realneighborcoords[index_lab] * neighborplanets, speedfactor);
                        
                    //build time of colo ship
                    float constructionTimeColoShip = ogh::getConstructionTimeInDays(ogh::Coloship, 1, 
                                                                                    realneighborcoords[index_robo], realneighborcoords[index_nani], realneighborcoords[index_shipyard], 
                                                                                    realneighborcoords[index_lab] * neighborplanets, speedfactor);
                    
                    //resources are produced during research period
                    newRess += neighborProduction.produce(constructionTimeDaysAstro + constructionTimeColoShip);
                    
                    Resources newPlanetBuildingsCosts{};
                    
                    for(auto index : {index_metmine, index_crystalmine, index_deutsynth, index_solarplant, index_fusionplant, index_lab, index_robo, index_nani, index_shipyard}){
                        newPlanetBuildingsCosts += ogh::getTotalCosts(entityInfoArray[index], realcoords[index]);
                    }
                    
                    //save time to gather resources for the buildings of new planet. this is just an upper bound because the increase in production while upgrading the new planet is not included.
                    const float savetimeDaysBuildings = ogh::get_save_duration_symmetrictrade(newRess.met, newRess.crystal, newRess.deut, /*have*/
                                                                                              newPlanetBuildingsCosts.met, newPlanetBuildingsCosts.crystal, newPlanetBuildingsCosts.deut, /*want*/
                                                                                            neighborProduction.met, neighborProduction.crystal, neighborProduction.deut, /*production*/
                                                                                            traderate);
                    
                    //upgrade of new planet can only start after both enough resources for the upgrade are saved and research + colo ship are done
                    //this is an upper bound since the resources for higher levels need not be present when building lower levels.
                    //resources during research period have already been produced
                    const float productiontime = std::max(savetimeDaysBuildings - (constructionTimeColoShip + constructionTimeDaysAstro), 0.0f);
                    
                    newRess = newRess + neighborProduction.produce(productiontime);
                    const std::int64_t diffm2 = newRess.met - newPlanetBuildingsCosts.met;
                    const std::int64_t diffk2 = newRess.crystal + (traderate[1]*diffm2)/traderate[0] - newPlanetBuildingsCosts.crystal;
                    const std::int64_t diffd2 = newRess.deut + (traderate[2]*diffk2)/traderate[1] - newPlanetBuildingsCosts.deut;								
                    newRess.met = 0;
                    newRess.crystal = 0;
                    newRess.deut = std::max(std::int64_t(0), diffd2);
                    
                    //calculate total upgrade time for buildings
                    float constructionTimeDaysBuildings = 0;
                    
                    //build robo at new planet
                    for(int robo = 1; robo <= realneighborcoords[index_robo]; robo++){
                        constructionTimeDaysBuildings += ogh::getConstructionTimeInDays(entityInfoArray[index_robo], robo, robo-1, 0, realneighborcoords[index_shipyard], realneighborcoords[index_lab] * neighborplanets, speedfactor);
                    }
                    
                    //build nani at new planet
                    for(int nani = 1; nani <= realneighborcoords[index_nani]; nani++){
                        constructionTimeDaysBuildings += ogh::getConstructionTimeInDays(entityInfoArray[index_nani], nani, realneighborcoords[index_robo], nani-1, realneighborcoords[index_shipyard], realneighborcoords[index_lab] * neighborplanets, speedfactor);
                    }
                    
                    //build remaining buildings at new planet
                    for(auto index : {index_metmine, index_crystalmine, index_deutsynth, index_solarplant, index_fusionplant, index_lab, index_shipyard}){
                        for(int level = 1; level <= realneighborcoords[index]; level++){
                            constructionTimeDaysBuildings += ogh::getConstructionTimeInDays(entityInfoArray[index], level, realneighborcoords[index_robo], realneighborcoords[index_nani], realneighborcoords[index_shipyard], realneighborcoords[index_lab] * neighborplanets, speedfactor);
                        }
                    }
                    
                    //resources are produced during construction period
                    newRess += neighborProduction.produce(constructionTimeDaysBuildings);
                                        
                    if(get_prev_value(neighborcoords) == noprev){
                        isRelaxed = true;
                        currentState.time = savetimeDaysAstro + std::max(savetimeDaysBuildings, constructionTimeDaysAstro + constructionTimeColoShip) + constructionTimeDaysBuildings;
                        currentState.savetime = savetimeDaysAstro + savetimeDaysBuildings;
                        currentState.ress = newRess;
                        set_prev_value(coords, entry);
                        
                    }else{
                        /*float newSaveTime = std::numeric_limits<float>::max();
                         *   float newTime = std::numeric_limits<float>::max();
                         *   
                         *   if((isResearchIndex(get_prev_value(neighborcoords)) && isResearchIndex(entry))
                         *       || (isBuildingIndex(get_prev_value(neighborcoords)) && isBuildingIndex(entry))){
                         *       
                         *       newSaveTime = neighborState.savetime + savetimeDays;
                         *       //previous upgrade was same type of upgrade as current upgrade, have to wait for previous upgrade to finish
                         *       newTime = std::max(newSaveTime, neighborState.time) + singleConstructionTimeDays;
                    }else{
                        newSaveTime = neighborState.savetime + savetimeDays;
                        //don't need to wait for previous upgrade, but make sure that finish time of current state cannot be less than finish time of previous state
                        newTime = std::max(newSaveTime + singleConstructionTimeDays, neighborState.time);
                    }
                    
                    assert(newSaveTime != std::numeric_limits<float>::max());
                    assert(newTime != std::numeric_limits<float>::max());
                    
                    if(!isRelaxed || newTime < currentState.time){
                        isRelaxed = true;
                        currentState.time = newTime;
                        currentState.savetime = newSaveTime;
                        currentState.ress = newRess;
                        set_prev_value(coords, entry);
                    }*/
                        const float newSaveTime = neighborState.savetime + savetimeDaysAstro + savetimeDaysBuildings;
                        const float newTime = neighborState.time + savetimeDaysAstro + std::max(savetimeDaysBuildings, constructionTimeDaysAstro + constructionTimeColoShip) + constructionTimeDaysBuildings;
                        
                        assert(newSaveTime != std::numeric_limits<float>::max());
                        assert(newTime != std::numeric_limits<float>::max());
                        
                        if(!isRelaxed || newTime < currentState.time){
                            isRelaxed = true;
                            currentState.time = newTime;
                            currentState.savetime = newSaveTime;
                            currentState.ress = newRess;
                            set_prev_value(coords, entry);
                        }
                    }
                    
                }else{
                    int costFactor = 1;
                    switch(entityInfoArray[entry].type){
                        case EntityType::Research: costFactor = 1; break;
                        case EntityType::Building: costFactor = neighborplanets; break;
                        case EntityType::Ship: costFactor = 1; break;
                        case EntityType::None: assert(false);
                    }
                    
                    Resources buildcosts = costFactor * ogh::getBuildCosts(entityInfoArray[entry], realcoords[entry]);
                    
                    const float savetimeDays = ogh::get_save_duration_symmetrictrade(neighborState.ress.met, neighborState.ress.crystal, neighborState.ress.deut, /*have*/
                                                                                     buildcosts.met, buildcosts.crystal, buildcosts.deut, /*want*/
                                                                                     neighborProduction.met, neighborProduction.crystal, neighborProduction.deut, /*production*/
                                                                                     traderate);
                    
                    if(savetimeDays == std::numeric_limits<float>::max())
                        continue;
                    
                    // produce resources in save time, then subtract buildcosts for the upgrade
                    Resources newRess = neighborState.ress + neighborProduction.produce(savetimeDays);
                    const std::int64_t diffm = newRess.met - buildcosts.met;
                    const std::int64_t diffk = newRess.crystal + (traderate[1]*diffm)/traderate[0] - buildcosts.crystal;
                    const std::int64_t diffd = newRess.deut + (traderate[2]*diffk)/traderate[1] - buildcosts.deut;								
                    newRess.met = 0;
                    newRess.crystal = 0;
                    newRess.deut = std::max(std::int64_t(0), diffd);
                    
                    //construction time of upgrade
                    float singleConstructionTimeDays = ogh::getConstructionTimeInDays(entityInfoArray[entry], realcoords[entry], realneighborcoords[index_robo], realneighborcoords[index_nani], realneighborcoords[index_shipyard], realneighborcoords[index_lab] * neighborplanets, speedfactor);
                    
                    //resources are produced during construction period
                    newRess += neighborProduction.produce(singleConstructionTimeDays);
                    
                    assert(savetimeDays != std::numeric_limits<float>::max());
                    assert(singleConstructionTimeDays != std::numeric_limits<float>::max());

                    if(get_prev_value(neighborcoords) == noprev){
                        isRelaxed = true;
                        currentState.time = savetimeDays + singleConstructionTimeDays;
                        currentState.savetime = savetimeDays;
                        currentState.ress = newRess;
                        set_prev_value(coords, entry);
                        
                    }else{
                        /*float newSaveTime = std::numeric_limits<float>::max();
                         *   float newTime = std::numeric_limits<float>::max();
                         *   
                         *   if((isResearchIndex(get_prev_value(neighborcoords)) && isResearchIndex(entry))
                         *       || (isBuildingIndex(get_prev_value(neighborcoords)) && isBuildingIndex(entry))){
                         *       
                         *       newSaveTime = neighborState.savetime + savetimeDays;
                         *       //previous upgrade was same type of upgrade as current upgrade, have to wait for previous upgrade to finish
                         *       newTime = std::max(newSaveTime, neighborState.time) + singleConstructionTimeDays;
                    }else{
                        newSaveTime = neighborState.savetime + savetimeDays;
                        //don't need to wait for previous upgrade, but make sure that finish time of current state cannot be less than finish time of previous state
                        newTime = std::max(newSaveTime + singleConstructionTimeDays, neighborState.time);
                    }
                    
                    assert(newSaveTime != std::numeric_limits<float>::max());
                    assert(newTime != std::numeric_limits<float>::max());
                    
                    if(!isRelaxed || newTime < currentState.time){
                        isRelaxed = true;
                        currentState.time = newTime;
                        currentState.savetime = newSaveTime;
                        currentState.ress = newRess;
                        set_prev_value(coords, entry);
                    }*/
                        const float newSaveTime = neighborState.savetime + savetimeDays;
                        const float newTime = neighborState.time + singleConstructionTimeDays + savetimeDays;
                        
                        assert(newSaveTime != std::numeric_limits<float>::max());
                        assert(newTime != std::numeric_limits<float>::max());
                        
                        if(!isRelaxed || newTime < currentState.time){
                            isRelaxed = true;
                            currentState.time = newTime;
                            currentState.savetime = newSaveTime;
                            currentState.ress = newRess;
                            set_prev_value(coords, entry);
                        }
                    }
                }
            }
        }
        
        if(!isRelaxed)
            set_prev_value(coords, illegalprev);
        else{
            auto currentPlanetProduction = setPercentToMaxProduction(currentState, realcoords);
            const int myplanets = realcoords[index_planetupgrades]+1;
            currentState.prod = myplanets * currentPlanetProduction;
        }
    }
    
    std::vector<PathNode> solve1(const Coords& begin, const Coords& target, int num_threads){
        using ogh::ItemRarity;
        
        if(!isValidCoordinate(target)){
            print_coords(std::cout, target);
            std::cout << " is not a valid configuration!\n";
            return {};
        }
        
        
        for(int i = 0; i < dimensions; i++){
            bounds[i] = target[i] - begin[i] + 1;
        }
        
        const std::size_t num_states = std::accumulate(bounds.begin(), bounds.end(), std::size_t(1), std::multiplies<std::size_t>{});
        int num_diagonals = 1;// + std::accumulate(bounds.begin(), bounds.end(), int(1), std::plus<int>{});
        for(const auto& i : bounds)
            num_diagonals += (i-1);
        
        
        const std::vector<std::size_t> number_of_coords_per_diagonal = get_numbers_of_coords_with_distance<dimensions>(bounds.data());
        
        if(!sparse){
            
        }
        
        State initialstate;
        initialstate.time = 0;
        initialstate.savetime = 0;
        initialstate.ress.met = 0;
        initialstate.ress.crystal = 0;
        initialstate.ress.deut = 0;
        
        initialstate.prod = ogh::getDailyProduction(begin[index_metmine], ItemRarity::None, 100, 
                                                    begin[index_crystalmine], ItemRarity::None, 100, 
                                                    begin[index_deutsynth], ItemRarity::None, 100, 
                                                    begin[index_solarplant], 100,
                                                    begin[index_fusionplant], 100, begin[index_energytech], 
                                                    temperature, 0, 100,
                                                    begin[index_plasmatech], speedfactor, 
                                                    officers.hasEngineer(), officers.hasGeologist(), officers.hasStaff());
        
        diagonalPrevs.resize(1);
        
        if(sparse){
            previousDiagonalStates[Coords{}] = initialstate;
            diagonalPrevs.resize(num_diagonals);
            diagonalPrevs[0][Coords{}] = noprev;
        }else{
            dpmatrix.resize(num_states);
            //initialize begin state
            dpmatrix[0] = initialstate;
            prevs.resize(num_states, int8_t(noprev));
        }
        
        
        auto handle_coords = [&, this](const auto& coords){
            if(isValidCoordinate(coords)){
                relax(coords, begin);
            }else{
                //discard state
                const std::size_t index = to_linear_index(coords, bounds);
                prevs[index] = illegalprev;
            }
        };
        
        std::ofstream prevstream(tmpdir + "/prevs.bin", std::ios_base::binary);
        std::ofstream statestream(tmpdir + "/states.bin", std::ios_base::binary);
        
        std::vector<std::size_t> prevBytesPrefixSum{0};
        std::vector<std::size_t> stateBytesPrefixSum{0};
        
        if(sparse){
            std::size_t prevsbytes = sizeof(std::pair<Coords, int8_t>) * diagonalPrevs[0].size();
            prevstream.write(reinterpret_cast<const char*>(&prevsbytes), sizeof(std::size_t));
            prevBytesPrefixSum.emplace_back(prevsbytes + sizeof(std::size_t) + prevBytesPrefixSum.back());
            
            for(const auto& p : diagonalPrevs[0]){
                prevstream.write(reinterpret_cast<const char*>(&p), sizeof(p));
            }
            
            std::size_t statesbytes = sizeof(std::pair<Coords, State>) * previousDiagonalStates.size();
            statestream.write(reinterpret_cast<const char*>(&statesbytes), sizeof(std::size_t));
            stateBytesPrefixSum.emplace_back(statesbytes + sizeof(std::size_t) + stateBytesPrefixSum.back());
            
            for(const auto& p : previousDiagonalStates){
                statestream.write(reinterpret_cast<const char*>(&p), sizeof(p));
            }
        }
        
        //relax along diagonals
        for(int diagonal = 1; diagonal < num_diagonals; diagonal++){
            
            std::cout << "Relaxing diagonal " << diagonal << " ...\n";
            currentDiagonal = diagonal;
            
            if(sparse){
                std::vector<std::vector<Coords>> coords_per_thread(num_threads);
                
                TIMERSTARTCPU(first_diag_pass);
                enumerate_manhattan_coords_increasing<dimensions>(bounds, diagonal, [&](const auto& coords){
                    auto realcoords = coords;
                    for(int i = 0; i < dimensions; i++){
                        realcoords[i] += begin[i];
                    }
                    
                    if(isValidCoordinate(realcoords)){
                        int threadid = omp_get_thread_num();
                        coords_per_thread[threadid].emplace_back(coords);
                    }
                }, num_threads);
                TIMERSTOPCPU(first_diag_pass);
                
                for(const auto& vec : coords_per_thread){
                    for(const auto& coords : vec){
                        currentDiagonalStates[coords] = State{};
                        diagonalPrevs[diagonal][coords] = noprev;
                    }
                }
                
                omp_set_num_threads(num_threads);
                
                TIMERSTART(relax)
                #pragma omp parallel
                {
                    int threadId = omp_get_thread_num();
                    int threads = omp_get_num_threads();
                    const std::size_t size = currentDiagonalStates.size();
                    
                    auto iter = currentDiagonalStates.begin();
                    std::advance(iter, threadId);
                    std::size_t i = threadId;
                    while(i < size){
                        
                        relax(iter->first, begin);
                        
                        std::advance(iter, threads);						
                        i += threads;
                    }
                    
                }
                TIMERSTOP(relax)
                
                TIMERSTART(fileoutput);
                
                std::size_t prevsbytes = sizeof(std::pair<Coords, int8_t>) * diagonalPrevs[diagonal].size();
                prevstream.write(reinterpret_cast<const char*>(&prevsbytes), sizeof(std::size_t));
                prevBytesPrefixSum.emplace_back(prevsbytes + sizeof(std::size_t) + prevBytesPrefixSum.back());
                
                for(const auto& p : diagonalPrevs[diagonal]){
                    prevstream.write(reinterpret_cast<const char*>(&p), sizeof(p));
                }
                
                std::size_t statesbytes = sizeof(std::pair<Coords, State>) * currentDiagonalStates.size();
                statestream.write(reinterpret_cast<const char*>(&statesbytes), sizeof(std::size_t));
                stateBytesPrefixSum.emplace_back(statesbytes + sizeof(std::size_t) + stateBytesPrefixSum.back());
                
                for(const auto& p : currentDiagonalStates){
                    statestream.write(reinterpret_cast<const char*>(&p), sizeof(p));
                }
                TIMERSTOP(fileoutput);
                
                if(sparse){
                    std::swap(previousDiagonalStates, currentDiagonalStates);
                    currentDiagonalStates.clear();
                    
                    if(diagonal > 1){
                        diagonalPrevs[diagonal-2].clear();
                    }
                }
                
            }else{
                TIMERSTART(relaxdiagonal);
                enumerate_manhattan_coords_increasing<dimensions>(bounds, diagonal, handle_coords, num_threads);
                TIMERSTOP(relaxdiagonal);
            }

            previousDiagonal = diagonal;
        }
        
        prevstream.flush();
        statestream.flush();
        
        
        
        
        //backtrack from target to origin
        std::vector<PathNode> path;
        Coords currentcoords = target;	
        for(int i = 0; i < dimensions; i++){
            currentcoords[i] -= begin[i];
        }
        int8_t currentprev;
        if(sparse){
            currentprev = get_prev_value(currentcoords);
        }else{
            currentprev = prevs[num_states-1];
        }
        auto realcoords = currentcoords;
        for(int i = 0; i < dimensions; i++){
            realcoords[i] += begin[i];
        }
        path.emplace_back(realcoords, get_state_of_previous_diagonal(currentcoords));
        
        std::ifstream previnstream(tmpdir + "/prevs.bin", std::ios_base::binary);
        
        
        int diag = num_diagonals-2;
                
        TIMERSTART(backtrack);
        while(currentprev != noprev && currentprev != illegalprev && diag >= 0){
            
            currentcoords[currentprev] -= 1;
            const std::size_t index = to_linear_index(currentcoords, bounds);
            realcoords = currentcoords;
            for(int i = 0; i < dimensions; i++){
                realcoords[i] += begin[i];
            }
            if(sparse){
                path.emplace_back(realcoords, State{});
            }else{
                path.emplace_back(realcoords, dpmatrix[index]);
            }
            
            if(sparse){
                
                //currentprev = get_prev_value(currentcoords);
                                
                currentprev = illegalprev;
                
                previnstream.seekg(prevBytesPrefixSum[diag] + sizeof(std::size_t));
                
                std::pair<Coords, int8_t> pair;
                while(std::size_t(previnstream.tellg()) < prevBytesPrefixSum[diag+1] 
                        && previnstream.read(reinterpret_cast<char*>(&pair), sizeof(std::pair<Coords, int8_t>))){
                                    
                    if(currentcoords == pair.first){
                        currentprev = pair.second;
                        break;
                    }
                    
                }
                
            }else{
                currentprev = prevs[num_states-1];
            }
            
            diag--;
        }	
        
        std::reverse(path.begin(), path.end());
        TIMERSTOP(backtrack);
        
        std::ifstream stateinstream(tmpdir + "/states.bin", std::ios_base::binary);
        
       
        
        if(sparse){
            TIMERSTART(scanfile);

            for(auto& node : path){
                std::pair<Coords, State> pair;
                size_t bytes = 0; //the number of bytes of the pairs belonging to current diagonal
                stateinstream.read(reinterpret_cast<char*>(&bytes), sizeof(std::size_t));
                
                std::size_t posBefore = stateinstream.tellg();
                
                while(std::size_t(stateinstream.tellg()) - posBefore < bytes && 
                        stateinstream.read(reinterpret_cast<char*>(&pair), sizeof(std::pair<Coords, State>))){
                    
                    auto realpaircoords = pair.first;
                    for(int i = 0; i < dimensions; i++){
                        realpaircoords[i] += begin[i];
                    }
                    if(node.coords == realpaircoords){
                        node.state = pair.second;
                        break;
                    }
                    
                }
                
                std::size_t posAfter = stateinstream.tellg();
                
                assert(posAfter - posBefore <= bytes);
                
                //skip remaining pairs of current diagonal
                stateinstream.seekg(bytes - (posAfter - posBefore), std::ios_base::cur);
            }

            TIMERSTOP(scanfile);
        }
        
        
        
        return path;
    }
                       
};




void print_time(std::ostream& os, float daysfloat){
    std::uint64_t seconds = daysfloat * 24ULL * 60ULL * 60ULL;
    std::uint64_t days = seconds / (24ULL*60ULL*60ULL);
    seconds = seconds % (24ULL*60ULL*60ULL);
    std::uint64_t hours = seconds / (60ULL*60ULL);
    seconds = seconds % (60ULL * 60ULL);
    std::uint64_t minutes = seconds / 60ULL;
    seconds = seconds % 60ULL;
    os << days << "d " << hours << "h " << minutes << "m " << seconds << "s";
}




void usage(int argc, char** argv){
    std::cout << "Usage:" << argv[0] << " Options\n\n";
    
    std::cout << "Calculates the fastest sequence of upgrades to reach a certain account state\n\n";
    std::cout << "Options:\n";
    std::cout << "--speed ecospeedfactor: Economy speed factor of universe. Default 1.\n";
    std::cout << "--threads numthreads: Number of CPU threads to use. Default 1.\n";
    std::cout << "--tmpdir directory: Temporary files are stored in directory. Default \".\"\n";
    
    std::cout << '\n';
    std::cout << "Example: " << argv[0] << " --speed 2 --threads 8" << std::endl;
}


int main(int argc, char** argv){
    using Coords = ListCalculator::Coords;
    
    int num_threads = 1;
    int speedfactor = 1;
    std::string tmpdir = ".";
    
    for(int i = 1; i < argc; i++){
        if(std::string(argv[i]) == "--help"){
            usage(argc, argv);
            return 0;
        }
        
        if(std::string(argv[i]) == "--speed"){
            assert(i+1 < argc);
            speedfactor = std::atoi(argv[i+1]);
            i++;
        }
        
        if(std::string(argv[i]) == "--threads"){
            assert(i+1 < argc);
            num_threads = std::atoi(argv[i+1]);
            i++;
        }
        
        if(std::string(argv[i]) == "--tmpdir"){
            assert(i+1 < argc);
            tmpdir = std::string(argv[i+1]);
            i++;
        }
    }
    
    ListCalculator calculator;
    calculator.setTmpDir(tmpdir);
    
    Coords begin = calculator.make_coords(MetLevel{0}, 
                                          CrysLevel{0},
                                          DeutLevel{0},
                                          SolarLevel{0},
                                          FusionLevel{0},
                                          EnergyLevel{0},
                                          PlasmaLevel{0},
                                          LabLevel{0},
                                          RoboLevel{0},
                                          NaniLevel{0},
                                          PlanetUpgradesLevel{0},
                                          SpyLevel{0},
                                          ImpulseLevel{0},
                                          ShipyardLevel{0});


/*    Coords target = calculator.make_coords(MetLevel{22}, 
                                          CrysLevel{18},
                                          DeutLevel{18},
                                          SolarLevel{24},
                                          FusionLevel{0},
                                          EnergyLevel{8},
                                          PlasmaLevel{5},
                                          LabLevel{8},
                                          RoboLevel{8},
                                          NaniLevel{0},
                                          PlanetUpgradesLevel{5},
                                          SpyLevel{4},
                                          ImpulseLevel{3},
                                          ShipyardLevel{4});*/
                                          
    
    
    Coords target = calculator.make_coords(MetLevel{16}, 
                                           CrysLevel{14},
                                           DeutLevel{12},
                                           SolarLevel{20},
                                           FusionLevel{0},
                                           EnergyLevel{8},
                                           PlasmaLevel{3},
                                           LabLevel{4},
                                           RoboLevel{2},
                                           NaniLevel{0},
                                           PlanetUpgradesLevel{1},
                                           SpyLevel{4},
                                           ImpulseLevel{3},
                                           ShipyardLevel{4});
    
    /*Coords target = calculator.make_coords(MetLevel{16}, 
                                        CrysLevel{14},
                                        DeutLevel{12},
                                        SolarLevel{20},
                                        FusionLevel{0},
                                        EnergyLevel{0},
                                        PlasmaLevel{0},
                                        LabLevel{0},
                                        RoboLevel{0},
                                        NaniLevel{0},
                                        PlanetUpgradesLevel{0},
                                        SpyLevel{0},
                                        ImpulseLevel{0},
                                        ShipyardLevel{0});*/
    
/*    Coords begin = calculator.make_coords(MetLevel{45}, 
                                          CrysLevel{39},
                                          DeutLevel{42},
                                          SolarLevel{0},
                                          FusionLevel{21},
                                          EnergyLevel{22},
                                          PlasmaLevel{20},
                                          LabLevel{18},
                                          RoboLevel{12},
                                          NaniLevel{5},
                                          PlanetUpgradesLevel{14},
                                          SpyLevel{10},
                                          ImpulseLevel{10},
                                          ShipyardLevel{10});
    
    Coords target = calculator.make_coords(MetLevel{47}, 
                                           CrysLevel{41},
                                           DeutLevel{44},
                                           SolarLevel{0},
                                           FusionLevel{22},
                                           EnergyLevel{22},
                                           PlasmaLevel{20},
                                           LabLevel{18},
                                           RoboLevel{13},
                                           NaniLevel{6},
                                           PlanetUpgradesLevel{14},
                                           SpyLevel{10},
                                           ImpulseLevel{10},
                                           ShipyardLevel{10});*/
    
    
    
    calculator.speedfactor = speedfactor;
    calculator.traderate = {3.0f,2.0f,1.0f};
    
    
    TIMERSTART(solve1);
    
    auto result = calculator.solve1(begin, target, num_threads);
    
    TIMERSTOP(solve1);
    
    auto print_coords = [&](std::ostream& os, const Coords& coords){
        os << "[";
        for(const auto i : coords){
            os << int(i) << ',';
        }
        os << "]";
    };
    
    
    
    std::for_each(result.begin(), result.end(), [&](const auto& node){		
        print_coords(std::cout, node.coords);
        std::cout << ", ";
        
        print_time(std::cout, node.state.time);
        
        std::cout << " " << node.state.percents.getMetPercents() << " " << node.state.percents.getCrysPercents() << " " << node.state.percents.getDeutPercents() << " " << node.state.percents.getFusionPercents();
        
        std::cout << '\n';
    });
    
    
    return 0;
}


