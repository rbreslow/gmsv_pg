file(GLOB SOURCES
	"*.cpp"
	"*.h"
    "*.hpp"
)

target_sources(pg PRIVATE ${SOURCES})
