// Game Management System - Complex Data Structures
// Demonstrates RPG-style game data with players, guilds, and quests

println("=== Fantasy RPG Management System ===");

// Game world with guilds, players, and quests
let gameWorld = {
    name: "Cyperia Online",
    version: "2.1.4",
    maxPlayers: 10000,
    guilds: [
        {
            name: "Dragon Slayers",
            level: 15,
            members: [
                {
                    name: "Aragorn",
                    class: "Warrior",
                    level: 45,
                    stats: { hp: 850, mp: 200, attack: 120, defense: 95 },
                    inventory: [
                        { item: "Dragon Sword", type: "weapon", value: 500 },
                        { item: "Steel Armor", type: "armor", value: 300 },
                        { item: "Health Potion", type: "consumable", value: 50 }
                    ],
                    achievements: ["Dragon Slayer", "Guild Master", "Legendary Warrior"]
                },
                {
                    name: "Legolas",
                    class: "Archer",
                    level: 42,
                    stats: { hp: 650, mp: 400, attack: 140, defense: 70 },
                    inventory: [
                        { item: "Elven Bow", type: "weapon", value: 450 },
                        { item: "Leather Armor", type: "armor", value: 200 },
                        { item: "Magic Arrow", type: "consumable", value: 25 }
                    ],
                    achievements: ["Master Archer", "Eagle Eye", "Forest Guardian"]
                }
            ],
            treasury: 15000,
            reputation: "Legendary"
        },
        {
            name: "Mystic Scholars",
            level: 12,
            members: [
                {
                    name: "Gandalf",
                    class: "Wizard",
                    level: 50,
                    stats: { hp: 500, mp: 900, attack: 160, defense: 60 },
                    inventory: [
                        { item: "Staff of Power", type: "weapon", value: 800 },
                        { item: "Wizard Robes", type: "armor", value: 250 },
                        { item: "Mana Crystal", type: "consumable", value: 100 }
                    ],
                    achievements: ["Archmage", "Spell Master", "Ancient Knowledge"]
                }
            ],
            treasury: 8500,
            reputation: "Respected"
        }
    ],
    activeQuests: [
        {
            id: "Q001",
            title: "The Ancient Dragon",
            difficulty: "Legendary",
            rewards: { gold: 5000, experience: 10000, items: ["Dragon Scale", "Ancient Gem"] },
            requirements: { minLevel: 40, minPlayers: 3 },
            assignedGuild: "Dragon Slayers",
            status: "in_progress"
        },
        {
            id: "Q002",
            title: "Lost Spell Tome",
            difficulty: "Epic",
            rewards: { gold: 2500, experience: 5000, items: ["Spell Scroll", "Magic Essence"] },
            requirements: { minLevel: 35, minPlayers: 1 },
            assignedGuild: "Mystic Scholars",
            status: "completed"
        }
    ]
};

// Display game world information
print("Game: ");
println(gameWorld.name);
print("Version: ");
println(gameWorld.version);
print("Max Players: ");
println(gameWorld.maxPlayers);
print("Active Guilds: ");
println(gameWorld.guilds.length);

println("");
println("=== Guild Analysis ===");

// Dragon Slayers Guild
let dragonSlayers = gameWorld.guilds[0];
print("Guild: ");
println(dragonSlayers.name);
print("Level: ");
println(dragonSlayers.level);
print("Members: ");
println(dragonSlayers.members.length);
print("Treasury: ");
print(dragonSlayers.treasury);
println(" gold");
print("Reputation: ");
println(dragonSlayers.reputation);

println("Guild Members:");

// Analyze Aragorn
let aragorn = dragonSlayers.members[0];
print("  Leader: ");
println(aragorn.name);
print("  Class: ");
println(aragorn.class);
print("  Level: ");
println(aragorn.level);
print("  HP: ");
println(aragorn.stats.hp);
print("  Attack: ");
println(aragorn.stats.attack);
print("  Main Weapon: ");
println(aragorn.inventory[0].item);
print("  Weapon Value: ");
print(aragorn.inventory[0].value);
println(" gold");
print("  Top Achievement: ");
println(aragorn.achievements[0]);

// Analyze Legolas
let legolas = dragonSlayers.members[1];
print("  Member: ");
println(legolas.name);
print("  Class: ");
println(legolas.class);
print("  Level: ");
println(legolas.level);
print("  MP: ");
println(legolas.stats.mp);
print("  Defense: ");
println(legolas.stats.defense);
print("  Specialty: ");
println(legolas.achievements[1]);

// Mystic Scholars Guild
let mysticScholars = gameWorld.guilds[1];
println("");
print("Guild: ");
println(mysticScholars.name);
print("Level: ");
println(mysticScholars.level);
print("Treasury: ");
print(mysticScholars.treasury);
println(" gold");

let gandalf = mysticScholars.members[0];
print("  Archmage: ");
println(gandalf.name);
print("  Level: ");
println(gandalf.level);
print("  Mana: ");
println(gandalf.stats.mp);
print("  Magical Attack: ");
println(gandalf.stats.attack);
print("  Staff: ");
println(gandalf.inventory[0].item);

println("");
println("=== Quest Management ===");

// Active quests analysis
print("Total Active Quests: ");
println(gameWorld.activeQuests.length);

// Dragon quest
let dragonQuest = gameWorld.activeQuests[0];
print("Quest: ");
println(dragonQuest.title);
print("Difficulty: ");
println(dragonQuest.difficulty);
print("Gold Reward: ");
print(dragonQuest.rewards.gold);
println(" gold");
print("XP Reward: ");
print(dragonQuest.rewards.experience);
println(" XP");
print("Assigned to: ");
println(dragonQuest.assignedGuild);
print("Status: ");
println(dragonQuest.status);
print("Min Level Required: ");
println(dragonQuest.requirements.minLevel);
print("Special Reward: ");
println(dragonQuest.rewards.items[0]);

// Spell tome quest
let spellQuest = gameWorld.activeQuests[1];
print("Quest: ");
println(spellQuest.title);
print("Difficulty: ");
println(spellQuest.difficulty);
print("Status: ");
println(spellQuest.status);
if (spellQuest.status == "completed") {
    println("✓ Quest completed successfully!");
    print("Reward claimed: ");
    println(spellQuest.rewards.items[1]);
}

println("");
println("=== Player Statistics ===");

// Calculate total player levels
let totalLevels: i32 = 0;
let totalPlayers: i32 = 0;

// Dragon Slayers stats
totalLevels = totalLevels + aragorn.level + legolas.level;
totalPlayers = totalPlayers + dragonSlayers.members.length;

// Mystic Scholars stats
totalLevels = totalLevels + gandalf.level;
totalPlayers = totalPlayers + mysticScholars.members.length;

print("Total Players: ");
println(totalPlayers);
print("Combined Levels: ");
println(totalLevels);
print("Average Level: ");
println(totalLevels / totalPlayers);

// Inventory analysis
println("Legendary Items Found:");
if (aragorn.inventory[0].value >= 500) {
    print("- ");
    print(aragorn.inventory[0].item);
    print(" (");
    print(aragorn.inventory[0].value);
    println(" gold)");
}
if (gandalf.inventory[0].value >= 500) {
    print("- ");
    print(gandalf.inventory[0].item);
    print(" (");
    print(gandalf.inventory[0].value);
    println(" gold)");
}

println("");
println("=== Combat Power Analysis ===");

// Calculate guild combat power
let dragonSlayersPower: i32 = aragorn.stats.attack + legolas.stats.attack;
let mysticScholarsPower: i32 = gandalf.stats.attack;

print("Dragon Slayers Combat Power: ");
println(dragonSlayersPower);
print("Mystic Scholars Combat Power: ");
println(mysticScholarsPower);

if (dragonSlayersPower > mysticScholarsPower) {
    println("Dragon Slayers have superior combat strength!");
} else {
    println("Mystic Scholars have superior magical power!");
}

println("");
println("=== Achievement Summary ===");
print("Total Achievements Unlocked: ");
let totalAchievements: i32 = aragorn.achievements.length + legolas.achievements.length + gandalf.achievements.length;
println(totalAchievements);

println("Notable Achievements:");
print("- ");
println(aragorn.achievements[2]);
print("- ");
println(gandalf.achievements[0]);
print("- ");
println(legolas.achievements[0]);

println("");
println("=== System Status ===");
println("✓ Guild management operational");
println("✓ Player progression tracking active");
println("✓ Quest system functioning");
println("✓ Inventory management working");
println("✓ Achievement system enabled");

println("");
println("Fantasy RPG system successfully managing complex game data!");
println("Cyperia Online is ready for adventure!");
