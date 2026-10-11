#pragma once
class StageDatabaseInfo;
// Interface used by the course flag; the miniature's layout is not yet recovered.
class CourseSelectMiniature {
public:
    int getCourseId() const;
    const StageDatabaseInfo* getStageInfo() const;
};
