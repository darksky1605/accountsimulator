#include "account.hpp"
#include "ogame.hpp"
#include "json.hpp"

namespace ogamehelpers{

    void to_json(nlohmann::json& j, const Resources& r);
    void from_json(const nlohmann::json& j, Resources& r);

    void to_json(nlohmann::json& j, const Production& p);
    void from_json(const nlohmann::json& j, Production& p);

}

void to_json(nlohmann::json& j, const PlanetState& s);
void from_json(const nlohmann::json& j, PlanetState& s);

void to_json(nlohmann::json& j, const ResearchState& s);
void from_json(const nlohmann::json& j, ResearchState& s);

void to_json(nlohmann::json& j, const OfficerState& s);
void from_json(const nlohmann::json& j, OfficerState& s);

void to_json(nlohmann::json& j, const Account& s);
void from_json(const nlohmann::json& j, Account& s);
