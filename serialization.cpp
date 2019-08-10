#include "account.hpp"

#include "json.hpp"

using json = nlohmann::json;

namespace ogh = ogamehelpers;

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


void to_json(nlohmann::json& j, const ResearchState& s);
void from_json(const nlohmann::json& j, ResearchState& s);

void to_json(nlohmann::json& j, const OfficerState& s);
void from_json(const nlohmann::json& j, OfficerState& s);

void to_json(nlohmann::json& j, const Account& s);
void from_json(const nlohmann::json& j, Account& s);
