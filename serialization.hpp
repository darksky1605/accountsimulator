#ifndef SERIALIZATION_HPP
#define SERIALIZATION_HPP

#include "account.hpp"
#include "json.hpp"
#include "ogame.hpp"

#include <chrono>
#include <string>

namespace ogamehelpers {

void to_json(nlohmann::json& j, const Resources& r);
void from_json(const nlohmann::json& j, Resources& r);

void to_json(nlohmann::json& j, const Production& p);
void from_json(const nlohmann::json& j, Production& p);

} // namespace ogamehelpers

void to_json(nlohmann::json& j, const PlanetState& s);
void from_json(const nlohmann::json& j, PlanetState& s);

void to_json(nlohmann::json& j, const ResearchState& s);
void from_json(const nlohmann::json& j, ResearchState& s);

void to_json(nlohmann::json& j, const OfficerState& s);
void from_json(const nlohmann::json& j, OfficerState& s);

void to_json(nlohmann::json& j, const Account& s);
void from_json(const nlohmann::json& j, Account& s);




std::string secondsToDHM(std::chrono::seconds s);
std::chrono::seconds secondsFromDHM(const std::string& s);

#endif
