#pragma once
#include <iostream>
#include <string>
#include "engine/utilsModule/reflection/reflection.h"

namespace ExampleReflection {

	// 0. Globals for Demonstration
	int g_GlobalVar = 42;

	inline void g_GlobalFunc() {
		std::cout << "  (g_GlobalFunc called!)\n";
	}

	// 1. Nested Structure
	struct InnerStruct {
		int innerVal = 10;
	};

	// 2. Main Reflectable Struct
	struct TestReflectable {
		// Public Field
		int simpleField = 123;
		
		// Nested Struct Field
		InnerStruct nested;

		// Private Field for Getter/Setter demo
		float mFloatVal = 3.14f;

		// Static Field
		static inline int staticInt = 999;

		// Method pair for Getter/Setter
		float getFloat() const { return mFloatVal; }
		void setFloat(float v) { mFloatVal = v; }

		// Static Method
		static int getStaticInt() { return staticInt; }
		static void setStaticInt(int v) { staticInt = v; }
	};

	class ReflectionDemo {
	public:
		static void run() {
			std::cout << "=== Starting Reflection Demo ===\n";
			using namespace IKIGAI::UTILS;
			auto& manager = ReflectionManager::Instance();

			// A. Registration
			// ---------------

			// 1. Register Inner Struct
			manager.registerType<InnerStruct>();
			manager.registerField<InnerStruct>("innerVal", &InnerStruct::innerVal);

			// 2. Register Main Struct
			manager.registerType<TestReflectable>();

			// 2.1 Direct Field
			manager.registerField<TestReflectable>("simpleField", &TestReflectable::simpleField);
			manager.registerField<TestReflectable>("nested", &TestReflectable::nested);

			// 2.2 Getter/Setter (Method)
			manager.registerField<TestReflectable>("floatProp", 
				&TestReflectable::getFloat, 
				&TestReflectable::setFloat);

			// 2.3 Getter/Setter (Lambda) - "Virtual" field derived from simpleField
			manager.registerField<TestReflectable, std::string>("stringifiedSimple",
				[](TestReflectable* obj) { return std::to_string(obj->simpleField); },
				[](TestReflectable* obj, std::string val) { 
					try { obj->simpleField = std::stoi(val); } catch(...) {} 
				}
			);

			// 2.4 Static Field (Pointer)
			manager.registerStaticField<TestReflectable>("staticInt", &TestReflectable::staticInt);

			// 2.5 Static Field (Getter/Setter Method)
			// manager.registerStaticField<TestReflectable>("staticIntProp", &TestReflectable::getStaticInt, &TestReflectable::setStaticInt);
			// Note: registerStaticField template for functions expects function pointers.
			
			// 2.6 Static Method
			manager.registerStaticMethod<TestReflectable>("getStaticIntFunc", &TestReflectable::getStaticInt);

			// 3. Global Registration
			manager.registerGlobalVariable("GlobalVar", &g_GlobalVar);
			manager.registerGlobalFunction("GlobalFunc", &g_GlobalFunc);


			// B. Usage / Demonstration
			// ------------------------
			TestReflectable obj;
			auto* typeInfo = manager.getType<TestReflectable>();

			std::cout << "\n[Introspection] Type: " << typeInfo->mName << "\n";
			
			// 1. Access Direct Field
			if (typeInfo->mFields.contains("simpleField")) {
				auto& f = typeInfo->mFields["simpleField"];
				std::cout << "  Reading simpleField: " << f.get<int>(&obj) << "\n";
				f.set(&obj, 777);
				std::cout << "  Wrote 777. Result: " << obj.simpleField << "\n";
			}

			// 2. Access Nested Struct
			if (typeInfo->mFields.contains("nested")) {
				auto& f = typeInfo->mFields["nested"];
				// We get a copy if Any stores value, or we assume it handles it. 
				// IKIGAI::Any stores by value (unique_ptr clone).
				InnerStruct inner = f.get<InnerStruct>(&obj); 
				std::cout << "  Reading nested.innerVal: " << inner.innerVal << "\n";
			}

			// 3. Access Property (Method G/S)
			if (typeInfo->mFields.contains("floatProp")) {
				auto& f = typeInfo->mFields["floatProp"];
				std::cout << "  Reading floatProp: " << f.get<float>(&obj) << "\n";
				f.set(&obj, 5.5f);
				std::cout << "  Wrote 5.5. Result in obj: " << obj.mFloatVal << "\n";
			}

			// 4. Access Property (Lambda G/S)
			if (typeInfo->mFields.contains("stringifiedSimple")) {
				auto& f = typeInfo->mFields["stringifiedSimple"];
				std::cout << "  Reading stringifiedSimple: " << f.get<std::string>(&obj) << "\n";
				f.set(&obj, std::string("999"));
				std::cout << "  Wrote '999'. Result in simpleField: " << obj.simpleField << "\n";
			}

			// 5. Static Field Access
			if (typeInfo->mFields.contains("staticInt")) {
				auto& f = typeInfo->mFields["staticInt"];
				std::cout << "  Reading staticInt: " << f.get<int>(nullptr) << "\n";
				f.set(nullptr, 111);
				std::cout << "  Wrote 111. Result staticInt: " << TestReflectable::staticInt << "\n";
			}

			// 6. Global Variable
			auto& globalScope = manager.getGlobalScope();
			if (globalScope.mFields.contains("GlobalVar")) {
				auto& f = globalScope.mFields["GlobalVar"];
				std::cout << "  Reading GlobalVar: " << f.get<int>(nullptr) << "\n";
				f.set(nullptr, 100);
				std::cout << "  Wrote 100. Result g_GlobalVar: " << g_GlobalVar << "\n";
			}

			// 7. Global Function
			if (globalScope.mMethods.contains("GlobalFunc")) {
				auto& m = globalScope.mMethods["GlobalFunc"];
				std::cout << "  Invoking GlobalFunc:\n";
				m.invoke(nullptr, {}); 
			}


			// 9. Auto-Storage Property
			// Register
			manager.registerProperty<int>("AutoProp");
			
			// Use 'property' accessor (Return reference)
			manager.property<int>("AutoProp") = 12345;
			std::cout << "  AutoProp set via ref to 12345. Read back: " << manager.property<int>("AutoProp") << "\n";

			// Access via standard Reflection Field API
			auto& globalProps = manager.getGlobalProperties();
			if (globalProps.contains("AutoProp")) {
				auto& f = globalProps.at("AutoProp");
				std::cout << "  Reading AutoProp via FieldInfo: " << f.get<int>(nullptr) << "\n";
				f.set(nullptr, 54321);
				std::cout << "  Wrote 54321 via FieldInfo. Read back via ref: " << manager.property<int>("AutoProp") << "\n";
			}

			// 10. Remove Property
			manager.removeProperty("AutoProp");
			if (!manager.getGlobalProperties().contains("AutoProp")) {
				std::cout << "  AutoProp field removed from global properties.\n";
			}
			try {
				manager.property<int>("AutoProp");
			} catch (const std::exception& e) {
				std::cout << "  Accessing removed property threw exception: " << e.what() << "\n";
			}

			std::cout << "=== Reflection Demo Finished ===\n" << std::endl;
		}
	};
}
