#pragma once
#include <functional>
#include <limits>
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace IKIGAI::SCENE_SYSTEM::DOCUMENT {
using Json = nlohmann::json;
using IdMap = std::map<int, int>;
struct Instance {
	Json entry;
	// Canonical runtime descriptors, captured before activation; used to prevent
	// unsupported edits from being silently discarded when saving a reference.
	Json baseline = Json::array();
};
inline int Id(const Json& value) {
	if (!value.is_number_integer())
		throw std::runtime_error("Entity ID must be an integer");
	const auto id = value.get<int64_t>();
	if (id <= 0 || id >= std::numeric_limits<int>::max())
		throw std::runtime_error("Entity ID out of range");
	return static_cast<int>(id);
}
inline std::string Path(std::string path) {
	for (auto& c : path)
		if (c == '\\')
			c = '/';
	if (path.empty() || path.find(':') != std::string::npos)
		throw std::runtime_error("Expected a VFS prefab path");
	std::string result;
	std::size_t start = 0;
	while (start <= path.size()) {
		const auto end = path.find('/', start);
		const auto part = path.substr(start, end == std::string::npos ? end : end - start);
		if (part == "..")
			throw std::runtime_error("Prefab path cannot contain ..");
		if (!part.empty() && part != ".")
			result += "/" + part;
		if (end == std::string::npos)
			break;
		start = end + 1;
	}
	if (!result.ends_with(".prefab.json"))
		throw std::runtime_error("Prefab must have .prefab.json extension");
	return result;
}
inline bool Component(const Json& component, const char* type) {
	return component.contains(type);
}
inline Json& Transform(Json& object) {
	for (auto& component : object.at("Components"))
		if (Component(component, "TransformComponentType"))
			return component;
	throw std::runtime_error("Object has no TransformComponent");
}
// Only declared entity-reference fields are visited. Other integer properties
// (particle counts, enum values, resource indices, etc.) are never remapped.
template <class F> void References(Json& object, F&& visit) {
	for (auto& component : object.at("Components")) {
		if (Component(component, "RenderTargetComponentType") && component.contains("CameraObjectId"))
			visit(component["CameraObjectId"], "CameraObjectId");
		if (Component(component, "Joint2DComponentType") && component.contains("PhysicsTargetId"))
			visit(component["PhysicsTargetId"], "PhysicsTargetId");
	}
}
inline void Remap(Json& object, const IdMap& ids) {
	const auto old = Id(object.at("Id"));
	object["Id"] = ids.at(old);
	const int parent = object.value("ParentId", -1);
	object["ParentId"] = parent > 0 ? ids.at(parent) : -1;
	References(object, [&](Json& value, const char* field) {
		if (value == 0 || value == -1)
			return;
		const int target = Id(value);
		if (!ids.contains(target))
			throw std::runtime_error("Object " + std::to_string(old) + " has external prefab reference: " + field + "=" + std::to_string(target));
		value = ids.at(target);
	});
}
inline void Validate(Json objects, int root = 0) {
	if (!objects.is_array())
		throw std::runtime_error("Objects must be an array");
	std::map<int, std::size_t> indices;
	int roots = 0;
	for (std::size_t i = 0; i < objects.size(); ++i) {
		auto& object = objects[i];
		if (object.contains("Prefab"))
			throw std::runtime_error("Nested prefabs are not supported; unpack first");
		const auto id = Id(object.at("Id"));
		if (!indices.emplace(id, i).second)
			throw std::runtime_error("Duplicate entity ID: " + std::to_string(id));
		if (!object.at("Components").is_array())
			throw std::runtime_error("Components must be an array");
		Transform(object);
		int transforms = 0;
		for (const auto& c : object["Components"])
			transforms += Component(c, "TransformComponentType");
		if (transforms != 1)
			throw std::runtime_error("Object must have exactly one TransformComponent");
		if (object.value("ParentId", -1) <= 0) {
			++roots;
			if (root && id != root)
				throw std::runtime_error("Prefab has an unexpected root");
		}
	}
	if (root && (roots != 1 || !indices.contains(root)))
		throw std::runtime_error("Prefab must have one RootId");
	std::vector<unsigned char> colors(objects.size());
	for (std::size_t start = 0; start < objects.size(); ++start) {
		std::vector<std::size_t> chain;
		auto i = start;
		while (colors[i] != 2) {
			if (colors[i] == 1)
				throw std::runtime_error("Scene parent cycle");
			colors[i] = 1;
			chain.push_back(i);
			const int parent = objects[i].value("ParentId", -1);
			if (parent <= 0)
				break;
			if (!indices.contains(parent))
				throw std::runtime_error("Missing parent: " + std::to_string(parent));
			i = indices.at(parent);
		}
		for (auto item : chain)
			colors[item] = 2;
		References(objects[start], [&](Json& value, const char* field) {
			if (value != 0 && value != -1 && !indices.contains(Id(value)))
				throw std::runtime_error("Missing entity reference: " + std::string(field));
		});
	}
}
inline void ValidatePrefab(const Json& asset) {
	if (asset.at("PrefabVersion") != 1)
		throw std::runtime_error("Unsupported PrefabVersion");
	Validate(asset.at("Objects"), Id(asset.at("RootId")));
}
inline IdMap ReadMap(const Json& entry) {
	IdMap ids;
	if (!entry.contains("IdMap"))
		return ids;
	if (!entry["IdMap"].is_object())
		throw std::runtime_error("IdMap must be an object");
	for (auto it = entry["IdMap"].begin(); it != entry["IdMap"].end(); ++it) {
		std::size_t used = 0;
		const int local = std::stoi(it.key(), &used);
		if (used != it.key().size() || local <= 0 || std::to_string(local) != it.key())
			throw std::runtime_error("Invalid local ID in IdMap");
		ids.emplace(local, Id(it.value()));
	}
	return ids;
}
inline Json WriteMap(const IdMap& ids) {
	Json result = Json::object();
	for (auto [local, runtime] : ids)
		result[std::to_string(local)] = runtime;
	return result;
}
inline void Overrides(Json& object, const Json& overrides) {
	if (!overrides.is_object())
		throw std::runtime_error("RootOverrides must be an object");
	for (auto it = overrides.begin(); it != overrides.end(); ++it) {
		if (it.key() == "Transform") {
			for (auto p = it.value().begin(); p != it.value().end(); ++p) {
				if (p.key() != "LocalPosition" && p.key() != "LocalRotation" && p.key() != "LocalScale")
					throw std::runtime_error("Unsupported root Transform override: " + p.key());
				Transform(object)[p.key()] = p.value();
			}
		}
		else if (it.key() == "Name" || it.key() == "Tag" || it.key() == "IsActive" || it.key() == "IsVisible") {
			object[it.key()] = it.value();
		}
		else
			throw std::runtime_error("Unsupported root override: " + it.key());
	}
}
struct Expanded {
	Json objects = Json::array();
	std::vector<Instance> instances;
};
// One resolver per document. All saved IDs are reserved before allocating any
// newly added prefab nodes, including IDs belonging to later array entries.
class Resolver {
	std::set<int> reserved;
	int next = 1;
	int Allocate() {
		while (reserved.contains(next)) {
			if (++next >= std::numeric_limits<int>::max())
				throw std::runtime_error("No free scene IDs");
		}
		reserved.insert(next);
		return next;
	}

public:
	explicit Resolver(const Json& entries) {
		if (!entries.is_array())
			throw std::runtime_error("Scene Objects must be an array");
		for (const auto& entry : entries) {
			const int root = Id(entry.at("Id"));
			if (!reserved.insert(root).second)
				throw std::runtime_error("Duplicate scene ID");
			if (entry.contains("Prefab")) {
				if (entry.contains("Components"))
					throw std::runtime_error("Prefab entry cannot contain Components");
				bool rootMapped = false;
				for (auto [local, runtime] : ReadMap(entry)) {
					if (runtime == root) {
						if (rootMapped)
							throw std::runtime_error("Duplicate root mapping");
						rootMapped = true;
					}
					else if (!reserved.insert(runtime).second)
						throw std::runtime_error("Colliding prefab ID map");
				}
			}
		}
		// A dangling reference must stay dangling: allocating its target ID to
		// a newly added asset node could otherwise silently retarget it.
		for (auto entry : entries) {
			const int parent = entry.value("ParentId", -1);
			if (parent > 0)
				reserved.insert(parent);
			if (!entry.contains("Prefab"))
				References(entry, [&](Json& value, const char*) {
					if (value != 0 && value != -1)
						reserved.insert(Id(value));
				});
		}
	}
	Expanded Expand(Json entry, const Json& asset = Json { }) {
		Expanded result;
		if (!entry.contains("Prefab")) {
			result.objects.push_back(std::move(entry));
			return result;
		}
		const auto path = Path(entry.at("Prefab").get<std::string>());
		try {
			ValidatePrefab(asset);
			const int localRoot = Id(asset.at("RootId"));
			const int root = Id(entry.at("Id"));
			auto ids = ReadMap(entry);
			if (ids.contains(localRoot) && ids.at(localRoot) != root)
				throw std::runtime_error("Root ID mapping mismatch");
			for (auto [local, runtime] : ids)
				if (runtime == root && local != localRoot)
					throw std::runtime_error("Wrong local root mapping");
			ids[localRoot] = root;
			IdMap live;
			for (const auto& object : asset.at("Objects")) {
				const auto local = Id(object.at("Id"));
				live[local] = ids.contains(local) ? ids.at(local) : Allocate();
			}
			for (auto object : asset.at("Objects")) {
				const bool isRoot = Id(object.at("Id")) == localRoot;
				Remap(object, live);
				if (isRoot) {
					object["ParentId"] = entry.value("ParentId", -1);
					Overrides(object, entry.value("RootOverrides", Json::object()));
				}
				result.objects.push_back(std::move(object));
			}
			entry["Prefab"] = path;
			entry["IdMap"] = WriteMap(live);
			result.instances.push_back({ std::move(entry) });
		} catch (const std::exception& e) {
			throw std::runtime_error(path + ": " + e.what());
		}
		return result;
	}
};
inline Expanded Expand(const Json& document, const std::function<Json(const std::string&)>& read) {
	Resolver resolver(document.at("Objects"));
	Expanded result;
	std::map<std::string, Json> cache;
	for (auto entry : document.at("Objects")) {
		Json asset;
		if (entry.contains("Prefab")) {
			const auto path = Path(entry.at("Prefab").get<std::string>());
			if (!cache.contains(path))
				cache[path] = read(path);
			asset = cache.at(path);
		}
		auto expanded = resolver.Expand(std::move(entry), asset);
		for (auto& object : expanded.objects)
			result.objects.push_back(std::move(object));
		for (auto& instance : expanded.instances)
			result.instances.push_back(std::move(instance));
	}
	Validate(result.objects);
	return result;
}
inline Json MakePrefab(Json objects, int root, IdMap& localToScene) {
	// Detach the exported root from its external parent before validation.
	for (auto& object : objects)
		if (Id(object.at("Id")) == root)
			object["ParentId"] = -1;
	Validate(objects, root);
	IdMap sceneToLocal;
	int local = 1;
	for (auto& object : objects) {
		const auto id = Id(object.at("Id"));
		sceneToLocal[id] = local;
		localToScene[local++] = id;
	}
	for (auto& object : objects)
		Remap(object, sceneToLocal);
	return Json { { "PrefabVersion", 1 }, { "RootId", sceneToLocal.at(root) }, { "Objects", objects } };
}
inline Json RootValues(Json object) {
	Json result;
	for (const char* key : { "Name", "Tag", "IsActive", "IsVisible" })
		if (object.contains(key))
			result[key] = object[key];
	auto transform = Transform(object);
	transform.erase("TransformComponentType");
	result["Transform"] = std::move(transform);
	return result;
}
inline Json StripRoot(Json object) {
	for (const char* key : { "Name", "Tag", "IsActive", "IsVisible", "ParentId" })
		object.erase(key);
	auto& transform = Transform(object);
	for (const char* key : { "LocalPosition", "LocalRotation", "LocalScale" })
		transform.erase(key);
	return object;
}
inline Json Compact(const Json& objects, const std::vector<Instance>& instances) {
	Validate(objects);
	std::map<int, Json> byId;
	for (const auto& object : objects)
		byId.emplace(Id(object.at("Id")), object);
	std::map<int, Json> entries;
	std::set<int> members;
	for (const auto& instance : instances) {
		auto entry = instance.entry;
		const int root = Id(entry.at("Id"));
		if (instance.baseline.empty())
			throw std::runtime_error("Missing prefab baseline");
		std::set<int> owned;
		for (auto [local, id] : ReadMap(entry)) {
			if (!members.insert(id).second)
				throw std::runtime_error("Overlapping prefab instances");
			owned.insert(id);
		}
		Json current = Json::array();
		for (auto object : objects)
			if (owned.contains(Id(object.at("Id")))) {
				if (Id(object.at("Id")) == root)
					object = StripRoot(object);
				current.push_back(std::move(object));
			}
		Json baseline = instance.baseline;
		for (auto& object : baseline)
			if (Id(object.at("Id")) == root)
				object = StripRoot(object);
		if (baseline != current)
			throw std::runtime_error(entry.at("Prefab").get<std::string>() + ": unsupported instance edit; unpack prefab before saving");
		for (const auto& object : objects)
			if (!owned.contains(Id(object.at("Id"))) && owned.contains(object.value("ParentId", -1)))
				throw std::runtime_error("Unpack prefab before adding children to an instance");
		entry["ParentId"] = byId.at(root).value("ParentId", -1);
		entry["RootOverrides"] = RootValues(byId.at(root));
		entries.emplace(root, std::move(entry));
	}
	Json result = Json::array();
	for (const auto& object : objects) {
		const int id = Id(object.at("Id"));
		if (entries.contains(id))
			result.push_back(entries.at(id));
		else if (!members.contains(id))
			result.push_back(object);
	}
	return Json { { "Objects", result } };
}
}
