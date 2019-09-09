#include "JsonSerializer.h"

 json11::Json Egg::Serializer::JsonSerializer::Serialize(const Egg::Math::Float4 & vec) {
	return json11::Json::object{
		{ "x", vec.x },
		{ "y", vec.y },
		{ "z", vec.z },
		{ "w", vec.w }
	};
}

 json11::Json Egg::Serializer::JsonSerializer::Serialize(const Egg::Math::Float3 & vec) {
	return json11::Json::object{
		{ "x", vec.x },
		{ "y", vec.y },
		{ "z", vec.z }
	};
}

 json11::Json Egg::Serializer::JsonSerializer::Serialize(const TransformComponent & tcomp) {
	return json11::Json::object{
		{ "position", Serialize(tcomp.Position) },
		{ "rotation", Serialize(tcomp.Rotation) },
		{ "scale", Serialize(tcomp.Scale) }
	};
}

 bool Egg::Serializer::JsonSerializer::Deserialize(const json11::Json & json, MeshComponent & mcomp) {
	 return false;
}

 bool Egg::Serializer::JsonSerializer::Deserialize(const json11::Json & json, ScriptComponent & scomp) {
	 return false;
}

 bool Egg::Serializer::JsonSerializer::Deserialize(const json11::Json & json, TransformComponent & tcomp) {
	if(json.is_null() || !json.is_object()) {
		return false;
	}

	static json11::Json::shape tcompShape = {
		{ "position", json11::Json::OBJECT },
		{ "rotation", json11::Json::OBJECT },
		{ "scale", json11::Json::OBJECT },
	};

	std::string err;
	if(!json.has_shape(tcompShape, err)) {
		return false;
	}

	Egg::Math::Float4 pos;
	Egg::Math::Float4 rot;
	Egg::Math::Float3 scale;

	if(Deserialize(json["position"], pos) &&
	   Deserialize(json["rotation"], rot) &&
	   Deserialize(json["scale"], scale)) {
		tcomp.Position = pos;
		tcomp.Rotation = rot;
		tcomp.Scale = scale;
		return true;
	}
	return false;
}

 bool Egg::Serializer::JsonSerializer::Deserialize(const json11::Json & json, Egg::Math::Float3 & vec) {
	if(json.is_null() || !json.is_object()) {
		return false;
	}

	static json11::Json::shape float3Shape = {
		{ "x", json11::Json::NUMBER },
		{ "y", json11::Json::NUMBER },
		{ "z", json11::Json::NUMBER },
	};

	std::string err;
	if(!json.has_shape(float3Shape, err)) {
		return false;
	}

	vec.x = (float)json["x"].number_value();
	vec.y = (float)json["y"].number_value();
	vec.z = (float)json["z"].number_value();

	return true;
}

 bool Egg::Serializer::JsonSerializer::Deserialize(const json11::Json & json, Egg::Math::Float4 & vec) {
	if(json.is_null() || !json.is_object()) {
		return false;
	}

	static json11::Json::shape float4Shape = {
		{ "x", json11::Json::NUMBER },
		{ "y", json11::Json::NUMBER },
		{ "z", json11::Json::NUMBER },
		{ "w", json11::Json::NUMBER },
	};

	std::string err;
	if(!json.has_shape(float4Shape, err)) {
		return false;
	}

	vec.x = (float)json["x"].number_value();
	vec.y = (float)json["y"].number_value();
	vec.z = (float)json["z"].number_value();
	vec.w = (float)json["w"].number_value();

	return true;
}

 json11::Json Egg::Serializer::JsonSerializer::Serialize(const MeshComponent & mc) {
	 return json11::Json::object{};
}

 json11::Json Egg::Serializer::JsonSerializer::Serialize(const ScriptComponent & mc) {
	 return json11::Json::object{};
}

 json11::Json Egg::Serializer::JsonSerializer::Serialize(Egg::GameObject * go) {
	json11::Json::object obj;

	obj["name"] = go->GetName();

	if(go->GetOwner() != nullptr) {
		obj["owner"] = (int)go->GetOwner()->GetId();
	}

	if(go->HasComponent<TransformComponent>()) {
		obj["transformComponent"] = Serialize(*(go->GetComponent<TransformComponent>()));
	}

	if(go->HasComponent<MeshComponent>()) {
		obj["meshComponent"] = Serialize(*(go->GetComponent<MeshComponent>()));
	}

	if(go->HasComponent<ScriptComponent>()) {
		obj["scriptComponent"] = Serialize(*(go->GetComponent<ScriptComponent>()));
	}

	return obj;
}

json11::Json Egg::Serializer::JsonSerializer::Serialize(const Egg::Scene & scene) {
	json11::Json::array gameObjects;

	for(unsigned int i = 0; i < scene.GetObjectCount(); ++i) {
		Egg::GameObject * gameObj = scene[i];
		gameObjects.push_back(Serialize(gameObj));
	}

	return json11::Json::object{
		{ "name", scene.GetName() },
		{ "gameObjects", gameObjects }
	};
}

bool Egg::Serializer::JsonSerializer::Deserialize(const json11::Json & json, Egg::Scene & scene) {
	return false;
}


bool Egg::Serializer::JsonSerializer::Deserialize(const json11::Json & json, Egg::GameObject * gobj) {
	return false;
}
