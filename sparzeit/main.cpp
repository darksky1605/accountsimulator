#include "../ogame.hpp"

#include <iostream>
#include <array>
#include <string>
#include <vector>

#include <cassert>

namespace ogh = ogamehelpers;

int main(){
    
    
    
    int metLevel = 0;
    const ogh::ItemRarity metItem = ogh::ItemRarity::None;
    const int metPercent = 100;   
    
    int crysLevel = 0;
    const ogh::ItemRarity crysItem = ogh::ItemRarity::None;
    const int crysPercent = 100;
    
    int deutLevel = 0;
    const ogh::ItemRarity deutItem= ogh::ItemRarity::None;
    const int deutPercent = 100;
    
    const int solarLevel = 57;
    const int solarplantPercent = 100;
    const int fusionLevel = 0;
    const int fusionPercent = 0;
    const int etechLevel = 0;
    const int temperature = 0;
    const int sats = 0;
    const int satsPercent = 0;
    const int plasmaLevel = 0;
    const int speedfactor = 4;
    const bool engineer = false;
    const bool geologist = false;
    const bool staff = false;
    
    
    const std::array<float, 3> traderate{2,1,1};
    
    ogh::EntityMap entityMap;
    auto metalMine = entityMap.getInfo(ogh::Entity::Metalmine);
    auto crysMine = entityMap.getInfo(ogh::Entity::Crystalmine);
    auto deutSynth = entityMap.getInfo(ogh::Entity::Deutsynth);
        
    while(metLevel <= 50 && crysLevel <= 50 && deutLevel <= 50){
        
        ogh::Production production = ogh::getDailyProduction(metLevel, metItem, metPercent, 
                                                            crysLevel, crysItem, crysPercent, 
                                                            deutLevel, deutItem, deutPercent, 
                                                            solarLevel, solarplantPercent,
                                                            fusionLevel, fusionPercent, etechLevel,
                                                            temperature, sats, satsPercent,
                                                            plasmaLevel, speedfactor, 
                                                            engineer, geologist, staff);
        
        const std::int64_t hm = 0;
        const std::int64_t hk = 0;
        const std::int64_t hd = 0;
        
        const std::int64_t pm = production.met;
        const std::int64_t pk = production.crystal;
        const std::int64_t pd = production.deut;
        
        const ogh::Resources buildcostsMet = ogh::getBuildCosts(metalMine, metLevel + 1);
        const ogh::Resources buildcostsCrys = ogh::getBuildCosts(crysMine, crysLevel + 1);
        const ogh::Resources buildcostsDeut = ogh::getBuildCosts(deutSynth, deutLevel + 1);
        
        const float savetimeMet = ogh::get_save_duration_symmetrictrade(hm, hk, hd, /*have*/
                                                                        buildcostsMet.met, buildcostsMet.crystal, buildcostsMet.deut, /*want*/
                                                                        pm, pk, pd, /*production*/
                                                                        traderate /*e.g 3:2:1*/);   
        const float savetimeCrys = ogh::get_save_duration_symmetrictrade(hm, hk, hd, /*have*/
                                                                         buildcostsCrys.met, buildcostsCrys.crystal, buildcostsCrys.deut, /*want*/
                                                                        pm, pk, pd, /*production*/
                                                                        traderate /*e.g 3:2:1*/);  
        const float savetimeDeut = ogh::get_save_duration_symmetrictrade(hm, hk, hd, /*have*/
                                                                         buildcostsDeut.met, buildcostsDeut.crystal, buildcostsDeut.deut, /*want*/
                                                                        pm, pk, pd, /*production*/
                                                                        traderate /*e.g 3:2:1*/);  
        float mymin = 0;
        
        if(savetimeMet <= savetimeCrys){
            if(savetimeMet <= savetimeDeut){
                // met <= crys && met <= deut
                std::cout << "met " << (metLevel+1) << " : " << (savetimeMet) << " " << savetimeMet << " " << savetimeCrys << " " << savetimeDeut << std::endl;
                metLevel++;
                mymin = savetimeMet;
            }else{
                //deut < met <= crys 
                std::cout << "deut " << (deutLevel+1) << " : " << (savetimeDeut) << " " << savetimeMet << " " << savetimeCrys << " " << savetimeDeut << std::endl;
                deutLevel++;
                mymin = savetimeDeut;
            }
        }else{            
            if(savetimeCrys <= savetimeDeut){
                // crys < met && crys <= deut
                std::cout << "kris " << (crysLevel+1) << " : " << (savetimeCrys) << " " << savetimeMet << " " << savetimeCrys << " " << savetimeDeut << std::endl;
                crysLevel++;
                mymin = savetimeCrys;
            }else{
                // crys < met && crys > deut
                std::cout << "deut " << (deutLevel+1) << " : " << (savetimeDeut) << " " << savetimeMet << " " << savetimeCrys << " " << savetimeDeut << std::endl;
                deutLevel++;
                mymin = savetimeDeut;
            }
        }
        
        assert(mymin == std::min(savetimeMet, std::min(savetimeCrys, savetimeDeut)));
    }
    

    return 0;
}
