#pragma once

#include "utilsModule/reflection/reflection_macros.h"
#include "mathModule/math.h"
#include <string>

namespace IKIGAI::TEST {

	IKI_ENUM()
	enum class TestEnum {
		Alpha,
		Beta = 5,
		Gamma
	};

	IKI_CLASS(TypeName="class IKIGAI::TEST::TestReflectionClass", ClassName="TestReflectionClass")
	class TestReflectionClass {
		IKI_GENERATED_BODY(TestReflectionClass)
	public:
		IKI_PROPERTY(EditRange=MATH::Vector2f{0.0f, 100.0f}, EditStep=0.1f)
		float normalFloat = 42.0f;

		IKI_PROPERTY()
		inline static int staticInt = 100;

		IKI_PROPERTY(Getter=GetName, Setter=SetName, Description="The hidden name of the object")
		std::string hiddenName = "Secret";

		const std::string& GetName() const { return hiddenName; }
		void SetName(const std::string& name) { hiddenName = name; }

		IKI_FUNCTION(Description="Prints a greeting", Tags=["Print", "Debug"])
		void SayHello(int count) const {
			// Dummy body
		}

		IKI_FUNCTION()
		virtual int Calculate(float x, float y) override {
			return static_cast<int>(x + y);
		}
	};
}
