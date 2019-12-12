#include <assert.h>
#include "track_events.h"


track_elem_t powerup_arr[NUM_POWERUPS] =  
    {{0, {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 
             {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}},
    {1, {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 
            {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}}};

track_elem_t hazard_arr[NUM_HAZARDS] = 
    {{2, {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, 
             {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}}};

/*
 * Helper function to verify that all uuids are unique and that all are < 8.
 */
void validate_uuids() {
    uint8_t used_bits = 0;
    size_t i;
    for (i = 0; i < NUM_POWERUPS; i++) {
        assert(powerup_arr[i].uuid < 8);
        assert((used_bits & (1 << powerup_arr[i].uuid)) == 0);
        used_bits |= (1 << powerup_arr[i].uuid);
    }
    for (i = 0; i < NUM_HAZARDS; i++) {
        assert(hazard_arr[i].uuid < 8);
        assert((used_bits & (1 << hazard_arr[i].uuid)) == 0);
        used_bits |= (1 << hazard_arr[i].uuid);
    }
}
