#include "object.h"

using namespace SE;

Object::Object(std::string name): name(name) {
    //setTag(this, tag);
    parentEntity = nullptr;
}

Object::Object(void) {
    parentEntity = nullptr;
}

Object::~Object(void) {
    if (!name.empty()) {
        auto taggedEntitiesVec = &Object::taggedEntities[name];
        taggedEntitiesVec->erase(std::remove(taggedEntitiesVec->begin(), taggedEntitiesVec->end(), this), taggedEntitiesVec->end());
    }
}