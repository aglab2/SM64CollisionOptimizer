s32 perform_ground_quarter_step(struct MarioState *m, Vec3f nextPos) {
    struct WallCollisionData lowerWall, upperWall;
    struct Surface *ceil, *floor;

    s16 i;
    s16 wallDYaw;
    s32 oldWallDYaw;

    resolve_and_return_wall_collisions(nextPos, 30.0f, 24.0f, &lowerWall);
    resolve_and_return_wall_collisions(nextPos, 60.0f, 50.0f, &upperWall);

    f32 floorHeight = find_floor(nextPos[0], nextPos[1], nextPos[2], &floor);
    f32 ceilHeight = find_mario_ceil(nextPos, floorHeight, &ceil);

    f32 waterLevel = find_water_level(nextPos[0], nextPos[2]);

    if (floor == NULL) {
        return GROUND_STEP_HIT_WALL_STOP_QSTEPS;
    }

    if ((m->action & ACT_FLAG_RIDING_SHELL) && floorHeight < waterLevel) {
        floorHeight = waterLevel;
        floor = &gWaterSurfacePseudoFloor;
        floor->originOffset = -floorHeight;
    }

    if (nextPos[1] > floorHeight + 100.0f) {
        if (nextPos[1] + 160.0f >= ceilHeight) {
            return GROUND_STEP_HIT_WALL_STOP_QSTEPS;
        }

        vec3f_copy(m->pos, nextPos);
        set_mario_floor(m, floor, floorHeight);
        return GROUND_STEP_LEFT_GROUND;
    }

    if (floorHeight + 160.0f >= ceilHeight) {
        return GROUND_STEP_HIT_WALL_STOP_QSTEPS;
    }

    vec3f_set(m->pos, nextPos[0], floorHeight, nextPos[2]);

    fail_warp_mario_set_safe_pos(m, floor);
    set_mario_floor(m, floor, floorHeight);

    if (m->wall != NULL) {
        oldWallDYaw = abs_angle_diff(m->wallYaw, m->faceAngle[1]);
    } else {
        oldWallDYaw = 0x0;
    }
    for (i = 0; i < upperWall.numWalls; i++) {
        wallDYaw = abs_angle_diff(SURFACE_YAW(upperWall.walls[i]), m->faceAngle[1]);
        if (wallDYaw > oldWallDYaw) {
            oldWallDYaw = wallDYaw;
            set_mario_wall(m, upperWall.walls[i]);
        }

        if (wallDYaw >= DEGREES(60) && wallDYaw <= DEGREES(120)) {
            continue;
        }

        return GROUND_STEP_HIT_WALL_CONTINUE_QSTEPS;
    }

    return GROUND_STEP_NONE;
}
