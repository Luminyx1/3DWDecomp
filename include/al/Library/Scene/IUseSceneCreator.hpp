#pragma once

namespace al {
class SceneCreator;

class IUseSceneCreator {
public:
    virtual ~IUseSceneCreator() { ; }
    virtual SceneCreator* getSceneCreator() const = 0;
    virtual void setSceneCreator(SceneCreator* pSceneCreator) = 0;
};
}  // namespace al
