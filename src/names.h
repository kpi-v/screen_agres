#ifndef NAMESH
#define NAMESH

struct SCIENTIST
{
    const char *name;
    const char *welcoming;
    int group;
    uint8_t uid[2];
};

SCIENTIST scientists[] = {
    {"ALEX", "Que tal esteu?", 1, {0x08, 0x9d}},
    {"PEP", "Tenia ganes de voret", 1, {0x3e, 0x98}},
    {"JOAN MARIA", "IE TIOS",1, {0x3a, 0x9e}},
    {"JULEN", "hola com esteu", 1, {0x3b, 0x98}},
    {"ISONA", "ES LA MES GUAPA DEL MON MUNDIAL", 1, {0x72, 0x99}},
    {"Ferran", "M'agraden molt les estrascolars de Natxo", 2, {0x6c, 0x99}},
    {"Bruna", "hola, Bruna", 2, {0x75, 0x99}},
    {"Yago", "tinc que anar al bany",2, {0x47, 0x98}},
    {"Lola", "quintoooos 2025",2, {}},
    {"Laura", "toque el trombo",2},
    {"Paula", "SIMONA ES LA MILLOR MESTRA DEL MON",2, {0x78, 0x99}},
    {"LLUIS", "bona bespra ", 2, {0x41, 0x98}}
};

#endif // NAMESH