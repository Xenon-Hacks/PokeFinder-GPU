import csv
import struct

from .embed_util import write_data

# Gen 7 wild encounter tables exported from the games' RomFS (a/0/8/2 Sun / Ultra Sun, a/0/8/3 Moon / Ultra Moon, file 9 + 11 x area,
# "EA" pack: day table 0x164 + night table 0x164) by the Gen 7 RNG research (results/*_encounters.tsv). Each area holds several
# encounter tables (the tile's encounter-data id picks one); each table has a level range and 10 slots with their rates
# (20/20/10/10/10/10/10/5/4/1 is the usual shape) for day and night.
#
# Blob: u16 area count, then per area: u16 area, u8 zone count, zones (u16 each), u8 table count, then per table: u8 table,
# per time (day, night): u8 min level, u8 max level, 10 x (u16 species | form << 11, u8 rate).

GAMES = ("moon", "sun", "us", "um")


def _species_ids(parent_dir: str):
    ids = {}
    with open(f"{parent_dir}/i18n/en/species_en.txt", "r", encoding="utf-8-sig") as f:
        for index, line in enumerate(f.read().split("\n")):
            if line:
                ids[line] = index + 1
    return ids


def _read(path: str, species_ids):
    areas = {}
    with open(path, "r", encoding="utf-8") as f:
        for row in csv.DictReader(f, delimiter="\t"):
            area = int(row["area"])
            zones = tuple(int(z) for z in row["zones"].split(",") if z)
            table = int(row["table"])
            time = 0 if row["time"] == "day" else 1
            entry = areas.setdefault(area, {"zones": zones, "tables": {}})
            slots = entry["tables"].setdefault(table, {})
            times = slots.setdefault(time, {"min": int(row["minlv"]), "max": int(row["maxlv"]), "slots": [None] * 10})
            specie = species_ids.get(row["species"], 0)  # "Egg" rows are empty slots
            times["slots"][int(row["slot"])] = (specie | (int(row["form"]) << 11), int(row["rate"]))
    return areas


def _blob(areas):
    data = bytearray()
    data += struct.pack("<H", len(areas))
    for area in sorted(areas):
        entry = areas[area]
        data += struct.pack("<HB", area, len(entry["zones"]))
        for zone in entry["zones"]:
            data += struct.pack("<H", zone)
        data += struct.pack("<B", len(entry["tables"]))
        for table in sorted(entry["tables"]):
            data += struct.pack("<B", table)
            for time in (0, 1):
                times = entry["tables"][table].get(time, {"min": 0, "max": 0, "slots": [None] * 10})
                data += struct.pack("<BB", times["min"], times["max"])
                for slot in times["slots"]:
                    specie, rate = slot if slot else (0, 0)
                    data += struct.pack("<HB", specie, rate)
    return bytes(data)


def _array(name: str, blob: bytes):
    string = f"constexpr std::array<u8, {len(blob)}> {name} = {{ "
    string += ", ".join(str(b) for b in blob)
    string += " };"
    return string


def embed_encounters7(parent_dir: str, output_dir: str):
    species_ids = _species_ids(parent_dir)
    arrays = []
    for game in GAMES:
        areas = _read(f"{parent_dir}/Gen7/encounters_{game}.tsv", species_ids)
        arrays.append(_array(f"ENCOUNTERS_{game.upper()}", _blob(areas)))

    write_data(arrays, f"{output_dir}/EncounterData7.hpp", ("Core/Global.hpp", "array"))
