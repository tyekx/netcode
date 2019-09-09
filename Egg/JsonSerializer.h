#pragma once

#include <json11/json11.hpp>
#include "Scene.h"
#include "Math/Math.h"

namespace Egg {

	namespace Serializer {

		class JsonSerializer {
		public:
			static json11::Json Serialize(const Egg::Math::Float4 & vec);

			static json11::Json Serialize(const Egg::Math::Float3 & vec);

			static json11::Json Serialize(const TransformComponent & tcomp);

			static json11::Json Serialize(const MeshComponent & mc);

			static json11::Json Serialize(const ScriptComponent & mc);

			static json11::Json Serialize(Egg::GameObject * go);

			static json11::Json Serialize(const Egg::Scene & scene);

			static bool Deserialize(const json11::Json & json, Egg::Math::Float3 & vec);

			static bool Deserialize(const json11::Json & json, Egg::Math::Float4 & vec);

			static bool Deserialize(const json11::Json & json, TransformComponent & tcomp);

			static bool Deserialize(const json11::Json & json, MeshComponent & mcomp);

			static bool Deserialize(const json11::Json & json, ScriptComponent & scomp);

			static bool Deserialize(const json11::Json & json, Egg::GameObject * gobj);

			static bool Deserialize(const json11::Json & json, Egg::Scene & scene);
		};

	}

}
