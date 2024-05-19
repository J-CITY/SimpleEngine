#pragma once
#include <functional>

#include "mathModule/math.h"

namespace IKIGAI::REFLECTION {

	class FunctionBase {
	public:
		virtual ~FunctionBase() = default;
	};

	// the Function is represented as mem_fn
	template <typename ReturnType, typename Type, typename... Params>
	class Function final : public FunctionBase {
		using FunctionType = std::_Mem_fn<ReturnType(Type::*)(Params...)>;

	public:
		Function(const FunctionType& _function) : m_function(_function) {}
		~Function() = default;

		ReturnType operator()(Type& _object, Params... _parameters) {
			return m_function(_object, _parameters...);
		}

	private:
		FunctionType m_function;
	};

	class Method {
	public:
		Method(const char* _name) : m_name(_name) {}
		const char* getName() const { return m_name; }

		template <typename ReturnType, typename Type, typename... Params>
		void setFunction(
			const std::_Mem_fn<ReturnType(Type::*)(Params...)>& _function) {
			m_function =
				std::make_shared<Function<ReturnType, Type, Params...>>(_function);
		}

		template <typename ReturnType, typename Type, typename... Params>
		ReturnType call(Type& _object, Params... _params) const {
			if (auto function =
				std::dynamic_pointer_cast<Function<ReturnType, Type, Params...>>(
				m_function))
				return (*function)(_object, _params...);
			else
				throw std::invalid_argument("wrong parameter to the method!");
		}

	private:
		std::shared_ptr<FunctionBase> m_function;
		const char* m_name;
	};

	class Member {
	public:
		Member(const char* _name, size_t _offset)
			: m_getter(nullptr),
			m_setter(nullptr),
			m_name(_name),
			m_offset(_offset) {
		}

		// no setters implemented as they are not needed
		// as there is will be no member name or offset change
		const char* getName() const { return m_name; }
		size_t getOffset() const { return m_offset; }

		void setSetterMethod(const Method& _setter) {
			m_setter = std::make_shared<Method>(_setter);
		}
		void setGetterMethod(const Method& _getter) {
			m_getter = std::make_shared<Method>(_getter);
		}

		const std::shared_ptr<Method> getGetterMethod() const { return m_getter; }
		const std::shared_ptr<Method> getSetterMethod() const { return m_setter; }

	private:
		std::shared_ptr<Method> m_getter;
		std::shared_ptr<Method> m_setter;
		const char* m_name;
		size_t m_offset;
	};

	class Metadata {
	public:
		Metadata(const char* _name, size_t _size, const Metadata* _parent = nullptr,
			std::initializer_list<Member> _members = {},
			std::initializer_list<Method> _methods = {})
			: m_name(_name),
			m_size(_size),
			m_parent(_parent),
			m_members(_members),
			m_methods(_methods) {
		}

		void addMember(Member _member) { m_members.push_back(_member); }
		void addMethod(Method _method) { m_methods.push_back(_method); }

		bool hasMember(const std::string& _memberName) const {
			return findInRangeByName(m_members.begin(), m_members.end(),
				_memberName.c_str()) != m_members.end();
		}

		bool hasMethod(const std::string& _methodName) const {
			return findInRangeByName(m_methods.begin(), m_methods.end(),
				_methodName.c_str()) != m_methods.end();
		}

		// throws invalid_argument exception in case the member name is not found
		const Member& getMember(const std::string& _memberName) const {
			auto itr = findInRangeByName(m_members.begin(), m_members.end(),
				_memberName.c_str());
			if (itr == m_members.end())
				throw std::invalid_argument("member name not found");
			return *itr;
		}

		// throws invalid_argument exception in case the method name is not found
		const Method& getMethod(const std::string& _methodName) const {
			auto itr = findInRangeByName(m_methods.begin(), m_methods.end(),
				_methodName.c_str());
			if (itr == m_methods.end())
				throw std::invalid_argument("method name not found");
			return *itr;
		}

		const std::vector<Member>& getMembers() const { return m_members; }
		const std::vector<Method>& getMethods() const { return m_methods; }
		const Metadata* getParent() const { return m_parent; }
		const char* getName() const { return m_name; }
		size_t getSize() const { return m_size; }

	private:
		// search in a range defined by _begin and _end for item that
		// has a name == _nameToFind
		template <typename ItrType>
		ItrType findInRangeByName(const ItrType& _begin, const ItrType& _end,
			const char* _nameToFind) const {
			return std::find_if(_begin, _end, [&](const auto& var) {
				return strcmp(_nameToFind, var.getName()) == 0;
				});
		}

		const char* m_name;
		size_t m_size;
		const Metadata* m_parent;
		std::vector<Member> m_members;
		std::vector<Method> m_methods;
	};

	template <typename T>
	struct MetadataCreator {
		static Metadata* get() {
			static Metadata* metadata = T::initMetadata();
			static bool firstTime = true;
			if (firstTime) {
				firstTime = false;
				T::initMetadata();
			}
			return metadata;
		}
	};

	template <typename T>
	using remove_qualifiers_t = std::decay_t<T>;

#define HAS_FUNCTION(functionName)                                     \
  template <typename T>                                                \
  struct has_##functionName {                                          \
   private:                                                            \
    template <typename C>                                              \
    static short test(decltype(&C::functionName));                     \
    template <typename C>                                              \
    static int test(...);                                              \
                                                                       \
   public:                                                             \
    static constexpr bool value = sizeof(test<T>(0)) == sizeof(short); \
  };                                                                   \
  template <typename T>                                                \
  constexpr bool has_##functionName##_v = has_##functionName<T>::value;

	// this is a heler class to be used to call a certain function
	// only if it exists in the input class
#define CALL_FUNCTION(functionName)                \
  template <typename T, bool has_function>         \
  struct call_##functionName {                     \
    static void call() {                           \
      if                                           \
        constexpr(has_function) T::functionName(); \
    }                                              \
  };

	HAS_FUNCTION(defineProperties);
	HAS_FUNCTION(defineMethods);

	CALL_FUNCTION(defineProperties);
	CALL_FUNCTION(defineMethods);

	class ReflectionManager {
	public:
		void registerType(const char* _name, Metadata* _metadata) {
			m_typeMap[_name] = _metadata;
		}

		Metadata* getMetadataByName(const char* _name) {
			if (m_typeMap.find(_name) != m_typeMap.end()) return m_typeMap[_name];
			return nullptr;
		}

		Metadata* getMetadataByName(const std::string& _name) const {
			return getMetadataByName(_name.c_str());
		}

		static ReflectionManager& getInstance() {
			static ReflectionManager manager;
			return manager;
		}

		template <typename T>
		static auto getMetadata() {
			return MetadataCreator<remove_qualifiers_t<T>>::get();
		}

		template <typename T>
		static auto getMetadata(T _object) {
			if
				constexpr (std::is_pointer<T>::value) return _object->getMetadata();
			else
				return _object.getMetadata();
		}

		template <typename T>
		static auto getClassName() {
			return MetadataCreator<remove_qualifiers_t<T>>::get()->getName();
		}

		template <typename T>
		static auto getClassSize() {
			return MetadataCreator<remove_qualifiers_t<T>>::get()->getSize();
		}

		template <typename T>
		static auto getClassParent() {
			return MetadataCreator<remove_qualifiers_t<T>>::get()->getParent();
		}

		template <typename T>
		static auto getClassName(const T& _object) {
			return getClassName<remove_qualifiers_t<T>>();
		}

		template <typename T>
		static auto getClassSize(const T& _object) {
			return getClassSize<remove_qualifiers_t<T>>();
		}

		template <typename T>
		static auto getClassParent(const T& _object) {
			return getClassParent<remove_qualifiers_t<T>>();
		}

		template <typename Base, typename Parent>
		static bool doesInheritFrom() {
			auto parentMetadata = MetadataCreator<remove_qualifiers_t<Parent>>::get();
			auto metadata = const_cast<Metadata*>(
				MetadataCreator<remove_qualifiers_t<Base>>::get());

			while (metadata && (metadata != parentMetadata)) {
				metadata = const_cast<Metadata*>(metadata->getParent());
			}
			return metadata == parentMetadata;
		}

		template <typename CastToType, typename T>
		static CastToType* dynamicCast(T* _object) {
			if (doesInheritFrom<CastToType, T>())
				return static_cast<CastToType>(_object);
			return nullptr;
		}

	private:
		std::unordered_map<const char*, Metadata*> m_typeMap;
	};

#define INIT_METADATA(className)                                       \
  static auto initMetadata() {                                         \
    static bool firstTime = true;                                      \
    if (firstTime) {                                                   \
      firstTime = false;                                               \
      return new Metadata(#className, sizeof(className));              \
    }                                                                  \
    call_defineProperties<Self, has_defineProperties_v<Self>>::call(); \
    call_defineMethods<Self, has_defineMethods_v<Self>>::call();       \
    ReflectionManager::getInstance().registerType(#className, MetadataCreator<Self>::get());\
    return MetadataCreator<Self>::get();                               \
  }


#define REFLECT_CLASS(className)           \
 public:                                   \
  using Self = className;                  \
  INIT_METADATA(className)                 \
                                           \
  inline virtual Metadata* getMetadata() { \
    return MetadataCreator<Self>::get();   \
  }                                        \
                                           \
 private:


#define PROPERTIES_BEGIN           \
 public:                           \
  static void defineProperties() { \
    auto metadata = MetadataCreator<Self>::get();


#define PROPERTIES_END }

#define GETTER(methodName)                              \
  {                                                     \
    Method method{#methodName};                         \
    method.setFunction(std::mem_fn(&Self::methodName)); \
    member.setGetterMethod(method);                     \
  }

#define SETTER(methodName)                              \
  {                                                     \
    Method method{#methodName};                         \
    method.setFunction(std::mem_fn(&Self::methodName)); \
    member.setSetterMethod(method);                     \
  }

#define PROPERTY(propertyName, ...)                                           \
  {                                                                           \
    Member member{                                                            \
        #propertyName,                                                        \
        (size_t)((char*)&(((Self*)nullptr)->propertyName) - (char*)nullptr)}; \
    __VA_ARGS__                                                               \
    metadata->addMember(member);                                              \
  }

#define METHODS_BEGIN           \
 public:                        \
  static void defineMethods() { \
    auto metadata = MetadataCreator<Self>::get();

#define METHOD(methodName)                              \
  {                                                     \
    Method method{#methodName};                         \
    method.setFunction(std::mem_fn(&Self::methodName)); \
    metadata->addMethod(method);                        \
  }

#define METHODS_END \
  }                 \
                    \
 private:


	struct Shape{
		void setWidth(int w) { width = w; }
		int getHeigth() { return heigth; }
		void setHeigth(int h) { heigth = h; }
		int length, width, heigth;

	public:
		using Self = Shape;
		static auto initMetadata() {
			static bool firstTime = true;
			if (firstTime) {
					firstTime = false;
					return new Metadata("Shape", sizeof(Shape));
			}
			call_defineProperties<Self, has_defineProperties_v<Self>>::call();
			call_defineMethods<Self, has_defineMethods_v<Self>>::call();
			ReflectionManager::getInstance().registerType("Shape", MetadataCreator<Self>::get());
			return MetadataCreator<Self>::get();
		}

		inline virtual Metadata* getMetadata() {
			return MetadataCreator<Self>::get();
		}
	public:
		static void defineProperties() {
			auto metadata = MetadataCreator<Self>::get();

			{
				Member member{
				"length",
				(size_t)((char*)&(((Self*)nullptr)->length) - (char*)nullptr)};
				metadata->addMember(member);
			}
		}

	};

	void _m() {
		
	}
}

namespace IKIGAI::UTILS
{

	enum class MetaParam {
		FLAGS,
		EDIT_RANGE,
		EDIT_STEP,
		EDIT_WIDGET,
		FILE_EXTENSION,
	};

	enum class MetaInfo {
		NONE = 0,
		USE_IN_EDITOR_ANIMATION = 1,
		USE_IN_EDITOR_COMPONENT_INSPECTOR = 1 << 1,
	};

	enum class WidgetType : uint32_t {
		DRAG_INT,
		DRAG_FLOAT,
		DRAG_FLOAT_3,
		DRAG_FLOAT_4,
		DRAG_COLOR_3,
		DRAG_COLOR_4,
		COMBO,
		BOOL,
		STRING,
		STRING_WITH_FILE_CHOOSE,
		STRINGS_ARRAY,
		OPT_STRING,

		MATERIAL,
		MODEL_LOD
	};

	using Meta_t = std::map<MetaParam, std::variant<bool, int, float, std::string, UTILS::MetaInfo, UTILS::WidgetType, MATH::Vector2f>>;

	template<class CLASS_T, class MEMBER_T>
	struct MemberInfo {
		using Class_t = CLASS_T;
		using Data_t = std::remove_reference<MEMBER_T>::type;

		MemberInfo(std::string_view name, Data_t Class_t::* memberPtr, Meta_t meta = Meta_t()) :
			mName(name),
			mPtr(memberPtr),
			mMetadata(meta) {
			mGetter = [](const MemberInfo* _this, Class_t& obj) {
				return obj.*(_this->mPtr);
			};
			mSetter = [](const MemberInfo* _this, Class_t& obj, Data_t data) {
				obj.*(_this->mPtr) = data;
			};
		}

		MemberInfo(std::string_view name, MEMBER_T(Class_t::* memberGetPtr)() const, void(Class_t::* memberSetPtr)(MEMBER_T), Meta_t meta = Meta_t()) :
			mName(name),
			mPtrGet(memberGetPtr),
			mPtrSet(memberSetPtr),
			mMetadata(meta) {
			mGetter = [](const MemberInfo* _this, Class_t& obj) {
				return (obj.*(_this->mPtrGet))();
			};
			mSetter = [](const MemberInfo* _this, Class_t& obj, Data_t data) {
				(obj.*(_this->mPtrSet))(data);
			};
		}

		MemberInfo(std::string_view name, std::function<MEMBER_T(Class_t&)> memberGetPtr, std::function<void(Class_t&, MEMBER_T)> memberSetPtr, Meta_t meta = Meta_t()) :
			mName(name),
			mPtrGetLambda(memberGetPtr),
			mPtrSetLambda(memberSetPtr),
			mMetadata(meta) {
			mGetter = [](const MemberInfo* _this, Class_t& obj) {
				return _this->mPtrGetLambda(obj);
			};
			mSetter = [](const MemberInfo* _this, Class_t& obj, Data_t data) {
				_this->mPtrSetLambda(obj, data);
			};
		}

		MEMBER_T get(Class_t& obj) const {
			return mGetter(this, obj);
		}
		void set(Class_t& obj, MEMBER_T data) const {
			mSetter(this, obj, data);
		}

		[[nodiscard]] const std::string& getName() const {
			return mName;
		}

		const Meta_t& getMetadata() {
			return mMetadata;
		};

		Data_t getPropType() { return Data_t(); }
	private:
		const std::string mName;

		Data_t Class_t::* mPtr = nullptr;
		MEMBER_T(Class_t::* mPtrGet)() const = nullptr;
		void(Class_t::* mPtrSet)(MEMBER_T) = nullptr;
		std::function<MEMBER_T(Class_t&)> mPtrGetLambda;
		std::function<void(Class_t&, MEMBER_T)> mPtrSetLambda;

		std::function<MEMBER_T(const MemberInfo*, Class_t&)> mGetter;
		std::function<void(const MemberInfo*, Class_t&, MEMBER_T)> mSetter;
		Meta_t mMetadata;
	};

	template <class CLASS, class PTR>
	MemberInfo<CLASS, PTR> MakeMemberInfo(std::string_view name, PTR CLASS::* ptr) {
		return MemberInfo<CLASS, PTR>(name, ptr);
	}

	template <class CLASS, class PTR>
	MemberInfo<CLASS, PTR> MakeMemberInfo(std::string_view name, PTR CLASS::* ptr, Meta_t meta) {
		return MemberInfo<CLASS, PTR>(name, ptr, std::move(meta));
	}

	//template <class CLASS, class PTR>
	//constexpr MemberInfo<CLASS, PTR> MakeMemberInfo(std::string_view name, PTR&(CLASS::* memberGetPtr)() const, void(CLASS::* memberSetPtr)(PTR&)) {
	//	return MemberInfo<CLASS, PTR>(name, memberGetPtr, memberSetPtr);
	//}
	//
	//template <class CLASS, class PTR>
	//constexpr MemberInfo<CLASS, PTR> MakeMemberInfo(std::string_view name, PTR&(CLASS::* memberGetPtr)() const, void(CLASS::* memberSetPtr)(PTR&), Meta_t meta) {
	//	return MemberInfo<CLASS, PTR>(name, memberGetPtr, memberSetPtr, std::move(meta));
	//}

	template <class CLASS, class PTR>
	MemberInfo<CLASS, PTR> MakeMemberInfo(std::string_view name, PTR (CLASS::* memberGetPtr)() const, void(CLASS::* memberSetPtr)(PTR)) {
		return MemberInfo<CLASS, PTR>(name, memberGetPtr, memberSetPtr);
	}

	template <class CLASS, class PTR>
	MemberInfo<CLASS, PTR> MakeMemberInfo(std::string_view name, PTR (CLASS::* memberGetPtr)() const, void(CLASS::* memberSetPtr)(PTR), Meta_t meta) {
		return MemberInfo<CLASS, PTR>(name, memberGetPtr, memberSetPtr, std::move(meta));
	}

	template <class CLASS, class PTR>
	MemberInfo<CLASS, PTR> MakeMemberInfoLambda(std::string_view name, std::function<PTR(CLASS&)> memberGetPtr, std::function<void(CLASS&, PTR)> memberSetPtr) {
		return MemberInfo<CLASS, PTR>(name, memberGetPtr, memberSetPtr);
	}

	template <class CLASS, class PTR>
	MemberInfo<CLASS, PTR> MakeMemberInfoLambda(std::string_view name, std::function<PTR(CLASS&)> memberGetPtr, std::function<void(CLASS&, PTR)> memberSetPtr, Meta_t meta) {
		return MemberInfo<CLASS, PTR>(name, memberGetPtr, memberSetPtr, std::move(meta));
	}
}
