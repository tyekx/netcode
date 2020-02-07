#include "pch.h"
#include "Manifest.h"


json11::Json::shape Manifest::jsonShape_root = {
	{ "name", json11::Json::Type::STRING },
	{ "base",  json11::Json::Type::OBJECT },
	{ "materials", json11::Json::Type::ARRAY },
	{ "colliders", json11::Json::Type::ARRAY },
	{ "animations", json11::Json::Type::ARRAY }
};

json11::Json::shape Manifest::jsonShape_Geometry = {
	{ "materialIndex", json11::Json::Type::NUMBER },
	{ "lods", json11::Json::Type::ARRAY }
};

json11::Json::shape Manifest::jsonShape_Material = {
	{ "source", json11::Json::Type::OBJECT },
	{ "modified_props", json11::Json::Type::OBJECT }
};

json11::Json::shape Manifest::jsonShape_Collider = {
	{ "type", json11::Json::Type::NUMBER },
	{ "shape", json11::Json::Type::NUMBER },
	{ "bone_reference", json11::Json::Type::NUMBER },
	{ "local_position", json11::Json::Type::ARRAY },
	{ "local_rotation", json11::Json::Type::ARRAY },
	{ "shape_data", json11::Json::Type::OBJECT }
};

json11::Json::shape Manifest::jsonShape_Animation = {
	{ "source", json11::Json::Type::OBJECT },
	{ "editor_playback_speed", json11::Json::Type::NUMBER },
	{ "editor_playback_loop",json11::Json::Type::BOOL }
};

json11::Json::shape Manifest::jsonShape_FbxReference = {
	{ "file", json11::Json::Type::STRING },
	{ "reference", json11::Json::Type::STRING }
};
