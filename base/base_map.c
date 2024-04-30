static u64 map_hash(u64 h) {
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53;
    h ^= h >> 33;
    return h;
}

static bool map_create(map *map, u64 cap) {
    zero_struct(map);
    u64 *keys = heap_alloc(cap * sizeof(u64));
    u64 *vals = heap_alloc(cap * sizeof(u64));

    bool ret = keys && vals;
    if (ret) {
        memset(keys, 0, cap * sizeof(u64));
        memset(vals, 0, cap * sizeof(u64));

        map->keys  = keys;
        map->vals  = vals;
        map->cap   = cap;
        map->count = 0;
    } else {
        map_destroy(map);
    }

    return ret;
}

static void map_destroy(map *map) {
    heap_free(map->keys);
    heap_free(map->vals);
    zero_struct(map);
}

static void map_rehash(map *src, map *dst) {
    for_n (u64, old_idx, dst->cap) {
        map_put(src, dst->keys[old_idx], dst->vals[old_idx]);
    }
}

static i64 map_lookup(map *map, u64 key) {
    i64 ret = -1;
    u64 hash = map_hash(key);
    for_n (u64, probe, map->cap) { // @Todo Non-linear probe.
        u64 idx = (hash + probe) % map->cap;
        if ((map->keys[idx] == 0) || (map->keys[idx] == key)) {
            ret = idx;
            break;
        }
    }
    return ret;
}

static bool map_put(map *map, u64 key, u64 val) {
    assert(key != 0); // NOTE(rune): Key 0 is reserved for empty slot.
    bool ret = false;
    if (key != 0) {
        if ((map->cap - map->count) <= (map->cap / 4)) {
            struct map expanded;
            if (map_create(&expanded, map->cap * 2)) {
                map_rehash(&expanded, map);
                map_destroy(map);
                *map = expanded;
            }
        }

        i64 idx = map_lookup(map, key);
        if (idx != -1) {
            if (map->keys[idx] == 0) {
                map->keys[idx] = key;
                map->count++;
            }

            map->vals[idx] = val;
            ret = true;
        }
    }

    return ret;
}

static bool map_remove(map *map, u64 key) {
    bool ret = false;
    i64 idx = map_lookup(map, key);
    if (idx != -1) {
        if (map->keys[idx] != 0) {
            map->keys[idx] = 0;
            map->count--;
        }

        ret = true;
    }
    return ret;
}

static bool map_get(map *map, u64 key, u64 *val) {
    bool ret = false;
    i64 slot = map_lookup(map, key);
    if (slot != -1 && map->keys[slot] != 0) {
        *val = map->vals[slot];
        ret = true;
    }
    return ret;
}
