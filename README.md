# Better SM64 Collision

Patcher for sm64 romhacks to get an improved collision system from HackerSM64 with extra features/configurations. Main goals are:
* Have a quicker collision engine, fix the lag in binary ROMs
* More precision than original extended boundaries to fix bizarre collision issues
* No 'collision RNG' - collision engine does not depend on order of triangles stored in the collision list
* Configurability to support all collision patches, for example Redmat walls

## Features

Due to replacing a significant chunk of collision engine, several collision related patches will no longer function. Tune the configuration via UI tool.

| Name | Description |
| - | - |
| **Wallkick Angle** | Tune the angle threshold at which wallkicks activate |
| **Quarter Steps** | Adjust how many sub-steps the collision system uses per frame. More steps = more precise collision but slightly higher performance cost |
| **Floor-Ceil/Wall Threshold** | Controls which extremely slopes are considered walls. If you use imprecise collision, it is recommended to use higher values like 0.05, otherwise 0.01 |
| **Wide Wallkick Surface Patch** | Enable the 89 degree wallkick collision patch for 'Redmat' walls |
| **Imprecise Collision Compatibility** | Simulate the behavior of the original extended boundaries patch; useful for existing hacks that may expect "wrong" angles |
| **No surface/wall cucking** | All surfaces are treated as equally based on 'best' pick instead of 'first' pick.  |

## Quick Start

1. Download the [latest release](https://github.com/aglab2/SM64CollisionOptimizer/releases/latest)
2. Run the tool
3. Click **Open ROM** and select your SM64 ROM file
4. Adjust the collision settings to your liking, likely defaults are fine
5. The patched ROM is saved as `<original_name>_patched.z64` in the same directory

## Technical Details

C code from HackerSM64 decomp is recompiled into binary and linked in place to replace majority of surface collision + qsteps routines. 

### Cell compression

2 main tricks are used to allow to store 4x level boundaries much faster:
1) Instead of reducing each coordinate by 4 to have 4x collision boundaries, instead collision triangles are not scaled down but cells are reinterpreted to store the data for 2x cell size. Same approach is used in HackerSM64. This allows to have 2x boundaries.
2) To store 4x boundaries, 4x amount of cells is gained from using slim pointers (`u16` with offset surface nodes). Original cell is using a full `struct Surface`, we can just store an offset to `struct SurfaceNode` instead giving us 4x space.

### Floor collision

Floors are picked based on the height instead of the first one. This is slower lookup that picking the first floor but it is independent of triangle store order. Generally this will cause more cost during lookups but this cost is amortised by compilation with better GCC flags.

### Wall collision

Implementation follows idea from Iwer Sonsch. There are no rounded corners anymore, instead original approach is used. To decide the 'best' wall, distance to the wall is used selecting the closest wall. 'Back' wall collisions are deprioritised. This allows to have corners not lock up Mario but snap to the side which is closer.
