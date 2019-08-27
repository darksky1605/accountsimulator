#ifndef IO_HPP
#define IO_HPP

#include "account.hpp"
#include "ogame.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct UpgradeJob {
    int location;
    ogamehelpers::EntityInfo entityInfo;

    UpgradeJob() = default;
    UpgradeJob(int l, ogamehelpers::EntityInfo e)
        : location(l), entityInfo(e) {}

    bool isResearch() const {
        return entityInfo.type == ogamehelpers::EntityType::Research;
    }

    bool isBuilding() const {
        return entityInfo.type == ogamehelpers::EntityType::Building;
    }

    bool operator==(const UpgradeJob& rhs) const {
        return location == rhs.location && entityInfo == rhs.entityInfo;
    }

    bool operator!=(const UpgradeJob& rhs) const {
        return !(operator==(rhs));
    }

    bool operator<(const UpgradeJob& rhs) const {
        if (location > rhs.location)
            return false;
        if (location < rhs.location)
            return true;
        return entityInfo.name < rhs.entityInfo.name;
    }
};

struct UpgradeTask {
    static int researchLocation;
    static int allCurrentPlanetsLocation;

    ogamehelpers::EntityInfo entityInfo{};
    std::vector<int> locations{};

    UpgradeTask() {}
    UpgradeTask(const ogamehelpers::EntityInfo e, const std::vector<int>& l) : entityInfo(e), locations(l) {}

    bool isResearch() const {
        return entityInfo.type == ogamehelpers::EntityType::Research;
    }

    const std::vector<int>& getLocations() const {
        return locations;
    }

    std::vector<UpgradeJob> getUpgradeJobs(int numPlanets) const {
        std::vector<UpgradeJob> result;
        if (locations.empty())
            return {};

        auto makeJob = [&](auto location) {
            result.emplace_back(location, entityInfo);
        };

        if (locations[0] == allCurrentPlanetsLocation) {
            for (int planetNumber = 1; planetNumber <= numPlanets; planetNumber++) {
                makeJob(planetNumber);
            }
        } else {
            std::for_each(locations.begin(), locations.end(), makeJob);
        }

        return result;
    }
};

struct UpgradeGroup {
    bool transposed = false;
    std::vector<UpgradeTask> tasks;

    UpgradeGroup() {}
    UpgradeGroup(bool t, const std::vector<UpgradeTask>& l) : transposed(t), tasks(l) {}

    bool isTransposed() const {
        return transposed;
    }

    bool valid() const {
        if (tasks.empty())
            return true;

        auto equalsfirstloc = [&](const auto& t) {
            return tasks[0].getLocations() == t.getLocations();
        };

        if (isTransposed()) {
            return std::all_of(tasks.begin() + 1, tasks.end(), equalsfirstloc);
        } else {
            return true;
        }
    }

    const std::vector<UpgradeTask>& getTasks() const {
        assert(valid());
        return tasks;
    }

    std::vector<UpgradeTask> getTasks(int numPlanets) const {
        assert(valid());

        if (tasks.empty() || tasks[0].getLocations().empty()) {
            return {};
        }

        if (isTransposed()) {
            std::vector<std::size_t> posInTask(tasks.size(), 0);

            if (tasks[0].getLocations()[0] == UpgradeTask::allCurrentPlanetsLocation) {
                std::vector<UpgradeTask> result;
                
                for (int planetNumber = 1; planetNumber <= numPlanets; planetNumber++) {
                    for (const auto& task : tasks) {
                        result.emplace_back(task);
                        result.back().locations = std::vector<int>{planetNumber};
                    }
                }
                return result;
            } else {
                std::vector<UpgradeTask> result;
                int maxPos = int(tasks[0].getLocations().size());
                for (int pos = 0; pos < maxPos; pos++) {
                    for (const auto& task : tasks) {
                        result.emplace_back(task);
                        result.back().locations = std::vector<int>{task.getLocations()[pos]};
                    }
                }
                return result;
            }
        } else {
            if (tasks[0].getLocations()[0] == UpgradeTask::allCurrentPlanetsLocation) {
                std::vector<UpgradeTask> result;
                for (const auto& task : tasks) {
                    for (int planetNumber = 1; planetNumber <= numPlanets; planetNumber++) {
                        result.emplace_back(task);
                        result.back().locations = std::vector<int>{planetNumber};
                    }
                }
                return result;
            } else {
                return tasks;
            }
        }
    }
};

struct PermutationGroup {
    std::vector<UpgradeGroup> groups;

    PermutationGroup() {}
    PermutationGroup(const std::vector<UpgradeGroup>& g) : groups(g) {}
};

std::ostream& operator<<(std::ostream& os, const UpgradeTask& r);
std::ostream& operator<<(std::ostream& os, const UpgradeGroup& r);
std::ostream& operator<<(std::ostream& os, const PermutationGroup& r);

bool operator==(const UpgradeTask& l, const UpgradeTask& r);
bool operator==(const UpgradeGroup& l, const UpgradeGroup& r);
bool operator==(const PermutationGroup& l, const PermutationGroup& r);

bool operator!=(const UpgradeTask& l, const UpgradeTask& r);
bool operator!=(const UpgradeGroup& l, const UpgradeGroup& r);
bool operator!=(const PermutationGroup& l, const PermutationGroup& r);

bool operator<(const UpgradeTask& l, const UpgradeTask& r);
bool operator<(const UpgradeGroup& l, const UpgradeGroup& r);
bool operator<(const PermutationGroup& l, const PermutationGroup& r);

Account parseAccountJsonFile(const std::string& filename);

std::vector<PermutationGroup> parseUpgradeFile(const std::string& filename);

#endif
