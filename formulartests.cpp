#include "ogame.hpp"

#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>

using namespace ogamehelpers;

template<class T>
std::string tsep(T value)
{
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << value;
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Resources& r){
    //os << r.met << ", " << r.crystal << ", " << r.deut;
    os << tsep(r.met) << ", " << tsep(r.crystal) << ", " << tsep(r.deut);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Production& p){
    //os << p.met << ", " << p.crystal << ", " << p.deut;
    os << tsep(p.met) << ", " << tsep(p.crystal) << ", " << tsep(p.deut);
    return os;
}

std::string convert_time(float daysfloat){
    std::stringstream ss;
    std::uint64_t seconds = daysfloat * 24ULL * 60ULL * 60ULL;
    std::uint64_t days = seconds / (24ULL*60ULL*60ULL);
    seconds = seconds % (24ULL*60ULL*60ULL);
    std::uint64_t hours = seconds / (60ULL*60ULL);
    seconds = seconds % (60ULL * 60ULL);
    std::uint64_t minutes = seconds / 60ULL;
    seconds = seconds % 60ULL;
    ss << days << "d " << hours << "h " << minutes << "m " << seconds << "s";
    return ss.str();
}


int main(){
    EntityMap entityMap;
    
    auto metMine = entityMap.getInfo(Entity::Metalmine);
    auto crysMine = entityMap.getInfo(Entity::Crystalmine);
    auto deutMine = entityMap.getInfo(Entity::Deutsynth);
    auto fkw = entityMap.getInfo(Entity::Fusion);
    auto robo = entityMap.getInfo(Entity::Robo);
    auto nani = entityMap.getInfo(Entity::Nanite);
    
    std::cout << "Robo 0, Nani 0, speed 2" << std::endl;
    
    for(auto building : {metMine, crysMine, deutMine, fkw}){
        for(auto lvl : {1,3,10,20}){
            auto buildCosts = getBuildCosts(building, lvl);
            float days = getConstructionTimeInDays(building, lvl, 0, 0, 0, 0, 2);
            auto dhm = convert_time(days);
            std::cout << "Costs " << building.name << " Lvl " << lvl << " : " << buildCosts << ". Construction time: " << days << " days. " << dhm << std::endl;
        }        
    }
    
    std::cout << "Robo 10, Nani 5, speed 2" << std::endl;
    
    for(auto building : {metMine, crysMine, deutMine}){
        for(auto lvl : {30,40,45}){
            auto buildCosts = getBuildCosts(building, lvl);
            float days = getConstructionTimeInDays(building, lvl, 10, 5, 0, 0, 2);
            auto dhm = convert_time(days);
            std::cout << "Costs " << building.name << " Lvl " << lvl << " : " << buildCosts << ". Construction time: " << days << " days. " << dhm << std::endl;
        }        
    }
    
    std::cout << "Nani 5, speed 2" << std::endl;
    
    for(auto building : {robo}){
        for(auto lvl : {11,12,13}){
            auto buildCosts = getBuildCosts(building, lvl);
            float days = getConstructionTimeInDays(building, lvl, lvl-1, 5, 0, 0, 2);
            auto dhm = convert_time(days);
            std::cout << "Costs " << building.name << " Lvl " << lvl << " : " << buildCosts << ". Construction time: " << days << " days. " << dhm << std::endl;
        }        
    }
    
    std::cout << "Robo 10, speed 2" << std::endl;
    
    for(auto building : {nani}){
        for(auto lvl : {1,2,3,4,5}){
            auto buildCosts = getBuildCosts(building, lvl);
            float days = getConstructionTimeInDays(building, lvl, 10, lvl-1, 0, 0, 2);
            auto dhm = convert_time(days);
            std::cout << "Costs " << building.name << " Lvl " << lvl << " : " << buildCosts << ". Construction time: " << days << " days. " << dhm << std::endl;
        }        
    }
    
    auto asdf1 = getDailyProduction(30, ItemRarity::None, 100, 
                                          26, ItemRarity::None, 100, 
                                          25, ItemRarity::None, 100, 
                                          33,100,
                                            0,0,0,
                                            0,0,0,
                                            0,2,
                                            false, true, false);
    auto prodfact1 = getMineProductionFactor(30,100,
                                             26,100,
                                             25,100,
                                             33,100,
                                             0,0,0, 
                                             0,0,0,
                                             false, false);
    
    std::cout << "Production m 30, k 26, d 25, skw 33 x2 speed geo: " << asdf1 << " per day. production factor : " << prodfact1 << std::endl;
    
    auto asdf2 = getDailyProduction(45, ItemRarity::None, 100, 
                                    39, ItemRarity::None, 100, 
                                    42, ItemRarity::None, 100, 
                                    48,100,
                                    0,0,0,
                                    0,0,0,
                                    0,5,
                                    false, true, false);
    auto prodfact2 = getMineProductionFactor(45,100,
                                             39,100,
                                             42,100,
                                             48,100,
                                             0,0,0, 
                                             0,0,0,
                                             false, false);
    
    
    std::cout << "Production m 45, k 39, d 42, skw 48 x5 speed geo: " << asdf2 << " per day. production factor : " << prodfact2 << std::endl;
    
    auto asdf3 = getDailyProduction(45, ItemRarity::None, 100, 
                                    39, ItemRarity::None, 100, 
                                    42, ItemRarity::None, 100, 
                                    0,0,
                                    21,100,22,
                                    0,0,0,
                                    0,5,
                                    false, true, false);
    auto prodfact3 = getMineProductionFactor(45,100,
                                             39,100,
                                             42,100,
                                             0,0,
                                             21,100,22, 
                                             0,0,0,
                                             false, false);
    
    
    std::cout << "Production m 45, k 39, d 42, fkw 21, etech 22 x5 speed geo: " << asdf3 << " per day. production factor : " << prodfact3 << std::endl;
    
    
    
}

