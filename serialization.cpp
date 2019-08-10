#include "account.hpp"
#include "ogame.hpp"
#include "json.hpp"

#include <algorithm>
#include <string>

using json = nlohmann::json;

namespace ogh = ogamehelpers;

namespace ogamehelpers{

    void to_json(nlohmann::json& j, const Resources& r){
        j = json{{"metal", r.met}, 
                {"crystal", r.crystal},
                {"deuterium", r.deut}};
    }

    void from_json(const nlohmann::json& j, Resources& r){
        j.at("metal").get_to(r.met);
        j.at("crystal").get_to(r.crystal);
        j.at("deuterium").get_to(r.deut);
    }

    void to_json(nlohmann::json& j, const Production& p){
        j = json{{"metal", p.met}, 
                {"crystal", p.crystal},
                {"deuterium", p.deut}};
    }

    void from_json(const nlohmann::json& j, Production& p){
        j.at("metal").get_to(p.met);
        j.at("crystal").get_to(p.crystal);
        j.at("deuterium").get_to(p.deut);
    }

}

void to_json(json& j, const PlanetState& p){
    j = json{{"planetId", p.planetId}, 
            {"metLevel", p.metLevel},
            {"crysLevel", p.crysLevel},
            {"deutLevel", p.deutLevel},
            {"solarLevel", p.solarLevel},
            {"fusionLevel", p.fusionLevel},
            {"labLevel", p.labLevel},
            {"roboLevel", p.roboLevel},
            {"naniteLevel", p.naniteLevel},
            {"shipyardLevel", p.shipyardLevel},
            {"metalStorageLevel", p.metalStorageLevel},
            {"crystalStorageLevel", p.crystalStorageLevel},
            {"deutStorageLevel", p.deutStorageLevel},
            {"allianceDepotLevel", p.allianceDepotLevel},
            {"missileSiloLevel", p.missileSiloLevel},
            {"temperature", p.temperature},
            {"metPercent", p.metPercent},
            {"crysPercent", p.crysPercent},
            {"deutPercent", p.deutPercent},
            {"fusionPercent", p.fusionPercent},
            {"metItem", ogh::itemRarityToName(p.metItem)},
            {"crysItem", ogh::itemRarityToName(p.crysItem)},
            {"deutItem", ogh::itemRarityToName(p.deutItem)},
            {"metItemDurationDays", p.metItemDurationDays},
            {"crysItemDurationDays", p.crysItemDurationDays},
            {"deutItemDurationDays", p.deutItemDurationDays},
            {"sats", p.sats},
            {"buildingQueueDays", p.buildingQueue},
            {"inQueue", p.entityInfoInQueue.name}};
}

void from_json(const json& j, PlanetState& p){
    j.at("planetId").get_to(p.planetId);
    j.at("metLevel").get_to(p.metLevel);
    j.at("crysLevel").get_to(p.crysLevel);
    j.at("deutLevel").get_to(p.deutLevel);
    j.at("solarLevel").get_to(p.solarLevel);
    j.at("fusionLevel").get_to(p.fusionLevel);
    j.at("labLevel").get_to(p.labLevel);
    j.at("roboLevel").get_to(p.roboLevel);
    j.at("naniteLevel").get_to(p.naniteLevel);
    j.at("shipyardLevel").get_to(p.shipyardLevel);
    j.at("metalStorageLevel").get_to(p.metalStorageLevel);
    j.at("crystalStorageLevel").get_to(p.crystalStorageLevel);
    j.at("deutStorageLevel").get_to(p.deutStorageLevel);
    j.at("allianceDepotLevel").get_to(p.allianceDepotLevel);
    j.at("missileSiloLevel").get_to(p.missileSiloLevel);
    j.at("temperature").get_to(p.temperature);
    j.at("metPercent").get_to(p.metPercent);
    j.at("crysPercent").get_to(p.crysPercent);
    j.at("deutPercent").get_to(p.deutPercent);
    j.at("fusionPercent").get_to(p.fusionPercent);
    p.metItem = ogh::parseItemRarityName(j.at("metItem"));
    p.crysItem = ogh::parseItemRarityName(j.at("crysItem"));
    p.deutItem = ogh::parseItemRarityName(j.at("deutItem"));
    j.at("metItemDurationDays").get_to(p.metItemDurationDays);
    j.at("crysItemDurationDays").get_to(p.crysItemDurationDays);
    j.at("deutItemDurationDays").get_to(p.deutItemDurationDays);
    j.at("sats").get_to(p.sats);
    j.at("buildingQueueDays").get_to(p.buildingQueue);
    p.entityInfoInQueue = ogh::parseEntityName(j.at("inQueue"));
}


void to_json(json& j, const ResearchState& s){

    j = json{{"espionageLevel", s.espionageLevel},
            {"computerLevel", s.computerLevel},
            {"weaponsLevel", s.weaponsLevel},
            {"shieldingLevel", s.shieldingLevel},
            {"armourLevel", s.armourLevel},
            {"etechLevel", s.etechLevel},
            {"hyperspacetechLevel", s.hyperspacetechLevel},
            {"combustionLevel", s.combustionLevel},
            {"impulseLevel", s.impulseLevel},
            {"hyperspacedriveLevel", s.hyperspacedriveLevel},
            {"laserLevel", s.laserLevel},
            {"ionLevel", s.ionLevel},
            {"plasmaLevel", s.plasmaLevel},
            {"igrnLevel", s.igrnLevel},
            {"astroLevel", s.astroLevel},
            {"researchQueueDays", s.researchQueue},
            {"inQueue", s.entityInfoInQueue.name}};
}

void from_json(const nlohmann::json& j, ResearchState& s){
    j.at("espionageLevel").get_to(s.espionageLevel);
    j.at("computerLevel").get_to(s.computerLevel);
    j.at("weaponsLevel").get_to(s.weaponsLevel);
    j.at("shieldingLevel").get_to(s.shieldingLevel);
    j.at("armourLevel").get_to(s.armourLevel);
    j.at("etechLevel").get_to(s.etechLevel);
    j.at("hyperspacetechLevel").get_to(s.hyperspacetechLevel);
    j.at("combustionLevel").get_to(s.combustionLevel);
    j.at("impulseLevel").get_to(s.impulseLevel);
    j.at("hyperspacedriveLevel").get_to(s.hyperspacedriveLevel);
    j.at("laserLevel").get_to(s.laserLevel);
    j.at("ionLevel").get_to(s.ionLevel);
    j.at("plasmaLevel").get_to(s.plasmaLevel);
    j.at("igrnLevel").get_to(s.igrnLevel);
    j.at("astroLevel").get_to(s.astroLevel);
    j.at("researchQueueDays").get_to(s.researchQueue);
    s.entityInfoInQueue = ogh::parseEntityName(j.at("inQueue"));
}

void to_json(nlohmann::json& j, const OfficerState& s){
    j = json{{"commanderDurationDays", s.commanderDurationDays}, 
            {"engineerDurationDays", s.engineerDurationDays},
            {"technocratDurationDays", s.technocratDurationDays},
            {"geologistDurationDays", s.geologistDurationDays},
            {"admiralDurationDays", s.admiralDurationDays}};
}

void from_json(const nlohmann::json& j, OfficerState& s){
    j.at("commanderDurationDays").get_to(s.commanderDurationDays);
    j.at("engineerDurationDays").get_to(s.engineerDurationDays);
    j.at("technocratDurationDays").get_to(s.technocratDurationDays);
    j.at("geologistDurationDays").get_to(s.geologistDurationDays);
    j.at("admiralDurationDays").get_to(s.admiralDurationDays);
}

void to_json(nlohmann::json& j, const Account& a){
    j = json{{"planets", a.planetStates}, 
            {"research", a.researchState},
            {"officers", a.officerState},
            {"resources", a.resources},
            {"dailyFarmIncome", a.dailyFarmIncome},
            {"dailyExpeditionIncome", a.dailyExpeditionIncome},
            {"traderate", a.traderate},
            {"ecospeed", a.speedfactor},
            {"numPlanets", a.planetStates.size()},
            {"planetType", "individual"}};
}

void from_json(const nlohmann::json& j, Account& a){
    j.at("planets").get_to(a.planetStates);
    j.at("research").get_to(a.researchState);
    j.at("officers").get_to(a.officerState);
    j.at("resources").get_to(a.resources);
    j.at("dailyFarmIncome").get_to(a.dailyFarmIncome);
    j.at("dailyExpeditionIncome").get_to(a.dailyExpeditionIncome);
    j.at("traderate").get_to(a.traderate);
    j.at("ecospeed").get_to(a.speedfactor);

    int numPlanets = j.at("numPlanets");
    std::string planetType = j.at("planetType");

    if(numPlanets == 0 || a.planetStates.size() == 0){
        throw std::runtime_error("Invalid account json data. Accounts must have at least 1 planet. Abort.");
    }

    if(numPlanets != int(a.planetStates.size())){
        if(planetType == "individual"){
            throw std::runtime_error("Invalid account json data. Abort.");
        }else if(planetType == "identical"){
            a.planetStates.resize(numPlanets);
            std::fill(a.planetStates.begin()+1, a.planetStates.end(), a.planetStates[0]);
		    for(int i = 1; i < numPlanets; i++){
			    a.planetStates[i].planetId = i+1;
		    }
        }else{
            throw std::runtime_error("Invalid account json data. planetType must be either \"individual\" or \"identical\"");
        }
    }

    auto fixPointers = [&](auto& p){
        p.researchStatePtr = &a.researchState;
		p.officerStatePtr = &a.officerState;
		p.accountPtr = &a;
    };

    std::for_each(a.planetStates.begin(), a.planetStates.end(), fixPointers);
}

