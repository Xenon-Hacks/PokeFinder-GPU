import struct

from .embed_util import write_data

# X/Y wild encounter data from PKHeX (Resources/legality/wild/Gen6/encounter_x.pkl, encounter_y.pkl). Each area is
# location u16, type u8, pad u8, then 4-byte slots (species | form << 11, min level, max level). Type 0 areas hold the
# game's 79-slot zone table in order: 12 grass, 12 yellow flowers, 12 purple flowers, 12 red flowers, 12 rough terrain,
# 5 surf, 5 rock smash, 3 old rod, 3 good rod, 3 super rod. Type 7 areas are the 3 hordes x 5 members, type 1 areas are
# the ambush spots (bushes, berry trees, cave shadows and drops).
#
# OR/AS data (encounter_or.pkl, encounter_as.pkl) has the same area format with the ORAS zone table: type 0 areas hold
# 12 grass, 12 long grass, 3 DexNav-only slots, 5 surf, 3 old rod, 3 good rod, 3 super rod; type 6 areas are the 5 Rock Smash
# slots and type 7 areas the 3 hordes x 5 members.

# Encounter6 enum order
GRASS, YELLOW, PURPLE, RED, ROUGH, SURF, ROCK_SMASH, OLD_ROD, GOOD_ROD, SUPER_ROD, HORDE_HONEY, HORDE_SWEET_SCENT, AMBUSH, SAFARI,     TALL_GRASS = range(15)
DEXNAV = None  # DexNav-only slots, not a wild encounter the tab generates

ZONE_LAYOUT = (
    (GRASS, 12), (YELLOW, 12), (PURPLE, 12), (RED, 12), (ROUGH, 12), (SURF, 5), (ROCK_SMASH, 5), (OLD_ROD, 3), (GOOD_ROD, 3),
    (SUPER_ROD, 3))

# Friend Safari species (PKHeX EncounterArea6XY), level 30; Floette forms 0/1/3 and Vivillon
FRIEND_SAFARI = (
    2, 5, 8, 12, 14, 16, 21, 25, 27, 35, 38, 39, 43, 44, 46, 49, 51, 56, 58, 61, 63, 67, 77, 82, 83, 84, 87, 89, 91, 95, 96, 98, 101,
    105, 112, 113, 114, 125, 126, 127, 130, 131, 132, 133, 148, 163, 165, 168, 175, 178, 184, 190, 191, 194, 195, 202, 203, 205, 206,
    209, 213, 214, 215, 216, 218, 219, 221, 222, 224, 225, 227, 231, 235, 236, 247, 262, 267, 268, 274, 281, 284, 286, 290, 294, 297,
    299, 302, 303, 307, 310, 313, 314, 317, 323, 326, 328, 332, 336, 342, 352, 353, 356, 357, 359, 361, 363, 372, 375, 400, 404, 415,
    417, 419, 423, 426, 437, 442, 444, 447, 452, 454, 459, 506, 510, 511, 513, 515, 517, 520, 523, 525, 527, 530, 531, 536, 538, 539,
    541, 544, 548, 551, 556, 557, 561, 569, 572, 575, 578, 581, 586, 587, 596, 597, 600, 608, 611, 614, 618, 619, 621, 623, 624, 627,
    629, 636, 651, 654, 657, 660, 662, 668, 673, 674, 677, 682, 684, 686, 689, 694, 701, 702, 705, 707, 708, 710, 712, 714)
FRIEND_SAFARI_LOCATION = 148

ZONE_LAYOUT_ORAS = (
    (GRASS, 12), (TALL_GRASS, 12), (DEXNAV, 3), (SURF, 5), (OLD_ROD, 3), (GOOD_ROD, 3), (SUPER_ROD, 3))


def _read_areas(path: str):
    with open(path, "rb") as f:
        data = f.read()

    count = struct.unpack_from("<H", data, 2)[0]
    offsets = [struct.unpack_from("<I", data, 4 + 4 * i)[0] for i in range(count + 1)]

    areas = []
    for i in range(count):
        area = data[offsets[i]:offsets[i + 1]]
        location, type = struct.unpack_from("<HB", area, 0)
        slots = []
        for offset in range(4, len(area), 4):
            specie, min, max = struct.unpack_from("<HBB", area, offset)
            slots.append((specie, min, max))
        areas.append((location, type, slots))
    return areas


def _split(areas, oras=False):
    """Groups slots by (encounter type, location); the slot order is kept, duplicates merged into one level range"""
    tables = {}

    def add(type, location, slots):
        table = tables.setdefault((type, location), [])
        for specie, min, max in slots:
            if specie == 0:
                continue
            for i, (s, lo, hi) in enumerate(table):
                if s == specie:
                    table[i] = (s, builtins_min(lo, min), builtins_max(hi, max))
                    break
            else:
                table.append((specie, min, max))

    def add_horde(location, slots):
        table = tables.setdefault((HORDE_HONEY, location), [])
        for horde in range(3):
            members = slots[horde * 5:horde * 5 + 5]
            if all(specie == 0 for specie, _, _ in members):
                continue
            if members not in table:
                table.append(members)

    for location, type, slots in areas:
        if type == 0:
            index = 0
            for encounter, size in (ZONE_LAYOUT_ORAS if oras else ZONE_LAYOUT):
                if encounter is not None:
                    add(encounter, location, slots[index:index + size])
                index += size
        elif type == 7:
            add_horde(location, slots)
        elif type == 1 and not oras:
            add(AMBUSH, location, slots)
        elif type == 6 and oras:
            add(ROCK_SMASH, location, slots)

    if oras:
        return tables

    tables[(SAFARI, FRIEND_SAFARI_LOCATION)] = [(specie, 30, 30) for specie in FRIEND_SAFARI] + [
        (670 | (1 << 11), 30, 30), (670 | (3 << 11), 30, 30)]
    return tables


builtins_min = min
builtins_max = max


def _blob(tables):
    """u16 area count, then per area: u16 location, u8 type, u8 slot count, slots (u16 species | form << 11, u8 min, u8 max).
    Horde areas list 5 slots per horde."""
    data = bytearray()
    keys = sorted(key for key, table in tables.items() if table)
    data += struct.pack("<H", len(keys))
    for key in keys:
        type, location = key
        table = tables[key]
        slots = []
        if type == HORDE_HONEY:
            for horde in table:
                slots.extend(horde)
        else:
            slots = table
        data += struct.pack("<HBB", location, type, len(slots))
        for specie, min, max in slots:
            data += struct.pack("<HBB", specie, min, max)
    return bytes(data)


def _array(name: str, blob: bytes):
    string = f"constexpr std::array<u8, {len(blob)}> {name} = {{ "
    string += ", ".join(str(b) for b in blob)
    string += " };"
    return string


def embed_encounters6(parent_dir: str, output_dir: str):
    arrays = []
    for game in ("x", "y", "or", "as"):
        areas = _read_areas(f"{parent_dir}/Encounters6/encounter_{game}.bin")
        arrays.append(_array(f"ENCOUNTERS_{game.upper()}", _blob(_split(areas, game in ("or", "as")))))

    write_data(arrays, f"{output_dir}/EncounterData6.hpp", ("Core/Global.hpp", "array"))
