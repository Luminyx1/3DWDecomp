#include "System/IslandDataList.hpp"

/**
 * @brief Convert or classify an island identifier.
 * @param islandId Island identifier; ocean quadrants use -1 through -4.
 * @return Quadrant index, defaulting to zero for ordinary islands.
 */
int IslandDataFunction::getQuadrantIndexFromIslandID(int islandId) {
    if (static_cast<unsigned int>(islandId + 4) <= 2) {
        return ~islandId;
    }
    return 0;
}

/**
 * @brief Convert or classify an island identifier.
 * @param quadrant Quadrant index from 0 through 3; other values return zero.
 * @return The negative ocean pseudo-island identifier.
 */
int IslandDataFunction::getIslandIDFromQuadrantIndex(int quadrant) {
    if (static_cast<unsigned int>(quadrant) <= 3) {
        return ~quadrant;
    }
    return 0;
}

/**
 * @brief Convert or classify an island identifier.
 * @param value Placement parameter from 1 through 4; other values return zero.
 * @return The negative pseudo-island identifier.
 */
int IslandDataFunction::getIslandIDFromParam(int value) {
    if (static_cast<unsigned int>(value - 1) <= 3) {
        return -value;
    }
    return 0;
}

/**
 * @brief Convert or classify an island identifier.
 * @param value Placement parameter selecting one of four quadrants.
 * @return The zero-based quadrant index.
 */
int IslandDataFunction::getQuadrantIndexFromParam(int value) {
    if (static_cast<unsigned int>(value - 2) <= 2) {
        return value - 1;
    }
    return 0;
}

/**
 * @brief Convert or classify an island identifier.
 * @param islandId Island identifier to test.
 * @return True for island identifiers 14 through 16.
 */
bool IslandDataFunction::isGigaBellIsland(int islandId) {
    return static_cast<unsigned int>(islandId - 14) < 3;
}
