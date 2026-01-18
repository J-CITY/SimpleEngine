#pragma once
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <map>
#include <vector>
#include <any>
#include <typeindex>
#include <memory>
#include <optional>
#include <variant>
#include <tuple>

#include "mathModule/math.h"

namespace IKIGAI::UTILS
{
	template<std::size_t size>
	struct fnv1a_traits;

	template<>
	struct fnv1a_traits<4> {
		using type = std::uint32_t;
		static constexpr std::uint32_t offset = 2166136261;
		static constexpr std::uint32_t prime = 16777619;
	};

	template<>
	struct fnv1a_traits<8> {
		using type = std::uint64_t;
		static constexpr std::uint64_t offset = 14695981039346656037ull;
		static constexpr std::uint64_t prime = 1099511628211ull;
	};

	constexpr std::size_t string_hash_seed(std::size_t seed, const char* str, std::size_t N) noexcept {
		using Traits = fnv1a_traits<sizeof(std::size_t)>;
		std::size_t value = seed;

		for (std::size_t i = 0; i < N; i++)
			value = (value ^ static_cast<Traits::type>(str[i])) * Traits::prime;

		return value;
	}

	constexpr std::size_t tring_hash_seed(std::size_t seed, const char* curr) noexcept {
		using Traits = fnv1a_traits<sizeof(std::size_t)>;
		std::size_t value = seed;

		while (*curr) {
			value = (value ^ static_cast<Traits::type>(*(curr++))) * Traits::prime;
		}

		return value;
	}

	constexpr std::size_t string_hash(const char* str, std::size_t N) noexcept {
		using Traits = fnv1a_traits<sizeof(std::size_t)>;
		return string_hash_seed(Traits::offset, str, N);
	}

	constexpr std::size_t string_hash(std::string_view str) noexcept { return string_hash(str.data(), str.size()); }

	using TypeName = std::string;
	using TypeId = size_t;

	// Custom Any implementation to support Move-Only types
	class Any {
	public:
		Any() = default;

		template <typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Any>>>
		Any(T&& value) : mStorage(std::make_unique<Model<std::decay_t<T>>>(std::forward<T>(value))) {}

		Any(const Any& other) {
			if (other.mStorage) mStorage = other.mStorage->clone();
		}

		Any(Any&& other) noexcept = default;
		Any& operator=(Any&& other) noexcept = default;

		Any& operator=(const Any& other) {
			if (this != &other) {
				if (other.mStorage) mStorage = other.mStorage->clone();
				else mStorage.reset();
			}
			return *this;
		}

		bool has_value() const { return mStorage != nullptr; }
		const std::type_info& type() const { return mStorage ? mStorage->type() : typeid(void); }

		template <typename T>
		friend T* any_cast(Any* operand);

		template <typename T>
		friend const T* any_cast(const Any* operand);

	private:
		struct Base {
			virtual ~Base() = default;
			virtual std::unique_ptr<Base> clone() const = 0;
			virtual const std::type_info& type() const noexcept = 0;
		};

		template <typename T>
		struct Model final : Base {
			T data;
			explicit Model(T&& val) : data(std::move(val)) {}
			explicit Model(const T& val) : data(val) {}

			std::unique_ptr<Base> clone() const override {
				if constexpr (std::is_copy_constructible_v<T>) {
					return std::make_unique<Model<T>>(data);
				}
				else {
					throw std::runtime_error("Attempted to copy Any containing a move-only type");
				}
			}

			const std::type_info& type() const noexcept override {
				return typeid(T);
			}
		};

		std::unique_ptr<Base> mStorage;
	};

	template <typename T>
	T* any_cast(Any* operand) {
		if (operand && operand->mStorage && operand->mStorage->type() == typeid(T)) {
			return &static_cast<Any::Model<T>*>(operand->mStorage.get())->data;
		}
		return nullptr;
	}

	template <typename T>
	const T* any_cast(const Any* operand) {
		if (operand && operand->mStorage && operand->mStorage->type() == typeid(T)) {
			return &static_cast<const Any::Model<T>*>(operand->mStorage.get())->data;
		}
		return nullptr;
	}

	template <typename T>
	T any_cast(Any& operand) {
		using U = std::remove_cv_t<std::remove_reference_t<T>>;
		auto* ptr = any_cast<U>(&operand);
		if (!ptr) throw std::bad_cast();
		return static_cast<T>(*ptr);
	}

	template <typename T>
	T any_cast(const Any& operand) {
		using U = std::remove_cv_t<std::remove_reference_t<T>>;
		auto* ptr = any_cast<U>(&operand);
		if (!ptr) throw std::bad_cast();
		return static_cast<const U&>(*ptr);
	}

	template <typename T>
	T any_cast(Any&& operand) {
		using U = std::remove_cv_t<std::remove_reference_t<T>>;
		auto* ptr = any_cast<U>(&operand);
		if (!ptr) throw std::bad_cast();
		return static_cast<T>(std::move(*ptr));
	}

	using MethodName = std::string;
	using FieldName = std::string;

	// Flexible Metadata System
	using MetaDataId = size_t;
	using MetaValue = Any;
	
	class Metadata {
		std::unordered_map<MetaDataId, Any> mMap;
	public:
		Metadata() = default;

		// Initialize from string keys (hashes them automatically)
		Metadata(std::initializer_list<std::pair<std::string_view, Any>> init) {
			for (auto& [k, v] : init) {
				mMap[string_hash(k)] = std::move(v);
			}
		}

		// Initialize from pre-hashed IDs (e.g. constants)
		Metadata(std::initializer_list<std::pair<MetaDataId, Any>> init) {
			for (auto& [k, v] : init) {
				mMap[k] = std::move(v);
			}
		}
		
		void add(std::string_view key, Any value) {
			mMap[string_hash(key)] = std::move(value);
		}

		void add(MetaDataId key, Any value) {
			mMap[key] = std::move(value);
		}

		template<typename T>
		T get(MetaDataId key, T defaultValue = {}) const {
			auto it = mMap.find(key);
			if (it == mMap.end()) return defaultValue;
			if (auto* p = any_cast<T>(&it->second)) return *p;
			return defaultValue;
		}
		
		template<typename T>
		T get(std::string_view key, T defaultValue = {}) const {
			return get<T>(string_hash(key), defaultValue);
		}

		bool contains(MetaDataId key) const {
			return mMap.contains(key);
		}
		
		bool contains(std::string_view key) const {
			return mMap.contains(string_hash(key));
		}
		
		const std::unordered_map<MetaDataId, Any>& getMap() const { return mMap; }
	};
	
	using Meta_t = Metadata;

	class MethodInfo;
	class FieldInfo;
	class ConstructorInfo;

	class FieldInfo {
	public:
		std::string name;
		TypeId typeId;
		Meta_t metadata;
		bool isStatic = false;
		bool isConst = false;

		// Type-erased accessors
		std::function<void(void*, Any)> setter;
		std::function<Any(void*)> getter;

		template<typename T>
		void set(void* instance, T value) {
			if (setter) setter(instance, std::move(value));
		}

		template<typename T>
		T get(void* instance) {
			if (!getter) return T{};
			return any_cast<T>(getter(instance));
		}
	};

	class MethodInfo {
	public:
		std::string name;
		TypeId returnType;
		std::vector<TypeId> argumentTypes;
		Meta_t metadata;
		bool isStatic = false;
		bool isConst = false;

		using Invoker = std::function<Any(void* instance, const std::vector<Any>& args)>;
		Invoker invoke;
	};

	class ConstructorInfo {
	public:
		std::vector<TypeId> argumentTypes;
		Meta_t metadata;

		using Creator = std::function<Any(const std::vector<Any>& args)>;
		Creator invoke;
	};

	class TypeInfo {
	public:
		TypeName mName;
		TypeId mId;

		std::unordered_map<FieldName, FieldInfo> mFields;
		std::unordered_map<MethodName, MethodInfo> mMethods;
		std::vector<ConstructorInfo> mConstructors;
	};

	namespace DETAILS {
		// Helper to invoke function with vector of Any
		template<typename Func, typename Tuple, size_t... I>
		auto call_helper(Func&& f, const std::vector<Any>& args, std::index_sequence<I...>) {
			return f(any_cast<typename std::tuple_element<I, Tuple>::type>(args[I])...);
		}

		template<typename R, typename... Args>
		Any invoke_method(std::function<R(Args...)> func, const std::vector<Any>& args) {
			if (args.size() != sizeof...(Args)) throw std::runtime_error("Argument count mismatch");
			if constexpr (std::is_void_v<R>) {
				call_helper<decltype(func), std::tuple<Args...>>(std::move(func), args, std::index_sequence_for<Args...>{});
				return {};
			}
			else {
				return call_helper<decltype(func), std::tuple<Args...>>(std::move(func), args, std::index_sequence_for<Args...>{});
			}
		}
		
		// For member functions
		template<class C, typename R, typename... Args>
		Any invoke_member(R(C::* func)(Args...), void* instance, const std::vector<Any>& args) {
			if (args.size() != sizeof...(Args)) throw std::runtime_error("Argument count mismatch");
			auto* obj = static_cast<C*>(instance);
			auto wrapper = [obj, func](Args... params) -> R { return (obj->*func)(params...); };
			
			if constexpr (std::is_void_v<R>) {
				call_helper<decltype(wrapper), std::tuple<Args...>>(std::move(wrapper), args, std::index_sequence_for<Args...>{});
				return {};
			} else {
				return call_helper<decltype(wrapper), std::tuple<Args...>>(std::move(wrapper), args, std::index_sequence_for<Args...>{});
			}
		}
		
		// Const member functions
		template<class C, typename R, typename... Args>
		Any invoke_member_const(R(C::* func)(Args...) const, void* instance, const std::vector<Any>& args) {
			if (args.size() != sizeof...(Args)) throw std::runtime_error("Argument count mismatch");
			auto* obj = static_cast<C*>(instance);
			auto wrapper = [obj, func](Args... params) -> R { return (obj->*func)(params...); };
			
			if constexpr (std::is_void_v<R>) {
				call_helper<decltype(wrapper), std::tuple<Args...>>(std::move(wrapper), args, std::index_sequence_for<Args...>{});
				return {};
			} else {
				return call_helper<decltype(wrapper), std::tuple<Args...>>(std::move(wrapper), args, std::index_sequence_for<Args...>{});
			}
		}
	}


	class ReflectionManager {
		std::unordered_map<TypeId, TypeInfo> mTypes;
		TypeInfo mGlobalScope; 

	public:
		static ReflectionManager& Instance() {
			static ReflectionManager instance;
			return instance;
		}

		template<typename T>
		static constexpr TypeId GetTypeId() {
			return string_hash(NAMEOF_TYPE(T));
		}
		
		template<auto CLASS>
		void registerType() {
			auto newType = NAMEOF_TYPE(CLASS);
			auto newId = string_hash(newType);
			if (mTypes.find(newId) == mTypes.end()) {
				mTypes[newId] = TypeInfo{ std::string(newType), newId };
			}
		}

		template<auto CLASS>
		TypeInfo* getType() {
			auto id = string_hash(NAMEOF_TYPE(CLASS));
			return getType(id);
		}

		TypeInfo* getType(std::string_view typeName) {
			auto id = string_hash(typeName);
			return getType(id);
		}

		TypeInfo* getType(TypeId id) {
			auto target = mTypes.find(id);
			if (target == mTypes.end()) {
				return nullptr;
			}
			return &target->second;
		}
		
		// --- Field Registration ---

		// 1. Direct Member Pointer
		template<typename C, typename T>
		void registerField(const std::string& fieldName, T C::* fieldPtr, Meta_t meta = {}) {
			TypeId typeId = GetTypeId<C>();
			TypeInfo* info = getType(typeId);
			if (!info) { registerType<C>(); info = getType(typeId); }

			FieldInfo fieldInfo;
			fieldInfo.name = fieldName;
			fieldInfo.typeId = GetTypeId<T>();
			fieldInfo.metadata = std::move(meta);
			fieldInfo.isStatic = false;
			fieldInfo.isConst = std::is_const_v<T>;
			
			fieldInfo.getter = [fieldPtr](void* instance) -> Any {
				return static_cast<C*>(instance)->*fieldPtr;
			};
			fieldInfo.setter = [fieldPtr](void* instance, Any value) {
				static_cast<C*>(instance)->*fieldPtr = any_cast<T>(std::move(value));
			};

			info->mFields[fieldName] = std::move(fieldInfo);
		}

		// 2. Member Methods (Getter/Setter)
		template<typename C, typename T>
		void registerField(const std::string& fieldName, T (C::*getter)() const, void (C::*setter)(T), Meta_t meta = {}) {
			TypeId typeId = GetTypeId<C>();
			TypeInfo* info = getType(typeId);
			if (!info) { registerType<C>(); info = getType(typeId); }

			FieldInfo fieldInfo;
			fieldInfo.name = fieldName;
			fieldInfo.typeId = GetTypeId<T>();
			fieldInfo.metadata = std::move(meta);
			fieldInfo.isStatic = false;
			fieldInfo.isConst = false;

			fieldInfo.getter = [getter](void* instance) -> Any {
				return (static_cast<C*>(instance)->*getter)();
			};
			fieldInfo.setter = [setter](void* instance, Any value) {
				(static_cast<C*>(instance)->*setter)(any_cast<T>(std::move(value)));
			};

			info->mFields[fieldName] = std::move(fieldInfo);
		}
		
		// 3. Lambdas (Getter/Setter)
		template<typename C, typename T>
		void registerField(const std::string& fieldName, std::function<T(C*)> getter, std::function<void(C*, T)> setter, Meta_t meta = {}) {
			TypeId typeId = GetTypeId<C>();
			TypeInfo* info = getType(typeId);
			if (!info) { registerType<C>(); info = getType(typeId); }

			FieldInfo fieldInfo;
			fieldInfo.name = fieldName;
			fieldInfo.typeId = GetTypeId<T>();
			fieldInfo.metadata = std::move(meta);
			fieldInfo.isStatic = false; 

			if (getter) {
				fieldInfo.getter = [getter](void* instance) -> Any {
					return getter(static_cast<C*>(instance));
				};
			}
			if (setter) {
				fieldInfo.setter = [setter](void* instance, Any value) {
					setter(static_cast<C*>(instance), any_cast<T>(std::move(value)));
				};
			}

			info->mFields[fieldName] = std::move(fieldInfo);
		}

		// --- Static Field Registration ---

		// 1. Pointer to Static Variable
		template<typename C, typename T>
		void registerStaticField(const std::string& fieldName, T* fieldPtr, Meta_t meta = {}) {
			TypeId typeId = GetTypeId<C>();
			TypeInfo* info = getType(typeId);
			if (!info) { registerType<C>(); info = getType(typeId); }
			
			FieldInfo fieldInfo;
			fieldInfo.name = fieldName;
			fieldInfo.typeId = GetTypeId<T>();
			fieldInfo.metadata = std::move(meta);
			fieldInfo.isStatic = true;
			
			fieldInfo.getter = [fieldPtr](void*) -> Any { return *fieldPtr; };
			fieldInfo.setter = [fieldPtr](void*, Any value) { *fieldPtr = any_cast<T>(std::move(value)); };
			
			info->mFields[fieldName] = std::move(fieldInfo);
		}

		// 2. Static Functions (Getter/Setter)
		template<typename C, typename T>
		void registerStaticField(const std::string& fieldName, T (*getter)(), void (*setter)(T), Meta_t meta = {}) {
			TypeId typeId = GetTypeId<C>();
			TypeInfo* info = getType(typeId);
			if (!info) { registerType<C>(); info = getType(typeId); }

			FieldInfo fieldInfo;
			fieldInfo.name = fieldName;
			fieldInfo.typeId = GetTypeId<T>();
			fieldInfo.metadata = std::move(meta);
			fieldInfo.isStatic = true;

			fieldInfo.getter = [getter](void*) -> Any { return getter(); };
			fieldInfo.setter = [setter](void*, Any value) { setter(any_cast<T>(std::move(value))); };

			info->mFields[fieldName] = std::move(fieldInfo);
		}

		// 3. Lambdas (Static Getter/Setter)
		template<typename C, typename T>
		void registerStaticField(const std::string& fieldName, std::function<T()> getter, std::function<void(T)> setter, Meta_t meta = {}) {
			TypeId typeId = GetTypeId<C>();
			TypeInfo* info = getType(typeId);
			if (!info) { registerType<C>(); info = getType(typeId); }

			FieldInfo fieldInfo;
			fieldInfo.name = fieldName;
			fieldInfo.typeId = GetTypeId<T>();
			fieldInfo.metadata = std::move(meta);
			fieldInfo.isStatic = true;

			if (getter) {
				fieldInfo.getter = [getter](void*) -> Any { return getter(); };
			}
			if (setter) {
				fieldInfo.setter = [setter](void*, Any value) { setter(any_cast<T>(std::move(value))); };
			}

			info->mFields[fieldName] = std::move(fieldInfo);
		}

		// Register Global Virtual Property (not bound to any class)
		template<typename T>
		void registerProperty(const std::string& fieldName, std::function<T()> getter, std::function<void(T)> setter, Meta_t meta = {}) {
			FieldInfo fieldInfo;
			fieldInfo.name = fieldName;
			fieldInfo.typeId = GetTypeId<T>();
			fieldInfo.metadata = std::move(meta);
			fieldInfo.isStatic = true; // Global properties are static by definition

			if (getter) {
				fieldInfo.getter = [getter](void*) -> Any {
					return getter();
				};
			}
			if (setter) {
				fieldInfo.setter = [setter](void*, Any value) {
					setter(any_cast<T>(std::move(value)));
				};
			}

			// Store in global scope
			mGlobalScope.mFields[fieldName] = std::move(fieldInfo);
		}
		
		// --- Method Registration ---

		template<typename C, typename R, typename... Args>
		void registerMethod(const std::string& name, R(C::* methodPtr)(Args...), Meta_t meta = {}) {
			TypeId typeId = GetTypeId<C>();
			TypeInfo* info = getType(typeId);
			if (!info) { registerType<C>(); info = getType(typeId); }

			MethodInfo methodInfo;
			methodInfo.name = name;
			methodInfo.returnType = GetTypeId<R>();
			methodInfo.argumentTypes = { GetTypeId<Args>()... };
			methodInfo.metadata = std::move(meta);
			methodInfo.isStatic = false;
			methodInfo.isConst = false;
			
			methodInfo.invoke = [methodPtr](void* instance, const std::vector<Any>& args) -> Any {
				return DETAILS::invoke_member(methodPtr, instance, args);
			};
			
			info->mMethods[name] = std::move(methodInfo);
		}

		template<typename C, typename R, typename... Args>
		void registerMethod(const std::string& name, R(C::* methodPtr)(Args...) const, Meta_t meta = {}) {
			TypeId typeId = GetTypeId<C>();
			TypeInfo* info = getType(typeId);
			if (!info) { registerType<C>(); info = getType(typeId); }

			MethodInfo methodInfo;
			methodInfo.name = name;
			methodInfo.returnType = GetTypeId<R>();
			methodInfo.argumentTypes = { GetTypeId<Args>()... };
			methodInfo.metadata = std::move(meta);
			methodInfo.isStatic = false;
			methodInfo.isConst = true;
			
			methodInfo.invoke = [methodPtr](void* instance, const std::vector<Any>& args) -> Any {
				return DETAILS::invoke_member_const(methodPtr, instance, args);
			};
			
			info->mMethods[name] = std::move(methodInfo);
		}

		template<typename C, typename R, typename... Args>
		void registerStaticMethod(const std::string& name, R(*methodPtr)(Args...), Meta_t meta = {}) {
			TypeId typeId = GetTypeId<C>();
			TypeInfo* info = getType(typeId);
			if (!info) { registerType<C>(); info = getType(typeId); }

			MethodInfo methodInfo;
			methodInfo.name = name;
			methodInfo.returnType = GetTypeId<R>();
			methodInfo.argumentTypes = { GetTypeId<Args>()... };
			methodInfo.metadata = std::move(meta);
			methodInfo.isStatic = true;
			
			auto func = [methodPtr](Args... args) -> R { return methodPtr(args...); };
			
			methodInfo.invoke = [func](void*, const std::vector<Any>& args) -> Any {
				return DETAILS::invoke_method(std::function<R(Args...)>(func), args);
			};
			
			info->mMethods[name] = std::move(methodInfo);
		}

		// --- Global Registration ---
		
		template<typename T>
		void registerGlobalVariable(const std::string& name, T* varPtr, Meta_t meta = {}) {
			FieldInfo fieldInfo;
			fieldInfo.name = name;
			fieldInfo.typeId = GetTypeId<T>();
			fieldInfo.metadata = std::move(meta);
			fieldInfo.isStatic = true;

			fieldInfo.getter = [varPtr](void*) -> Any { return *varPtr; };
			fieldInfo.setter = [varPtr](void*, Any value) { *varPtr = any_cast<T>(std::move(value)); };
			
			mGlobalScope.mFields[name] = std::move(fieldInfo);
		}

		template<typename R, typename... Args>
		void registerGlobalFunction(const std::string& name, R(*funcPtr)(Args...), Meta_t meta = {}) {
			MethodInfo methodInfo;
			methodInfo.name = name;
			methodInfo.returnType = GetTypeId<R>();
			methodInfo.argumentTypes = { GetTypeId<Args>()... };
			methodInfo.metadata = std::move(meta);
			methodInfo.isStatic = true;
			
			auto func = [funcPtr](Args... args) -> R { return funcPtr(args...); };
			
			methodInfo.invoke = [func](void*, const std::vector<Any>& args) -> Any {
				return DETAILS::invoke_method(std::function<R(Args...)>(func), args);
			};
			
			mGlobalScope.mMethods[name] = std::move(methodInfo);
		}
		
		TypeInfo& getGlobalScope() { return mGlobalScope; }
	};
}
