
s32 bonk_or_hit_lava_wall(struct MarioState *m, struct WallCollisionData *wallData) {
    s16 i;
    s16 wallDYaw;
    s32 oldWallDYaw;
    s32 result = AIR_STEP_NONE;

    if (m->wall != NULL) {
        oldWallDYaw = abs_angle_diff(m->wallYaw, m->faceAngle[1]);
    } else {
        oldWallDYaw = 0x0;
    }

    for (i = 0; i < wallData->numWalls; i++) {
        if (wallData->walls[i] != NULL) {
            if (wallData->walls[i]->type == SURFACE_BURNING) {
                set_mario_wall(m, wallData->walls[i]);
                return AIR_STEP_HIT_LAVA_WALL;
            }

            // Update wall reference (bonked wall) only if the new wall has a better facing angle
            wallDYaw = abs_angle_diff(SURFACE_YAW(wallData->walls[i]), m->faceAngle[1]);
            if (wallDYaw > oldWallDYaw) {
                oldWallDYaw = wallDYaw;
                set_mario_wall(m, wallData->walls[i]);

                if (wallDYaw > DEGREES(180 - WALL_KICK_DEGREES)) {
                    m->flags |= MARIO_AIR_HIT_WALL;
                    result = AIR_STEP_HIT_WALL;
                }
            }
        }
    }

    return result;
}
