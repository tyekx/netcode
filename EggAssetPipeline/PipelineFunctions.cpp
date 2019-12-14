#include "PipelineFunctions.h"
#include <cstdio>

void PrintHelp() {
	printf("Usage(1): $ EggAssetPipeline --name=<string> --lods=<file>[,<file>,...] --animations=[<file>,...] [--generate_tangent_space]\r\n");
	printf("Usage(2): $ EggAssetPipeline --name=<string> --map=<file>\r\n");
	printf("Usage(3): $ EggAssetPipeline --manifest=<file>\r\n");
	printf("Usage(4): $ EggAssetPipeline\r\n\r\n");

	printf("\t(3): will load the expected arguments from a file, the arguments follow the (1) pattern in the manifest file\r\n");
	printf("\t(4): will try to load the default manifest.txt file in the working directory\r\n\r\n");

	printf("Parameters:\r\n");
	printf("\t	  --name				   : Name of the output file\r\n");
	printf("\t    --lods				   : Different level of details for the same model, in a descending LOD order\r\n");
	printf("\t    --animations			   : List of animations to be used for the mesh\r\n");
	printf("\t    --generate_tangent_space : Tangents and binormals will be calculated for each mesh\r\n");
	printf("\t    --map                    : Process a map file\r\n");
	printf("\t	  --manifest               : Loads the program arguments from a manifest file\r\n");
}
