#include "account.hpp"
#include "json.hpp"
#include "ogame.hpp"
#include "serialization.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <string>
#include <sstream>

using json = nlohmann::json;

namespace ogh = ogamehelpers;

namespace ogamehelpers {

void to_json(nlohmann::json& j, const Resources& r) {
    j = json{{"metal", r.metal()},
             {"crystal", r.crystal()},
             {"deuterium", r.deuterium()}};
}

void from_json(const nlohmann::json& j, Resources& r) {
    std::int64_t m,k,d;
    j.at("metal").get_to(m);
    j.at("crystal").get_to(k);
    j.at("deuterium").get_to(d);
    r = Resources(m,k,d);
}

void to_json(nlohmann::json& j, const Production& p) {
    j = json{{"metal", p.metal()},
             {"crystal", p.crystal()},
             {"deuterium", p.deuterium()}};
}

void from_json(const nlohmann::json& j, Production& p) {
    std::int64_t m,k,d;
    j.at("metal").get_to(m);
    j.at("crystal").get_to(k);
    j.at("deuterium").get_to(d);
    p = Production::makeProductionPerHour(m,k,d);
}

} // namespace ogamehelpers

void to_json(json& j, const PlanetState& p) {
    j = json{{"planetId", p.planetId},
             {"metLevel", p.buildingLevels.metLevel},
             {"crysLevel", p.buildingLevels.crysLevel},
             {"deutLevel", p.buildingLevels.deutLevel},
             {"solarLevel", p.buildingLevels.solarLevel},
             {"fusionLevel", p.buildingLevels.fusionLevel},
             {"labLevel", p.buildingLevels.labLevel},
             {"roboLevel", p.buildingLevels.roboLevel},
             {"naniteLevel", p.buildingLevels.naniteLevel},
             {"shipyardLevel", p.buildingLevels.shipyardLevel},
             {"metalStorageLevel", p.buildingLevels.metalStorageLevel},
             {"crystalStorageLevel", p.buildingLevels.crystalStorageLevel},
             {"deutStorageLevel", p.buildingLevels.deutStorageLevel},
             {"allianceDepotLevel", p.buildingLevels.allianceDepotLevel},
             {"missileSiloLevel", p.buildingLevels.missileSiloLevel},
             {"temperature", p.temperature},
             {"metPercent", p.metPercent},
             {"crysPercent", p.crysPercent},
             {"deutPercent", p.deutPercent},
             {"fusionPercent", p.fusionPercent},
             {"metItem", ogh::itemRarityToName(p.metItem)},
             {"crysItem", ogh::itemRarityToName(p.crysItem)},
             {"deutItem", ogh::itemRarityToName(p.deutItem)},
             {"metItemDuration", secondsToDHM(p.metItemDuration)},
             {"crysItemDuration", secondsToDHM(p.crysItemDuration)},
             {"deutItemDuration", secondsToDHM(p.deutItemDuration)},
             {"sats", p.sats},
             {"crawler", p.crawler},
             {"crawlerPercent", p.crawlerPercent},
             {"buildingQueueDuration", secondsToDHM(p.buildingQueue)},
             {"inQueue", ogh::getEntityName(p.entityInQueue)}};
}

void from_json(const json& j, PlanetState& p) {
    j.at("planetId").get_to(p.planetId);
    j.at("metLevel").get_to(p.buildingLevels.metLevel);
    j.at("crysLevel").get_to(p.buildingLevels.crysLevel);
    j.at("deutLevel").get_to(p.buildingLevels.deutLevel);
    j.at("solarLevel").get_to(p.buildingLevels.solarLevel);
    j.at("fusionLevel").get_to(p.buildingLevels.fusionLevel);
    j.at("labLevel").get_to(p.buildingLevels.labLevel);
    j.at("roboLevel").get_to(p.buildingLevels.roboLevel);
    j.at("naniteLevel").get_to(p.buildingLevels.naniteLevel);
    j.at("shipyardLevel").get_to(p.buildingLevels.shipyardLevel);
    j.at("metalStorageLevel").get_to(p.buildingLevels.metalStorageLevel);
    j.at("crystalStorageLevel").get_to(p.buildingLevels.crystalStorageLevel);
    j.at("deutStorageLevel").get_to(p.buildingLevels.deutStorageLevel);
    j.at("allianceDepotLevel").get_to(p.buildingLevels.allianceDepotLevel);
    j.at("missileSiloLevel").get_to(p.buildingLevels.missileSiloLevel);
    j.at("temperature").get_to(p.temperature);
    j.at("metPercent").get_to(p.metPercent);
    j.at("crysPercent").get_to(p.crysPercent);
    j.at("deutPercent").get_to(p.deutPercent);
    j.at("fusionPercent").get_to(p.fusionPercent);
    p.metItem = ogh::parseItemRarityName(j.at("metItem"));
    p.crysItem = ogh::parseItemRarityName(j.at("crysItem"));
    p.deutItem = ogh::parseItemRarityName(j.at("deutItem"));
    p.metItemDuration = secondsFromDHM(j.at("metItemDuration"));
    p.crysItemDuration = secondsFromDHM(j.at("crysItemDuration"));
    p.deutItemDuration = secondsFromDHM(j.at("deutItemDuration"));
    p.buildingQueue = secondsFromDHM(j.at("buildingQueueDuration"));
    j.at("sats").get_to(p.sats);
    j.at("crawler").get_to(p.crawler);
    j.at("crawlerPercent").get_to(p.crawlerPercent);
    p.entityInQueue = ogh::parseEntityName(j.at("inQueue")).entity;
}

void to_json(json& j, const ResearchState& s) {

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
             {"researchQueueDuration", secondsToDHM(s.researchQueue)},
             {"inQueue", ogh::getEntityName(s.entityInQueue)}};
}

void from_json(const nlohmann::json& j, ResearchState& s) {
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
    s.researchQueue = secondsFromDHM(j.at("researchQueueDuration"));
    s.entityInQueue = ogh::parseEntityName(j.at("inQueue")).entity;
}

void to_json(nlohmann::json& j, const OfficerState& s) {
    j = json{{"commanderDuration", secondsToDHM(s.commanderDuration)},
             {"engineerDuration", secondsToDHM(s.engineerDuration)},
             {"technocratDuration", secondsToDHM(s.technocratDuration)},
             {"geologistDuration", secondsToDHM(s.geologistDuration)},
             {"admiralDuration", secondsToDHM(s.admiralDuration)}};
}

void from_json(const nlohmann::json& j, OfficerState& s) {
    s.commanderDuration = secondsFromDHM(j.at("commanderDuration"));
    s.engineerDuration = secondsFromDHM(j.at("engineerDuration"));
    s.technocratDuration = secondsFromDHM(j.at("technocratDuration"));
    s.geologistDuration = secondsFromDHM(j.at("geologistDuration"));
    s.admiralDuration = secondsFromDHM(j.at("admiralDuration"));
}

void to_json(nlohmann::json& j, const Account& a) {
    j = json{{"planets", a.planets},
             {"research", a.researches},
             {"officers", a.officers},
             {"resources", a.resources},
             {"dailyFarmIncomePerSlot", a.dailyFarmIncomePerSlot},
             {"dailyExpeditionIncomePerSlot", a.dailyExpeditionIncomePerSlot},
             {"saveslots", a.saveslots},
             {"traderate", a.traderate},
             {"ecospeed", a.speedfactor},
             {"characterClass", ogh::characterClassToName(a.characterClass)},
             {"numPlanets", a.planets.size()},
             {"planetType", "individual"},
             {"researchDurationDivisor", a.researchDurationDivisor}};
}

void from_json(const nlohmann::json& j, Account& a) {
    j.at("planets").get_to(a.planets);
    j.at("research").get_to(a.researches);
    j.at("officers").get_to(a.officers);
    j.at("resources").get_to(a.resources);
    j.at("dailyFarmIncomePerSlot").get_to(a.dailyFarmIncomePerSlot);
    j.at("dailyExpeditionIncomePerSlot").get_to(a.dailyExpeditionIncomePerSlot);
    j.at("saveslots").get_to(a.saveslots);
    j.at("traderate").get_to(a.traderate);
    j.at("ecospeed").get_to(a.speedfactor);
    j.at("researchDurationDivisor").get_to(a.researchDurationDivisor);
    a.characterClass = ogh::parseCharacterClassName(j.at("characterClass"));
    //std::cerr << ogh::characterClassToName(a.characterClass) << "\n";

    int numPlanets = j.at("numPlanets");
    std::string planetType = j.at("planetType");

    if (numPlanets == 0 || a.planets.size() == 0) {
        throw std::runtime_error("Invalid account json data. Accounts must have at least 1 planet. Abort.");
    }

    if (numPlanets != int(a.planets.size())) {
        if (planetType == "individual") {
            throw std::runtime_error("Invalid account json data. Abort.");
        } else if (planetType == "identical") {
            a.planets.resize(numPlanets);
            std::fill(a.planets.begin() + 1, a.planets.end(), a.planets[0]);
            for (int i = 1; i < numPlanets; i++) {
                a.planets[i].planetId = i + 1;
            }
        } else {
            throw std::runtime_error("Invalid account json data. planetType must be either \"individual\" or \"identical\"");
        }
    }

    auto fixPointers = [&](auto& p) {
        p.accountPtr = &a;
    };

    std::for_each(a.planets.begin(), a.planets.end(), fixPointers);

    a.updateDailyFarmIncome();
    a.updateDailyExpeditionIncome();
}


std::string secondsToDHM(std::chrono::seconds time){
    std::stringstream ss;
    auto days = std::chrono::duration_cast<std::chrono::hours>(time) / 24;
    time -= days * 24;
    auto hours = std::chrono::duration_cast<std::chrono::hours>(time);
    time -= hours;
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(time);
    time -= minutes;
    auto seconds = time;

    ss << days.count() << ":" << std::setfill('0') << std::setw(2) << hours.count() << ":" << std::setfill('0') << std::setw(2) << minutes.count() << ":" << std::setfill('0') << std::setw(2) << seconds.count();
    return ss.str();
}

std::chrono::seconds secondsFromDHM(const std::string& s){
    auto split = [](const std::string& str, char c){
        std::vector<std::string> result;

        std::stringstream ss(str);
        std::string s;

        while (std::getline(ss, s, c)) {
                result.emplace_back(s);
        }

        return result;
    };

    auto tokens = split(s, ':');
    if(tokens.size() > 4){
        return std::chrono::seconds::zero();
    }

    std::int64_t seconds = 0;
    std::int64_t factor = 1;
    for(int i = int(tokens.size()) - 1; i >= 0; i--){
        auto num = std::stoi(tokens[i]);
        seconds += num * factor;
        if(factor != 3600){
            factor *= 60;
        }else{
            factor *= 24;
        }
    }

    return std::chrono::seconds{seconds};
}
