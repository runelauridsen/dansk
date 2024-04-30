typedef struct map map;
struct map {
    u64 *keys;
    u64 *vals;
    u64 count;
    u64 cap;
};

static bool map_create(map *map, u64 cap);
static void map_destroy(map *map);
static bool map_get(map *map, u64 key, u64 *val);
static bool map_put(map *map, u64 key, u64 val);
static bool map_remove(map *map, u64 key);
