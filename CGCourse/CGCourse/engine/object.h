

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <typeindex>
#include <vector>

#include "components/component.h"

namespace SE {
    class Object {
    public:
        Object(std::string name);
        Object();
        ~Object(void);

        void addChild(std::unique_ptr<Object> child);

        template <class T>
        inline void addComponent(std::unique_ptr<T> component) {
            component->setParent(this);
            componentsByTypeid[typeid(T)].push_back(component);
            components.push_back(component);
        }


        std::vector<std::unique_ptr<Object>>& getChildren();
        std::vector<std::unique_ptr<Component>>& getComponents();

        Mathgl::Mat4& getWorldMatrix(void);

        Mathgl::Vec3 getPosition(void);
        Mathgl::Vec4 getDirection(void);

        template <class T>
        inline std::reference_wrapper<std::vector<std::unique_ptr<T>>> getComponentsByType() {
            auto i = componentsByTypeid.find(typeid(T));
            if (i == componentsByTypeid.end()) {
                return std::vector<std::shared_ptr<T>>();
            }
            else {
                auto vec = i->second;

                std::vector<std::shared_ptr<T>> target(vec.size());
                std::transform(vec.begin(), vec.end(), target.begin(), [](std::shared_ptr<Component> t) {
                    return std::dynamic_pointer_cast<T>(t);
                });
                return target;
            }
        }

        template <class T>
        inline std::shared_ptr<T> getComponent(void) {
            auto i = componentsByTypeid.find(typeid(T));
            if (i == componentsByTypeid.end()) {
                return nullptr;
            }
            else {
                auto vec = i->second;
                if (vec.size() > 0) {
                    return std::dynamic_pointer_cast<T>(vec[0]);
                }
                else {
                    return nullptr;
                }
            }
        }

        static std::vector<std::reference_wrapper<std::unique_ptr<Object>>> findByTag(const std::string& tag);
    private:
        
        Object* parentEntity;

        std::vector<std::unique_ptr<Object>> children;
        std::vector<std::shared_ptr<Component>> components;

        Mathgl::Mat4 worldMatrix;

        std::string name;


        static std::map<std::string, std::vector<Object*>> taggedEntities;

        std::map<std::type_index, std::vector<std::shared_ptr<Component>>> componentsByTypeid;
    };
};
