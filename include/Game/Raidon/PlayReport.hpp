#pragma once

/**
 * @brief Game-side play report sender (partially reconstructed).
 */
class PlayReport {
  public:
    static PlayReport* getInstance();
    void Init();
};
