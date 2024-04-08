#pragma once
#include <functional>

#include "mathModule/math.h"

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
