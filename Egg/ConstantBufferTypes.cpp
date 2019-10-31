#include "ConstantBufferTypes.h"
#include "Common.h"


static struct {
	int id;
	const char * name;
} cbufferNames[] = { 

	{ PerMeshCb::id, "PerMeshCb" },
	{ PerFrameCb::id, "PerFrameCb"},
	{ BoneDataCb::id, "BoneDataCb"},
	{ PerObjectCb::id, "PerObjectCb" }

};

int GetConstantBufferIdFromName(const char * name) {
	for(int i = 0; i < _countof(cbufferNames); ++i) {
		if(strcmp(name, cbufferNames[i].name) == 0) {
			return cbufferNames[i].id;
		}
	}
	return -1;
}
