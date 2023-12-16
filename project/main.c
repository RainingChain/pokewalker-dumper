//(c) 2020 DmitryGR
//GPLv3 license

#include <VFSMgr.h>
#include <PalmOS.h>
#include "comms.h"
#include "res.h"


static const uint8_t add_watts_expoint_upload_to_0xF956[] = {
	0x56,
		
	0x79, 0x00, 0x27, 0xf,		//mov.w		#9999, r0
	0x5E, 0x00, 0x1f, 0x40,		//jsr		addWatts_word ( == addWatts + 2)
	
	0x79, 0x00, 0x08, 0xd6,		//mov.w		#&irHandleRxedByteIfAnyHasBeenRxed, r0
	
	0x5a, 0x00, 0x69, 0x3a,		//jmp		setProcToCallByMainInLoop
};


//set step count high too (optional)
static const char set_high_stepcount_exploit_upload_to_0xF79C[] = {
	0x9C,

	0x00, 0x01, 0x86, 0x9f
};


static const uint8_t rom_dump_exploit_upload_to_0xF956[] = {
	0x56,
	
	0x5E, 0x00, 0xBA, 0x42,		//jsr		common_prologue
	
	0x19, 0x55,					//sub.w		r5, r5
	
//lbl_big_loop:
	0x79, 0x06, 0xf8, 0xd6,		//mov.w		0xf8d6, r6
	
	0xfc, 0x80,					//mov.b		0x80, r4l
	
	0x7b, 0x5c, 0x59, 0x8f,		//eemov.b
	
	0x79, 0x00, 0xaa, 0x80,		//mov.w		#0xaa80, r0
	
	0x5e, 0x00, 0x07, 0x72,		//jsr		sendPacket
	
	0x5E, 0x00, 0x25, 0x9E,		//jsr		wdt_pet

	0x79, 0x25, 0xc0, 0x00,		//cmp.w		r5, #0xc000
	
	0x46, 0xe4,					//bne		$-0x1c		//lbl_big_loop
	
	0x79, 0x00, 0x08, 0xd6,		//mov.w		#&irHandleRxedByteIfAnyHasBeenRxed, r0
	
	0x5e, 0x00, 0x69, 0x3a,		//jsr		setProcToCallByMainInLoop
	
	0x5a, 0x00, 0xba, 0x62		//jmp		common_epilogue
};



static const uint8_t trigger_uploaded_code_upload_to_0xF7E0[] = {
	0xe0,

	0xf9, 0x56
};


//lengths set may not include zero terminators. watch out. FIRST ITEM MUST HAVE TERMINATORS AND NOT BE THE ONLY OR LIST DRAWING TO WORK
static const char mPokeList[][10] = {"None", "Bulbasaur", "Ivysaur", "Venusaur", "Charmander", "Charmeleon", "Charizard", "Squirtle", "Wartortle", "Blastoise", "Caterpie", "Metapod", "Butterfree", "Weedle", "Kakuna", "Beedrill", "Pidgey", "Pidgeotto", "Pidgeot", "Rattata", "Raticate", "Spearow", "Fearow", "Ekans", "Arbok", "Pikachu", "Raichu", "Sandshrew", "Sandslash", "Nidoran F", "Nidorina", "Nidoqueen", "Nidoran M", "Nidorino", "Nidoking", "Clefairy", "Clefable", "Vulpix", "Ninetales", "Jigglypuff", "Wigglytuff", "Zubat", "Golbat", "Oddish", "Gloom", "Vileplume", "Paras", "Parasect", "Venonat", "Venomoth", "Diglett", "Dugtrio", "Meowth", "Persian", "Psyduck", "Golduck", "Mankey", "Primeape", "Growlithe", "Arcanine", "Poliwag", "Poliwhirl", "Poliwrath", "Abra", "Kadabra", "Alakazam", "Machop", "Machoke", "Machamp", "Bellsprout", "Weepinbell", "Victreebel", "Tentacool", "Tentacruel", "Geodude", "Graveler", "Golem", "Ponyta", "Rapidash", "Slowpoke", "Slowbro", "Magnemite", "Magneton", "Farfetch'd", "Doduo", "Dodrio", "Seel", "Dewgong", "Grimer", "Muk", "Shellder", "Cloyster", "Gastly", "Haunter", "Gengar", "Onix", "Drowzee", "Hypno", "Krabby", "Kingler", "Voltorb", "Electrode", "Exeggcute", "Exeggutor", "Cubone", "Marowak", "Hitmonlee", "Hitmonchan", "Lickitung", "Koffing", "Weezing", "Rhyhorn", "Rhydon", "Chansey", "Tangela", "Kangaskhan", "Horsea", "Seadra", "Goldeen", "Seaking", "Staryu", "Starmie", "Mr.", "Scyther", "Jynx", "Electabuzz", "Magmar", "Pinsir", "Tauros", "Magikarp", "Gyarados", "Lapras", "Ditto", "Eevee", "Vaporeon", "Jolteon", "Flareon", "Porygon", "Omanyte", "Omastar", "Kabuto", "Kabutops", "Aerodactyl", "Snorlax", "Articuno", "Zapdos", "Moltres", "Dratini", "Dragonair", "Dragonite", "Mewtwo", "Mew", "Chikorita", "Bayleef", "Meganium", "Cyndaquil", "Quilava", "Typhlosion", "Totodile", "Croconaw", "Feraligatr", "Sentret", "Furret", "Hoothoot", "Noctowl", "Ledyba", "Ledian", "Spinarak", "Ariados", "Crobat", "Chinchou", "Lanturn", "Pichu", "Cleffa", "Igglybuff", "Togepi", "Togetic", "Natu", "Xatu", "Mareep", "Flaaffy", "Ampharos", "Bellossom", "Marill", "Azumarill", "Sudowoodo", "Politoed", "Hoppip", "Skiploom", "Jumpluff", "Aipom", "Sunkern", "Sunflora", "Yanma", "Wooper", "Quagsire", "Espeon", "Umbreon", "Murkrow", "Slowking", "Misdreavus", "Unown", "Wobbuffet", "Girafarig", "Pineco", "Forretress", "Dunsparce", "Gligar", "Steelix", "Snubbull", "Granbull", "Qwilfish", "Scizor", "Shuckle", "Heracross", "Sneasel", "Teddiursa", "Ursaring", "Slugma", "Magcargo", "Swinub", "Piloswine", "Corsola", "Remoraid", "Octillery", "Delibird", "Mantine", "Skarmory", "Houndour", "Houndoom", "Kingdra", "Phanpy", "Donphan", "Porygon2", "Stantler", "Smeargle", "Tyrogue", "Hitmontop", "Smoochum", "Elekid", "Magby", "Miltank", "Blissey", "Raikou", "Entei", "Suicune", "Larvitar", "Pupitar", "Tyranitar", "Lugia", "Ho-Oh", "Celebi", "Treecko", "Grovyle", "Sceptile", "Torchic", "Combusken", "Blaziken", "Mudkip", "Marshtomp", "Swampert", "Poochyena", "Mightyena", "Zigzagoon", "Linoone", "Wurmple", "Silcoon", "Beautifly", "Cascoon", "Dustox", "Lotad", "Lombre", "Ludicolo", "Seedot", "Nuzleaf", "Shiftry", "Taillow", "Swellow", "Wingull", "Pelipper", "Ralts", "Kirlia", "Gardevoir", "Surskit", "Masquerain", "Shroomish", "Breloom", "Slakoth", "Vigoroth", "Slaking", "Nincada", "Ninjask", "Shedinja", "Whismur", "Loudred", "Exploud", "Makuhita", "Hariyama", "Azurill", "Nosepass", "Skitty", "Delcatty", "Sableye", "Mawile", "Aron", "Lairon", "Aggron", "Meditite", "Medicham", "Electrike", "Manectric", "Plusle", "Minun", "Volbeat", "Illumise", "Roselia", "Gulpin", "Swalot", "Carvanha", "Sharpedo", "Wailmer", "Wailord", "Numel", "Camerupt", "Torkoal", "Spoink", "Grumpig", "Spinda", "Trapinch", "Vibrava", "Flygon", "Cacnea", "Cacturne", "Swablu", "Altaria", "Zangoose", "Seviper", "Lunatone", "Solrock", "Barboach", "Whiscash", "Corphish", "Crawdaunt", "Baltoy", "Claydol", "Lileep", "Cradily", "Anorith", "Armaldo", "Feebas", "Milotic", "Castform", "Kecleon", "Shuppet", "Banette", "Duskull", "Dusclops", "Tropius", "Chimecho", "Absol", "Wynaut", "Snorunt", "Glalie", "Spheal", "Sealeo", "Walrein", "Clamperl", "Huntail", "Gorebyss", "Relicanth", "Luvdisc", "Bagon", "Shelgon", "Salamence", "Beldum", "Metang", "Metagross", "Regirock", "Regice", "Registeel", "Latias", "Latios", "Kyogre", "Groudon", "Rayquaza", "Jirachi", "Deoxys", "Turtwig", "Grotle", "Torterra", "Chimchar", "Monferno", "Infernape", "Piplup", "Prinplup", "Empoleon", "Starly", "Staravia", "Staraptor", "Bidoof", "Bibarel", "Kricketot", "Kricketune", "Shinx", "Luxio", "Luxray", "Budew", "Roserade", "Cranidos", "Rampardos", "Shieldon", "Bastiodon", "Burmy", "Wormadam", "Mothim", "Combee", "Vespiquen", "Pachirisu", "Buizel", "Floatzel", "Cherubi", "Cherrim", "Shellos", "Gastrodon", "Ambipom", "Drifloon", "Drifblim", "Buneary", "Lopunny", "Mismagius", "Honchkrow", "Glameow", "Purugly", "Chingling", "Stunky", "Skuntank", "Bronzor", "Bronzong", "Bonsly", "Mime", "Happiny", "Chatot", "Spiritomb", "Gible", "Gabite", "Garchomp", "Munchlax", "Riolu", "Lucario", "Hippopotas", "Hippowdon", "Skorupi", "Drapion", "Croagunk", "Toxicroak", "Carnivine", "Finneon", "Lumineon", "Mantyke", "Snover", "Abomasnow", "Weavile", "Magnezone", "Lickilicky", "Rhyperior", "Tangrowth", "Electivire", "Magmortar", "Togekiss", "Yanmega", "Leafeon", "Glaceon", "Gliscor", "Mamoswine", "Porygon-Z", "Gallade", "Probopass", "Dusknoir", "Froslass", "Rotom", "Uxie", "Mesprit", "Azelf", "Dialga", "Palkia", "Heatran", "Regigigas", "Giratina", "Cresselia", "Phione", "Manaphy", "Darkrai", "Shaymin", "Arceus"};
static const char mItemList[][12] = {"None", "Master Ball", "Ultra Ball", "Great Ball", "Poke Ball", "Safari Ball", "Net Ball", "Dive Ball", "Nest Ball", "Repeat Ball", "Timer Ball", "Luxury Ball", "Premier Ball", "Dusk Ball", "Heal Ball", "Quick Ball", "Cherish Ball", "Potion", "Antidote", "Burn Heal", "Ice Heal", "Awakening", "Parlyz Heal", "Full Restore", "Max Potion", "Hyper Potion", "Super Potion", "Full Heal", "Revive", "Max Revive", "Fresh Water", "Soda Pop", "Lemonade", "Moomoo Milk", "EnergyPowder", "Energy Root", "Heal Powder", "Revival Herb", "Ether", "Max Ether", "Elixir", "Max Elixir", "Lava Cookie", "Berry Juice", "Sacred Ash", "HP Up", "Protein", "Iron", "Carbos", "Calcium", "Rare Candy", "PP Up", "Zinc", "PP Max", "Old Gateau", "Guard Spec.", "Dire Hit", "X Attack", "X Defense", "X Speed", "X Accuracy", "X Special", "X Sp. Def", "Poke Doll", "Fluffy Tail", "Blue Flute", "Yellow Flute", "Red Flute", "Black Flute", "White Flute", "Shoal Salt", "Shoal Shell", "Red Shard", "Blue Shard", "Yellow Shard", "Green Shard", "Super Repel", "Max Repel", "Escape Rope", "Repel", "Sun Stone", "Moon Stone", "Fire Stone", "Thunderstone", "Water Stone", "Leaf Stone", "TinyMushroom", "Big Mushroom", "Pearl", "Big Pearl", "Stardust", "Star Piece", "Nugget", "Heart Scale", "Honey", "Growth Mulch", "Damp Mulch", "Stable Mulch", "Gooey Mulch", "Root Fossil", "Claw Fossil", "Helix Fossil", "Dome Fossil", "Old Amber", "Armor Fossil", "Skull Fossil", "Rare Bone", "Shiny Stone", "Dusk Stone", "Dawn Stone", "Oval Stone", "Odd Keystone", "Griseous Orb", "GlitchItm113", "GlitchItm114", "GlitchItm115", "GlitchItm116", "GlitchItm117", "GlitchItm118", "GlitchItm119", "GlitchItm120", "GlitchItm121", "GlitchItm122", "GlitchItm123", "GlitchItm124", "GlitchItm125", "GlitchItm126", "GlitchItm127", "GlitchItm128", "GlitchItm129", "GlitchItm130", "GlitchItm131", "GlitchItm132", "GlitchItm133", "GlitchItm134", "Adamant Orb", "Lustrous Orb", "Grass Mail", "Flame Mail", "Bubble Mail", "Bloom Mail", "Tunnel Mail", "Steel Mail", "Heart Mail", "Snow Mail", "Space Mail", "Air Mail", "Mosaic Mail", "Brick Mail", "Cheri Berry", "Chesto Berry", "Pecha Berry", "Rawst Berry", "Aspear Berry", "Leppa Berry", "Oran Berry", "Persim Berry", "Lum Berry", "Sitrus Berry", "Figy Berry", "Wiki Berry", "Mago Berry", "Aguav Berry", "Iapapa Berry", "Razz Berry", "Bluk Berry", "Nanab Berry", "Wepear Berry", "Pinap Berry", "Pomeg Berry", "Kelpsy Berry", "Qualot Berry", "Hondew Berry", "Grepa Berry", "Tamato Berry", "Cornn Berry", "Magost Berry", "Rabuta Berry", "Nomel Berry", "Spelon Berry", "Pamtre Berry", "Watmel Berry", "Durin Berry", "Belue Berry", "Occa Berry", "Passho Berry", "Wacan Berry", "Rindo Berry", "Yache Berry", "Chople Berry", "Kebia Berry", "Shuca Berry", "Coba Berry", "Payapa Berry", "Tanga Berry", "Charti Berry", "Kasib Berry", "Haban Berry", "Colbur Berry", "Babiri Berry", "Chilan Berry", "Liechi Berry", "Ganlon Berry", "Salac Berry", "Petaya Berry", "Apicot Berry", "Lansat Berry", "Starf Berry", "Enigma Berry", "Micle Berry", "Custap Berry", "Jaboca Berry", "Rowap Berry", "BrightPowder", "White Herb", "Macho Brace", "Exp. Share", "Quick Claw", "Soothe Bell", "Mental Herb", "Choice Band", "King's Rock", "SilverPowder", "Amulet Coin", "Cleanse Tag", "Soul Dew", "DeepSeaTooth", "DeepSeaScale", "Smoke Ball", "Everstone", "Focus Band", "Lucky Egg", "Scope Lens", "Metal Coat", "Leftovers", "Dragon Scale", "Light Ball", "Soft Sand", "Hard Stone", "Miracle Seed", "BlackGlasses", "Black Belt", "Magnet", "Mystic Water", "Sharp Beak", "Poison Barb", "NeverMeltIce", "Spell Tag", "TwistedSpoon", "Charcoal", "Dragon Fang", "Silk Scarf", "Up-Grade", "Shell Bell", "Sea Incense", "Lax Incense", "Lucky Punch", "Metal Powder", "Thick Club", "Stick", "Red Scarf", "Blue Scarf", "Pink Scarf", "Green Scarf", "Yellow Scarf", "Wide Lens", "Muscle Band", "Wise Glasses", "Expert Belt", "Light Clay", "Life Orb", "Power Herb", "Toxic Orb", "Flame Orb", "Quick Powder", "Focus Sash", "Zoom Lens", "Metronome", "Iron Ball", "Lagging Tail", "Destiny Knot", "Black Sludge", "Icy Rock", "Smooth Rock", "Heat Rock", "Damp Rock", "Grip Claw", "Choice Scarf", "Sticky Barb", "Power Bracer", "Power Belt", "Power Lens", "Power Band", "Power Anklet", "Power Weight", "Shed Shell", "Big Root", "Choice Specs", "Flame Plate", "Splash Plate", "Zap Plate", "Meadow Plate", "Icicle Plate", "Fist Plate", "Toxic Plate", "Earth Plate", "Sky Plate", "Mind Plate", "Insect Plate", "Stone Plate", "Spooky Plate", "Draco Plate", "Dread Plate", "Iron Plate", "Odd Incense", "Rock Incense", "Full Incense", "Wave Incense", "Rose Incense", "Luck Incense", "Pure Incense", "Protector", "Electirizer", "Magmarizer", "Dubious Disc", "Reaper Cloth", "Razor Claw", "Razor Fang", "TM01", "TM02", "TM03", "TM04", "TM05", "TM06", "TM07", "TM08", "TM09", "TM10", "TM11", "TM12", "TM13", "TM14", "TM15", "TM16", "TM17", "TM18", "TM19", "TM20", "TM21", "TM22", "TM23", "TM24", "TM25", "TM26", "TM27", "TM28", "TM29", "TM30", "TM31", "TM32", "TM33", "TM34", "TM35", "TM36", "TM37", "TM38", "TM39", "TM40", "TM41", "TM42", "TM43", "TM44", "TM45", "TM46", "TM47", "TM48", "TM49", "TM50", "TM51", "TM52", "TM53", "TM54", "TM55", "TM56", "TM57", "TM58", "TM59", "TM60", "TM61", "TM62", "TM63", "TM64", "TM65", "TM66", "TM67", "TM68", "TM69", "TM70", "TM71", "TM72", "TM73", "TM74", "TM75", "TM76", "TM77", "TM78", "TM79", "TM80", "TM81", "TM82", "TM83", "TM84", "TM85", "TM86", "TM87", "TM88", "TM89", "TM90", "TM91", "TM92", "HM01", "HM02", "HM03", "HM04", "HM05", "HM06", "HM07", "HM08", "Explorer Kit", "Loot Sack", "Rule Book", "Poke Radar", "Point Card", "Journal", "Seal Case", "Fashion Case", "Seal Bag", "Pal Pad", "Works Key", "Old Charm", "Galactic Key", "Red Chain", "Town Map", "Vs. Seeker", "Coin Case", "Old Rod", "Good Rod", "Super Rod", "Sprayduck", "Poffin Case", "Bicycle", "Suite Key", "Oak's Letter", "Lunar Wing", "Member Card", "Azure Flute", "S.S. Ticket", "Contest Pass", "Magma Stone", "Parcel", "Coupon 1", "Coupon 2", "Coupon 3", "Storage Key", "SecretPotion", "Vs. Recorder", "Gracidea", "Secret Key", "Apricorn Box", "Unown Report", "Berry Pots", "Dowsing MCHN", "Blue Card", "SlowpokeTail", "Clear Bell", "Card Key", "Basement Key", "SquirtBottle", "Red Scale", "Lost Item", "Pass", "Machine Part", "Silver Wing", "Rainbow Wing", "Mystery Egg", "Red Apricorn", "Ylw Apricorn", "Blu Apricorn", "Grn Apricorn", "Pnk Apricorn", "Wht Apricorn", "Blk Apricorn", "Fast Ball", "Level Ball", "Lure Ball", "Heavy Ball", "Love Ball", "Friend Ball", "Moon Ball", "Sport Ball", "Park Ball", "Photo Album", "GB Sounds", "Tidal Bell", "RageCandyBar", "Data Card 01", "Data Card 02", "Data Card 03", "Data Card 04", "Data Card 05", "Data Card 06", "Data Card 07", "Data Card 08", "Data Card 09", "Data Card 10", "Data Card 11", "Data Card 12", "Data Card 13", "Data Card 14", "Data Card 15", "Data Card 16", "Data Card 17", "Data Card 18", "Data Card 19", "Data Card 20", "Data Card 21", "Data Card 22", "Data Card 23", "Data Card 24", "Data Card 25", "Data Card 26", "Data Card 27", "Jade Orb", "Lock Capsule", "Red Orb", "Blue Orb", "Enigma Stone"};
static const char mMoveList[][13] = {"None", "Pound", "Karate Chop", "Double Slap", "Comet Punch", "Mega Punch", "Pay Day", "Fire Punch", "Ice Punch", "Thunder Punch", "Scratch", "Vise Grip", "Guillotine", "Razor Wind", "Swords Dance", "Cut", "Gust", "Wing Attack", "Whirlwind", "Fly", "Bind", "Slam", "Vine Whip", "Stomp", "Double Kick", "Mega Kick", "Jump Kick", "Rolling Kick", "Sand Attack", "Headbutt", "Horn Attack", "Fury Attack", "Horn Drill", "Tackle", "Body Slam", "Wrap", "Take Down", "Thrash", "Double-Edge", "Tail Whip", "Poison Sting", "Twineedle", "Pin Missile", "Leer", "Bite", "Growl", "Roar", "Sing", "Supersonic", "Sonic Boom", "Disable", "Acid", "Ember", "Flamethrower", "Mist", "Water Gun", "Hydro Pump", "Surf", "Ice Beam", "Blizzard", "Psybeam", "Bubble Beam", "Aurora Beam", "Hyper Beam", "Peck", "Drill Peck", "Submission", "Low Kick", "Counter", "Seismic Toss", "Strength", "Absorb", "Mega Drain", "Leech Seed", "Growth", "Razor Leaf", "Solar Beam", "Poison Powder", "Stun Spore", "Sleep Powder", "Petal Dance", "String Shot", "Dragon Rage", "Fire Spin", "Thunder Shock", "Thunderbolt", "Thunder Wave", "Thunder", "Rock Throw", "Earthquake", "Fissure", "Dig", "Toxic", "Confusion", "Psychic", "Hypnosis", "Meditate", "Agility", "Quick Attack", "Rage", "Teleport", "Night Shade", "Mimic", "Screech", "Double Team", "Recover", "Harden", "Minimize", "Smokescreen", "Confuse Ray", "Withdraw", "Defense Curl", "Barrier", "Light Screen", "Haze", "Reflect", "Focus Energy", "Bide", "Metronome", "Mirror Move", "Self-Destruct", "Egg Bomb", "Lick", "Smog", "Sludge", "Bone Club", "Fire Blast", "Waterfall", "Clamp", "Swift", "Skull Bash", "Spike Cannon", "Constrict", "Amnesia", "Kinesis", "Soft-Boiled", "Hi Jump Kick", "Glare", "Dream Eater", "Poison Gas", "Barrage", "Leech Life", "Lovely Kiss", "Sky Attack", "Transform", "Bubble", "Dizzy Punch", "Spore", "Flash", "Psywave", "Splash", "Acid Armor", "Crabhammer", "Explosion", "Fury Swipes", "Bonemerang", "Rest", "Rock Slide", "Hyper Fang", "Sharpen", "Conversion", "Tri Attack", "Super Fang", "Slash", "Substitute", "Struggle", "Sketch", "Triple Kick", "Thief", "Spider Web", "Mind Reader", "Nightmare", "Flame Wheel", "Snore", "Curse", "Flail", "Conversion 2", "Aeroblast", "Cotton Spore", "Reversal", "Spite", "Powder Snow", "Protect", "Mach Punch", "Scary Face", "Feint Attack", "Sweet Kiss", "Belly Drum", "Sludge Bomb", "Mud-Slap", "Octazooka", "Spikes", "Zap Cannon", "Foresight", "Destiny Bond", "Perish Song", "Icy Wind", "Detect", "Bone Rush", "Lock-On", "Outrage", "Sandstorm", "Giga Drain", "Endure", "Charm", "Rollout", "False Swipe", "Swagger", "Milk Drink", "Spark", "Fury Cutter", "Steel Wing", "Mean Look", "Attract", "Sleep Talk", "Heal Bell", "Return", "Present", "Frustration", "Safeguard", "Pain Split", "Sacred Fire", "Magnitude", "Dynamic Punch", "Megahorn", "Dragon Breath", "Baton Pass", "Encore", "Pursuit", "Rapid Spin", "Sweet Scent", "Iron Tail", "Metal Claw", "Vital Throw", "Morning Sun", "Synthesis", "Moonlight", "Hidden Power", "Cross Chop", "Twister", "Rain Dance", "Sunny Day", "Crunch", "Mirror Coat", "Psych Up", "Extreme Speed", "Ancient Power", "Shadow Ball", "Future Sight", "Rock Smash", "Whirlpool", "Beat Up", "Fake Out", "Uproar", "Stockpile", "Spit Up", "Swallow", "Heat Wave", "Hail", "Torment", "Flatter", "Will-O-Wisp", "Memento", "Facade", "Focus Punch", "SmellingSalt", "Follow Me", "Nature Power", "Charge", "Taunt", "Helping Hand", "Trick", "Role Play", "Wish", "Assist", "Ingrain", "Superpower", "Magic Coat", "Recycle", "Revenge", "Brick Break", "Yawn", "Knock Off", "Endeavor", "Eruption", "Skill Swap", "Imprison", "Refresh", "Grudge", "Snatch", "Secret Power", "Dive", "Arm Thrust", "Camouflage", "Tail Glow", "Luster Purge", "Mist Ball", "Feather Dance", "Teeter Dance", "Blaze Kick", "Mud Sport", "Ice Ball", "Needle Arm", "Slack Off", "Hyper Voice", "Poison Fang", "Crush Claw", "Blast Burn", "Hydro Cannon", "Meteor Mash", "Astonish", "Weather Ball", "Aromatherapy", "Fake Tears", "Air Cutter", "Overheat", "Odor Sleuth", "Rock Tomb", "Silver Wind", "Metal Sound", "Grass Whistle", "Tickle", "Cosmic Power", "Water Spout", "Signal Beam", "Shadow Punch", "Extrasensory", "Sky Uppercut", "Sand Tomb", "Sheer Cold", "Muddy Water", "Bullet Seed", "Aerial Ace", "Icicle Spear", "Iron Defense", "Block", "Howl", "Dragon Claw", "Frenzy Plant", "Bulk Up", "Bounce", "Mud Shot", "Poison Tail", "Covet", "Volt Tackle", "Magical Leaf", "Water Sport", "Calm Mind", "Leaf Blade", "Dragon Dance", "Rock Blast", "Shock Wave", "Water Pulse", "Doom Desire", "Psycho Boost", "Roost", "Gravity", "Miracle Eye", "Wake-Up Slap", "Hammer Arm", "Gyro Ball", "Healing Wish", "Brine", "Natural Gift", "Feint", "Pluck", "Tailwind", "Acupressure", "Metal Burst", "U-turn", "Close Combat", "Payback", "Assurance", "Embargo", "Fling", "Psycho Shift", "Trump Card", "Heal Block", "Wring Out", "Power Trick", "Gastro Acid", "Lucky Chant", "Me First", "Copycat", "Power Swap", "Guard Swap", "Punishment", "Last Resort", "Worry Seed", "Sucker Punch", "Toxic Spikes", "Heart Swap", "Aqua Ring", "Magnet Rise", "Flare Blitz", "Force Palm", "Aura Sphere", "Rock Polish", "Poison Jab", "Dark Pulse", "Night Slash", "Aqua Tail", "Seed Bomb", "Air Slash", "X-Scissor", "Bug Buzz", "Dragon Pulse", "Dragon Rush", "Power Gem", "Drain Punch", "Vacuum Wave", "Focus Blast", "Energy Ball", "Brave Bird", "Earth Power", "Switcheroo", "Giga Impact", "Nasty Plot", "Bullet Punch", "Avalanche", "Ice Shard", "Shadow Claw", "Thunder Fang", "Ice Fang", "Fire Fang", "Shadow Sneak", "Mud Bomb", "Psycho Cut", "Zen Headbutt", "Mirror Shot", "Flash Cannon", "Rock Climb", "Defog", "Trick Room", "Draco Meteor", "Discharge", "Lava Plume", "Leaf Storm", "Power Whip", "Rock Wrecker", "Cross Poison", "Gunk Shot", "Iron Head", "Magnet Bomb", "Stone Edge", "Captivate", "Stealth Rock", "Grass Knot", "Chatter", "Judgment", "Bug Bite", "Charge Beam", "Wood Hammer", "Aqua Jet", "Attack Order", "Defend Order", "Heal Order", "Head Smash", "Double Hit", "Roar of Time", "Spacial Rend", "Lunar Dance", "Crush Grip", "Magma Storm", "Dark Void", "Seed Flare", "Ominous Wind", "Shadow Force"};
static const char mAbilityList[][13] = {"None", "Stench", "Drizzle", "Speed Boost", "Battle Armor", "Sturdy", "Damp", "Limber", "Sand Veil", "Static", "Volt Absorb", "Water Absorb", "Oblivious", "Cloud Nine", "Compound Eyes", "Insomnia", "Color Change", "Immunity", "Flash Fire", "Shield Dust", "Own Tempo", "Suction Cups", "Intimidate", "Shadow Tag", "Rough Skin", "Wonder Guard", "Levitate", "Effect Spore", "Synchronize", "Clear Body", "Natural Cure", "Lightning Rod", "Serene Grace", "Swift Swim", "Chlorophyll", "Illuminate", "Trace", "Huge Power", "Poison Point", "Inner Focus", "Magma Armor", "Water Veil", "Magnet Pull", "Soundproof", "Rain Dish", "Sand Stream", "Pressure", "Thick Fat", "Early Bird", "Flame Body", "Run Away", "Keen Eye", "Hyper Cutter", "Pickup", "Truant", "Hustle", "Cute Charm", "Plus", "Minus", "Forecast", "Sticky Hold", "Shed Skin", "Guts", "Marvel Scale", "Liquid Ooze", "Overgrow", "Blaze", "Torrent", "Swarm", "Rock Head", "Drought", "Arena Trap", "Vital Spirit", "White Smoke", "Pure Power", "Shell Armor", "Air Lock", "Tangled Feet", "Motor Drive", "Rivalry", "Steadfast", "Snow Cloak", "Gluttony", "Anger Point", "Unburden", "Heatproof", "Simple", "Dry Skin", "Download", "Iron Fist", "Poison Heal", "Adaptability", "Skill Link", "Hydration", "Solar Power", "Quick Feet", "Normalize", "Sniper", "Magic Guard", "No Guard", "Stall", "Technician", "Leaf Guard", "Klutz", "Mold Breaker", "Super Luck", "Aftermath", "Anticipation", "Forewarn", "Unaware", "Tinted Lens", "Filter", "Slow Start", "Scrappy", "Storm Drain", "Ice Body", "Solid Rock", "Snow Warning", "Honey Gather", "Frisk", "Reckless", "Multitype", "Flower Gift", "Bad Dreams"};


#define DUMP_FILE_NAME		"/POKEROM.BIN"


struct PokeBasicInfo {		//all multi-byte values are LE (and m68k is not)
	uint16_t poke;
	uint16_t item;
	uint16_t move[4];
	uint8_t level;
	uint8_t subspecies;
	uint8_t flags;
	uint8_t padding;
};

struct PokeEventInfo {
	uint32_t unused;
	uint16_t tid;
	uint16_t sid;
	uint16_t unk0;
	uint16_t locMet;	//2008 = distant land
	uint16_t unk1;
	uint16_t otName[8];
	uint8_t encounterType;
	uint8_t ability;
	uint16_t ballType;	//as item type
	uint8_t unk3[10];
};

static void* FrmGetObjectPtrById(FormPtr fp, uint16_t id)
{
	UInt16 idx = FrmGetObjectIndex(fp, id);
	
	if (idx == frmInvalidObjectId)
		return NULL;

	return FrmGetObjectPtr(fp, idx);
}

static void setFieldNumericTextEditable(FormPtr fp, uint16_t fieldObjectIdx, int32_t value)
{
	FieldPtr fi = FrmGetObjectPtrById(fp, fieldObjectIdx);
	MemHandle mh = MemHandleNew(16), old;
	
	StrIToA(MemHandleLock(mh), value);
	MemHandleUnlock(mh);
	
	old = FldGetTextHandle(fi);
	FldSetTextHandle(fi, mh);
	if (FrmVisible(fp))
		FldDrawField(fi);
	
	if (old)
		MemHandleFree(old);
}

static int32_t getFieldNumericText(FormPtr fp, uint16_t fieldObjectIdx)
{
	return StrAToI(FldGetTextPtr(FrmGetObjectPtrById(fp, fieldObjectIdx)));
}

static void pokeBasicEditUpdateOneLabel(FormPtr fp, uint16_t srcTxtBoxId, uint16_t dstLabelId, void* data, uint8_t perItem, uint16_t numItems)
{
	int32_t val;
	char x[64];
	
	if (!FrmGetObjectPtrById(fp, srcTxtBoxId))	//to allow reuse on smaller forms as long as IDs match
		return;
	
	val = getFieldNumericText(fp, srcTxtBoxId);
	if (val < 0 || val >= numItems)
		StrCopy(x, "<INVALID>");
	else {
		
		MemMove(x, ((char*)data) + (uint16_t)val * perItem, perItem);
		x[perItem] = 0;
	}
	
	StrCat(x, "                     ");	//fuck erasing shit
	
	FrmCopyLabel(fp, dstLabelId, x);
}

static void pokeBasicEditUpdateLabels(FormPtr fp)
{
	uint8_t i;
	
	pokeBasicEditUpdateOneLabel(fp, 1000, 3000, (void*)mPokeList, sizeof(mPokeList[0]), sizeof(mPokeList) / sizeof(*mPokeList));
	pokeBasicEditUpdateOneLabel(fp, 1001, 3001, (void*)mItemList, sizeof(mItemList[0]), sizeof(mItemList) / sizeof(*mItemList));
	
	for (i = 0; i < 4; i++)
		pokeBasicEditUpdateOneLabel(fp, 1002 + i, 3002 + i, (void*)mMoveList, sizeof(mMoveList[0]), sizeof(mMoveList) / sizeof(*mMoveList));
	
	pokeBasicEditUpdateOneLabel(fp, 1009, 3009, (void*)mAbilityList, sizeof(mAbilityList[0]), sizeof(mAbilityList) / sizeof(*mAbilityList));
}

static uint16_t getListItem(char *dst, void *items, uint16_t idx)		//return length, always terminates too
{
	char *raw = (char*)items;
	uint16_t i, eachLen;
	
	//get per-item length
	for (eachLen = StrLen(raw); !raw[eachLen]; eachLen++);
	
	//get ptr to our text
	raw += eachLen * idx;
	
	//get cur len
	for (i = 0; i < eachLen && raw[i]; i++);
	
	MemMove(dst, raw, i);
	dst[i] = 0;
	
	return i;
}

static void pokeEditBasicPopupListForFieldListDrawFunc(int16_t itemNum, RectanglePtr bounds, Char **itemsText)
{
	char str[32];
	
	WinDrawChars(str, getListItem(str, (void*)itemsText, itemNum), bounds->topLeft.x, bounds->topLeft.y);
}

static void pokeEditBasicPopupListForField(FormPtr fp, uint16_t fieldObjIdx, void* data, uint8_t each, uint16_t num)
{
	ListPtr L = FrmGetObjectPtrById(fp, 8000);
	int16_t ret;
	
	LstSetDrawFunction(L, pokeEditBasicPopupListForFieldListDrawFunc);
	LstSetListChoices(L, (char**)data, num);
	ret = LstPopupList(L);
	
	if (ret >= 0) {
		setFieldNumericTextEditable(fp, fieldObjIdx, ret);
		pokeBasicEditUpdateLabels(fp);
	}
}

static Boolean pokeBasicEditEventHandler(EventType *evt)
{
	FormPtr fp = FrmGetActiveForm();

	if (evt->eType == ctlExitEvent || evt->eType == ctlEnterEvent || evt->eType == fldEnterEvent || evt->eType == popSelectEvent || evt->eType == keyDownEvent) 
		pokeBasicEditUpdateLabels(fp);
	
	if (evt->eType == ctlSelectEvent) switch (evt->data.ctlSelect.controlID) {
		case 2000:
			pokeEditBasicPopupListForField(fp, 1000, (void*)mPokeList, sizeof(mPokeList[0]), sizeof(mPokeList) / sizeof(*mPokeList));
			return true;
		
		case 2001:
			pokeEditBasicPopupListForField(fp, 1001, (void*)mItemList, sizeof(mItemList[0]), sizeof(mItemList) / sizeof(*mItemList));
			return true;
		
		case 2002:
		case 2003:
		case 2004:
		case 2005:
			pokeEditBasicPopupListForField(fp, evt->data.ctlSelect.controlID - 1000, (void*)mMoveList, sizeof(mMoveList[0]), sizeof(mMoveList) / sizeof(*mMoveList));
			return true;
		
		case 2009:
			pokeEditBasicPopupListForField(fp, 1009, (void*)mAbilityList, sizeof(mAbilityList[0]), sizeof(mAbilityList) / sizeof(*mAbilityList));
			return true;
			
	}
	
	return false;
}

static bool pokeBasicEdit(struct PokeBasicInfo *info, uint8_t *abilityP)
{
	FormPtr fp = FrmInitForm(1000);
	EventType e;
	uint8_t i;
	bool ret;
	
	setFieldNumericTextEditable(fp, 1000, info->poke);
	setFieldNumericTextEditable(fp, 1001, info->item);
	for (i = 0; i < 4; i++)
		setFieldNumericTextEditable(fp, 1002 + i, info->move[i]);
	setFieldNumericTextEditable(fp, 1006, info->level);
	
	CtlSetValue(FrmGetObjectPtrById(fp, 1007), !!(info->flags & 0x02));
	CtlSetValue(FrmGetObjectPtrById(fp, 1008), !!(info->subspecies & 0x20));
	
	if (abilityP)
		setFieldNumericTextEditable(fp, 1009, *abilityP);
	else {
		FrmHideObject(fp, FrmGetObjectIndex(fp, 1009));
		FrmHideObject(fp, FrmGetObjectIndex(fp, 2009));
		FrmHideObject(fp, FrmGetObjectIndex(fp, 3009));
		FrmHideObject(fp, FrmGetObjectIndex(fp, 7009));
	}
	
	pokeBasicEditUpdateLabels(fp);
	
	FrmSetEventHandler(fp, pokeBasicEditEventHandler);
	ret = 9000 == FrmDoDialog(fp);
	
	info->poke = getFieldNumericText(fp, 1000);
	info->item = getFieldNumericText(fp, 1001);
	for (i = 0; i < 4; i++)
		info->move[i] = getFieldNumericText(fp, 1002 + i);
	info->level = getFieldNumericText(fp, 1006);
	info->flags = (info->flags &~ 0x02) | (CtlGetValue(FrmGetObjectPtrById(fp, 1007)) ? 0x02 : 0x00);
	info->subspecies = (info->subspecies &~ 0x20) | (CtlGetValue(FrmGetObjectPtrById(fp, 1008)) ? 0x20 : 0x00);
	if (abilityP)
		*abilityP = getFieldNumericText(fp, 1009);
	
	
	FrmDeleteForm(fp);

	return ret && info->poke;
}

static bool stringToImg(void *dstP, int16_t w, int16_t h, const char *str, bool centered)
{
	uint16_t r, c, rr, err, L = StrLen(str);
	uint16_t *dst = (uint16_t*)dstP;
	WinDrawOperation wdo;
	uint8_t font, bc, tc;
	int16_t cw, cx, cy;
	WinHandle wh, dw;
	BitmapPtr b;
	
	
	b = BmpCreate(w, h, 2, NULL, &err);
	if (!b)
		return false;
	
	wh = WinCreateBitmapWindow(b, &err);
	if (!wh) {
		BmpDelete(b);
		return false;
	}
	
	dw = WinSetDrawWindow(wh);
	font = FntSetFont(stdFont);
	bc = WinSetBackColor(0);
	tc = WinSetTextColor(1);
	wdo = WinSetDrawMode(winOverlay);
	
	cw = FntCharsWidth(str, L);
	if (centered)
		cx = (w - cw) / 2;
	else
		cx = 3;
	cy = (h - FntCharHeight()) / 2;
	
	WinPaintChars(str, L, cx + 0, cy + 1);
	WinPaintChars(str, L, cx + 1, cy + 0);
	WinPaintChars(str, L, cx + 1, cy + 1);
	WinSetTextColor(3);
	WinPaintChars(str, L, cx, cy);
	
	//get data
	for (r = 0; r < h; r += 8) {
		for (c = 0; c < w; c++) {
			const uint16_t masks[] = {0x0000, 0x0001, 0x0100, 0x0101};
			uint16_t val = 0;
			
			for (rr = 0; rr < 8; rr++)
				val |= masks[WinGetPixel(c, r + rr)] << rr;
			
			*dst++ = val;
		}
	}
	
	WinSetDrawMode(wdo);
	WinSetTextColor(tc);
	WinSetBackColor(bc);
	FntSetFont(font);
	WinSetDrawWindow(dw);
	WinDeleteWindow(wh, false);
	BmpDelete(b);
	
	return true;
}

static uint16_t swap16(uint16_t val)
{
	return (val >> 8) | ( val << 8);
}

static uint32_t swap32(uint32_t val)
{
	return ((val >> 24) & 0xff) | ((val >> 8) & 0xff00) | ((val & 0xff00) << 8) | ((val & 0xff) << 24);
}

static void eventPoke(void)
{
	struct PokeBasicInfo pbi = {0, };
	struct PokeEventInfo pei = {0, };
	uint8_t *pokeImg, *pokeName, i;
	struct Comms *comms;
	char name[32];
	
	pei.locMet = swap16(2008);				//distant land
	pei.tid = SysRandom(0);											//no need to byteswap it. random is random
	pei.sid = SysRandom(0);											//no need to byteswap it. random is random
	pei.ballType = 0x0c;	//premier ball
	pbi.level = 1;

	if (!pokeBasicEdit(&pbi, &pei.ability))
		return;
	
	pokeImg = MemPtrNew(0x180);
	pokeName = MemPtrNew(0x140);
	
	getListItem(name, (void*)mPokeList, pbi.poke);
	stringToImg(pokeName, 80, 16, name, false);
	stringToImg(pokeImg + 0xc0 * 0, 32, 24, "EVT", true);
	stringToImg(pokeImg + 0xc0 * 1, 32, 24, "POKe", true);
	
	pbi.poke = swap16(pbi.poke);
	pbi.item = swap16(pbi.item);
	for (i = 0; i < 4; i++)
		pbi.move[i] = swap16(pbi.move[i]);
	
	
	pei.otName[0] = swap16(0x012E);	//D
	pei.otName[1] = swap16(0x0151);	//m
	pei.otName[2] = swap16(0x014D);	//i
	pei.otName[3] = swap16(0x0158);	//t
	pei.otName[4] = swap16(0x0156);	//r
	pei.otName[5] = swap16(0x015D);	//y
	pei.otName[6] = swap16(0x0131);	//G
	pei.otName[7] = 0xFFFF;			//NUL
	
	WinDrawChars("READY TO SEND", 13, 0, 0);
	
	comms = commsOpen();
	if (!comms)
		FrmCustomAlert(ALERT_ID_ERROR, "Cannot open comms", "", "");
	else {
		
		while(1) {
			
			bool established = true;
			
			while (!commsTryAcceptWalkerConnection(comms)) {
				if (KeyCurrentState()) {
					established = false;
					break;
				}
			}
			
			if (!established)
				break;
				
			if (!commsEepromWrite(comms, &pbi, 0xba44,  sizeof(pbi)))
				FrmCustomAlert(ALERT_ID_ERROR, "Cannot write poke basic info", "", "");
			else if (!commsEepromWrite(comms, &pei, 0xba54,  sizeof(pei)))
				FrmCustomAlert(ALERT_ID_ERROR, "Cannot write poke extd info", "", "");
			else if (!commsEepromWrite(comms, pokeImg, 0xba80,  0x180))
				FrmCustomAlert(ALERT_ID_ERROR, "Cannot write poke image", "", "");
			else if (!commsEepromWrite(comms, pokeName, 0xbc00,  0x140))
				FrmCustomAlert(ALERT_ID_ERROR, "Cannot write poke name", "", "");
			else if (!commsEventPokeRxed(comms))
				FrmCustomAlert(ALERT_ID_ERROR, "Cannot trigger event", "", "");
			else {
				FrmCustomAlert(ALERT_ID_INFO, "SUCCESS", "", "");
				break;
			}
			
			commsDisconnect(comms);
		}
		
		commsClose(comms);
	}
	
	MemPtrFree(pokeName);
	MemPtrFree(pokeImg);
}

static void romDump(void)
{
	const uint16_t romSize = 0xc000, step = 0x80;
	struct Comms *comms = commsOpen();

	if (!comms)
		FrmCustomAlert(ALERT_ID_ERROR, "Cannot open comms", "", "");
	else {
		
		uint8_t *dump = MemPtrNew(romSize);
		char data[9];

		if (!dump)
			FrmCustomAlert(ALERT_ID_ERROR, "cannot allocate memory", NULL, NULL);
		else {
			
			bool established = true;
			
			WinDrawChars("Press any key to exit", 21, 0, 0);
			WinDrawChars("Aim walker at palm, start comms. wait.", 38, 0, 11);
			

			while (!commsTryAcceptWalkerConnection(comms)) {
				if (KeyCurrentState()) {
					established = false;
					break;
				}
			}
			
			if(established) {
			
				data[sizeof(data) - 1] = 0;
				if (!commsEepromRead(comms, data, 0, sizeof(data) - 1))
					FrmCustomAlert(ALERT_ID_ERROR, "Cannot read", "", "");
				
				if (StrCompare(data, "nintendo")) {
				
					FrmCustomAlert(ALERT_ID_ERROR, "EEPROM read failed to produce valid data. Not proceeding", NULL, NULL);
					commsDisconnect(comms);
				}
				else {
					
					struct PokePacket pkt;
					uint16_t i;
					
					pkt.cmd = 6;
					pkt.detail = 0xf9;
					
					if (!commsPacketRawTx(comms, &pkt, rom_dump_exploit_upload_to_0xF956, sizeof(rom_dump_exploit_upload_to_0xF956)))
						FrmCustomAlert(ALERT_ID_ERROR, "cannot tx exploit", NULL, NULL);
					
					if (commsPacketRawRx(comms, &pkt, NULL, 0))
						FrmCustomAlert(ALERT_ID_ERROR, "cannot get ack for exploit", NULL, NULL);
					
					pkt.cmd = 6;
					pkt.detail = 0xf7;
					
					if (!commsPacketRawTx(comms, &pkt, trigger_uploaded_code_upload_to_0xF7E0, sizeof(trigger_uploaded_code_upload_to_0xF7E0)))
						FrmCustomAlert(ALERT_ID_ERROR, "cannot tx trigger", NULL, NULL);
					
					if (commsPacketRawRx(comms, &pkt, NULL, 0))
						FrmCustomAlert(ALERT_ID_ERROR, "cannot get ack for trigger", NULL, NULL);
										
					for (i = 0; i < romSize;) {
					
						char x[128];
						
						if (0x80 != commsPacketRawRx(comms, &pkt, dump + i, step)) {
							
							StrPrintF(x, "Error getting data at 0x%04x", i);
							FrmCustomAlert(ALERT_ID_INFO, x, NULL, NULL);
							break;
						}
						
						i += step;
						if (!(i & 0xfff)) {
							StrIToH(x, i);
							x[3] = 'x';
							WinDrawChars(x + 2, 6, 0, 20);
						}
					}
					
					if (i == romSize) {
						
						uint32_t vi = vfsIteratorStart;
						uint16_t vrn;
						FileRef f;
						Err e;
						
						if (errNone != VFSVolumeEnumerate(&vrn, &vi))
							FrmCustomAlert(ALERT_ID_ERROR, "cannot find a memory card to write the rom to", NULL, NULL);
						else if (errNone != VFSFileOpen(vrn, DUMP_FILE_NAME, vfsModeWrite | vfsModeCreate | vfsModeTruncate, &f))
							FrmCustomAlert(ALERT_ID_ERROR, "cannot openrom dump file", NULL, NULL);
						else {
							
							for (i = 0; i < romSize;) {
								
								uint32_t now;
								
								if (errNone != VFSFileWrite(f, romSize - i, dump + i, &now)) {
									FrmCustomAlert(ALERT_ID_ERROR, "cannot write to dump file", NULL, NULL);
									break;
								}
								i += now;
							}
							VFSFileClose(f);
							
							if (i == romSize)
								FrmCustomAlert(ALERT_ID_INFO, "ROM dumped to ", DUMP_FILE_NAME, NULL);
						}
					}
				}
			}
			MemPtrFree(dump);
		}
		commsClose(comms);
	}
}

static void itemGift(bool useEventItemMethod)
{
	uint8_t *itemName = MemPtrNew(0x180), itemData[40] = {0,}, i, uploadLen = sizeof(itemData);
	FormPtr fp = FrmInitForm(1002);
	struct Comms *comms;
	int32_t item;
	bool proceed;
	char name[32];
	
	if (!useEventItemMethod)
		FrmCustomAlert(ALERT_ID_INFO, "Pokewalker will show item name wrong and show a weird animation. Do not worry, item will be correct in the game.", "", "");
	
	setFieldNumericTextEditable(fp, 1001, 0);
	
	
	pokeBasicEditUpdateLabels(fp);
	FrmSetEventHandler(fp, pokeBasicEditEventHandler);
	proceed = 9000 == FrmDoDialog(fp);
	item = getFieldNumericText(fp, 1001);
	FrmDeleteForm(fp);

	if (proceed && item && item < sizeof(mItemList) / sizeof(*mItemList)) {
		
		if (useEventItemMethod) {
			getListItem(name, (void*)mItemList, item);
			stringToImg(itemName, 96, 16, name, false);
		
			itemData[6] = item;
			itemData[7] = item >> 8;
		}
		else {
		
			for (i = 0; i < 10; i++) {
				itemData[i * 4 + 0] = item;
				itemData[i * 4 + 1] = item >> 8;
			}
		}
		
		WinDrawChars("READY TO SEND", 13, 0, 0);

		comms = commsOpen();
		if (!comms)
			FrmCustomAlert(ALERT_ID_ERROR, "Cannot open comms", "", "");
		else {
			
			while(1) {
				
				bool established = true;
				
				while (!commsTryAcceptWalkerConnection(comms)) {
					if (KeyCurrentState()) {
						established = false;
						break;
					}
				}
				
				if (!established)
					break;
				
				if (useEventItemMethod) {
					
					if (!commsEepromWrite(comms, itemData, 0xbd40, 8))
						FrmCustomAlert(ALERT_ID_ERROR, "Cannot write item basic info", "", "");
					else if (!commsEepromWrite(comms, itemName, 0xbd48,  0x180))
						FrmCustomAlert(ALERT_ID_ERROR, "Cannot write item image", "", "");
					else if (!commsEventItemRxed(comms))
						FrmCustomAlert(ALERT_ID_ERROR, "Cannot trigger event", "", "");
					else {
						FrmCustomAlert(ALERT_ID_INFO, "SUCCESS", "", "");
						break;
					}
					commsDisconnect(comms);
				}
				else {
					
					if (!commsEepromWrite(comms, itemData, 0xcec8, 40))
						FrmCustomAlert(ALERT_ID_ERROR, "Cannot write items basic info", "", "");
					else {
						struct PokePacket pkt;
						
						pkt.cmd = 0x16;
						pkt.detail = 1;
						
						commsPacketRawTx(comms, &pkt, NULL, 0);

						commsDisconnect(comms);
						FrmCustomAlert(ALERT_ID_INFO, "SUCCESS", "", "");
						break;
					}
				}
			}
			commsClose(comms);
		}
		
		MemPtrFree(itemName);
	}
}

static void manyWatts(void)
{
	struct Comms *comms;
	int32_t item;
	char name[32];
	
		
	WinDrawChars("READY TO SEND", 13, 0, 0);

	comms = commsOpen();
	if (!comms)
		FrmCustomAlert(ALERT_ID_ERROR, "Cannot open comms", "", "");
	else {
		
		while(1) {
			
			struct PokePacket pkt;
			bool established = true;
			
			while (!commsTryAcceptWalkerConnection(comms)) {
				if (KeyCurrentState()) {
					established = false;
					break;
				}
			}
			
			if (!established)
				break;
			
			
			pkt.cmd = 6;
			pkt.detail = 0xf9;
			
			if (!commsPacketRawTx(comms, &pkt, add_watts_expoint_upload_to_0xF956, sizeof(add_watts_expoint_upload_to_0xF956)))
				FrmCustomAlert(ALERT_ID_ERROR, "cannot tx exploit", NULL, NULL);
			
			if (commsPacketRawRx(comms, &pkt, NULL, 0))
				FrmCustomAlert(ALERT_ID_ERROR, "cannot get ack for exploit", NULL, NULL);
			
			pkt.cmd = 6;
			pkt.detail = 0xf7;
			
			if (!commsPacketRawTx(comms, &pkt, set_high_stepcount_exploit_upload_to_0xF79C, sizeof(set_high_stepcount_exploit_upload_to_0xF79C)))
				FrmCustomAlert(ALERT_ID_ERROR, "cannot tx stepcount", NULL, NULL);
			
			if (commsPacketRawRx(comms, &pkt, NULL, 0))
				FrmCustomAlert(ALERT_ID_ERROR, "cannot get ack for stepcount", NULL, NULL);
			
			
			pkt.cmd = 6;
			pkt.detail = 0xf7;
			
			if (!commsPacketRawTx(comms, &pkt, trigger_uploaded_code_upload_to_0xF7E0, sizeof(trigger_uploaded_code_upload_to_0xF7E0)))
				FrmCustomAlert(ALERT_ID_ERROR, "cannot tx trigger", NULL, NULL);
			
			if (commsPacketRawRx(comms, &pkt, NULL, 0))
				FrmCustomAlert(ALERT_ID_ERROR, "cannot get ack for trigger", NULL, NULL);
			
			
			FrmCustomAlert(ALERT_ID_INFO, "SUCCESS", "", "");
			break;
			
		}
		commsClose(comms);
	}
}

static void UI(void)
{
	FormPtr fp = FrmInitForm(1001);
	UInt16 i;
	
	while (1) switch (FrmDoDialog(fp)) {
		case 1000:
			eventPoke();
			break;
		
		case 1001:
			itemGift(true);
			break;
		
		case 1002:
			itemGift(false);
			break;
		
		case 1003:
			romDump();
			break;
		
		case 1004:
			manyWatts();
			break;
		
		default:
			FrmDeleteForm(fp);
			return;
	}
}

uint32_t __Startup__(void)
{
	SysAppInfoPtr appInfoP;
	void *prevGlobalsP;
	void *globalsP;
	
	SysAppStartup(&appInfoP, &prevGlobalsP, &globalsP);
	if (appInfoP->cmd == sysAppLaunchCmdNormalLaunch)
		UI();
	SysAppExit(appInfoP, prevGlobalsP, globalsP);
	
	return 0;
}