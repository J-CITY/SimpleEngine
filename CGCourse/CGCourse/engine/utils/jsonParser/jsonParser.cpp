#include "jsonParser.h"

using namespace KUMA;
using namespace KUMA::UTILS;

namespace  {
	rttr::variant extract_basic_types(nlohmann::json& json_value) {
		switch (json_value.type()) {
		case nlohmann::detail::value_t::string: {
			return json_value.get<std::string>();
			break;
		}
		case nlohmann::detail::value_t::null: break;
		case nlohmann::detail::value_t::boolean: {
			return json_value.get<bool>();
			break;
		}
		case nlohmann::detail::value_t::number_float: {
			return json_value.get<float>();
			break;
		}
		case nlohmann::detail::value_t::number_integer: {
			return json_value.get<int>();
			break;
		}
		case nlohmann::detail::value_t::number_unsigned: {
			return json_value.get<unsigned>();
			break;
		}
		// we handle only the basic types here
		case nlohmann::detail::value_t::object:
		case nlohmann::detail::value_t::array: return rttr::variant();
		}
		return rttr::variant();
	}


	/////////////////////////////////////////////////////////////////////////////////////////

	static void write_array_recursively(rttr::variant_sequential_view& view, nlohmann::json& json_array_value) {
		view.set_size(json_array_value.size());
		const rttr::type array_value_type = view.get_rank_type(1);

		for (int i = 0; i < json_array_value.size(); ++i) {
			auto& json_index_value = json_array_value[i];
			if (json_index_value.is_array()) {
				auto sub_array_view = view.get_value(i).create_sequential_view();
				write_array_recursively(sub_array_view, json_index_value);
			}
			else if (json_index_value.is_object()) {
				rttr::variant var_tmp = view.get_value(i);
				rttr::variant wrapped_var = var_tmp.extract_wrapped_value();
				fromJsonRecursively(wrapped_var, json_index_value);
				view.set_value(i, wrapped_var);
			}
			else {
				rttr::variant extracted_value = extract_basic_types(json_index_value);
				if (extracted_value.convert(array_value_type)) {
					view.set_value(i, extracted_value);
				}
			}
		}
	}

	rttr::variant extract_value(nlohmann::json::iterator& itr, const rttr::type& t) {
		auto& json_value = *itr;
		rttr::variant extracted_value = extract_basic_types(json_value);
		const bool could_convert = extracted_value.convert(t);
		if (!could_convert)
		{
			if (json_value.is_object())
			{
				rttr::constructor ctor = t.get_constructor();
				for (auto& item : t.get_constructors())
				{
					if (item.get_instantiated_type() == t)
						ctor = item;
				}
				extracted_value = ctor.invoke();
				fromJsonRecursively(extracted_value, json_value);
			}
		}

		return extracted_value;
	}

	static void write_associative_view_recursively(rttr::variant_associative_view& view, nlohmann::json& json_array_value) {
		for (int i = 0; i < json_array_value.size(); ++i) {
			auto& json_index_value = json_array_value[i];
			if (json_index_value.is_object()) {
				auto key_itr = json_index_value.find("key");
				auto value_itr = json_index_value.find("value");

				if (key_itr != json_index_value.end() && value_itr != json_index_value.end()) {
					auto key_var = extract_value(key_itr, view.get_key_type());
					auto value_var = extract_value(value_itr, view.get_value_type());
					if (key_var && value_var) {
						view.insert(key_var, value_var);
					}
				}
			}
			else {
				rttr::variant extracted_value = extract_basic_types(json_index_value);
				if (extracted_value && extracted_value.convert(view.get_key_type())) {
					view.insert(extracted_value);
				}
			}
		}
	}

	////////////////////// TO JSON ///////////////////////////////////////
    void to_json_recursively(const rttr::instance& obj2, nlohmann::json& writer);
    bool write_variant(const rttr::variant& var, nlohmann::json& writer);

    bool write_atomic_types_to_json(const rttr::type& t, const rttr::variant& var, nlohmann::json& writer) {
        if (t.is_arithmetic()) {
            if (t == rttr::type::get<bool>())
                writer = (var.to_bool());
            else if (t == rttr::type::get<char>())
                writer = (var.to_bool());
            else if (t == rttr::type::get<int8_t>())
                writer = (var.to_int8());
            else if (t == rttr::type::get<int16_t>())
                writer = (var.to_int16());
            else if (t == rttr::type::get<int32_t>())
                writer = (var.to_int32());
            else if (t == rttr::type::get<int64_t>())
                writer = (var.to_int64());
            else if (t == rttr::type::get<uint8_t>())
                writer = (var.to_uint8());
            else if (t == rttr::type::get<uint16_t>())
                writer = (var.to_uint16());
            else if (t == rttr::type::get<uint32_t>())
                writer = (var.to_uint32());
            else if (t == rttr::type::get<uint64_t>())
                writer = (var.to_uint64());
            else if (t == rttr::type::get<float>())
                writer = (var.to_double());
            else if (t == rttr::type::get<double>())
                writer = (var.to_double());
            return true;
        }
        else if (t.is_enumeration()) {
            bool ok = false;
            auto result = var.to_string(&ok);
            if (ok) {
                writer = (var.to_string());
            }
            else {
                ok = false;
                auto value = var.to_uint64(&ok);
                if (ok)
                    writer = (value);
                else {

                }
            }

            return true;
        }
        else if (t == rttr::type::get<std::string>()) {
            writer = (var.to_string());
            return true;
        }
        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    static void write_array(const rttr::variant_sequential_view& view, nlohmann::json& writer) {
        //writer.StartArray();
        for (const auto& item : view)
        {
            if (item.is_sequential_container())
            {
                write_array(item.create_sequential_view(), writer);
            }
            else
            {
	            rttr::variant wrapped_var = item.extract_wrapped_value();
                rttr::type value_type = wrapped_var.get_type();
                if (value_type.is_arithmetic() || value_type == rttr::type::get<std::string>() || value_type.is_enumeration())
                {
                    writer.push_back({});
                    write_atomic_types_to_json(value_type, wrapped_var, writer.back());
                }
                else // object
                {
                    writer.push_back({});
                    to_json_recursively(wrapped_var, writer.back());
                }
            }
        }
        //writer.EndArray();
    }


    /////////////////////////////////////////////////////////////////////////////////////////

    static void write_associative_container(const rttr::variant_associative_view& view, nlohmann::json& writer)
    {
        static const rttr::string_view key_name("key");
        static const rttr::string_view value_name("value");

        //writer.StartArray();

        if (view.is_key_only_type())
        {
            for (auto& item : view)
            {
                write_variant(item.first, writer);
            }
        }
        else
        {
            for (auto& item : view)
            {
                writer.push_back({});
                //writer.StartObject();
                //writer.String(key_name.data(), static_cast<rapidjson::SizeType>(key_name.length()), false);

                write_variant(item.first, writer.back()[key_name.to_string()]);

                //writer.String(value_name.data(), static_cast<rapidjson::SizeType>(value_name.length()), false);

                write_variant(item.second, writer.back()[value_name.to_string()]);

                //writer.EndObject();
            }
        }

        //writer.EndArray();
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    bool write_variant(const rttr::variant& var, nlohmann::json& writer)
    {
        auto value_type = var.get_type();
        auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
        bool is_wrapper = wrapped_type != value_type;

        if (write_atomic_types_to_json(is_wrapper ? wrapped_type : value_type,
            is_wrapper ? var.extract_wrapped_value() : var, writer))
        {
        }
        else if (var.is_sequential_container())
        {
            write_array(var.create_sequential_view(), writer);
        }
        else if (var.is_associative_container())
        {
            write_associative_container(var.create_associative_view(), writer);
        }
        else {
            auto child_props = is_wrapper ? wrapped_type.get_properties() : value_type.get_properties();
            if (!child_props.empty())
            {
                to_json_recursively(var, writer);
            }
            else
            {
                bool ok = false;
                auto text = var.to_string(&ok);
                if (!ok)
                {
                    writer = text;
                    return false;
                }

                writer = text;
            }
        }

        return true;
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    void to_json_recursively(const rttr::instance& obj2, nlohmann::json& writer) {
        rttr::instance obj = obj2.get_type().get_raw_type().is_wrapper() ? obj2.get_wrapped_instance() : obj2;

        auto prop_list = obj.get_derived_type().get_properties();
        for (auto prop : prop_list) {
            if (prop.get_metadata("NO_SERIALIZE"))
                continue;

            rttr::variant prop_value = prop.get_value(obj);
            if (!prop_value)
                continue; // cannot serialize, because we cannot retrieve the value

            const auto name = prop.get_name();
            //writer[name.to_string()] = static_cast<rapidjson::SizeType>(name.length()), false);
            if (!write_variant(prop_value, writer[name.to_string()])) {
                std::cerr << "cannot serialize property: " << name << std::endl;
            }
        }
        //writer.EndObject();
    }



}

Result<int, JsonError> KUMA::UTILS::fromJsonRecursively(rttr::instance obj2, nlohmann::json& json_object) {
	rttr::instance obj = obj2.get_type().get_raw_type().is_wrapper() ? obj2.get_wrapped_instance() : obj2;
	const auto prop_list = obj.get_derived_type().get_properties();

	for (auto prop : prop_list) {
		auto ret = json_object.find(prop.get_name().data());
		if (ret == json_object.end()) {

			continue;
		}
		const rttr::type value_t = prop.get_type();

		auto jsonType = ret->type();
		auto& json_value = ret.value();
		switch (jsonType) {
		case nlohmann::detail::value_t::array: {
			rttr::variant var;
			if (value_t.is_sequential_container()) {
				var = prop.get_value(obj);
				auto view = var.create_sequential_view();
				write_array_recursively(view, json_value);
			}
			else if (value_t.is_associative_container()) {
				var = prop.get_value(obj);
				auto associative_view = var.create_associative_view();
				write_associative_view_recursively(associative_view, json_value);
			}

			prop.set_value(obj, var);
			break;
		}
		case nlohmann::detail::value_t::object: {
			rttr::variant var = prop.get_value(obj);
			fromJsonRecursively(var, json_value);
			prop.set_value(obj, var);
			break;
		}
		default: {
			rttr::variant extracted_value = extract_basic_types(json_value);
			if (extracted_value.convert(value_t)) // REMARK: CONVERSION WORKS ONLY WITH "const type", check whether this is correct or not!
				prop.set_value(obj, extracted_value);
		}
		}
	}

    return Ok(1);
}

std::string KUMA::UTILS::ToJson(rttr::instance obj) {
    if (!obj.is_valid()) {
        return std::string();
    }

    nlohmann::json writer;
	to_json_recursively(obj, writer);

	return writer.dump(4);
}

