# Accountsimulator
WORK IN PROGRESS / NO SUPPORT

The account simulator is capable of iteratively simulating a given upgrade plan for a given account while correctly accounting for save time (how long until enough resources are available for build),
blocked time (wait for blocked build queue / research queue), and construction time. 

Additionally, the account simulator can find the optimal permutation of an upgrade plan, using a brute-force approach which simply simulates all unique permutations of an upgrade plan.
Here, the optimal permutation can be optimal with respect to either:
    shortest completion time,
    shortest time until last upgrade is started,
    shortest save time, or
    shortest blocked time

Tested on Ubuntu and on Windows Subsystem for Linux in Windows 10.

# Restrictions / Limitations / ...
  - No moons
  - No ships / def
  - Cannot build things which require energy
  - All account resources are instantly available at every planet at any time, i.e. no transport delay
  - Trades withing save time calculations are unlimited. No storage restrictions. No DM costs. Those trades happen instantly ,i.e. no transport delay.
  - Resource production is not limited by storage capacity.
  - No requirements checks
  - Everything else I forgot to mention.

# How to build
Requires GNU Make and a g++ with support of c++14.
Run `make accountupgrade`.

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

Times can be specified in seconds. Alternatively, the format days:hours:minutes:seconds can be used.

##### Valid item rarity values 
```
"bronze", "silver", "gold", "platinum", "none".
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

Multiple consecutive single upgrades with the same planet specifier in the same line can be interleaved by surrounding the respective upgrades by `(.` and `.)`. (. indicates whitespace).
For example, 

`1 2 3 met 1 2 3 deut` is equivalent to `1 met 2 met 3 met 1 deut 2 deut 3 deut`,  
whereas
`( 1 2 3 met 1 2 3 deut )` is equivalent to `1 met 1 deut 2 met 2 deut 3 met 3 deut`.

Important: When calculating the optimal permutation, only permutations of lines are considered!

Here is another example which uses multiple lines, interleaved upgrades and multiple upgrades per line. 

```
met
( crys fusionplant ) plasma
1 robo 1 nani 1 2 met
```

For an account with two planets, this would be equivalent to 

```
1 met 2 met 1 crys 1 fusionplant 2 crys 2 fusionplant plasma 1 robo 1 nani 1 met 2 met
```


##### Valid upgrade names
```
"metalmine", "met", "crystalmine", "kris", "crys", "deutsynth", "deut", "solarplant", "skw", 
"fusionplant", "fkw", "robofactory", "robo", "nanitefactory", "nani", "researchlab", "lab", 
"shipyard", "werft", "metalstorage", "mstorage", "crystalstorage", "cstorage", "deutstorage", "dstorage", 
"alliancedepot", "allydepot", "missilesilo", "silo", "energytech", "etech", "plasmatech", "plasma", 
"astrophysics", "astro", "researchnetwork", "igfn", "igrn", "computertech", "comp", "espionagetech", "spiotech", 
"weaponstech", "waffen", "shieldingtech", "schild", "armourtech", "panzer", "hyperspacetech", "hypertech", 
"combustiondrive", "verbrenner", "impulsedrive", "impuls", "hyperspacedrive", "hyperantrieb", 
"lasertech", "laser", "iontech", "ion", "none"
```
Upgrade names are not case-sensitive.


# Example output
### Default
```
The selected upgrades take 123:02:49:44 days:hours:minutes:seconds.
Last upgrade started after 118:09:04:13 days:hours:minutes:seconds.
The required saving time is 118:09:04:13 days:hours:minutes:seconds.
Days lost because a queue was full: 0:00:00:00 days:hours:minutes:seconds.
```

### With --percentages

```
17:10:06:14 Planet 11: Changed percents from m 100, c 100, d 100, f 100 to m 100, c 100, d 100, f 80 after construction of Fusion Reactor 21. Production factor: 0.975287->1. Production increased by 2.82854 % DSE
18:20:28:21 Planet 12: Changed percents from m 100, c 100, d 100, f 100 to m 100, c 100, d 100, f 80 after construction of Fusion Reactor 21. Production factor: 0.975287->1. Production increased by 2.82854 % DSE
19:03:56:03 Planet 13: Changed percents from m 100, c 100, d 100, f 100 to m 100, c 100, d 100, f 80 after construction of Fusion Reactor 21. Production factor: 0.975287->1. Production increased by 2.82854 % DSE
29:03:49:44 Planet 1: Changed percents from m 100, c 100, d 100, f 80 to m 100, c 100, d 100, f 90 after construction of Metal Mine 41. Production factor: 1->1. Production increased by 5.33287 % DSE
34:07:19:37 Planet 2: Changed percents from m 100, c 100, d 100, f 80 to m 100, c 100, d 100, f 90 after construction of Metal Mine 41. Production factor: 1->1. Production increased by 5.33287 % DSE
39:10:19:23 Planet 3: Changed percents from m 100, c 100, d 100, f 80 to m 100, c 100, d 100, f 90 after construction of Metal Mine 41. Production factor: 1->1. Production increased by 5.33287 % DSE
```

### With --printlist
```
Planet 11: Fusion Reactor 21. Saving period begin: 14:13:08:51, Waiting period begin: 15:23:39:45, Construction begin: 15:23:39:45, Construction time: 1:17:49:49, Save time: 1:10:30:54
Planet 12: Fusion Reactor 21. Saving period begin: 15:23:39:45, Waiting period begin: 17:10:06:14, Construction begin: 17:10:06:14, Construction time: 1:17:49:49, Save time: 1:10:26:29
Planet 13: Fusion Reactor 21. Saving period begin: 17:10:06:14, Waiting period begin: 18:20:28:21, Construction begin: 18:20:28:21, Construction time: 1:17:49:49, Save time: 1:10:22:07
Planet 1: Metal Mine 41. Saving period begin: 18:20:28:21, Waiting period begin: 24:00:12:10, Construction begin: 24:00:12:10, Construction time: 8:23:57:49, Save time: 5:03:43:49
Planet 2: Metal Mine 41. Saving period begin: 24:00:12:10, Waiting period begin: 29:03:49:44, Construction begin: 29:03:49:44, Construction time: 8:23:57:49, Save time: 5:03:37:34
Planet 3: Metal Mine 41. Saving period begin: 29:03:49:44, Waiting period begin: 34:07:19:37, Construction begin: 34:07:19:37, Construction time: 8:23:57:49, Save time: 5:03:29:53
```
