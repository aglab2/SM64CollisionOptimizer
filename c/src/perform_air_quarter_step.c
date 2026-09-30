
s32 perform_air_quarter_step(struct MarioState *m, Vec3f intendedPos, u32 stepArg) {
    s16 i;
    s32 stepResult = AIR_STEP_NONE;

    Vec3f nextPos, ledgePos;
    struct WallCollisionData upperWall, lowerWall;
    struct Surface *ceil, *floor, *ledgeFloor;
    struct Surface *grabbedWall = NULL;

    vec3f_copy(nextPos, intendedPos);

    resolve_and_return_wall_collisions(nextPos, 150.0f, 50.0f, &upperWall);
    resolve_and_return_wall_collisions(nextPos, 30.0f, 50.0f, &lowerWall);

    f32 floorHeight = find_floor(nextPos[0], nextPos[1], nextPos[2], &floor);
    f32 ceilHeight = find_mario_ceil(nextPos, floorHeight, &ceil);

    f32 waterLevel = find_water_level(nextPos[0], nextPos[2]);

    //! The water pseudo floor is not referenced when your intended qstep is
    // out of bounds, so it won't detect you as landing.

    if (floor == NULL) {
        if (nextPos[1] <= m->floorHeight) {
            m->pos[1] = m->floorHeight;
            return AIR_STEP_LANDED;
        }

        m->pos[1] = nextPos[1];
        return AIR_STEP_HIT_WALL;
    }

    if ((m->action & ACT_FLAG_RIDING_SHELL) && floorHeight < waterLevel) {
        floorHeight = waterLevel;
        floor = &gWaterSurfacePseudoFloor;
        floor->originOffset = -floorHeight;
    }

    //! This check uses f32, but findFloor uses short (overflow jumps)
    if (nextPos[1] <= floorHeight) {
        if (ceilHeight - floorHeight > 160.0f) {
            m->pos[0] = nextPos[0];
            m->pos[2] = nextPos[2];
            set_mario_floor(m, floor, floorHeight);
        }

        //! When ceilHeight - floorHeight <= 160, the step result says that
        // Mario landed, but his movement is cancelled and his referenced floor
        // isn't updated (pedro spots)
        m->pos[1] = floorHeight;
        return AIR_STEP_LANDED;
    }

    if (nextPos[1] + 160.0f > ceilHeight) {
        if (m->vel[1] >= 0.0f) {
            m->vel[1] = 0.0f;

#ifdef HANGING_FIX
            // Grab ceiling unless they just were grabbing a ceiling
            if (!(m->prevAction & ACT_FLAG_HANGING) && ceil != NULL && ceil->type == SURFACE_HANGABLE) {
#else
            if ((stepArg & AIR_STEP_CHECK_HANG) && ceil != NULL && ceil->type == SURFACE_HANGABLE) {
#endif
                return AIR_STEP_GRABBED_CEILING;
            }

            return AIR_STEP_NONE;
        }

        //! Potential subframe downwarp->upwarp?
        if (nextPos[1] <= m->floorHeight) {
            m->pos[1] = m->floorHeight;
            return AIR_STEP_LANDED;
        }

        m->pos[1] = nextPos[1];
        return AIR_STEP_HIT_CEILING;
    }

    //! When the wall is not completely vertical or there is a slight wall
    // misalignment, you can activate these conditions in unexpected situations

    if ((stepArg & AIR_STEP_CHECK_LEDGE_GRAB) && upperWall.numWalls == 0 && lowerWall.numWalls != 0) {
        for (i = 0; i < lowerWall.numWalls; i++) {
            grabbedWall = check_ledge_grab(m, grabbedWall, lowerWall.walls[i], intendedPos, nextPos, ledgePos, &ledgeFloor);
            if (grabbedWall != NULL) {
                stepResult = AIR_STEP_GRABBED_LEDGE;
            }
        }

        if (stepResult == AIR_STEP_GRABBED_LEDGE && grabbedWall != NULL && ledgeFloor != NULL) {
            vec3f_copy(m->pos, ledgePos);
            set_mario_floor(m, floor, ledgePos[1]);
            m->faceAngle[0] = 0x0;
            m->faceAngle[1] = SURFACE_YAW(grabbedWall) + 0x8000;
        } else {
            vec3f_copy(m->pos, nextPos);
            set_mario_floor(m, floor, floorHeight);
        }
        return stepResult;
    }

    vec3f_copy(m->pos, nextPos);
    set_mario_floor(m, floor, floorHeight);

    if (upperWall.numWalls > 0) {
        stepResult  = bonk_or_hit_lava_wall(m, &upperWall);
        if (stepResult != AIR_STEP_NONE) {
            return stepResult;
        }
    }

    return (lowerWall.numWalls > 0) ? bonk_or_hit_lava_wall(m, &lowerWall) : AIR_STEP_NONE;
}
