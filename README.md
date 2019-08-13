# accountsimulator
WORK IN PROGRESS

The account simulator is capable of iteratively simulating a given upgrade plan for a given account while correctly accounting for save time (how long until enough resources are available for build),
blocked time (wait for blocked build queue / research queue), and construction time. 

Additionally, the account simulator can find the optimal permutation of an upgrade plan, using a brute-force approach which simply simulates all unique permutations of an upgrade plan.
Here, the optimal permutation can be optimal with respect to either:
    shortest completion time,
    shortest time until last upgrade is started,
    shortest save time, or
    shortest blocked time


# Restrictions / Limitations / ...
  - No moons
  - No ships / def
  - Cannot build things which require energy
  - All account resources are instantly available at every planet at any time, i.e. no transport delay
  - Trades withing save time calculations are unlimited. No storage restrictions. No DM costs. Those trades happen instantly ,i.e. no transport delay.
  - Resource production is not limited by storage capacity.
  - Everything else I forgot to mention.

# How to use
The program requires an account file describing the account (buildings, researches,...), and an upgrade file describing the upgrades to be performed.
In its simplest form, the program can be called with `./accountupgrade --accountfile account.json --upgradefile upgrades.txt`

A list of options is available when using the commandline argument `./accountupgrade --help`, or by running the program without any arguments.

### Account file
The account file must be given in json format. A default account file 'file.json' can be created via `./accountupgrade --newAccount file.json`
Planets with their respective buildings are described by the entries numPlanets, planetType and planets. 
numPlanets is the number of planets of the account.
planetType must be either "identical" or "individual".
If planetType is "identical" the first planet entry in planets is used for each of the numPlanets planets of the account.
If planetType is "individual", the number of planet entries in planets must equal numPlanets.

Production booster are identified by their rarity. 

##### Valid item rarity values 
```
"bronze", "silver", "gold", "none".
```

Fleet income is specified by entries saveslots, dailyExpeditionIncomePerSlot, and dailyFarmIncomePerSlot.
saveslots are substracted from the total number of slots available in the account. Afterwards, the maximum possible number of expedition slots is used for expeditions.
Each expedition slot produces dailyExpeditionIncomePerSlot per day. The remaining slots are used for farming. Each farm slot produces dailyFarmIncomePerSlot per day.

It is possible to specify ongoing constructions and researches. This is done by setting inQueue to the respective name, and buildingQueueDays / researchQueueDays to the remaining time in fractional days.
##### Valid building names 
```
"Metal Mine", "Crystal Mine", "Deuterium Synthesizer", "Solar Plant", "Fusion Reactor", "Robotics Factory",
"Nanite Factory", "Shipyard", "Metal Storage", "Crystal Storage", "Deuterium Tank", "Research Lab", "Terraformer",
"Alliance Depot", "Space Dock", "Missile Silo", "none"
```

##### Valid research names
```
"Espionage Technology", "Computer Technology", "Weapons Technology", "Shielding Technology", "Armor Technology",
"Energy Technology", "Hyperspace Technology", "Combustion Drive", "Impulse Drive", "Hyperspace Drive",
"Laser Technology", "Ion Technology", "Plasma Technology", "Intergalactic Research Network", "Astrophysics",
"Graviton Technology", "none"
```

### Upgrade file
A single upgrade is given by zero or more planet specifiers separated by whitespace, followed by an upgrade name.
If no planet specifier is given, the upgrade is performed on every planet. The following example upgrades metal mine on every planet. Afterwards, deut synth is build on the second planet.
```
met
2 deut
```

It is also possible to place multiple single upgrades in one line. Upgrades are performed from left to right within a line.
```
met 2 deut
```

multiple consecutive single upgrades with the same planet specifier in the same line can be interleaved by surrounding the respective upgrades by `(.` and `.)`. (. indicates whitespace).
For example, 

`1 2 met 1 2 deut` is equivalent to `1 met 2 met 3 met 1 deut 2 deut 3 deut`, 
whereas
`( 1 2 met 1 2 deut )` is equivalent to `1 met 1 deut 2 met 2 deut 3 met 3 deut`.

When calculating the optimal permutation, only permutations of lines are regarded.



