#include "account.hpp"

#include "json.hpp"

void to_json(nlohmann::json& j, const PlanetState& s);
void from_json(const nlohmann::json& j, PlanetState& s);

void to_json(nlohmann::json& j, const ResearchState& s);
void from_json(const nlohmann::json& j, ResearchState& s);

void to_json(nlohmann::json& j, const OfficerState& s);
void from_json(const nlohmann::json& j, OfficerState& s);

void to_json(nlohmann::json& j, const Account& s);
void from_json(const nlohmann::json& j, Account& s);
