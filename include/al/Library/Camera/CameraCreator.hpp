#pragma once

namespace al {
class CameraPoser;
class CameraPoserAnim;
class PlacementId;

class CameraCreator {
public:
    CameraPoser* createObjectCamera(const PlacementId& rId);
    CameraPoser* createProgramableCamera(const PlacementId& rId);
    CameraPoserAnim* createAnimCamera();
};

}  // namespace al
